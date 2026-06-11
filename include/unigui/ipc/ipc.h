#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace unigui::ipc {

/// IPC abstraction over ZMQ + Shared Memory.
class Channel {
public:
    virtual ~Channel() = default;
    virtual bool Send(const std::string& msg) = 0;
    virtual void OnReceive(std::function<void(const std::string&)> cb) = 0;
    virtual void Close() = 0;
};

/// TCP-based ZMQ publisher (server).
class Server : public Channel {
public:
    Server(const std::string& address = "tcp://*:5555");
    ~Server();
    bool Send(const std::string& msg) override;
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
