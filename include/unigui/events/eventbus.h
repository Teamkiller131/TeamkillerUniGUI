#pragma once
#include <any>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace unigui::events {

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

    struct Subscription {
        SubID id;
        std::string topic;
        Handler handler;
    };
    std::vector<Subscription> subs_;
    std::mutex mutex_;
    std::atomic<SubID> nextId_{1};

    std::queue<std::pair<std::string, std::any>> asyncQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    std::thread worker_;
    std::atomic<bool> running_{true};
};

} // namespace unigui::events
