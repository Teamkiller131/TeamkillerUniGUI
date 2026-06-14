#include <unigui/core/log.h>
#include <unigui/events/eventbus.h>

#include <algorithm>

namespace unigui::events {

// ── Subscription RAII handle ────────────────────────────────────────────────

Subscription::Subscription(Bus& bus, Bus::SubID id) noexcept
        : bus_(&bus)
        , id_(id) {}

Subscription::~Subscription() {
    Unsubscribe();
}

Subscription::Subscription(Subscription&& other) noexcept
        : bus_(other.bus_)
        , id_(other.id_) {
    other.bus_ = nullptr;
    other.id_ = 0;
}

Subscription& Subscription::operator=(Subscription&& other) noexcept {
    if (this != &other) {
        Unsubscribe();
        bus_ = other.bus_;
        id_ = other.id_;
        other.bus_ = nullptr;
        other.id_ = 0;
    }
    return *this;
}

bool Subscription::Valid() const noexcept {
    return bus_ != nullptr && id_ != 0;
}

void Subscription::Unsubscribe() {
    if (bus_ && id_) {
        bus_->Unsubscribe(id_);
        bus_ = nullptr;
        id_ = 0;
    }
}

Bus::SubID Subscription::GetID() const noexcept {
    return id_;
}

// ── Bus ─────────────────────────────────────────────────────────────────────

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
    entries_.push_back({id, topic, std::move(handler)});
    return id;
}

Subscription Bus::SubscribeScoped(const std::string& topic, Handler handler) {
    SubID id = Subscribe(topic, std::move(handler));
    return Subscription(*this, id);
}

void Bus::Unsubscribe(SubID id) {
    std::lock_guard lock(mutex_);
    entries_.erase(
        std::remove_if(entries_.begin(), entries_.end(), [id](auto& s) { return s.id == id; }),
        entries_.end());
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
    std::vector<Handler> matching;
    {
        std::lock_guard lock(mutex_);
        for (auto& e : entries_) {
            if (MatchTopic(e.topic, topic))
                matching.push_back(e.handler);
        }
    }
    for (auto& h : matching) {
        try {
            h(event);
        } catch (const std::exception& e) {
            UNIGUI_LOG_ERROR("EventBus: handler for topic '{}' threw: {}", topic, e.what());
        } catch (...) {
            UNIGUI_LOG_ERROR("EventBus: handler for topic '{}' threw a non-std exception", topic);
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
