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
#include <tuple>
#include <type_traits>
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

    /// A weak token that expires when this Observable is destroyed. Lets a holder
    /// of a raw `Observable*` confirm the observable is still alive before
    /// dereferencing it (e.g. a widget pushing a two-way-bound value back).
    std::weak_ptr<const void> Lifetime() const noexcept { return reg_; }

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

// ─────────────────────────────────────────────────────────────────────────────
// Computed<T> — a read-only derived observable.
//
// Recomputes its value from one or more source observables whenever any of them
// changes, and notifies its own subscribers (change-detected, just like
// Observable). It composes: a Computed exposes Get()/Subscribe(), so it can serve
// as a source for another Computed or for Bind.
//
//     Observable<int> a{2}, b{3};
//     Computed<int> sum{[](int x, int y){ return x + y; }, a, b};
//     sum.Get();                       // 5
//     auto s = sum.Subscribe(...);     // fires whenever a or b move the sum
//     a = 10;                          // sum recomputes to 13, notifies once
//
// Lifetime: the Computed caches each source's latest value (refreshed via that
// source's own subscription) and computes from the cache — it never reads a
// source through a stored reference. A source may therefore be destroyed before
// the Computed without dangling; it simply stops contributing updates and its
// last value is retained. A Computed is address-sensitive (its source
// subscriptions capture `this`), so it is neither copyable nor movable; hold it
// in stable storage.
//
// Consistency: propagation is push-based and *eventually consistent*, not
// glitch-free. In a multi-path (diamond) graph — a node that depends on a source
// both directly and through an intermediate Computed — a recompute may briefly
// read an intermediate value and may notify subscribers more than once per
// upstream change before settling. For glitch-free results, derive in a single
// Computed that reads only Observable leaves (`compute(a, b)`) rather than
// chaining Computeds into a diamond.
// ─────────────────────────────────────────────────────────────────────────────
template <typename T> class Computed {
public:
    /// Construct from a compute function and the sources it reads. `compute` is
    /// invoked as `compute(sourceValues...)` and must return T. Each source must
    /// expose `.Get()` and `.Subscribe(...)` — an Observable or another Computed
    /// both qualify.
    template <typename Fn, typename... Sources> explicit Computed(Fn compute, Sources&... sources) {
        // Snapshot each source's value into a shared cache; the evaluator computes
        // from the cache, never from a live source reference, so a source going
        // away can never dangle — its slot just stops updating.
        using Cache = std::tuple<std::decay_t<decltype(sources.Get())>...>;
        auto cache = std::make_shared<Cache>(sources.Get()...);
        evaluator_ = [compute = std::move(compute), cache]() -> T {
            return std::apply(compute, *cache);
        };
        SubscribeSources(cache, std::index_sequence_for<Sources...>{}, sources...);
        value_.ForceSet(evaluator_()); // seed the cached value (no observers yet)
    }

    Computed(const Computed&) = delete;
    Computed& operator=(const Computed&) = delete;
    Computed(Computed&&) = delete;
    Computed& operator=(Computed&&) = delete;

    const T& Get() const { return value_.Get(); }
    operator const T&() const { return value_.Get(); }

    /// Subscribe to recomputations; the returned handle unsubscribes on destruction.
    [[nodiscard]] Subscription Subscribe(typename Observable<T>::Observer obs) {
        return value_.Subscribe(std::move(obs));
    }
    [[nodiscard]] Subscription SubscribeAndFire(typename Observable<T>::Observer obs) {
        return value_.SubscribeAndFire(std::move(obs));
    }
    std::size_t ObserverCount() const { return value_.ObserverCount(); }

    /// Expose the backing observable so a Computed can be used wherever an
    /// `Observable<T>&` is expected (Bind, or a source for a further Computed).
    Observable<T>& AsObservable() { return value_; }
    const Observable<T>& AsObservable() const { return value_; }

private:
    template <typename Cache, std::size_t... Is, typename... Sources>
    void SubscribeSources(std::shared_ptr<Cache> cache, std::index_sequence<Is...>,
                          Sources&... sources) {
        // On each source change: refresh that slot of the cache, then recompute.
        (srcSubs_.push_back(sources.Subscribe([this, cache](const auto& v) {
            std::get<Is>(*cache) = v;
            Recompute();
        })),
         ...);
    }
    void Recompute() { value_.Set(evaluator_()); }

    // Declaration order matters: srcSubs_ is declared LAST so it is destroyed
    // first, unsubscribing from every source before evaluator_/value_ are torn
    // down — no source can fire Recompute() into a half-destroyed Computed.
    std::function<T()> evaluator_;
    Observable<T> value_;
    std::vector<Subscription> srcSubs_;
};

/// Bind an observable to a sink: invokes `sink` immediately with the current
/// value and on every subsequent change. Returns the owning subscription.
template <typename T, typename Sink>
[[nodiscard]] Subscription Bind(Observable<T>& source, Sink&& sink) {
    return source.SubscribeAndFire(std::forward<Sink>(sink));
}

/// Bind a Computed to a sink: fires immediately with the current derived value
/// and on every subsequent recomputation. Returns the owning subscription.
template <typename T, typename Sink>
[[nodiscard]] Subscription Bind(Computed<T>& source, Sink&& sink) {
    return source.SubscribeAndFire(std::forward<Sink>(sink));
}

} // namespace unigui
