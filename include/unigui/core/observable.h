#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// observable.h — a tiny, header-only reactive value (unigui::Observable<T>).
//
// An Observable<T> wraps a value and notifies subscribers when it changes, so
// retained widgets can update from model changes without manual Set* plumbing.
// Subscriptions are RAII (`Subscription`): they auto-unsubscribe on destruction,
// and survive the Observable being destroyed first (no dangling) via a shared
// registry + weak reference.
//
//     Observable<int> qty{1};
//     auto sub = qty.Subscribe([](const int& n){ /* react */ });
//     qty = 5;                 // fires the observer
//     qty.Set(5);              // no-op: unchanged (operator== compares)
//
// This pairs naturally with the trading models (Horizon 3) and the broader
// data-binding workstream (Horizon 5).
// ─────────────────────────────────────────────────────────────────────────────

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace unigui {

/// Move-only RAII handle for one subscription. Destroying (or reset()-ing) it
/// removes the observer. Safe to outlive its Observable.
class Subscription {
public:
    Subscription() = default;
    explicit Subscription(std::function<void()> unsub)
            : unsub_(std::move(unsub)) {}

    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;

    Subscription(Subscription&& o) noexcept
            : unsub_(std::move(o.unsub_)) {
        o.unsub_ = nullptr;
    }
    Subscription& operator=(Subscription&& o) noexcept {
        if (this != &o) {
            Reset();
            unsub_ = std::move(o.unsub_);
            o.unsub_ = nullptr;
        }
        return *this;
    }
    ~Subscription() { Reset(); }

    /// Unsubscribe now (idempotent).
    void Reset() {
        if (unsub_) {
            auto fn = std::move(unsub_);
            unsub_ = nullptr;
            fn();
        }
    }
    /// True while still subscribed.
    bool Active() const { return static_cast<bool>(unsub_); }

private:
    std::function<void()> unsub_;
};

template <typename T> class Observable {
public:
    using Observer = std::function<void(const T&)>;

    Observable() = default;
    explicit Observable(T value)
            : value_(std::move(value)) {}

    // Move-only: the registry (and thus existing subscriptions) travels with the
    // moved-from instance's shared control block. Copying would silently share
    // observers, which is rarely intended.
    Observable(const Observable&) = delete;
    Observable& operator=(const Observable&) = delete;
    Observable(Observable&&) noexcept = default;
    Observable& operator=(Observable&&) noexcept = default;

    const T& Get() const { return value_; }
    operator const T&() const { return value_; }

    /// Set the value, notifying observers only if it actually changed
    /// (compared with operator==). Returns true if a change/notification happened.
    bool Set(T value) {
        if (value == value_)
            return false;
        value_ = std::move(value);
        Notify();
        return true;
    }
    Observable& operator=(T value) {
        Set(std::move(value));
        return *this;
    }

    /// Assign and notify unconditionally (even if equal) — useful when equality
    /// can't capture a meaningful change.
    void ForceSet(T value) {
        value_ = std::move(value);
        Notify();
    }

    /// Mutate the value in place through a callback, then notify unconditionally.
    template <typename Fn> void Mutate(Fn&& fn) {
        std::forward<Fn>(fn)(value_);
        Notify();
    }

    /// Subscribe to changes. The returned handle unsubscribes on destruction.
    [[nodiscard]] Subscription Subscribe(Observer obs) {
        const std::uint64_t id = reg_->nextId++;
        reg_->observers.emplace_back(id, std::move(obs));
        std::weak_ptr<Registry> weak = reg_;
        return Subscription([weak, id] {
            if (auto r = weak.lock()) {
                auto& v = r->observers;
                v.erase(std::remove_if(v.begin(), v.end(),
                                       [id](const auto& p) { return p.first == id; }),
                        v.end());
            }
        });
    }

    /// Subscribe and immediately invoke the observer with the current value.
    [[nodiscard]] Subscription SubscribeAndFire(Observer obs) {
        obs(value_);
        return Subscribe(std::move(obs));
    }

    std::size_t ObserverCount() const { return reg_->observers.size(); }

private:
    struct Registry {
        std::vector<std::pair<std::uint64_t, Observer>> observers;
        std::uint64_t nextId = 1;
    };

    void Notify() {
        // Snapshot so an observer may (un)subscribe during its own callback.
        const auto snapshot = reg_->observers;
        for (const auto& [id, obs] : snapshot)
            obs(value_);
    }

    std::shared_ptr<Registry> reg_ = std::make_shared<Registry>();
    T value_{};
};

/// Bind an observable to a sink: invokes `sink` immediately with the current
/// value and on every subsequent change. Returns the owning subscription.
template <typename T, typename Sink>
[[nodiscard]] Subscription Bind(Observable<T>& source, Sink&& sink) {
    return source.SubscribeAndFire(std::forward<Sink>(sink));
}

} // namespace unigui
