# Building apps with UniGUI — the framework guide

UniGUI gives you four ways to put pixels on screen: raw Dear ImGui, the themed
immediate layer (`unigui::im`), retained widgets (`unigui::Button`, …), and the
**declarative framework** (`unigui::dsl`). This guide is about the last one — the
**golden path** for building real applications.

> **The golden path:** describe your UI as **Components** whose views are a
> function of reactive **State**; let the framework own the loop and re-render only
> what changed. Reach for `im::`/raw ImGui only as an escape hatch (via
> `dsl::Custom`), and use retained widgets as leaf building blocks.

The other three layers don't go away — they're how you drop down when you need
custom drawing — but the component model is what an *application* should be built
from.

---

## 1. Components and State

A **Component** owns reactive **State** and declares its view in `Build()`:

```cpp
using namespace unigui::dsl;

class Counter : public Component {
    State<int> count_{this, 0};                 // reactive cell, owned by the component
public:
    NodePtr Build() override {                  // view = f(state)
        return VBox({
            Text("Count: " + std::to_string(count_())),
            Button("Increment", [this] { count_ = count_() + 1; }),
        });
    }
};
```

- `count_()` reads the value; `count_ = …` (or `count_.Set(…)` / `count_.Mutate(…)`)
  writes it.
- A write **change-detects** (`operator==`) and, on a real change, marks the
  component dirty.
- `Render()` (called once per frame) mounts the component on first use, **rebuilds
  the view tree only when state changed**, and draws the cached tree. So `Build()`
  doesn't run every frame — only after a state change. This is the
  rebuild-on-setState model, adapted to immediate mode.

Hold a component in stable storage (a member, or on the heap) — it's
address-sensitive (its `State` holds a back-pointer) and therefore non-copyable
and non-movable.

```cpp
Counter app;                       // a member or a static
Run([&] { app.Render(); });        // each frame
```

## 2. Composition

Embed one component in another's view with `Host` — each child keeps its own state
and dirty tracking:

```cpp
class Dashboard : public Component {
    Counter left_, right_;
public:
    NodePtr Build() override {
        return Flex({ Host(left_), Host(right_) }, 12.0f);
    }
};
```

Bumping `left_`'s state rebuilds **only** `left_`, not the whole dashboard.

## 3. Shared state — `Store`

`State` is component-local. For state shared across screens, use a **`Store`**,
held outside the component tree, and `Watch` it so the component re-renders on
change:

```cpp
Store<int> cartCount{0};                       // app-wide

class Header : public Component {
public:
    void OnMount() override { Watch(cartCount); } // re-render when the cart changes
    NodePtr Build() override {
        return Text("Cart: " + std::to_string(cartCount()));
    }
};
```

`Watch` accepts anything with `AsObservable()` — a `Store`, another component's
`State`, a `Computed`, or a raw `Observable` — so derived values compose:

```cpp
Computed<std::string> label{[](int n){ return "Items: " + std::to_string(n); },
                            cartCount.AsObservable()};
```

## 4. Effects and lifecycle

`OnMount()` runs once before the first build — set up subscriptions, timers, or
kick off an async load there. Pair each with `OnCleanup(...)`, run (in reverse
order) when the component **unmounts**:

```cpp
void OnMount() override {
    auto sub = feed.Subscribe([this](auto&){ MarkDirty(); });
    OnCleanup([sub = std::move(sub)]() mutable { /* sub drops → unsubscribe */ });
}
```

For background work, do it on a worker thread and marshal the result back with
`unigui::InvokeOnMainThread`, updating a `State`/`Store` from there.

## 5. Navigation — `Navigator`

A **`Navigator`** is a stack of screens (each a `Component`). Push/pop/replace,
and render the top one each frame; leaving a screen unmounts it (running its
cleanups):

```cpp
Navigator nav;
nav.Push(std::make_shared<HomeScreen>(&nav));
Run([&] { WindowScope w{"App"}; if (w) nav.Render(); });
```

A screen navigates from a button callback — `nav->Push(...)` / `nav->Pop()` — even
mid-render; the navigator keeps the current screen alive until its `Render()`
returns.

## 6. Inspecting

`dsl::DrawInspector()` draws a live list of every mounted component with its
dirty state and rebuild count — call it inside a window to see *what* is
re-rendering and *why*:

```cpp
WindowScope insp{"Inspector"}; if (insp) DrawInspector();
```

## 7. Escape hatch

When you need custom immediate-mode drawing inside a declarative tree, wrap it in
`dsl::Custom`:

```cpp
NodePtr Build() override {
    return VBox({
        Text("My chart:"),
        Custom([this]{ im::PlotLines("##c", data_.data(), (int)data_.size()); }),
    });
}
```

(`Host` is itself just a `Custom` that renders a child component.)

---

## Worked example

[`examples/framework_demo`](../examples/framework_demo/main.cc) is a complete
multi-screen app built entirely in this idiom — a shared `Store`, a `Navigator`
with two screens, component-local `State`, `Watch`, an effect with cleanup, and
the inspector overlay. Run it headless with `./framework_demo --frames 10`.

## When to use which layer

| You want… | Use |
|-----------|-----|
| An application with screens, shared state, navigation | **`dsl` framework** (this guide) |
| A reusable stateful control with validation/undo | a **retained widget** (`unigui::Button`, `DataTable<T>`, …) — often as a leaf inside `Custom` |
| A one-off control or custom drawing | **`unigui::im`** inside `dsl::Custom` |
| Something the wrapper doesn't cover | **raw `ImGui::`** inside `dsl::Custom` |
