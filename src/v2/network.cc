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
        if (msg->type == ix::WebSocketMessageType::Message && onMsg_)
            onMsg_(msg->str);
        else if (msg->type == ix::WebSocketMessageType::Open && onOpen_)
            onOpen_();
        else if (msg->type == ix::WebSocketMessageType::Close && onClose_)
            onClose_();
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
    onMsg_ = std::move(cb);
}
void WebSocketClient::OnOpen(std::function<void()> cb) {
    onOpen_ = std::move(cb);
}
void WebSocketClient::OnClose(std::function<void()> cb) {
    onClose_ = std::move(cb);
}
bool WebSocketClient::IsConnected() const {
    return ws_.getReadyState() == ix::ReadyState::Open;
}
void WebSocketClient::Disconnect() {
    ws_.stop();
}

} // namespace unigui::network
