#include <unigui/core/log.h>
#include <unigui/events/eventbus.h>

#include <algorithm>

namespace unigui::events {

Bus::Bus() {
    worker_ = std::thread(&Bus::WorkerThread, this);
}

Bus& Bus::Instance() {
    static Bus eb;
    return eb;
}

Bus::SubID Bus::Subscribe(const std::string& topic, Handler handler) {
    std::lock_guard lock(mutex_);
    SubID id = nextId_++;
    subs_.push_back({id, topic, std::move(handler)});
    return id;
}

void Bus::Unsubscribe(SubID id) {
    std::lock_guard lock(mutex_);
    subs_.erase(std::remove_if(subs_.begin(), subs_.end(), [id](auto& s) { return s.id == id; }),
                subs_.end());
}

Bus::SubID Bus::SubscribeAll(Handler handler) {
    return Subscribe("*", std::move(handler));
}

bool Bus::MatchTopic(const std::string& pattern, const std::string& topic) {
    if (pattern == "*")
        return true;
    if (pattern.find('*') == std::string::npos)
        return pattern == topic;

    // Linear-time glob match: '*' matches any (possibly empty) run of characters,
    // every other character (including '.') is matched literally. This replaces
    // the previous std::regex(re) that recompiled a regex on every Publish() ×
    // subscriber, which was the hot-path cost flagged in review.
    size_t p = 0, s = 0;
    size_t star = std::string::npos, mark = 0;
    while (s < topic.size()) {
        if (p < pattern.size() && pattern[p] == topic[s]) {
            ++p;
            ++s;
        } else if (p < pattern.size() && pattern[p] == '*') {
            star = p++; // remember star position, consume zero chars for now
            mark = s;
        } else if (star != std::string::npos) {
            p = star + 1; // backtrack: let the last '*' absorb one more char
            s = ++mark;
        } else {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == '*')
        ++p;
    return p == pattern.size();
}

void Bus::Publish(const std::string& topic, const std::any& event) {
    std::lock_guard lock(mutex_);
    for (auto& s : subs_) {
        if (MatchTopic(s.topic, topic)) {
            try {
                s.handler(event);
            } catch (const std::exception& e) {
                UNIGUI_LOG_ERROR("EventBus: handler for topic '{}' threw: {}", topic, e.what());
            } catch (...) {
                UNIGUI_LOG_ERROR("EventBus: handler for topic '{}' threw a non-std exception",
                                 topic);
            }
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
        if (!running_)
            break;
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
    if (worker_.joinable())
        worker_.join();
}

} // namespace unigui::events
