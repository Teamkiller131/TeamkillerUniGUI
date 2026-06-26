#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// app.h — the application layer  (namespace unigui::dsl).
//
// Builds on the component model (component.h) to give an application a structure:
//   • Store<T>   — shared, app-wide reactive state (vs. component-local State);
//   • Navigator  — a stack of screens (Components) you push/pop/replace.
//
// Components react to a Store with Component::Watch(store); effects (subscriptions,
// timers, async loads kicked off in OnMount) clean up via Component::OnCleanup.
//
//     Store<int> counter{0};
//     class Screen : public dsl::Component {
//     public:
//         void OnMount() override { Watch(counter); }      // re-render on change
//         dsl::NodePtr Build() override {
//             return dsl::Text("counter = " + std::to_string(counter()));
//         }
//     };
//     Navigator nav;
//     nav.Push(std::make_shared<Screen>());
//     Run([&] { nav.Render(); });                          // each frame
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/dsl/component.h>

#include <memory>
#include <utility>
#include <vector>

namespace unigui::dsl {

/// Shared application state — an `Observable<T>` with store ergonomics. Unlike a
/// component-local `State`, a `Store` lives outside the component tree (a global, a
/// member of an app object, or injected) and is shared; components subscribe to it
/// with `Component::Watch(store)` and read it with `store()` / `store.Get()`.
template <typename T> class Store {
public:
    explicit Store(T initial = T{})
            : value_(std::move(initial)) {}

    const T& Get() const { return value_.Get(); }
    const T& operator()() const { return value_.Get(); }

    /// Set the value (change-detected); notifies watchers only on a real change.
    void Set(T v) { value_.Set(std::move(v)); }
    /// Mutate in place, then notify unconditionally.
    template <typename Fn> void Update(Fn&& fn) { value_.Mutate(std::forward<Fn>(fn)); }

    Observable<T>& AsObservable() { return value_; }
    const Observable<T>& AsObservable() const { return value_; }

private:
    Observable<T> value_;
};

/// A stack-based screen navigator: push/pop/replace Components (screens) and render
/// the top one each frame. Owns its screens (shared_ptr); leaving a screen unmounts
/// it (running its effect cleanups + OnUnmount).
class Navigator {
public:
    using ScreenPtr = std::shared_ptr<Component>;

    /// Push a screen onto the stack (it mounts on its first Render()).
    void Push(ScreenPtr screen) { stack_.push_back(std::move(screen)); }

    /// Replace the top screen (unmounts the outgoing one).
    void Replace(ScreenPtr screen) {
        if (!stack_.empty()) {
            stack_.back()->Unmount();
            stack_.pop_back();
        }
        stack_.push_back(std::move(screen));
    }

    /// Pop the top screen (unmounts it), revealing the one beneath.
    void Pop() {
        if (!stack_.empty()) {
            stack_.back()->Unmount();
            stack_.pop_back();
        }
    }

    /// Render the current (top) screen. Call once per frame. Holds a strong ref so
    /// the screen survives its own Render() even if a callback it fires Pop()s or
    /// Replace()s it mid-frame — it then destructs safely after Render() returns.
    void Render() {
        if (stack_.empty())
            return;
        ScreenPtr top = stack_.back();
        top->Render();
    }

    Component* Top() const { return stack_.empty() ? nullptr : stack_.back().get(); }
    int Depth() const { return static_cast<int>(stack_.size()); }
    bool Empty() const { return stack_.empty(); }
    void Clear() {
        while (!stack_.empty())
            Pop();
    }

private:
    std::vector<ScreenPtr> stack_;
};

} // namespace unigui::dsl
