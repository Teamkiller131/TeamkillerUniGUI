#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace unigui::ipc {

/// IPC abstraction over ZMQ + Shared Memory.
///
/// Topology note: this is a ZMQ PUB/SUB pair — messages flow one way, from a
/// Server (PUB, broadcast) to any connected Clients (SUB). Only `Client` actually
/// receives, so `OnReceive`/`Poll` are Client-side; `Server::OnReceive` is a no-op
/// (a PUB socket cannot receive) and warns if given a callback.
class Channel {
public:
    virtual ~Channel() = default;
    virtual bool Send(const std::string& msg) = 0;
    virtual void OnReceive(std::function<void(const std::string&)> cb) = 0;
    virtual void Close() = 0;
};

/// Terminate the process-wide ZMQ context (created lazily on first channel use).
/// Optional — the OS reclaims it at process exit — but call it for a clean shutdown
/// or before unloading the module as a DLL, after all Server/Client channels are
/// closed. Idempotent; channels created afterward transparently get a fresh context.
///
/// Constraint: an existing Server/Client caches the context it was constructed with,
/// so a channel that was merely Close()d (not destroyed) must NOT be reopened across a
/// Shutdown() — its cached context is now terminated. Only channels *constructed* after
/// Shutdown() get the fresh context. Destroy old channels before Shutdown(), and create
/// new ones after it.
void Shutdown();

/// TCP-based ZMQ publisher (server). PUB socket: send-only (see the Channel note).
class Server : public Channel {
public:
    Server(const std::string& address = "tcp://*:5555");
    ~Server();
    bool Send(const std::string& msg) override;
    /// No-op: a PUB socket cannot receive. Warns if given a callback.
    void OnReceive(std::function<void(const std::string&)> cb) override;
    void Close() override;
    bool Start();

private:
    void* ctx_ = nullptr;
    void* socket_ = nullptr;
    std::string address_;
    std::function<void(const std::string&)> onRecv_;
    bool running_ = false;
};

/// TCP-based ZMQ subscriber (client).
class Client : public Channel {
public:
    Client(const std::string& address = "tcp://localhost:5555");
    ~Client();
    bool Connect();
    bool Send(const std::string& msg) override;
    void OnReceive(std::function<void(const std::string&)> cb) override;
    void Close() override;
    bool Poll(int timeoutMs = 0);

private:
    void* ctx_ = nullptr;
    void* socket_ = nullptr;
    std::string address_;
    std::function<void(const std::string&)> onRecv_;
};

} // namespace unigui::ipc
