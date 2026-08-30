#include <unigui/network/network.h>

#include <chrono>
#include <gtest/gtest.h>
#include <httplib.h>
#include <string>
#include <thread>

using namespace std::chrono_literals;

// Live loopback HTTP round-trip: an in-process httplib server on an ephemeral
// 127.0.0.1 port answers GET/POST, and unigui's HttpClient (the httplib-backed
// wrapper) drives real requests against it. This is the first end-to-end proof that
// HttpClient::Get/Post actually speak HTTP — the module previously had only
// URL-splitting unit tests and no live-transport coverage.
namespace {

// Spins up the server on its own thread and blocks until it is actually accepting.
// bind_to_any_port() assigns a free ephemeral port synchronously, so parallel CI jobs
// never collide on a hard-coded port; the accept loop then runs on the thread.
struct LoopbackHttpServer {
    httplib::Server svr;
    int port = -1;
    std::thread th;

    LoopbackHttpServer() {
        svr.Get("/ping", [](const httplib::Request&, httplib::Response& res) {
            res.set_content("pong", "text/plain");
        });
        svr.Post("/echo", [](const httplib::Request& req, httplib::Response& res) {
            // Echo the request body back verbatim, preserving its content type.
            const auto it = req.headers.find("Content-Type");
            res.set_content(req.body,
                            it != req.headers.end() ? it->second : "application/octet-stream");
        });
        port = svr.bind_to_any_port("127.0.0.1");
        th = std::thread([this] { svr.listen_after_bind(); });
        // The socket is already in the listen state after bind_to_any_port (the backlog
        // queues connections before accept()), but poll is_running() so the first client
        // request never races the thread starting its accept loop.
        for (int i = 0; i < 500 && !svr.is_running(); ++i)
            std::this_thread::sleep_for(2ms);
    }
    ~LoopbackHttpServer() {
        svr.stop();
        if (th.joinable())
            th.join();
    }
    std::string url(const std::string& path) const {
        return "http://127.0.0.1:" + std::to_string(port) + path;
    }
};

} // namespace

TEST(HttpLoopbackTest, Get_ReturnsBody) {
    LoopbackHttpServer server;
    ASSERT_GT(server.port, 0);
    ASSERT_TRUE(server.svr.is_running());

    const auto r = unigui::network::HttpClient::Get(server.url("/ping"));
    EXPECT_EQ(r.status, 200);
    EXPECT_EQ(r.body, "pong");
}

TEST(HttpLoopbackTest, Post_EchoesBody) {
    LoopbackHttpServer server;
    ASSERT_GT(server.port, 0);
    ASSERT_TRUE(server.svr.is_running());

    const std::string payload = R"({"msg":"hi","n":42})";
    const auto r =
        unigui::network::HttpClient::Post(server.url("/echo"), payload, "application/json");
    EXPECT_EQ(r.status, 200);
    EXPECT_EQ(r.body, payload);
}

// An unknown route must surface the real 404 (not the status-0 "transport failed"
// sentinel), proving the client distinguishes a reached-but-refused request from a
// connection failure.
TEST(HttpLoopbackTest, Get_UnknownRoute_Returns404) {
    LoopbackHttpServer server;
    ASSERT_GT(server.port, 0);

    const auto r = unigui::network::HttpClient::Get(server.url("/no-such-route"));
    EXPECT_EQ(r.status, 404);
}
