#include <unigui/v2/eventbus.h>
#include <unigui/core/log.h>

namespace unigui::v2 {

EventBus::EventBus() { worker_ = std::thread(&EventBus::WorkerThread, this); }

EventBus& EventBus::Instance() { static EventBus eb; return eb; }

EventBus::SubID EventBus::Subscribe(const std::string& topic, Handler handler) {
    std::lock_guard lock(mutex_);
    SubID id = nextId_++;
    subs_.push_back({id, topic, std::move(handler)});
    return id;
}

void EventBus::Unsubscribe(SubID id) {
    std::lock_guard lock(mutex_);
    subs_.erase(std::remove_if(subs_.begin(), subs_.end(),
        [id](auto& s) { return s.id == id; }), subs_.end());
}

EventBus::SubID EventBus::SubscribeAll(Handler handler) {
    return Subscribe("*", std::move(handler));
}

bool EventBus::MatchTopic(const std::string& pattern, const std::string& topic) {
    if (pattern == "*") return true;
    // Convert glob pattern to regex: "window.*" -> "window\..*"
    if (pattern.find('*') != std::string::npos) {
        std::string re = "^";
        for (char c : pattern) {
            if (c == '*') re += ".*";
            else if (c == '.') re += "\\.";
            else re += c;
        }
        re += "$";
        return std::regex_match(topic, std::regex(re));
    }
    return pattern == topic;
}

void EventBus::Publish(const std::string& topic, const std::any& event) {
    std::lock_guard lock(mutex_);
    for (auto& s : subs_) {
        if (MatchTopic(s.topic, topic)) {
            try { s.handler(event); } catch (...) {}
        }
    }
}

void EventBus::PublishAsync(const std::string& topic, const std::any& event) {
    std::lock_guard lock(queueMutex_);
    asyncQueue_.push({topic, event});
    queueCV_.notify_one();
}

void EventBus::WorkerThread() {
    while (running_) {
        std::unique_lock lock(queueMutex_);
        queueCV_.wait_for(lock, std::chrono::milliseconds(100));
        if (!running_) break;
        while (!asyncQueue_.empty()) {
            auto [topic, event] = std::move(asyncQueue_.front());
            asyncQueue_.pop();
            lock.unlock();
            Publish(topic, event);
            lock.lock();
        }
    }
}

void EventBus::Shutdown() {
    running_ = false;
    queueCV_.notify_all();
    if (worker_.joinable()) worker_.join();
}

} // namespace unigui::v2
