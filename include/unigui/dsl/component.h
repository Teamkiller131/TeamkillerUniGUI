#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// component.h — reactive Component + State  (namespace unigui::dsl).
//
// The application model. A Component owns reactive State, declares its view as a
// DSL node tree in Build(), and the framework re-Build()s it only when state
// changes (dirty tracking) and renders the cached tree each frame. This turns
// "widgets you call every frame" into "an app built from self-contained, stateful,
// composable units" — the spine that makes UniGUI a framework rather than a
// widget library.
//
//     class Counter : public dsl::Component {
//         dsl::State<int> count_{this, 0};
//     public:
//         dsl::NodePtr Build() override {
//             return dsl::VBox({
//                 dsl::Text("Count: " + std::to_string(count_())),
//                 dsl::Button("Increment", [this] { count_ = count_() + 1; }),
//             });
//         }
//     };
//     Counter app;                 // hold it in stable storage
//     Run([&] { app.Render(); });  // each frame
//
// Compose with Host(child) inside a Build() tree; each child keeps its own State
// and dirty tracking.
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/core/observable.h>
#include <unigui/dsl/dsl.h>

#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace unigui::dsl {

class Component;

/// A reactive state cell owned by a Component. Reading returns the current value;
/// writing change-detects (operator==) and, on a real change, marks the owning
/// component dirty so it re-Build()s on the next frame. Built on Observable<T>, so
/// it also feeds Computed<T> / Bind via AsObservable().
template <typename T> class State {
public:
    State(Component* owner, T initial = T{})
            : value_(std::move(initial))
            , owner_(owner) {}

    State(const State&) = delete;
    State& operator=(const State&) = delete;

    const T& operator()() const { return value_.Get(); } // read: state_()
    const T& Get() const { return value_.Get(); }

    /// Set the value; if it actually changed, mark the owning component dirty.
    void Set(T v);
    State& operator=(T v) {
        Set(std::move(v));
        return *this;
    }
    /// Mutate the value in place, then mark dirty unconditionally.
    template <typename Fn> void Mutate(Fn&& fn) {
        value_.Mutate(std::forward<Fn>(fn));
        MarkOwnerDirty();
    }

    Observable<T>& AsObservable() { return value_; }
    const Observable<T>& AsObservable() const { return value_; }

private:
    void MarkOwnerDirty();
    Observable<T> value_;
    Component* owner_;
};

namespace detail {
/// Live-component registry powering DrawInspector(). Single-threaded UI, so a plain
/// vector is fine. Each Component registers in its ctor and removes itself in its
/// dtor. Intentionally leaked (heap, never freed) so a Component held in a static
/// can still deregister at program teardown regardless of static-destruction order.
inline std::vector<Component*>& ComponentRegistry() {
    static std::vector<Component*>* reg = new std::vector<Component*>();
    return *reg;
}
} // namespace detail

/// Base class for a reactive UI component. Override Build() to declare the view as
/// a DSL node tree from the component's State; the framework rebuilds it only when
/// state changes and renders the cached tree each frame.
///
/// A Component is address-sensitive — its State cells hold a back-pointer to it —
/// so it is neither copyable nor movable; hold it in stable storage (a member, or
/// on the heap).
class Component {
public:
    Component() { detail::ComponentRegistry().push_back(this); }
    virtual ~Component() {
        auto& reg = detail::ComponentRegistry();
        reg.erase(std::remove(reg.begin(), reg.end(), this), reg.end());
    }

    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    /// Declare the view as a DSL node tree from the component's current State.
    virtual NodePtr Build() = 0;

    /// Display name for the inspector (override to identify a component type).
    virtual const char* InspectorName() const { return "Component"; }

    /// Lifecycle. OnMount runs once, just before the first Build(). OnUnmount is a
    /// hook the app/navigator calls when the component leaves the tree; it is not
    /// auto-invoked from the destructor (a virtual call there would dispatch to the
    /// base). Call Unmount() explicitly to fire it.
    virtual void OnMount() {}
    virtual void OnUnmount() {}

    /// Request a rebuild on the next Render(). Called automatically by State writes.
    void MarkDirty() { dirty_ = true; }
    bool IsDirty() const { return dirty_; }
    bool IsMounted() const { return mounted_; }
    /// Number of times Build() has run (rebuilds). Useful for the inspector.
    int BuildCount() const { return buildCount_; }

    /// React to *shared* state: re-render this component whenever `source` changes.
    /// `source` is anything with `AsObservable()` — a `Store`, another component's
    /// `State`, a `Computed`, or a raw `Observable`. Establish in OnMount() (or the
    /// constructor); the subscription lives for the component's lifetime. This is
    /// the bridge from app-wide state to a component's view.
    template <typename W> void Watch(W& source) {
        watches_.push_back(source.AsObservable().Subscribe([this](const auto&) { MarkDirty(); }));
    }

    /// Register a teardown to run when the component unmounts — the cleanup half of
    /// an effect set up in OnMount() (cancel a subscription, stop a timer, …). Run
    /// in reverse order of registration.
    void OnCleanup(std::function<void()> fn) { cleanups_.push_back(std::move(fn)); }

    /// Render once per frame: mount on the first call, (re)Build the view tree when
    /// state changed since the last build, then draw the cached tree.
    void Render() {
        if (!mounted_) {
            mounted_ = true;
            OnMount();
        }
        if (dirty_) {
            tree_ = Build();
            ++buildCount_;
            dirty_ = false;
        }
        dsl::Render(tree_);
    }

    /// Unmount (idempotent): run the registered cleanups (reverse order), then
    /// OnUnmount. The app/navigator calls this when removing the component from the
    /// live tree.
    void Unmount() {
        if (!mounted_)
            return;
        for (auto it = cleanups_.rbegin(); it != cleanups_.rend(); ++it)
            if (*it)
                (*it)();
        cleanups_.clear();
        OnUnmount();
        mounted_ = false;
    }

private:
    NodePtr tree_;
    bool dirty_ = true;
    bool mounted_ = false;
    int buildCount_ = 0;
    std::vector<Subscription> watches_;           // external-state subscriptions (lifetime)
    std::vector<std::function<void()>> cleanups_; // effect teardowns, run on Unmount
};

template <typename T> void State<T>::Set(T v) {
    if (value_.Set(std::move(v)))
        MarkOwnerDirty();
}
template <typename T> void State<T>::MarkOwnerDirty() {
    if (owner_)
        owner_->MarkDirty();
}

/// Embed a child Component inside a parent's Build() tree (composition). The child
/// keeps its own State and dirty tracking. The parent MUST own the child (e.g. as a
/// member) so it outlives the rendered tree.
inline NodePtr Host(Component& child) {
    return Custom([&child] { child.Render(); });
}

/// Debug overlay: list every live Component with its name, mount/dirty state, and
/// build count (rebuild count). Call inside a window each frame — it draws with the
/// immediate layer. Pairs with ImGui's own metrics window for diagnosing why a
/// component is (or isn't) rebuilding.
inline void DrawInspector() {
    const auto& reg = detail::ComponentRegistry();
    im::Text("Live components: " + std::to_string(reg.size()));
    for (const Component* c : reg) {
        std::string line = c->InspectorName();
        line += c->IsMounted() ? "  [mounted" : "  [unmounted";
        if (c->IsDirty())
            line += ", dirty";
        line += "]  builds=" + std::to_string(c->BuildCount());
        im::BulletText(line);
    }
}

} // namespace unigui::dsl
