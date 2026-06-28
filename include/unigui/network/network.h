#pragma once
// Ensure <winsock2.h> (pulled by httplib/ixwebsocket) wins over the legacy <winsock.h>
// that a prior <windows.h> include would otherwise bring in — see src/network/network.cc.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
// httplib is an implementation detail of HttpClient (used only in network.cc), so it
// is NOT included here. ixwebsocket must stay: WebSocketClient embeds ix::WebSocket by
// value.
#include <functional>
#include <ixwebsocket/IXWebSocket.h>
#include <map>
#include <mutex>
#include <string>
#include <thread>

namespace unigui::network {

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
    // Guards the three callbacks below: the IXWebSocket background thread reads them
    // from the message handler while OnMessage/OnOpen/OnClose may rewrite them from
    // another thread. Without this, re-registering a callback races the read and can
    // free the previously-installed target while it is executing.
    mutable std::mutex cbMutex_;
    std::function<void(const std::string&)> onMsg_;
    std::function<void()> onOpen_;
    std::function<void()> onClose_;
};

} // namespace unigui::network
