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
#include <unigui/core/strutil.h>
#include <unigui/network/network.h>

#include <httplib.h>
#include <string>

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
    // Split an optional :port off the authority, validating it WITHOUT throwing — httplib's
    // single-arg "scheme://host:port" Client ctor std::stoi's the port and crashes the caller
    // on a malformed/overflow value. For a bracketed IPv6 literal ("[::1]") a port can only
    // follow the closing ']'; otherwise it follows the last ':'. Only an all-digit suffix
    // (≤5 digits, in 1..65535) is taken; anything else leaves the host intact and port 0.
    std::string::size_type portColon = std::string::npos;
    if (!r.host.empty() && r.host.front() == '[') {
        const auto rb = r.host.find(']');
        if (rb != std::string::npos && rb + 1 < r.host.size() && r.host[rb + 1] == ':')
            portColon = rb + 1; // the ':' immediately after ']' (host keeps its brackets)
    } else {
        portColon = r.host.rfind(':');
    }
    if (portColon != std::string::npos) {
        const std::string portStr = r.host.substr(portColon + 1);
        bool digits = !portStr.empty() && portStr.size() <= 5;
        for (char c : portStr)
            if (c < '0' || c > '9') {
                digits = false;
                break;
            }
        if (digits) {
            const int p = ToIntOr(portStr, 0); // non-throwing; ≤5 digits can't overflow int
            if (p > 0 && p <= 65535) {
                r.port = p;
                r.host = r.host.substr(0, portColon);
            }
            // else: out-of-range — leave host intact, port stays 0 → scheme default. No throw.
        }
    }
    return r;
}

// ── HTTP ────────────────────────────────────────────────────────────────────

// Build an httplib client for the parsed URL. Returns nullptr (a logged failure the callers
// turn into HttpResponse.status==0) when HTTPS is requested but this build has no TLS.
static httplib::Client* makeClient(const ParsedUrl& u) {
    const int port = u.port != 0 ? u.port : (u.https ? 443 : 80);
    if (u.https) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        // TLS available: a scheme-qualified address makes httplib build an SSL client. The
        // port here is our own validated integer, so httplib's internal parse sees no garbage.
        auto* cli = new httplib::Client("https://" + u.host + ":" + std::to_string(port));
        cli->set_follow_location(true);
        return cli;
#else
        // No TLS in this build: refuse rather than silently downgrade an https:// request to
        // cleartext (which the dead `https` flag previously did).
        UNIGUI_LOG_ERROR("network: HTTPS requested for '{}' but this build has no TLS support "
                         "(OpenSSL off); refusing to send in cleartext",
                         u.host);
        return nullptr;
#endif
    }
    // Plain HTTP via the explicit host+int-port ctor: httplib does NO string parsing here, so a
    // malformed port can never reach its throwing std::stoi.
    auto* cli = new httplib::Client(u.host, port);
    cli->set_follow_location(true);
    return cli;
}

HttpResponse HttpClient::Get(const std::string& url,
                             const std::map<std::string, std::string>& headers) {
    HttpResponse resp;
    const ParsedUrl u = SplitUrl(url);
    auto* cli = makeClient(u);
    if (!cli)
        return resp; // resp.status stays 0 == failure (e.g. https with no TLS support)
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
    if (!cli)
        return resp; // resp.status stays 0 == failure (e.g. https with no TLS support)
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
