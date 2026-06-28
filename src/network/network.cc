// On Windows, WIN32_LEAN_AND_MEAN must be defined before ANYTHING pulls in <windows.h>
// (spdlog, via core/log.h, does). Otherwise <windows.h> includes the legacy <winsock.h>,
// defines _WINSOCKAPI_, and the later <winsock2.h>/<ws2tcpip.h> from httplib/ixwebsocket
// are skipped — producing a flood of undeclared-identifier errors in ws2tcpip.h.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include <unigui/core/log.h>
#include <unigui/network/network.h>

#include <httplib.h>

namespace unigui::network {

// ── URL parsing (pure, unit-tested in tests/network/url_parse_test.cc) ────────

ParsedUrl SplitUrl(const std::string& url) {
    ParsedUrl r;
    std::string rest = url;
    if (rest.rfind("https://", 0) == 0) {
        r.https = true;
        rest = rest.substr(8);
    } else if (rest.rfind("http://", 0) == 0) {
        rest = rest.substr(7);
    }
    const auto slash = rest.find('/');
    if (slash != std::string::npos) {
        r.host = rest.substr(0, slash);
        r.path = rest.substr(slash);
    } else {
        r.host = rest;
        r.path = "/";
    }
    return r;
}

// ── HTTP ────────────────────────────────────────────────────────────────────

static httplib::Client* makeClient(const ParsedUrl& u) {
    auto* cli = new httplib::Client(u.host.c_str());
    cli->set_follow_location(true);
    return cli;
}

HttpResponse HttpClient::Get(const std::string& url,
                             const std::map<std::string, std::string>& headers) {
    HttpResponse resp;
    const ParsedUrl u = SplitUrl(url);
    auto* cli = makeClient(u);
    httplib::Headers hdrs;
    for (auto& [k, v] : headers)
        hdrs.emplace(k, v);
    auto res = cli->Get(u.path, hdrs);
    if (res) {
        resp.status = res->status;
        resp.body = res->body;
    }
    delete cli;
    return resp;
}

HttpResponse HttpClient::Post(const std::string& url, const std::string& body,
                              const std::string& contentType,
                              const std::map<std::string, std::string>& headers) {
    HttpResponse resp;
    const ParsedUrl u = SplitUrl(url);
    auto* cli = makeClient(u);
    httplib::Headers hdrs;
    hdrs.emplace("Content-Type", contentType);
    for (auto& [k, v] : headers)
        hdrs.emplace(k, v);
    auto res = cli->Post(u.path, hdrs, body, contentType.c_str());
    if (res) {
        resp.status = res->status;
        resp.body = res->body;
    }
    delete cli;
    return resp;
}

// ── WebSocket ───────────────────────────────────────────────────────────────

WebSocketClient::WebSocketClient() {
    ws_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        // Snapshot only the relevant callback under the lock, then invoke it OUTSIDE
        // the critical section — user code may re-enter the client (e.g. Send) and we
        // must not hold cbMutex_ across it. Copying keeps the target alive for the call
        // even if a setter swaps the member concurrently.
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::function<void(const std::string&)> cb;
            {
                std::lock_guard<std::mutex> g(cbMutex_);
                cb = onMsg_;
            }
            if (cb)
                cb(msg->str);
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            std::function<void()> cb;
            {
                std::lock_guard<std::mutex> g(cbMutex_);
                cb = onOpen_;
            }
            if (cb)
                cb();
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            std::function<void()> cb;
            {
                std::lock_guard<std::mutex> g(cbMutex_);
                cb = onClose_;
            }
            if (cb)
                cb();
        }
    });
}

WebSocketClient::~WebSocketClient() {
    Disconnect();
}

bool WebSocketClient::Connect(const std::string& url) {
    ws_.setUrl(url);
    ws_.start();
    return true;
}

void WebSocketClient::Send(const std::string& msg) {
    ws_.send(msg);
}
void WebSocketClient::OnMessage(std::function<void(const std::string&)> cb) {
    std::lock_guard<std::mutex> g(cbMutex_);
    onMsg_ = std::move(cb);
}
void WebSocketClient::OnOpen(std::function<void()> cb) {
    std::lock_guard<std::mutex> g(cbMutex_);
    onOpen_ = std::move(cb);
}
void WebSocketClient::OnClose(std::function<void()> cb) {
    std::lock_guard<std::mutex> g(cbMutex_);
    onClose_ = std::move(cb);
}
bool WebSocketClient::IsConnected() const {
    return ws_.getReadyState() == ix::ReadyState::Open;
}
void WebSocketClient::Disconnect() {
    ws_.stop();
}

} // namespace unigui::network
