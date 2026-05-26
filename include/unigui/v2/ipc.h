#pragma once
#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace unigui::v2 {

/// IPC abstraction over ZMQ + Shared Memory.
class IPCChannel {
public:
    virtual ~IPCChannel() = default;
    virtual bool Send(const std::string& msg) = 0;
    virtual void OnReceive(std::function<void(const std::string&)> cb) = 0;
    virtual void Close() = 0;
};

/// TCP-based ZMQ publisher (server).
class IPCServer : public IPCChannel {
public:
    IPCServer(const std::string& address = "tcp://*:5555");
    ~IPCServer();
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
class IPCClient : public IPCChannel {
public:
    IPCClient(const std::string& address = "tcp://localhost:5555");
    ~IPCClient();
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

/// Shared memory channel (process-local, ultra-fast).
class SharedMemory {
public:
    SharedMemory(const std::string& name, size_t size);
    ~SharedMemory();
    void Write(const void* data, size_t size, size_t offset = 0);
    void Read(void* data, size_t size, size_t offset = 0);
    size_t Size() const { return size_; }
    void* Data() { return data_; }
private:
#ifdef _WIN32
    void* handle_ = nullptr;
#else
    int fd_ = -1;
#endif
    void* data_ = nullptr;
    size_t size_ = 0;
    std::string name_;
};

} // namespace unigui::v2
