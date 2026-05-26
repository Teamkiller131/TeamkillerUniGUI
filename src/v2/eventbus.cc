#include <unigui/events/eventbus.h>
#include <unigui/core/log.h>
#include <algorithm>

namespace unigui::events {

Bus::Bus() { worker_ = std::thread(&Bus::WorkerThread, this); }

Bus& Bus::Instance() { static Bus eb; return eb; }

Bus::SubID Bus::Subscribe(const std::string& topic, Handler handler) {
    std::lock_guard lock(mutex_);
    SubID id = nextId_++;
    subs_.push_back({id, topic, std::move(handler)});
    return id;
}

void Bus::Unsubscribe(SubID id) {
    std::lock_guard lock(mutex_);
    subs_.erase(std::remove_if(subs_.begin(), subs_.end(),
        [id](auto& s) { return s.id == id; }), subs_.end());
}

Bus::SubID Bus::SubscribeAll(Handler handler) {
    return Subscribe("*", std::move(handler));
}

bool Bus::MatchTopic(const std::string& pattern, const std::string& topic) {
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

void Bus::Publish(const std::string& topic, const std::any& event) {
    std::lock_guard lock(mutex_);
    for (auto& s : subs_) {
        if (MatchTopic(s.topic, topic)) {
            try { s.handler(event); } catch (...) {}
        }
    }
}

void Bus::PublishAsync(const std::string& topic, const std::any& event) {
    std::lock_guard lock(queueMutex_);
    asyncQueue_.push({topic, event});
    queueCV_.notify_one();
}

void Bus::WorkerThread() {
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

void Bus::Shutdown() {
    running_ = false;
    queueCV_.notify_all();
    if (worker_.joinable()) worker_.join();
}

} // namespace unigui::events
