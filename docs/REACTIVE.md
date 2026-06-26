# Reactive Layer

UniGUI's reactive layer turns "widgets you call every frame and `Set*` by hand"
into "values that notify when they change, and views that update themselves." It
is a small, header-only foundation — one observable value type, an RAII
subscription handle, a derived-value type, and a couple of binding helpers — that
the DSL component framework (`unigui::dsl`) and the retained widgets build on.

Everything here lives in the `unigui` namespace and ships in
[`<unigui/core/observable.h>`](../include/unigui/core/observable.h); the DSL
extensions (`State`, `Store`) live in `unigui::dsl`
([`<unigui/dsl/component.h>`](../include/unigui/dsl/component.h),
[`<unigui/dsl/app.h>`](../include/unigui/dsl/app.h)). Widget binding hooks live on
`ValueWidget<T>` ([`<unigui/widgets/value_widget.h>`](../include/unigui/widgets/value_widget.h))
and `Label` ([`<unigui/widgets/label.h>`](../include/unigui/widgets/label.h)).

> Version: this document tracks UniGUI **3.16.0**.

## The pieces at a glance

| Type / function | Header | Role |
|---|---|---|
| `Observable<T>` | `core/observable.h` | A value that notifies subscribers when it changes. The reactive primitive. |
| `Subscription` | `core/observable.h` | Move-only RAII handle; destroying it unsubscribes. Safe to outlive its source. |
| `Computed<T>` | `core/observable.h` | A read-only value derived from one or more sources; recomputes when any source changes. |
| `Bind(source, sink)` | `core/observable.h` | Fire `sink` now with the current value, and on every change. Returns the owning `Subscription`. |
| `dsl::State<T>` | `dsl/component.h` | A component-local reactive cell; writing marks the owning `Component` dirty. Built on `Observable<T>`. |
| `dsl::Store<T>` | `dsl/app.h` | Shared, app-wide reactive state. Built on `Observable<T>`. |
| `ValueWidget<T>::BindValue` | `widgets/value_widget.h` | Two-way (default) binding between a widget's value and an `Observable<T>`. |
| `Label::BindText` | `widgets/label.h` | One-way binding of a label's text to an `Observable<std::string>`. |

All four DSL/widget integration points reduce to the same primitive: they expose
an `Observable<T>` (via `AsObservable()`) or wrap a `Subscription`. Learn
`Observable` and `Subscription` and the rest follows.

---

## `Observable<T>`

An `Observable<T>` wraps a value of type `T` and notifies registered observers
when that value changes. `T` must be equality-comparable (`operator==`) for
change detection to work.

### Construction and reading

```cpp
#include <unigui/core/observable.h>
using namespace unigui;

Observable<int> qty;          // value-initialized: T{} == 0
Observable<int> qty2{5};      // explicit initial value

int n = qty2.Get();           // 5  — const T& accessor
int m = qty2;                 // 5  — implicit operator const T&()
```

Two read accessors exist and return a reference to the live value:

```cpp
const T& Get() const;
operator const T&() const;
```

The implicit conversion lets an `Observable<T>` stand in for a `const T&` in most
expressions (`std::to_string(qty)`, `qty + 1`, …), which keeps call sites clean.

### Writing: `Set`, `operator=`, `ForceSet`, `Mutate`

```cpp
bool Set(T value);             // change-detected; returns true if it changed
Observable& operator=(T value);// sugar for Set
void ForceSet(T value);        // assign + notify unconditionally
template <typename Fn> void Mutate(Fn&& fn); // edit in place, then notify
```

`Set` is **change-detecting**: it compares the new value to the current one with
`operator==` and only assigns + notifies if they differ. It returns `true` when a
change (and notification) actually happened. This is what makes binding loops
safe — pushing an unchanged value is a no-op.

```cpp
Observable<int> qty{1};
auto sub = qty.Subscribe([](const int& v) { /* react to v */ });

qty = 5;          // changed (1 → 5): fires the observer
qty.Set(5);       // unchanged: returns false, does NOT fire
qty.ForceSet(5);  // fires anyway — useful when operator== can't see the change
```

`ForceSet` skips the equality check and always notifies. Use it when equality
can't capture a meaningful change (e.g. you mutated the value through aliasing,
or the value compares equal but you reloaded it from a source of truth).

`Mutate` lets you edit the wrapped value in place through a callback, then
notifies **unconditionally** (it does not compare before/after):

```cpp
Observable<std::vector<int>> items{{1, 2, 3}};
items.Mutate([](std::vector<int>& v) { v.push_back(4); });  // notifies once
```

`Mutate` is the right tool for container-typed observables where copying the
whole value just to call `Set` would be wasteful.

### Subscribing: `Subscribe`, `SubscribeAndFire`

```cpp
using Observer = std::function<void(const T&)>;
[[nodiscard]] Subscription Subscribe(Observer obs);
[[nodiscard]] Subscription SubscribeAndFire(Observer obs);
```

`Subscribe` registers an observer and returns a `Subscription` that owns the
registration. **You must keep the returned `Subscription` alive** — when it is
destroyed the observer is removed. (The `[[nodiscard]]` attribute will warn if
you drop it on the floor.)

```cpp
Observable<std::string> name{"Ada"};
Subscription sub = name.Subscribe([](const std::string& s) {
    std::printf("name is now %s\n", s.c_str());
});
name = "Grace";   // prints "name is now Grace"
// when `sub` goes out of scope, the observer is removed
```

`SubscribeAndFire` is the same, except it invokes the observer **once
immediately** with the current value before returning. This is the usual choice
for a view that needs to display the current value right away and then track
changes:

```cpp
Subscription sub = name.SubscribeAndFire([&](const std::string& s) {
    label.SetText(s);   // shows "Ada" immediately, then every change
});
```

### Change-detection contract

Notification only happens through `Set`/`operator=` when `new == old` is `false`,
or unconditionally through `ForceSet`/`Mutate`. Observers receive the **new**
value as a `const T&`. During a notification, the observer list is snapshotted
before iteration, so an observer may safely add or remove subscriptions (even its
own) from within its own callback without invalidating the in-flight loop.

### Introspection: `ObserverCount`, `Lifetime`, `AsObservable`

```cpp
std::size_t ObserverCount() const;
std::weak_ptr<const void> Lifetime() const noexcept;
Observable&       AsObservable()       noexcept;
const Observable& AsObservable() const noexcept;
```

- `ObserverCount()` returns the number of live observers — handy in tests.
- `Lifetime()` returns a weak token that expires when the `Observable` is
  destroyed. A holder of a raw `Observable*` can `lock()`/check `expired()` on
  this token to confirm the source is still alive before dereferencing the
  pointer. This is exactly how two-way widget bindings avoid dangling (see
  [`ValueWidget`](#valuewidgett-two-way-binding) below).
- `AsObservable()` is an identity accessor. Its purpose is uniformity: generic
  code (most importantly `Component::Watch`) accepts "anything with an
  `AsObservable()`," and `Observable`, `Computed`, `State`, and `Store` all
  provide one. For a plain `Observable` it simply returns `*this`.

### Move-only semantics

```cpp
Observable(const Observable&)            = delete;
Observable& operator=(const Observable&) = delete;
Observable(Observable&&) noexcept            = default;
Observable& operator=(Observable&&) noexcept = default;
```

`Observable<T>` is **move-only**. Copying is deliberately forbidden: the observer
registry is a shared control block, so a copy would silently share observers with
the original — almost never what you want. Moving transfers the registry (and
therefore the existing subscriptions) to the moved-into instance.

> Note: a widget or component that has *bound* itself with a `this`-capturing
> observer (see binding sections) must not be moved after binding, because the
> captured `this` would dangle. The observable itself stays movable, but a bound
> *holder* does not.

---

## `Subscription`

`Subscription` is a move-only RAII handle representing one registered observer.

```cpp
class Subscription {
public:
    Subscription() = default;                            // empty / inactive
    explicit Subscription(std::function<void()> unsub);  // (built internally)

    Subscription(const Subscription&)            = delete;
    Subscription& operator=(const Subscription&) = delete;
    Subscription(Subscription&&) noexcept;
    Subscription& operator=(Subscription&&) noexcept;
    ~Subscription();                                     // unsubscribes

    void Reset();          // unsubscribe now (idempotent)
    bool Active() const;   // true while still subscribed
};
```

Key properties:

- **RAII:** destroying the `Subscription` unsubscribes. Storing one in a member
  ties the observer's lifetime to the owning object.
- **Move-only:** you can move a subscription into a container or a member, but
  not copy it. Move-assigning over an active subscription resets the old one
  first.
- **`Reset()`** unsubscribes immediately and is idempotent — calling it again
  does nothing. `Active()` reports whether the subscription still holds a live
  observer.
- **Safe after the source is destroyed.** The unsubscribe callback holds a
  `std::weak_ptr` to the observable's registry. If the `Observable` is destroyed
  first, the weak pointer fails to lock and the `Subscription`'s destruction
  becomes a harmless no-op — never a dangling access. So a `Subscription` may
  freely outlive its `Observable`.

```cpp
Subscription sub;
{
    Observable<int> tmp{0};
    sub = tmp.Subscribe([](int) {});
}                  // tmp destroyed here; sub now refers to nothing
// sub's destructor runs safely — no dangling, no crash
```

Because a `Subscription` member makes its owner move-only-friendly (the member
itself is move-only and self-detaching), widgets like `Label` and `ValueWidget`
remain movable *until bound* and become non-relocatable once a binding captures
`this`.

---

## `Computed<T>`

`Computed<T>` is a **read-only derived observable**. It computes its value from
one or more source observables, recomputes whenever any source changes, and
notifies its own subscribers (with the same change-detection as `Observable`).

```cpp
template <typename Fn, typename... Sources>
explicit Computed(Fn compute, Sources&... sources);
```

`compute` is invoked as `compute(sourceValues...)` and must return `T`. Each
source must expose `.Get()` and `.Subscribe(...)` — an `Observable` or another
`Computed` both qualify, and the source value types may be heterogeneous.

```cpp
Observable<int> a{2}, b{3};
Computed<int> sum{[](int x, int y) { return x + y; }, a, b};

sum.Get();                            // 5
auto s = sum.Subscribe([](int v) { /* v = new sum */ });
a = 10;                               // sum recomputes to 13, notifies once
```

### Reading and composing

`Computed<T>` mirrors `Observable<T>`'s read API and most of its subscription
API:

```cpp
const T& Get() const;
operator const T&() const;
[[nodiscard]] Subscription Subscribe(typename Observable<T>::Observer obs);
[[nodiscard]] Subscription SubscribeAndFire(typename Observable<T>::Observer obs);
std::size_t ObserverCount() const;
Observable<T>&       AsObservable();
const Observable<T>& AsObservable() const;
```

Because a `Computed` exposes `Get()` and `Subscribe()`, it can itself serve as a
**source** for another `Computed`, and because it exposes `AsObservable()` it can
be passed to `Bind` or to `Component::Watch`.

```cpp
Observable<double> price{100.0}, qty{3.0};
Computed<double> notional{[](double p, double q) { return p * q; }, price, qty};
Computed<std::string> label{
    [](double n) { return "Total: $" + std::to_string(n); }, notional};
// label updates whenever price or qty change
```

Heterogeneous sources are fine — the compute function's parameter list just has
to match the sources' value types in order:

```cpp
Observable<int>         shares{100};
Observable<double>      px{42.5};
Observable<std::string> sym{"ACME"};
Computed<std::string> line{
    [](int n, double p, const std::string& s) {
        return s + ": " + std::to_string(n) + " @ " + std::to_string(p);
    },
    shares, px, sym};
```

### Lifetime safety: per-source value caching

A `Computed` does **not** read its sources through stored references. At
construction it snapshots each source's current value into an internal cache; on
each source change, that source's own subscription writes the new value into its
cache slot and triggers a recompute that reads only from the cache.

The consequence: a source may be destroyed **before** the `Computed` without
dangling. The destroyed source simply stops contributing updates, and its last
cached value is retained in the derivation. (Conversely, when the `Computed` is
destroyed, its source subscriptions are torn down first, so no source can fire a
recompute into a half-destroyed `Computed`.)

### `Computed` is neither copyable nor movable

```cpp
Computed(const Computed&)            = delete;
Computed& operator=(const Computed&) = delete;
Computed(Computed&&)                 = delete;
Computed& operator=(Computed&&)      = delete;
```

A `Computed` is **address-sensitive**: its source subscriptions capture `this`.
It is therefore neither copyable nor movable — hold it in stable storage (a class
member, or on the heap). This is stricter than `Observable<T>`, which is movable.

### Consistency: eventual consistency and the diamond glitch

Propagation is **push-based and eventually consistent — not glitch-free.**

In a *diamond* dependency graph — where one node depends on a source both
directly and through an intermediate `Computed` — a recompute may briefly read a
stale intermediate value, and the node may notify its subscribers **more than
once** per upstream change before the graph settles on the final value.

```cpp
Observable<int> a{1};
Computed<int> doubled{[](int x) { return x * 2; }, a};
// DIAMOND: depends on `a` directly AND via `doubled`
Computed<int> bad{[](int x, int d) { return x + d; }, a, doubled};
a = 5;   // `bad` may fire twice and briefly read an inconsistent (a, doubled) pair
```

**Guidance: derive from leaves.** For glitch-free results, derive the final value
in a *single* `Computed` that reads only `Observable` leaves, rather than chaining
`Computed`s into a diamond:

```cpp
Observable<int> a{1};
// One Computed reading only the leaf `a`: computes a + 2*a in one shot.
Computed<int> good{[](int x) { return x + x * 2; }, a};
a = 5;   // recomputes exactly once to 15, no intermediate glitch
```

Linear chains (A → B → C, no shared upstream) are fine; only multi-path
(diamond) graphs exhibit the transient inconsistency.

---

## `Bind(source, sink)`

`Bind` is a convenience over `SubscribeAndFire`: it invokes `sink` immediately
with the current value and on every subsequent change, returning the owning
`Subscription`. There are overloads for both `Observable` and `Computed`:

```cpp
template <typename T, typename Sink>
[[nodiscard]] Subscription Bind(Observable<T>& source, Sink&& sink);

template <typename T, typename Sink>
[[nodiscard]] Subscription Bind(Computed<T>& source, Sink&& sink);
```

Keep the returned `Subscription` alive for as long as the binding should last.

```cpp
Observable<int> count{0};
std::string display;
Subscription b = Bind(count, [&](int n) { display = "n = " + std::to_string(n); });
// display is "n = 0" immediately; updates on every change to count

Computed<int> dbl{[](int n) { return n * 2; }, count};
Subscription b2 = Bind(dbl, [&](int n) { /* n = 2*count */ });
```

`Bind` is the generic, widget-agnostic way to drive *any* sink (a plain
variable, a logging call, a non-widget object) from a reactive value. The
widget-specific `BindValue`/`BindText` below are the same idea specialized for
retained widgets.

---

## DSL: `State<T>` and `Store<T>`

The DSL component framework adds two reactive cells, both built **on top of**
`Observable<T>`. They differ only in *who owns them* and *what a write does*.

### `dsl::State<T>` — component-local reactive cell

`State<T>` is a reactive cell owned by a `Component`. Reading returns the current
value; writing change-detects with `operator==` and, on a *real* change, marks
the owning component dirty so it re-`Build()`s on the next frame.

```cpp
template <typename T> class State {
public:
    State(Component* owner, T initial = T{});

    const T& operator()() const;   // read: state_()
    const T& Get() const;

    void Set(T v);                 // change-detected; marks owner dirty on change
    State& operator=(T v);
    template <typename Fn> void Mutate(Fn&& fn);  // edit in place, mark dirty

    Observable<T>&       AsObservable();
    const Observable<T>& AsObservable() const;
};
```

`State` is constructed with a back-pointer to its owning component (the `this` you
pass in the member initializer). It is non-copyable. Reading is most ergonomic
through the call operator: `count_()`. Because it exposes `AsObservable()`, a
`State` can feed a `Computed`, a `Bind`, or another component's `Watch`.

```cpp
#include <unigui/dsl/component.h>
using namespace unigui;

class Counter : public dsl::Component {
    dsl::State<int> count_{this, 0};
public:
    dsl::NodePtr Build() override {
        return dsl::VBox({
            dsl::Text("Count: " + std::to_string(count_())),
            dsl::Button("Increment", [this] { count_ = count_() + 1; }),
        });
    }
};
```

Each `count_ = …` write that actually changes the value marks the `Counter`
dirty; the framework re-`Build()`s the tree on the next `Render()`. Writes that
leave the value unchanged cost nothing — no rebuild.

### `dsl::Store<T>` — shared, app-wide reactive state

`Store<T>` is shared state that lives **outside** the component tree — a global, a
member of an app object, or an injected dependency. Components subscribe to it
with `Component::Watch(store)` and read it with `store()` / `store.Get()`.

```cpp
template <typename T> class Store {
public:
    explicit Store(T initial = T{});

    const T& Get() const;
    const T& operator()() const;

    void Set(T v);                                  // change-detected
    template <typename Fn> void Update(Fn&& fn);    // mutate in place, notify

    Observable<T>&       AsObservable();
    const Observable<T>& AsObservable() const;
};
```

Note the naming difference from `State`: `Store` uses **`Update`** for in-place
mutation (where `State`/`Observable` use `Mutate`). Both ultimately call
`Observable::Mutate`.

A `Store` does not, by itself, know about any component — it is just an
`Observable<T>` with store ergonomics. Components opt in to reacting via
`Watch`:

```cpp
#include <unigui/dsl/app.h>
using namespace unigui;

dsl::Store<int> counter{0};

class Screen : public dsl::Component {
public:
    void OnMount() override { Watch(counter); }   // re-render on change
    dsl::NodePtr Build() override {
        return dsl::Text("counter = " + std::to_string(counter()));
    }
};
```

### How `Watch` ties shared state to a view

`Component::Watch` is the bridge from app-wide state to a component's view:

```cpp
template <typename W> void Watch(W& source) {
    watches_.push_back(source.AsObservable().Subscribe([this](const auto&) { MarkDirty(); }));
}
```

`source` is anything with `AsObservable()` — a `Store`, another component's
`State`, a `Computed`, or a raw `Observable`. The subscription is stored in the
component and lives for the component's lifetime. Establish watches in
`OnMount()` (or the constructor). Each change to the watched source marks the
component dirty, triggering a rebuild on the next frame.

> `State` writes mark their owner dirty *directly* (the owner is known at
> construction), so a component does **not** need to `Watch` its own `State` —
> only *external/shared* reactive sources.

---

## Widget data binding

The retained widget layer wires `Observable<T>` straight into widgets. Two entry
points exist: a two-way binding on the generic `ValueWidget<T>` base, and a
one-way binding on `Label`.

### `ValueWidget<T>` two-way binding

`ValueWidget<T>` is the base for editable, value-carrying widgets. It binds a
widget's value to an `Observable<T>`:

```cpp
void BindValue(Observable<T>& src, bool twoWay = true);
void Unbind();
```

On bind, the widget **adopts the observable's current value** (the binding is set
up with `SubscribeAndFire`). Afterwards:

- **Observable → widget** (always): when the observable changes, the new value
  flows into the widget through the `ApplyBoundValue` hook.
- **Widget → observable** (only when `twoWay`, the default): a user edit writes
  the new value back to the observable via `Observable::Set` — change-detected,
  so it cannot feed back into an infinite loop.

The widget owns the subscription, so the binding auto-detaches when the widget is
destroyed (no dangling). Calling `BindValue` again rebinds to a new source.
`Unbind()` detaches an active binding.

```cpp
Observable<std::string> name{"Ada"};

InputText input{"name_input", "Name"};   // a ValueWidget<std::string>
input.BindValue(name);                    // two-way by default

// elsewhere: name = "Grace";  ->  input now shows "Grace"
// user types in input          ->  name updates to the typed text
```

For a read-only display that should reflect the observable but not write back,
pass `false`:

```cpp
input.BindValue(name, /*twoWay=*/false);  // observable drives the widget only
```

#### The `ApplyBoundValue` hook

When a bound observable pushes a new value into the widget, it arrives through a
virtual hook:

```cpp
protected:
    virtual void ApplyBoundValue(T v) { value_ = std::move(v); }
```

The default just stores the value. Widgets that cache **derived** state — for
example a text input keeping an editable character buffer alongside the
`std::string` value — override `ApplyBoundValue` to keep that derived state in
sync with the pushed value. Override this (not `SetValue`) when you need a
binding-aware update path.

The complementary half is `NotifyChange(T oldVal)`, which a widget's `Render`
calls after a user edit (passing the pre-edit value). It fires the `OnChange`
callback and, for a live two-way binding, pushes the new value back to the
source — but only after checking `boundLife_` is not `expired()`, so a source
that outlived its widget binding is never dereferenced through a stale pointer.
This is where `Observable::Lifetime()` earns its keep.

#### Address-sensitivity

`BindValue` registers a `this`-capturing observer inside the source observable.
A **bound** `ValueWidget` therefore must not be relocated — bind only after the
widget has reached its final home. *Unbound* widgets remain movable (copy is
ill-formed via the move-only `Subscription` member), so they can still live in
containers before you bind them.

### `Label::BindText` one-way binding

`Label` carries text, not an editable value, so it offers a **one-way** binding
to an `Observable<std::string>`:

```cpp
void BindText(Observable<std::string>& src);
```

The label adopts the observable's current text immediately (again via
`SubscribeAndFire`) and updates on every subsequent change. The label owns the
subscription, so it auto-detaches on destruction. The same address-sensitivity
rule applies: a bound `Label` must not be relocated; an unbound one stays
movable.

```cpp
Observable<std::string> status{"Ready"};

Label label{"status_label"};
label.BindText(status);          // shows "Ready" now, tracks changes

status = "Loading…";             // label text updates automatically
```

There is no write-back path — a `Label` never edits its source.

---

## Worked example: model → view

This example wires a plain data model into retained widgets through observables,
with no per-frame `Set*` plumbing. Editing the input updates the model; updating
the model anywhere updates both the input and the (derived) label.

```cpp
#include <unigui/core/observable.h>
#include <unigui/widgets/label.h>
#include <unigui/widgets/inputtext.h>   // InputText: a ValueWidget<std::string>
#include <unigui/app/app.h>
using namespace unigui;

int main() {
    // ── Model: reactive state held outside the widgets ──────────────────────
    Observable<std::string> username{"ada"};

    // A derived, read-only greeting computed from the model.
    Computed<std::string> greeting{
        [](const std::string& u) { return "Hello, " + u + "!"; },
        username};

    // ── View: retained widgets bound to the model ──────────────────────────
    InputText nameInput{"name", "Name"};
    nameInput.BindValue(username);          // two-way: edits write back to model

    Label greetingLabel{"greeting"};
    greetingLabel.BindText(greeting.AsObservable());  // one-way: tracks the derivation

    // ── Frame loop ──────────────────────────────────────────────────────────
    RunApp(AppConfig{}, [&] {
        nameInput.Render();
        greetingLabel.Render();
        // Type in the input  -> username updates -> greeting recomputes
        //                    -> greetingLabel updates. No manual Set* needed.
    });
}
```

Note `greeting.AsObservable()` — `BindText` wants an `Observable<std::string>&`,
and `Computed::AsObservable()` exposes the backing observable so a derived value
can be bound exactly like a stored one.

## Worked example: `Computed` derivation in a DSL app

This example combines `Store` (shared state), `State` (local state), `Computed`
(derivation), and `Component::Watch` (reacting to shared state).

```cpp
#include <unigui/dsl/app.h>
#include <unigui/app/app.h>
#include <string>
using namespace unigui;

// Shared, app-wide reactive values.
dsl::Store<double> price{100.0};
dsl::Store<int>    quantity{3};

class OrderPanel : public dsl::Component {
    // Derived total — recomputed whenever price or quantity change.
    // Held as a stable member because Computed is address-sensitive.
    Computed<double> total_{
        [](double p, int q) { return p * q; },
        price.AsObservable(), quantity.AsObservable()};
public:
    const char* InspectorName() const override { return "OrderPanel"; }

    void OnMount() override {
        // Re-render this component whenever shared state changes.
        Watch(price);
        Watch(quantity);
    }

    dsl::NodePtr Build() override {
        return dsl::VBox({
            dsl::Text("Price:    " + std::to_string(price())),
            dsl::Text("Quantity: " + std::to_string(quantity())),
            dsl::Text("Total:    " + std::to_string(total_.Get())),
            dsl::Button("Add one", [] { quantity.Set(quantity() + 1); }),
        });
    }
};

int main() {
    OrderPanel panel;                    // stable storage (Component is non-movable)
    return RunApp(AppConfig{}, [&] { panel.Render(); });
}
```

Pressing "Add one" calls `quantity.Set(...)`. Two things happen, both automatic:
the `Computed total_` recomputes (its quantity cache slot updates), and the
`Watch(quantity)` subscription marks `OrderPanel` dirty, so the next `Render()`
rebuilds the view and the new total appears.

> Because `total_` derives directly from the two leaf stores (not through an
> intermediate `Computed`), it is glitch-free — exactly the "derive from leaves"
> pattern recommended above.

---

## Trading models are reactive-ready

The trading value types in
[`<unigui/trading/quote.h>`](../include/unigui/trading/quote.h) —
`Quote`, `Position`, `Order`, and `Trade` — each declare defaulted value
equality:

```cpp
bool operator==(const Quote&)    const = default;
bool operator==(const Position&) const = default;
bool operator==(const Order&)    const = default;
bool operator==(const Trade&)    const = default;
```

Equality-comparability is precisely the requirement `Observable<T>` and
`Computed<T>` rely on for change detection. So these structs drop straight into
the reactive layer: an `Observable<Quote>` only notifies when a field actually
changes, and a `Computed` can derive metrics (mid, spread, P&L, fill ratio) from
them and recompute on each real update.

```cpp
#include <unigui/core/observable.h>
#include <unigui/trading/quote.h>
using namespace unigui;
using namespace unigui::trading;

Observable<Quote> acme{Quote{.symbol = "ACME", .bid = 99.0, .ask = 101.0}};

// Derive the mid-price; recomputes only when the quote really changes.
Computed<double> mid{[](const Quote& q) { return q.Mid(); }, acme};

acme.Set(Quote{.symbol = "ACME", .bid = 99.5, .ask = 101.0});  // changed -> mid -> 100.25
acme.Set(Quote{.symbol = "ACME", .bid = 99.5, .ask = 101.0});  // identical -> no notify
```

The derived getters on these types (`Quote::Mid`, `Quote::Spread`,
`Position::UnrealizedPnL`, `Order::FillRatio`, `Trade::Notional`, …) are pure
functions of the struct, so they make natural `Computed` bodies.

---

## Quick reference: choosing the right tool

- **A value that notifies on change** → `Observable<T>`.
- **A value derived from other reactive values** → `Computed<T>` (derive from
  leaves to stay glitch-free; hold it in stable storage).
- **Run a side effect now and on every change** → `Bind(source, sink)` (keep the
  `Subscription`).
- **Component-local UI state that triggers a rebuild** → `dsl::State<T>` (no
  `Watch` needed for your own state).
- **Shared, app-wide state** → `dsl::Store<T>` + `Component::Watch(store)`.
- **Drive an editable widget from state (and write edits back)** →
  `ValueWidget<T>::BindValue` (two-way by default).
- **Drive a read-only label from state** → `Label::BindText` (one-way).

Throughout, lifetime is handled by RAII: keep `Subscription`s (and the
`State`/`Store`/widget objects that own them) alive for as long as the binding
should last, and everything detaches itself safely on destruction — in either
order, source-first or subscriber-first.
