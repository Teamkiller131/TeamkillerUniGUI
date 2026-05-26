#pragma once
#include <string>
#include <functional>
#include <map>
#include <thread>
#include <mutex>
#include <httplib.h>
#include <ixwebsocket/IXWebSocket.h>

namespace unigui::v2 {

struct HttpResponse {
    int status = 0;
    std::string body;
    std::map<std::string, std::string> headers;
};

/// HTTP client (synchronous).
class HttpClient {
public:
    /// GET request. Returns response.
    static HttpResponse Get(const std::string& url,
                            const std::map<std::string, std::string>& headers = {});

    /// POST request. Returns response.
    static HttpResponse Post(const std::string& url, const std::string& body = "",
                             const std::string& contentType = "application/json",
                             const std::map<std::string, std::string>& headers = {});
};

/// WebSocket client.
class WebSocketClient {
public:
    WebSocketClient();
    ~WebSocketClient();

    bool Connect(const std::string& url);
    void Send(const std::string& msg);
    void OnMessage(std::function<void(const std::string&)> cb);
    void OnOpen(std::function<void()> cb);
    void OnClose(std::function<void()> cb);
    bool IsConnected() const;
    void Disconnect();

private:
    ix::WebSocket ws_;
    std::function<void(const std::string&)> onMsg_;
    std::function<void()> onOpen_;
    std::function<void()> onClose_;
};

} // namespace unigui::v2
