#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <winsock2.h>
#endif
#include <unigui/core/log.h>
#include <unigui/ipc/ipc.h>

#include <cstring>
#include <zmq.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace unigui::ipc {

// ── ZMQ Helpers ─────────────────────────────────────────────────────────────

static void* zmqCtx() {
    static void* ctx = zmq_ctx_new();
    return ctx;
}

// ── Server ───────────────────────────────────────────────────────────────

Server::Server(const std::string& address)
        : address_(address) {
    ctx_ = zmqCtx();
}

Server::~Server() {
    Close();
}

bool Server::Start() {
    if (socket_)
        return true;
    socket_ = zmq_socket(ctx_, ZMQ_PUB);
    if (!socket_) {
        UNIGUI_LOG_ERROR("IPC server: zmq_socket failed");
        return false;
    }
    int linger = 0;
    zmq_setsockopt(socket_, ZMQ_LINGER, &linger, sizeof(linger));
    if (zmq_bind(socket_, address_.c_str()) != 0) {
        UNIGUI_LOG_ERROR("IPC server: bind failed: {}", address_);
        zmq_close(socket_);
        socket_ = nullptr;
        return false;
    }
    running_ = true;
    UNIGUI_LOG_INFO("IPC server listening: {}", address_);
    return true;
}

bool Server::Send(const std::string& msg) {
    if (!socket_ || !running_)
        return false;
    return zmq_send(socket_, msg.c_str(), msg.size(), 0) == (int) msg.size();
}

void Server::OnReceive(std::function<void(const std::string&)> cb) {
    onRecv_ = std::move(cb);
}

void Server::Close() {
    running_ = false;
    if (socket_) {
        zmq_close(socket_);
        socket_ = nullptr;
    }
}

// ── Client ───────────────────────────────────────────────────────────────

Client::Client(const std::string& address)
        : address_(address) {
    ctx_ = zmqCtx();
}

Client::~Client() {
    Close();
}

bool Client::Connect() {
    if (socket_)
        return true;
    socket_ = zmq_socket(ctx_, ZMQ_SUB);
    if (!socket_)
        return false;
    int linger = 0;
    zmq_setsockopt(socket_, ZMQ_LINGER, &linger, sizeof(linger));
    zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, "", 0);
    if (zmq_connect(socket_, address_.c_str()) != 0) {
        UNIGUI_LOG_ERROR("IPC client: connect failed: {}", address_);
        zmq_close(socket_);
        socket_ = nullptr;
        return false;
    }
    UNIGUI_LOG_INFO("IPC client connected: {}", address_);
    return true;
}

bool Client::Send(const std::string& msg) {
    if (!socket_)
        return false;
    return zmq_send(socket_, msg.c_str(), msg.size(), 0) == (int) msg.size();
}

void Client::OnReceive(std::function<void(const std::string&)> cb) {
    onRecv_ = std::move(cb);
}

bool Client::Poll(int timeoutMs) {
    if (!socket_ || !onRecv_)
        return false;
    zmq_pollitem_t items[] = {{socket_, 0, ZMQ_POLLIN, 0}};
    if (zmq_poll(items, 1, timeoutMs) > 0 && (items[0].revents & ZMQ_POLLIN)) {
        char buf[65536];
        int n = zmq_recv(socket_, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
            buf[n] = 0;
            onRecv_(std::string(buf, n));
            return true;
        }
    }
    return false;
}

void Client::Close() {
    if (socket_) {
        zmq_close(socket_);
        socket_ = nullptr;
    }
}

// NOTE: SharedMemory lives in shmem.{h,cc}. It was previously duplicated here, but
// SharedMemory is not declared in ipc.h (only Channel/Server/Client are), so this
// translation unit could never have compiled that copy — and building both ipc.cc
// and shmem.cc would have been a duplicate-symbol link error. The canonical and only
// definition is in src/ipc/shmem.cc.

} // namespace unigui::ipc
