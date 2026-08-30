#include <unigui/network/network.h>

#include <chrono>
#include <condition_variable>
#include <gtest/gtest.h>
#include <ixwebsocket/IXNetSystem.h> // socket primitives (winsock2 / sys-socket), initNetSystem
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <memory>
#include <mutex>
#include <string>

using namespace std::chrono_literals;

// Live loopback WebSocket echo: an in-process ix::WebSocketServer echoes every message,
// and unigui's WebSocketClient connects, sends, and receives it back. This is the first
// end-to-end proof the WS client actually completes a handshake and round-trips a frame —
// the prior WS test only exercised the thread-safety of the callback setters, with no live
// connection. It also validates the WSAStartup init that WebSocketClient::Connect now does.
namespace {

// Ask the OS for a free loopback port: bind a throwaway socket to 127.0.0.1:0, read the
// port it was assigned, then close it. IXWebSocket v11.4.6 does NOT read an ephemeral
// (port 0) bind back through getsockname(), so its getPort() would return 0 — we must
// hand it a concrete port. A bind-then-close socket that never connected leaves no
// TIME_WAIT, so the port is immediately reusable; the caller retries to cover the tiny
// window in which another process could still grab it.
int pickFreePort() {
#ifdef _WIN32
    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET)
        return 0;
#else
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return 0;
#endif
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0; // 0 → OS picks a free ephemeral port
    int port = 0;
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0) {
        socklen_t len = sizeof(addr);
        if (getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) == 0)
            port = ntohs(addr.sin_port);
    }
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
    return port;
}

} // namespace

TEST(WebSocketLoopbackTest, ConnectSendEcho_RoundTrips) {
    // WSAStartup on Windows; a no-op elsewhere. Required before the server opens its
    // listen socket (and before pickFreePort's socket() call on Windows). Idempotent and
    // process-wide, so it also covers the client. Not torn down — the process reclaims it
    // at exit, and later tests in this binary reuse the still-inited stack.
    ASSERT_TRUE(ix::initNetSystem());

    // Stand the echo server up on an OS-chosen free port, retrying if we lose the race.
    std::unique_ptr<ix::WebSocketServer> server;
    int port = 0;
    std::string lastErr;
    for (int attempt = 0; attempt < 20 && !server; ++attempt) {
        const int candidate = pickFreePort();
        if (candidate <= 0)
            continue;
        auto s = std::make_unique<ix::WebSocketServer>(candidate, "127.0.0.1");
        s->setOnClientMessageCallback([](std::shared_ptr<ix::ConnectionState>, ix::WebSocket& ws,
                                         const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Message)
                ws.send(msg->str); // echo the payload straight back
        });
        auto [ok, err] = s->listen();
        if (ok) {
            server = std::move(s);
            port = candidate;
        } else {
            lastErr = err; // lost the race for this port — try another
        }
    }
    ASSERT_TRUE(server) << "could not bind an echo server after 20 tries; last error: " << lastErr;
    ASSERT_GT(port, 0);
    server->start();

    std::mutex m;
    std::condition_variable cv;
    bool opened = false;
    std::string received;

    unigui::network::WebSocketClient client;
    client.OnOpen([&] {
        std::lock_guard<std::mutex> g(m);
        opened = true;
        cv.notify_all();
    });
    client.OnMessage([&](const std::string& s) {
        std::lock_guard<std::mutex> g(m);
        received = s;
        cv.notify_all();
    });
    ASSERT_TRUE(client.Connect("ws://127.0.0.1:" + std::to_string(port)));

    { // block on the handshake completing (bounded — never hangs CI)
        std::unique_lock<std::mutex> lk(m);
        ASSERT_TRUE(cv.wait_for(lk, 5s, [&] { return opened; })) << "WebSocket never opened";
    }
    EXPECT_TRUE(client.IsConnected());

    client.Send("echo-me");
    { // block on the echoed frame arriving
        std::unique_lock<std::mutex> lk(m);
        ASSERT_TRUE(cv.wait_for(lk, 5s, [&] { return received == "echo-me"; }))
            << "no echo received; got: '" << received << "'";
    }
    EXPECT_EQ(received, "echo-me");

    client.Disconnect();
    server->stop();
}
