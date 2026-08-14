# Declarative DSL (`unigui::dsl`)

The **DSL** is UniGUI's view language. Instead of calling immediate-mode widget
functions one at a time, you *describe* a UI as a tree of value-type builder
calls and hand the tree to `Render()` once per frame. It is the most concise of
the toolkit's ease-of-use layers, and it renders through the themed
immediate-mode layer (`unigui::im`), so DSL output matches the rest of UniGUI's
look and feel.

```cpp
#include <unigui/dsl/dsl.h>
using namespace unigui::dsl;

auto ui = Window("Demo", VBox({
    Text("Welcome!"),
    Separator(),
    HBox({ Button("Save", ButtonVariant::Primary, []{ save(); }),
           Button("Exit", []{ std::exit(0); }) }),
    CheckBox("Enabled", &enabled),
    SliderFloat("Gain", &gain, 0.f, 1.f),
    For(3, [](int i){ return Label("Row " + std::to_string(i)); }),
}));

// each frame:
Render(ui);
```

> **Header:** `include/unigui/dsl/dsl.h` · **Namespace:** `unigui::dsl`
>
> This document covers the *view language* — building and rendering a node
> tree. For the reactive application framework built on top of it (`Component`,
> `State`, `Store`, `Navigator`, `Host`, `DrawInspector`), see
> [FRAMEWORK.md](FRAMEWORK.md). This page references those types only where the
> DSL hands off to them.

---

## The model

A DSL tree is a `std::shared_ptr<Node>` (aliased `NodePtr`). Every builder
function below allocates one `Node` and returns a `NodePtr`:

```cpp
using NodePtr = std::shared_ptr<Node>;
```

The whole API has three moving parts:

1. **Build** — call builder functions to assemble a `NodePtr` tree. Builders are
   plain value constructors: they allocate a node, set its fields, and (for
   containers) take ownership of their children. No drawing happens yet.
2. **Render** — call `Render(tree)` once per frame. It walks the tree and emits
   the corresponding `unigui::im` immediate-mode calls in order.
3. **Persist** — stateful nodes (`CheckBox`, `SliderFloat`, `InputText`) keep
   their value *inside the node* unless you bind them to an external variable. As
   long as you re-`Render()` **the same tree object** every frame, user input is
   preserved across frames.

```cpp
// Build ONCE, store the tree in stable storage:
NodePtr ui = Window("Settings", VBox({
    CheckBox("Wireframe"),          // node-held state — persists
    SliderFloat("Opacity", 0, 1),   // node-held state — persists
}));

// Render EVERY frame with the same NodePtr:
unigui::Run([&]{ Render(ui); });
```

If you rebuild the tree from scratch every frame, any node-held state is
discarded each frame (a freshly built `CheckBox()` starts back at `false`). Two
ways to keep state across frames:

- **Build once, render the same `NodePtr` repeatedly** (as above), or
- **Bind controls to external variables** so the storage lives outside the tree
  and rebuilding is harmless.

For an app whose view depends on changing state and is rebuilt when that state
changes, use a `Component` (see [FRAMEWORK.md](FRAMEWORK.md)) — its `Build()`
returns a DSL tree and the framework caches it, rebuilding only on a real state
change.

### `Node`

Every builder yields a `Node` whose `Kind` selects how `Render()` draws it. The
struct is public, so you *can* construct or mutate nodes by hand, but the builder
functions are the supported surface. Relevant fields per kind are documented in
the [builder reference](#builder-reference) below; the full set is:

```cpp
struct Node {
    enum class Kind { /* see table at the end of this doc */ };
    Kind kind = Kind::Label;

    std::vector<NodePtr> children;   // container children (If: [0]=then, [1]=else)
    std::string text;                // text-bearing nodes
    std::string label;               // control label
    std::string title;               // Window title

    ButtonVariant buttonVariant = ButtonVariant::Default;
    std::function<void()> onClick;

    bool* boolBinding = nullptr;     // CheckBox: bound var, else boolState
    bool  boolState   = false;
    std::function<void(bool)> onToggle;

    float* floatBinding = nullptr;   // SliderFloat: bound var, else floatState
    float  floatState   = 0.0f;
    float  minValue = 0.0f;
    float  maxValue = 1.0f;
    std::function<void(float)> onChangeFloat;

    std::string* strBinding = nullptr; // InputText: bound var, else strState
    std::string  strState;
    std::function<void(const std::string&)> onChangeText;

    std::function<bool()> condition;   // If / IfElse
    int count = 0;                     // For
    std::function<NodePtr(int)> itemBuilder;

    std::vector<float> flexGrow;       // Flex per-child weights
    float flexGap = 0.0f;
    FlexJustify flexJustify = FlexJustify::Start;

    std::function<void()> customDraw;  // Custom escape hatch
};
```

### Type aliases

The DSL re-exports two enums from neighbouring modules so you don't have to
include them yourself:

```cpp
using ButtonVariant = im::ButtonVariant;       // from <unigui/im/im.h>
using FlexJustify   = layout::FlexJustify;      // from <unigui/core/flex_layout.h>
```

`ButtonVariant` values: `Default`, `Primary`, `Danger`, `Success`, `Warning`.

`FlexJustify` values: `Start`, `End`, `Center`, `SpaceBetween`, `SpaceAround`,
`SpaceEvenly`.

---

## Builder summary

| Builder | Returns | Purpose |
|---|---|---|
| `Window(title, child)` / `Window(title, children)` | `NodePtr` | Top-level themed window |
| `VBox(children)` | `NodePtr` | Vertical stack (default ImGui flow) |
| `HBox(children)` | `NodePtr` | Horizontal row (`SameLine` between children) |
| `Flex(children, gap, justify)` / `Flex(children, weights, gap, justify)` | `NodePtr` | CSS-flexbox row |
| `Label(text)` | `NodePtr` | Plain text |
| `Text(text)` | `NodePtr` | Plain text (alias of `Label`'s render path) |
| `TextWrapped(text)` | `NodePtr` | Word-wrapped text |
| `TextDisabled(text)` | `NodePtr` | Dimmed text |
| `BulletText(text)` | `NodePtr` | Bullet-prefixed text |
| `Button(label, onClick)` / `Button(label, variant, onClick)` | `NodePtr` | Themed button |
| `CheckBox(label, onChange)` / `CheckBox(label, bound, onChange)` | `NodePtr` | Toggle (self-state or bound) |
| `SliderFloat(label, min, max, onChange)` / `SliderFloat(label, bound, min, max, onChange)` | `NodePtr` | Float slider (self-state or bound) |
| `InputText(label, onChange)` / `InputText(label, bound, onChange)` | `NodePtr` | Single-line text input (self-state or bound) |
| `Separator()` | `NodePtr` | Horizontal rule |
| `Spacing()` | `NodePtr` | Vertical gap |
| `If(condition, thenNode)` | `NodePtr` | Conditional subtree |
| `IfElse(condition, thenNode, elseNode)` | `NodePtr` | Conditional with else branch |
| `For(count, builder)` | `NodePtr` | Repeat `builder(i)` for `i ∈ [0, count)` |
| `Custom(draw)` | `NodePtr` | Escape hatch: raw immediate-mode drawing |
| `Render(root)` | `void` | Render a tree (call once per frame) |

---

## Builder reference

All signatures below are quoted verbatim from `include/unigui/dsl/dsl.h`.

### Containers

#### `Window`

```cpp
NodePtr Window(std::string title, NodePtr child);
NodePtr Window(std::string title, std::vector<NodePtr> children);
```

A top-level themed window identified by `title`. The single-child overload is
sugar for one root subtree (usually a `VBox`); the multi-child overload wraps the
children directly. Children render inside the window's content region in order.

```cpp
auto a = Window("Single", VBox({ Text("one"), Text("two") }));
auto b = Window("Multi",  { Text("one"), Text("two") });   // children overload
```

#### `VBox`

```cpp
NodePtr VBox(std::vector<NodePtr> children);
```

Vertical stack. Children render top-to-bottom in ImGui's default flow direction
(each subsequent widget starts on a new line).

#### `HBox`

```cpp
NodePtr HBox(std::vector<NodePtr> children);
```

Horizontal row. Children are placed left-to-right on a single line (the renderer
keeps them on the same line between siblings).

```cpp
HBox({
    Button("OK", ButtonVariant::Primary, []{ accept(); }),
    Button("Cancel", []{ cancel(); }),
});
```

#### `Flex`

```cpp
NodePtr Flex(std::vector<NodePtr> children, float gap = 0.0f,
             FlexJustify justify = FlexJustify::Start);
NodePtr Flex(std::vector<NodePtr> children, std::vector<float> weights, float gap = 0.0f,
             FlexJustify justify = FlexJustify::Start);
```

A horizontal flexbox row, rendered through `unigui::Layout::FlexRow` at a fixed
one-line height (`ImGui::GetFrameHeightWithSpacing()`). Children share the
available width by their flex-grow weight; with the default weight (`1`) every
child gets an equal slice.

- `gap` — pixel gap between adjacent children.
- `justify` — how leftover main-axis space is distributed
  (`FlexJustify::Start`, `End`, `Center`, `SpaceBetween`, `SpaceAround`,
  `SpaceEvenly`).
- `weights` (second overload) — per-child flex-grow weights, parallel to
  `children`. A shorter or empty vector falls back to weight `1` for the missing
  entries.

```cpp
// Equal thirds:
Flex({ Button("A"), Button("B"), Button("C") }, /*gap=*/8.0f);

// "B" twice as wide as its neighbours:
Flex({ Button("A"), Button("B"), Button("C") },
     /*weights=*/{1.0f, 2.0f, 1.0f}, /*gap=*/8.0f);

// Pack to the right edge:
Flex({ Button("Save"), Button("Close") }, /*gap=*/8.0f, FlexJustify::End);
```

> **Limitations (v1):** single line only (no wrap), one fixed row height, and
> cross-axis alignment is not exposed. For multi-line or aligned layouts, drop to
> `unigui::Layout` via a `Custom` node.

#### `Custom`

```cpp
NodePtr Custom(std::function<void()> draw);
```

The escape hatch. `draw` runs verbatim each frame at the current cursor
position, so any immediate-mode (`unigui::im`) drawing — or a hosted
`Component` — can live inside a DSL tree. This is how the DSL composes with the
rest of UniGUI when a builder for what you need doesn't exist.

```cpp
Custom([]{
    unigui::im::Text("Drawn with the immediate layer");
    if (unigui::im::Button("Raw button")) { /* ... */ }
});
```

`Custom` is also the bridge to the component framework — `dsl::Host(child)` is
implemented as `Custom([&child]{ child.Render(); })`. See
[FRAMEWORK.md](FRAMEWORK.md).

### Text

```cpp
NodePtr Label(std::string text);
NodePtr Text(std::string text);
NodePtr TextWrapped(std::string text);
NodePtr TextDisabled(std::string text);
NodePtr BulletText(std::string text);
```

| Builder | Effect |
|---|---|
| `Label` / `Text` | Plain themed text. |
| `TextWrapped` | Word-wraps to the available content width. |
| `TextDisabled` | Dimmed (disabled) text colour. |
| `BulletText` | Text prefixed with a bullet glyph. |

```cpp
VBox({
    Text("Title"),
    TextDisabled("subtitle"),
    TextWrapped("A longer paragraph that wraps to the window width..."),
    BulletText("first point"),
    BulletText("second point"),
});
```

### Buttons

```cpp
NodePtr Button(std::string label, std::function<void()> onClick = nullptr);
NodePtr Button(std::string label, ButtonVariant variant,
               std::function<void()> onClick = nullptr);
```

A themed button. The optional `onClick` callback fires on the frame the button is
clicked. The second overload selects a colour `variant`
(`Default`, `Primary`, `Danger`, `Success`, `Warning`); the first uses
`ButtonVariant::Default`.

```cpp
Button("Apply", []{ apply(); });
Button("Delete", ButtonVariant::Danger, []{ erase(); });
Button("No-op");   // a button with no handler is valid
```

### Stateful inputs

Each stateful control comes in two flavours:

- **Self-state** — the value lives in the `Node`. It persists as long as you
  re-render the same `NodePtr` each frame.
- **Bound** — the value lives in an external variable you pass by pointer. The
  caller owns the storage; the node reads/writes through the pointer, so the
  value persists regardless of whether the tree is rebuilt. The binding takes
  precedence (the node uses the pointer when it is non-null, otherwise the
  in-node field).

All variants accept an optional `onChange` callback invoked with the new value
when the user edits the control.

#### `CheckBox`

```cpp
NodePtr CheckBox(std::string label, std::function<void(bool)> onChange = nullptr);
NodePtr CheckBox(std::string label, bool* bound,
                 std::function<void(bool)> onChange = nullptr);
```

```cpp
bool darkMode = true;

CheckBox("Show grid");                          // self-state
CheckBox("Dark mode", &darkMode);               // bound to external bool
CheckBox("Notify", [](bool on){ setNotify(on); }); // self-state + callback
```

#### `SliderFloat`

```cpp
NodePtr SliderFloat(std::string label, float min, float max,
                    std::function<void(float)> onChange = nullptr);
NodePtr SliderFloat(std::string label, float* bound, float min, float max,
                    std::function<void(float)> onChange = nullptr);
```

```cpp
float gain = 0.5f;

SliderFloat("Volume", 0.0f, 1.0f);                       // self-state
SliderFloat("Gain", &gain, 0.0f, 2.0f);                  // bound
SliderFloat("Speed", 0.0f, 10.0f, [](float v){ set(v); }); // self-state + callback
```

#### `InputText`

```cpp
NodePtr InputText(std::string label,
                  std::function<void(const std::string&)> onChange = nullptr);
NodePtr InputText(std::string label, std::string* bound,
                  std::function<void(const std::string&)> onChange = nullptr);
```

```cpp
std::string name;

InputText("Search");                                       // self-state
InputText("Name", &name);                                  // bound
InputText("Tag", [](const std::string& s){ apply(s); });   // self-state + callback
```

### Spacers

```cpp
NodePtr Separator();
NodePtr Spacing();
```

`Separator()` draws a horizontal rule; `Spacing()` inserts a vertical gap. Both
take no arguments and carry no state.

### Control flow

```cpp
NodePtr If(std::function<bool()> condition, NodePtr thenNode);
NodePtr IfElse(std::function<bool()> condition, NodePtr thenNode, NodePtr elseNode);
NodePtr For(int count, std::function<NodePtr(int)> builder);
```

#### `If` / `IfElse`

The `condition` predicate is evaluated **every frame** at render time. `If`
renders `thenNode` only when the predicate returns `true`; `IfElse` renders
`thenNode` when `true` and `elseNode` otherwise. Because the condition is a
callback, you can capture live state and let the visible subtree change frame to
frame without rebuilding the tree.

```cpp
bool loggedIn = false;

If([&]{ return loggedIn; },
   Text("Welcome back!"));

IfElse([&]{ return loggedIn; },
       /*then=*/ Button("Log out", [&]{ loggedIn = false; }),
       /*else=*/ Button("Log in",  [&]{ loggedIn = true; }));
```

> Internally, `If`/`IfElse` store the branches as `children` (`children[0]` =
> then, `children[1]` = else) and the predicate in `condition`. Both branch
> subtrees are built once; the condition only decides which one renders.

#### `For`

```cpp
NodePtr For(int count, std::function<NodePtr(int)> builder);
```

Calls `builder(i)` for `i ∈ [0, count)` and renders each returned subtree in
order. The builder is invoked at render time, so the list can reflect current
data.

```cpp
std::vector<std::string> items = {"alpha", "beta", "gamma"};

For(static_cast<int>(items.size()), [&](int i){
    return BulletText(items[i]);
});
```

### Render

```cpp
void Render(NodePtr root);
```

Walks `root` and emits the corresponding immediate-mode calls. Call exactly once
per frame, inside your UniGUI frame loop. A null `root` is a no-op.

```cpp
#include <unigui/app/app.h>
#include <unigui/dsl/dsl.h>
using namespace unigui::dsl;

int main() {
    unigui::AppConfig cfg; cfg.title = "DSL demo";
    if (!unigui::Init(cfg)) return 1;

    NodePtr ui = Window("Hello", VBox({ Text("Built once, rendered each frame.") }));

    unigui::Run([&]{ Render(ui); });   // Run() calls Shutdown() when the loop ends
    return 0;
}
```

### ToSource

```cpp
std::string ToSource(const NodePtr& root);
```

The designer-tool "emit code" half: serialises a tree back into the equivalent
builder expression — a complete, compilable snippet:

```cpp
using namespace unigui::dsl;

NodePtr ui =
    Window("Demo", VBox({
        Text("Welcome!"),
        ...
    }));
```

What round-trips exactly: structure, labels/text, numeric parameters (slider
min/max, flex gap/weights/justify) and button variants. What cannot:
callbacks and conditions — a `std::function` cannot be recovered, so
`onClick`/`onToggle`/`onChange`, `If` conditions, `For` item builders and
`Custom` bodies are emitted as *compilable* placeholder forms
(`[] { return true; } /* condition */`, a `Label(std::to_string(i))` item
builder, a `/* draw lambda */` body), and external bindings become a trailing
`// bound to an external ...` note. A null root yields an empty string.

See the `designer` example (examples/designer) for the live preview +
code-emission workflow around it.

### Scene text format (`ParseScene`)

```cpp
#include <unigui/dsl/dsl_scene.h>
SceneParseResult ParseScene(std::string_view text);
```

The designer tool's in-app scene-editing format: an indentation-based text
description of a DSL tree, parseable into a renderable `NodePtr` (result:
`tree` on success, a line-numbered `error` string on failure — never throws).
Lines indent with spaces (any consistent step; each line becomes a child of
the nearest preceding line with a smaller indent), `#` starts a comment, and
the root must be a `window`:

```text
# demo scene
window "Settings"
  vbox
    text "Welcome"
    separator
    hbox
      checkbox "Wireframe"
      button "Save" primary
    slider_float "Gain" 0 1.5
    for 3
      label "item"
```

| Keyword | Meaning |
|---------|---------|
| `window "title"` | The root (mandatory, exactly once) |
| `vbox` / `hbox` / `flex` | Containers |
| `label` / `text` / `text_wrapped` / `text_disabled` / `bullet_text "text"` | Text nodes |
| `button "label" [default\|primary\|danger\|success\|warning]` | Button (variant optional) |
| `checkbox "label"` | Node-held checkbox |
| `slider_float "label" min max` | Node-held slider |
| `input_text "label"` | Node-held text input |
| `separator` / `spacing` | Spacers |
| `for count` | Repeats its (indented) template `count` times; the template is cloned per iteration so stateful template children get fresh instances |

Callbacks and conditions cannot be expressed in text: `if`/`if_else`/`custom`
are rejected with a clear error instead of silently degrading. Strings use
`\"`, `\\`, `\n`, `\t` escapes.

---

## Binding: pointer-bound vs node-held state

The single most important DSL concept is *where a control's value lives*. It
determines whether you can rebuild the tree freely and whether you can read the
value from elsewhere in your program.

| | Node-held (self-state) | Pointer-bound |
|---|---|---|
| **Where the value lives** | inside the `Node` (`boolState` / `floatState` / `strState`) | in a variable you own; the node holds a pointer (`boolBinding` / `floatBinding` / `strBinding`) |
| **Persists across frames** | only if you re-render the *same* `NodePtr` | always (storage is external) |
| **Survives rebuilding the tree** | no — a fresh node starts from its default | yes |
| **Readable elsewhere in code** | only via the callback | directly from your variable |
| **Builder form** | `CheckBox("x")` | `CheckBox("x", &flag)` |

```cpp
// Node-held: read value only through the callback; persists if `ui` is reused.
auto a = CheckBox("Wireframe", [](bool on){ renderer.wireframe = on; });

// Pointer-bound: `flag` is the source of truth and you can read it anywhere.
bool flag = false;
auto b = CheckBox("Wireframe", &flag);
// ... later, anywhere:
if (flag) drawWireframe();
```

When the node holds a non-null binding pointer it always uses that pointer; the
in-node field is ignored. Combine binding with a callback to mirror a variable
*and* react to edits:

```cpp
float exposure = 1.0f;
SliderFloat("Exposure", &exposure, 0.0f, 4.0f,
            [](float v){ recomputeHistogram(v); });
```

For app-wide shared state and automatic rebuilds, prefer a `Component` with
`State<T>` / `Store<T>` over hand-managed bindings — see
[FRAMEWORK.md](FRAMEWORK.md).

---

## Worked example: a small form

A settings form combining containers, text, bound and self-state inputs,
control flow, and a button row. The tree is built once and stored; bound
variables hold the live values.

```cpp
#include <unigui/app/app.h>
#include <unigui/dsl/dsl.h>

#include <string>

using namespace unigui::dsl;

struct Settings {
    std::string username = "guest";
    bool        notifications = true;
    bool        advanced = false;
    float       volume = 0.7f;
};

NodePtr BuildForm(Settings& s, bool& saved) {
    return Window("Preferences", VBox({
        Text("Account"),
        Separator(),
        InputText("Username", &s.username),
        CheckBox("Email notifications", &s.notifications),

        Spacing(),
        Text("Audio"),
        Separator(),
        SliderFloat("Volume", &s.volume, 0.0f, 1.0f),

        Spacing(),
        CheckBox("Show advanced options", &s.advanced),

        // Conditional subtree — evaluated every frame.
        If([&]{ return s.advanced; },
           VBox({
               BulletText("Experimental rendering"),
               BulletText("Verbose logging"),
           })),

        Spacing(),
        Separator(),

        // Action row: equal-width buttons with an 8px gap.
        Flex({
            Button("Save", ButtonVariant::Primary, [&]{ saved = true; }),
            Button("Reset", ButtonVariant::Danger,  [&]{ s = Settings{}; }),
            Button("Close", [&]{ std::exit(0); }),
        }, /*gap=*/8.0f),

        // Feedback line that appears after a save.
        If([&]{ return saved; },
           TextDisabled("Saved.")),
    }));
}

int main() {
    unigui::AppConfig cfg; cfg.title = "Form demo";
    if (!unigui::Init(cfg)) return 1;

    Settings settings;
    bool saved = false;
    NodePtr form = BuildForm(settings, saved);   // build ONCE

    unigui::Run([&]{ Render(form); });           // render each frame
    return 0;
}
```

Notes on the example:

- The whole tree is built once before the loop; the inputs are pointer-bound to
  `settings`, so the form is the single source of truth and could even be rebuilt
  per frame without losing values.
- `If(...)` predicates capture `settings`/`saved` by reference, so the advanced
  block and the "Saved." line appear/disappear as those flags change — no tree
  rebuild required.
- `Flex(..., 8.0f)` lays the three actions out as equal-width columns with an
  8-pixel gap.

---

## Relationship to the component framework

The DSL is the *view* half of UniGUI's framework. The reactive half lives in
`unigui/dsl/component.h` and `unigui/dsl/app.h` and is documented in
[FRAMEWORK.md](FRAMEWORK.md):

- **`Component`** — override `Build()` to return a DSL `NodePtr` tree; the
  framework caches it and rebuilds only when state changes.
- **`State<T>`** — component-local reactive state that marks the component dirty
  on change.
- **`Store<T>`** — shared, app-wide reactive state; components subscribe with
  `Component::Watch(store)`.
- **`Navigator`** — a push/pop/replace stack of screen `Component`s.
- **`Host(child)`** — embed a child `Component` inside a parent's DSL tree; it is
  literally a `Custom` node that calls `child.Render()`.

The hand-off in both directions is through the DSL: a `Component::Build()`
*returns* a DSL tree, and a `Custom`/`Host` node lets a DSL tree *contain* a
component or raw immediate-mode drawing.

---

## `Node::Kind` reference

Every builder produces exactly one of these kinds. `Render()` dispatches on
`kind`.

| `Node::Kind` | Produced by | Notes |
|---|---|---|
| `Window` | `Window(...)` | Top-level themed window (`title`, `children`). |
| `VBox` | `VBox(...)` | Vertical stack of `children`. |
| `HBox` | `HBox(...)` | Horizontal row of `children`. |
| `Flex` | `Flex(...)` | Flexbox row (`children`, `flexGrow`, `flexGap`, `flexJustify`). |
| `Button` | `Button(...)` | `label`, `buttonVariant`, `onClick`. |
| `Label` | `Label(...)` | Plain text (`text`). |
| `Text` | `Text(...)` | Plain text (`text`). |
| `TextWrapped` | `TextWrapped(...)` | Word-wrapped text. |
| `TextDisabled` | `TextDisabled(...)` | Dimmed text. |
| `BulletText` | `BulletText(...)` | Bulleted text. |
| `CheckBox` | `CheckBox(...)` | `boolBinding` or `boolState`, `onToggle`. |
| `SliderFloat` | `SliderFloat(...)` | `floatBinding`/`floatState`, `minValue`, `maxValue`, `onChangeFloat`. |
| `InputText` | `InputText(...)` | `strBinding` or `strState`, `onChangeText`. |
| `Separator` | `Separator()` | Horizontal rule. |
| `Spacing` | `Spacing()` | Vertical gap. |
| `If` | `If(...)` / `IfElse(...)` | `condition`; `children[0]`=then, `children[1]`=else. |
| `For` | `For(...)` | `count`, `itemBuilder`. |
| `Custom` | `Custom(...)` / `Host(...)` | `customDraw` runs verbatim each frame. |

> There is no distinct `IfElse` kind — `If` and `IfElse` both produce
> `Kind::If`; the presence of a second child (`children[1]`) is what gives
> `IfElse` its else branch.
