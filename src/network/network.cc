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

// ── HTTP ────────────────────────────────────────────────────────────────────

static httplib::Client* makeClient(const std::string& url) {
    // Parse scheme+host from URL
    if (url.find("https://") == 0) {
        auto host = url.substr(8);
        auto slash = host.find('/');
        std::string path = (slash != std::string::npos) ? host.substr(slash) : "/";
        host = (slash != std::string::npos) ? host.substr(0, slash) : host;
        auto* cli = new httplib::Client(host.c_str());
        cli->set_follow_location(true);
        return cli;
    }
    auto host = url.find("http://") == 0 ? url.substr(7) : url;
    auto slash = host.find('/');
    host = (slash != std::string::npos) ? host.substr(0, slash) : host;
    auto* cli = new httplib::Client(host.c_str());
    cli->set_follow_location(true);
    return cli;
}

HttpResponse HttpClient::Get(const std::string& url,
                             const std::map<std::string, std::string>& headers) {
    HttpResponse resp;
    auto* cli = makeClient(url);
    httplib::Headers hdrs;
    for (auto& [k, v] : headers)
        hdrs.emplace(k, v);
    auto slash = url.find('/', url.find("://") != std::string::npos ? url.find("://") + 3 : 0);
    std::string path = (slash != std::string::npos) ? url.substr(slash) : "/";
    auto res = cli->Get(path, hdrs);
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
    auto* cli = makeClient(url);
    httplib::Headers hdrs;
    hdrs.emplace("Content-Type", contentType);
    for (auto& [k, v] : headers)
        hdrs.emplace(k, v);
    auto slash = url.find('/', url.find("://") != std::string::npos ? url.find("://") + 3 : 0);
    std::string path = (slash != std::string::npos) ? url.substr(slash) : "/";
    auto res = cli->Post(path, hdrs, body, contentType.c_str());
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
