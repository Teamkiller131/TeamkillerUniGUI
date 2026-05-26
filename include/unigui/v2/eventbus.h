#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <any>
#include <regex>
#include <atomic>
#include <thread>
#include <queue>
#include <condition_variable>

namespace unigui::v2 {

/// EventBus: publish-subscribe messaging across all v2 components.
/// Thread-safe. Supports synchronous + async publishing.
class EventBus {
public:
    using Handler = std::function<void(const std::any& event)>;
    using SubID = uint64_t;

    static EventBus& Instance();

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
    EventBus();
    bool MatchTopic(const std::string& pattern, const std::string& topic);
    void WorkerThread();

    struct Subscription { SubID id; std::string topic; Handler handler; };
    std::vector<Subscription> subs_;
    std::mutex mutex_;
    std::atomic<SubID> nextId_{1};

    std::queue<std::pair<std::string, std::any>> asyncQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    std::thread worker_;
    std::atomic<bool> running_{true};
};

} // namespace unigui::v2
