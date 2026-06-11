#pragma once
#include <any>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace unigui::events {

/// RAII subscription handle. Unsubscribes on destruction; move-only.
class Subscription;

/// EventBus: publish-subscribe messaging across all components.
/// Thread-safe. Supports synchronous + async publishing.
class Bus {
public:
    using Handler = std::function<void(const std::any& event)>;
    using SubID = uint64_t;

    ~Bus() { Shutdown(); }
    static Bus& Instance();

    /// Subscribe to a topic. Supports wildcards: "window.*" matches "window.close".
    /// Returns subscription ID for Unsubscribe().
    SubID Subscribe(const std::string& topic, Handler handler);

    /// Subscribe with RAII auto-unsubscribe. Returns a move-only handle.
    Subscription SubscribeScoped(const std::string& topic, Handler handler);

    /// Unsubscribe using the ID returned by Subscribe().
    void Unsubscribe(SubID id);

    /// Publish an event synchronously to all matching subscribers.
    void Publish(const std::string& topic, const std::any& event);

    /// Publish asynchronously (queued to thread pool).
    void PublishAsync(const std::string& topic, const std::any& event);

    /// Subscribe to all events (debug/logging).
    SubID SubscribeAll(Handler handler);

    /// Shutdown the async worker thread.
    void Shutdown();

private:
    Bus();
    bool MatchTopic(const std::string& pattern, const std::string& topic);
    void WorkerThread();

    struct Entry {
        SubID id;
        std::string topic;
        Handler handler;
    };
    std::vector<Entry> entries_;
    std::mutex mutex_;
    std::atomic<SubID> nextId_{1};

    std::queue<std::pair<std::string, std::any>> asyncQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    std::thread worker_;
    std::atomic<bool> running_{true};
};

/// RAII subscription handle. Unsubscribes on destruction; move-only.
class Subscription {
public:
    Subscription() noexcept = default;
    Subscription(Bus& bus, Bus::SubID id) noexcept;
    ~Subscription();

    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;

    Subscription(Subscription&& other) noexcept;
    Subscription& operator=(Subscription&& other) noexcept;

    /// True if still subscribed (not yet moved-from or unsubscribed).
    bool Valid() const noexcept;

    /// Manual unsubscribe (no-op if already unsubscribed).
    void Unsubscribe();

    /// Underlying subscription ID.
    Bus::SubID GetID() const noexcept;

private:
    Bus* bus_ = nullptr;
    Bus::SubID id_ = 0;
};

} // namespace unigui::events
