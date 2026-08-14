# UniGUI Architecture

A big-picture tour of how **TeamkillerUniGUI** (v4.9.0) is put together — the
layers that sit between your application code and [Dear ImGui](https://github.com/ocornut/imgui),
and how to choose the right one for any given piece of UI.

UniGUI is a C++23 framework built *on top of* Dear ImGui. ImGui handles the
low-level draw-list generation and per-frame input; UniGUI layers a portable
backend abstraction, a theme engine, four cooperating UI layers (retained
widgets, an immediate-mode helper layer, a declarative DSL, and a reactive
component framework), several cross-cutting capability layers (reactive state,
flexbox layout, CSS styling, an event bus), and a set of optional modules
(SQLite, config, IPC, networking, trading, plugins) gated by CMake.

Everything is reachable through a single umbrella header:

```cpp
#include <unigui/unigui.h>
```

That header pulls in the backend factory, core utilities, the theme engine, all
~95 retained widgets, the immediate-mode layer (`unigui::im`), the DSL +
component framework (`unigui::dsl`), the EventBus, styling, plugins, and — guarded
by `UNIGUI_HAS_*` macros — whichever optional modules were compiled in.

---

## The layer cake

UniGUI is strictly layered: each layer is built from the ones beneath it and
never reaches around them. From the bottom up:

```
┌───────────────────────────────────────────────────────────────────────┐
│  YOUR APPLICATION                                                       │
├───────────────────────────────────────────────────────────────────────┤
│  UI LAYERS  (pick per use-case — they coexist in the same frame)        │
│                                                                         │
│   Component framework   ── unigui::dsl::Component / Store / Navigator   │
│        builds ▲                                                         │
│   Declarative DSL       ── unigui::dsl::{Window,VBox,Button,...}        │
│        renders through ▼                                                │
│   Immediate layer       ── unigui::im::{Button,Checkbox,SliderFloat}    │
│   Retained widgets      ── unigui::{Button, Table, Form, TreeView,...}  │
├───────────────────────────────────────────────────────────────────────┤
│  CROSS-CUTTING CAPABILITY LAYERS                                        │
│   Reactive  unigui::{Observable,Computed,State,Store}                   │
│   Layout    unigui::layout (flex solver) + unigui::Layout::FlexRow      │
│   Styling   unigui::styling::Engine (CSS-like)                          │
│   Events    unigui::events::Bus (pub/sub)                               │
├───────────────────────────────────────────────────────────────────────┤
│  THEME ENGINE   unigui::ApplyTheme / theme::SurfaceStyle / ColorTokens  │
├───────────────────────────────────────────────────────────────────────┤
│  APP LOOP       unigui::{Init,NewFrame,Render,Run,RunApp}               │
├───────────────────────────────────────────────────────────────────────┤
│  BACKEND ABSTRACTION   PlatformBackend  ×  RendererBackend              │
│     GLFW / SDL3 / Emscripten   ×   GL3 / Vulkan / DX11 / DX12 / Metal   │
├───────────────────────────────────────────────────────────────────────┤
│  Dear ImGui                                                             │
└───────────────────────────────────────────────────────────────────────┘
```

The two "UI layers" callout — retained widgets and the immediate layer — are the
foundation; the DSL renders *through* the immediate layer, and the component
framework builds *on top of* the DSL. The reactive, layout, styling, and event
layers cut across all of them.

---

## 1. Backend abstraction (platforms × renderers)

The lowest UniGUI layer makes the rest of the toolkit independent of any one
windowing system or graphics API. It is two orthogonal abstract interfaces —
a **platform** (window + input) and a **renderer** (draws ImGui's draw lists) —
that are paired at runtime.

From `<unigui/backend/platform_backend.h>`:

```cpp
class PlatformBackend {
public:
    virtual bool Init(void* native_window_handle = nullptr) = 0;
    virtual void Shutdown() = 0;
    virtual void NewFrame() = 0;
    virtual void PollEvents() = 0;
    virtual bool ShouldClose() const = 0;
    virtual void SwapBuffers() {}
    // ... window-handle / size / Vulkan-surface hooks
};
```

From `<unigui/backend/renderer_backend.h>`:

```cpp
class RendererBackend {
public:
    virtual bool Init(ImGuiContext* context) = 0;
    virtual void Shutdown() = 0;
    virtual void RenderDrawData(ImDrawData* draw_data) = 0;
    virtual void SetClearColor(float r, float g, float b, float a) = 0;
};
```

You rarely instantiate these directly. A `BackendType` enum and a factory pick a
matching pair (`<unigui/backend/backend_types.h>`, `backend_factory.h`):

```cpp
enum class BackendType {
    GLFW_GL3,    // GLFW platform + OpenGL 3 renderer (default)
    SDL3_Vulkan, // SDL3 platform + shared Vulkan renderer (opt-in; needs SDL3)
    DX11,        // DirectX 11 renderer (Windows only)
    DX12,        // DirectX 12 renderer (Windows only)
    Metal,       // Metal renderer (macOS only)
    WebGPU,      // WebGPU renderer (cross-platform via Dawn/WGPU)
    Emscripten,  // Emscripten/Web platform
    Vulkan,      // GLFW platform + shared Vulkan renderer (cross-platform)
};

unigui::DefaultBackend bk = unigui::CreateBackend(unigui::BackendType::GLFW_GL3);
// bk.platform : std::unique_ptr<PlatformBackend>
// bk.renderer : std::unique_ptr<RendererBackend>
```

Each `case` in `CreateBackend()` is guarded by a feature macro
(`UNIGUI_HAS_DX11`, `UNIGUI_HAS_VULKAN`, `UNIGUI_HAS_SDL3`, `__APPLE__`, …); a
backend that wasn't compiled in returns `{nullptr, nullptr}`. The matrix is
*platforms × renderers*: GLFW or SDL3 (or Emscripten) provide the window/input;
GL3, Vulkan, DX11/DX12, Metal, or WebGPU draw. DX11/DX12 are Windows-only;
Metal/WebGPU/Emscripten renderers are currently **stubs** and should not be
assumed functional when wiring new backend-level features.

**You almost never touch this directly.** The app loop owns a backend pair and
drives it for you — see the next section. Selection is a one-liner via
`AppConfig::backend`.

> See **[docs/MODULES.md](MODULES.md)** for the `UNIGUI_BACKEND_*` CMake options
> and per-platform notes. (A dedicated `BACKENDS.md` is planned; the backend
> matrix currently lives in MODULES.md and the CLAUDE.md "things that trip people
> up" notes.)

### The app loop

`<unigui/app/app.h>` wraps the backend, theme, and ImGui context into a tiny
lifecycle. Most apps use `RunApp` and never see a backend object:

```cpp
#include <unigui/unigui.h>

int main() {
    unigui::AppConfig cfg;
    cfg.title = "My App";
    cfg.backend = unigui::BackendType::GLFW_GL3; // DX11 by default on Windows
    return unigui::RunApp(cfg, [] {
        unigui::im::Text("Hello, UniGUI!");
    });
}
```

`AppConfig` carries the window size, title, a `ThemeConfig`, the chosen
`BackendType`, and a `dpiScaleFonts` flag. The primitives behind `RunApp` —
`Init`, `NewFrame`, `Render`, `Run`, `Shutdown`, `ShouldClose` — are available
if you need to own the loop yourself. The `maxFrames` parameter (render *N*
frames and exit) makes every example headless-/CI-friendly.

> **Translucent surfaces depend on the loop's clear color.** The app loop clears
> every backend to `unigui::GetBackdropColor()` so Glass/Frosted/Acrylic
> materials read against the theme backdrop instead of black — see the theme
> engine below.

---

## 2. Theme engine

One call configures colors, geometry tokens, DPI scaling, fonts, and a surface
material for the whole UI. From `<unigui/theme/theme.h>`:

```cpp
enum class ThemePreset { Dark, Light };

struct ThemeConfig {
    ThemePreset preset = ThemePreset::Dark;
    float dpi_scale = 0.0f;            // 0 = auto-detect from system DPI
    float font_size = 16.0f;          // logical px at 96 DPI
    const char* font_path = nullptr;  // nullptr = auto-detect system CJK font
    bool emoji_fallback = true;
    theme::SurfaceStyle surface = theme::SurfaceStyle::Glass; // default material
};

void ApplyTheme(const ThemeConfig& config); // call after the ImGui context exists
```

The **palette** (Dark/Light) sets ImGui colors; **geometry tokens** unify
rounding/spacing/borders; a **surface material** then multiplies surface alphas
on top of the palette to produce a look. From `<unigui/theme/surface_style.h>`:

```cpp
enum class SurfaceStyle {
    Solid,   // flat, fully opaque
    Glass,   // frosted glass / glassmorphism (default) — translucent + bright rim
    Frosted, // heavier translucency, stronger rim
    Acrylic, // Fluent-style acrylic
    Minimal, // near-opaque, borderless, quiet
};
```

Because translucent materials reveal whatever is painted behind ImGui windows,
the backend must clear to a tinted backdrop, not black. The theme engine exposes
that color so the app loop can use it:

```cpp
ImVec4 GetBackdropColor(); // opaque clear color for the active theme + material
```

Widgets that need semantic colors read them from the active theme rather than
hard-coding, so they track the accent:

```cpp
const theme::ColorTokens& GetColorTokens();
ImVec4 GetSemanticColor(theme::Semantic role); // Success / Warning / Danger / Info
```

The engine also handles DPI (`DetectDPIScale`, deferred font-atlas rebuilds via
`HasPendingFontRebuild` / `ApplyPendingFontRebuild`), text wrapping
(`BeginTextWrap` / `EndTextWrap`), and JSON import/export of the current colors
(`ExportThemeJSON` / `ImportThemeJSON`). A registry of named presets lives under
`<unigui/theme/presets/registry.h>`.

> A dedicated `THEMING.md` walkthrough is planned; until then the headers above
> and `examples/theme_demo` are the reference.

---

## 3. The four UI layers

UniGUI deliberately offers four ways to express UI, stacked from lowest-level to
highest. They coexist in the same frame, so you can mix them freely.

### 3a. Retained-mode widgets — `unigui::`

The ~95 widget **classes** are the building blocks: persistent objects that hold
state, support validation, undo/redo, serialization, accessibility, shadows, and
elevation. Every widget derives from `Widget` (`<unigui/widgets/widget_base.h>`),
which provides the common surface — visibility, tooltips, focus, enabled state,
min/max size, accessibility, shadow/elevation — and **mandatory ID scoping**:
each widget is constructed with a unique `name` and scopes its ImGui IDs with
`PushID(name)/PopID()`.

Widgets are configured with a **fluent `With*` API**. The CRTP base
`FluentWidget<Derived>` preserves the derived type through a chain so
widget-specific setters can follow the base ones:

```cpp
unigui::Button btn{"save_btn", "Save"};      // (name, label)
btn.WithPrimary()
   .WithTooltip("Persist changes")
   .WithOnClick([] { save(); });

// each frame:
btn.Render();
```

```cpp
class Button : public FluentWidget<Button> {
public:
    enum ColorVariant { Default, Primary, Danger, Success, Muted };
    enum Size { Small, Medium, Large };
    Button& WithVariant(ColorVariant);  // chainable
    Button& WithOnClick(std::function<void()>);
    // ...
};
```

Retained widgets are the right tool whenever a control owns meaningful state:
tables with sorting/selection, forms with validation, tree views, editable data
grids, wizards, command palettes, and the trading widgets.

> Per-widget reference: **[docs/WIDGET_API.md](WIDGET_API.md)**; one minimal
> example each in **[docs/WIDGET_EXAMPLES.md](WIDGET_EXAMPLES.md)**.

### 3b. Immediate layer — `unigui::im`

A thin, stateless, allocation-light layer of **free functions** over ImGui for
the "just draw a control" case — no `shared_ptr`s, no unique names, no
`ImGui::` prefix. From `<unigui/im/im.h>`:

```cpp
namespace unigui::im {
enum class ButtonVariant { Default, Primary, Danger, Success, Warning };

bool Button(std::string_view label, const ImVec2& size = ImVec2(0, 0));
bool Button(std::string_view label, ButtonVariant variant, const ImVec2& size = ImVec2(0, 0));
void Text(std::string_view text);
bool Checkbox(std::string_view label, bool* value);
bool SliderFloat(std::string_view label, float* value, float min, float max,
                 std::string_view format = "%.3f");
bool InputText(std::string_view label, std::string* value, std::size_t maxLength = 256,
               ImGuiInputTextFlags flags = 0);
// + sliders/drags/inputs in 2/3/4 variants, vertical sliders, ...
}
```

Usage is a one-liner, themed to match the rest of the toolkit:

```cpp
if (unigui::im::Button("Save", unigui::im::ButtonVariant::Primary)) save();
unigui::im::Checkbox("Enabled", &enabled);
unigui::im::SliderFloat("Gain", &gain, 0.f, 1.f);
```

These live in `unigui::im` (not plain `unigui`) on purpose: the retained widgets
are same-named *classes* in `unigui`, so free functions would collide. The
immediate layer **complements** the retained widgets — it does not replace them.
Reach for `im` for one-off controls, custom drawing inside a bespoke window, and
the inner draw of a `dsl::Custom` node.

> A dedicated `IM_API.md` is planned; the full function list is the header
> `<unigui/im/im.h>` (~120 signatures).

### 3c. Declarative DSL — `unigui::dsl`

Describe a UI as a tree of value-type builder calls, then `Render()` the tree
each frame. The DSL is the most concise UniGUI layer and renders *through* the
immediate layer, so its output matches the toolkit's look. From
`<unigui/dsl/dsl.h>`:

```cpp
using namespace unigui::dsl;

NodePtr ui = Window("Demo", VBox({
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

A `Node` is a tagged struct (`Node::Kind` = `Window`, `VBox`, `HBox`, `Flex`,
`Button`, `Label`, `Text`, `CheckBox`, `SliderFloat`, `InputText`, `If`, `For`,
`Custom`, …). Stateful controls (`CheckBox` / `SliderFloat` / `InputText`)
either **bind** to an external variable via pointer or keep their state inside
the node, so re-`Render()`-ing the same tree preserves user input. Control flow
is data: `If` / `IfElse` / `For(count, builder)`. A horizontal `Flex` row is
rendered through `unigui::Layout::FlexRow` (see the layout layer).

The **escape hatch** is `Custom`:

```cpp
NodePtr Custom(std::function<void()> draw); // runs verbatim each frame
```

`Custom` lets *any* immediate-mode (`unigui::im`) drawing — or a hosted
Component — live inside a DSL tree. This is the seam through which you drop down
to raw ImGui when the declarative builders don't cover something.

### 3d. Component framework — `unigui::dsl::Component`

The application model on top of the DSL. A `Component` owns reactive `State`,
declares its view as a DSL tree in `Build()`, and the framework re-`Build()`s it
**only when state changes** (dirty tracking), caching and rendering the tree each
frame. This is what turns "widgets you call every frame" into "an app of
self-contained, stateful, composable units." From `<unigui/dsl/component.h>`:

```cpp
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

Counter app;                               // hold in stable storage
unigui::Run([&] { app.Render(); });        // each frame
```

`State<T>` reads with `state_()`, writes with `state_ = v` (change-detected;
marks the owner dirty only on a real change), and exposes `AsObservable()` so it
plugs into `Computed`/`Bind`. Components compose with `Host(child)` inside a
`Build()` tree; each child keeps its own state and dirty tracking. Lifecycle
hooks (`OnMount`, `OnUnmount`, `OnCleanup`), external-state reactions
(`Watch(source)`), and a live `DrawInspector()` overlay round it out.

The **application layer** (`<unigui/dsl/app.h>`) adds two pieces:

```cpp
template <typename T> class Store;   // shared, app-wide reactive state
class Navigator;                     // a push/pop/replace stack of screens
```

`Store<T>` is shared state that lives *outside* the component tree; components
subscribe with `Component::Watch(store)`. `Navigator` owns a stack of screen
Components, unmounting the outgoing one (running its cleanups + `OnUnmount`) on
pop/replace.

```cpp
dsl::Store<int> counter{0};
dsl::Navigator nav;
nav.Push(std::make_shared<HomeScreen>());
unigui::Run([&] { nav.Render(); });
```

> Full framework guide: **[docs/FRAMEWORK.md](FRAMEWORK.md)**. A dedicated
> `DSL.md` is planned; the builder catalogue is `<unigui/dsl/dsl.h>`.

---

## 4. Cross-cutting capability layers

These layers are orthogonal to the UI layers — any of them can use any of these.

### Reactive — `unigui::Observable` / `Computed` / `State` / `Store`

A tiny header-only reactive core (`<unigui/core/observable.h>`). `Observable<T>`
wraps a value and notifies subscribers when it changes; subscriptions are RAII
(`Subscription`) and survive the `Observable` being destroyed first (no
dangling).

```cpp
unigui::Observable<int> qty{1};
auto sub = qty.Subscribe([](const int& n){ /* react */ });
qty = 5;        // fires observers
qty.Set(5);     // no-op: unchanged (operator== compares)
```

`Computed<T>` is a read-only derived value that recomputes when any source
changes and composes (it can feed another `Computed` or `Bind`):

```cpp
unigui::Observable<int> a{2}, b{3};
unigui::Computed<int> sum{[](int x, int y){ return x + y; }, a, b};
sum.Get();                     // 5
auto s = sum.Subscribe([](int v){ /* ... */ });
a = 10;                        // sum recomputes to 13
```

`Bind(source, sink)` invokes `sink` immediately and on every change. This same
core powers the component framework's `State<T>` (component-local, dirty-marking)
and `Store<T>` (app-wide, shared) — all four expose `AsObservable()`, so generic
code (and `Component::Watch`) treats them uniformly.

> A dedicated `REACTIVE.md` is planned; the canonical reference is the
> well-documented header `<unigui/core/observable.h>`.

### Layout — `unigui::layout` (solver) + `unigui::Layout::FlexRow`

A genuine CSS-flexbox **main-axis solver**, kept pure (no ImGui, no allocation
beyond the result vector) so it is unit-testable headlessly
(`<unigui/core/flex_layout.h>`):

```cpp
using namespace unigui::layout;
auto spans = SolveFlex({{.basis = 100, .grow = 1}, {.basis = 100, .grow = 2}},
                       {.containerSize = 400});
// free space = 400 - 100 - 100 = 200px, split 1:2 by the grow factors
// spans[0] = {offset 0,       size 166.667}   (100 + 1/3 of the 200px free)
// spans[1] = {offset 166.667, size 233.333}   (100 + 2/3 of the 200px free)
```

`FlexItem` carries `basis`/`grow`/`shrink`/`minSize`/`maxSize`/`crossSize`;
`FlexParams` carries `containerSize`/`crossSize`/`gap`/`justify`/`align`.
`justify` is `FlexJustify` (`Start`, `End`, `Center`, `SpaceBetween`,
`SpaceAround`, `SpaceEvenly`); cross-axis `align` is `FlexAlign` (`Start`,
`Center`, `End`, `Stretch`). `SolveFlexWrap` adds CSS `flex-wrap` line breaking.

The **ImGui-facing** container that applies the solver is
`unigui::Layout::FlexRow` (`<unigui/widgets/layout.h>`), which draws each child
inside an ImGui child region sized to its solved width:

```cpp
using namespace unigui;
Layout::FlexRow("toolbar",
    { {.item = {.grow = 1}, .render = [] { im::Button("Left");  }},
      {.item = {.grow = 2}, .render = [] { im::Button("Right"); }} },
    {.gap = 8.f, .justify = layout::FlexJustify::SpaceBetween});
```

The same `Layout` namespace also offers simpler helpers (`HBox`/`VBox`,
`BeginHSplit`/`NextHSplit`/`EndHSplit`) and RAII `HBox`/`VBox` guards in the
`unigui` namespace. The DSL's `Flex(...)` node renders through `FlexRow`.

> Full guide: a dedicated `LAYOUT.md` is planned; the solver is fully documented
> in `<unigui/core/flex_layout.h>`.

### Styling — `unigui::styling::Engine`

A CSS-like style engine (`<unigui/styling/style_engine.h>`) that parses
stylesheets, supports selectors (type / `.class` / `#id` / `:pseudo`), CSS
variables, and `@media` rules, and applies matching rules to the ImGui style
system — with file hot-reload for the "edit the `.css` and see it update" loop:

```cpp
auto& eng = unigui::styling::Engine::Instance();
eng.LoadFile("theme.css");                 // returns rule count; watched for reload
eng.EvaluateMedia(viewW, viewH, /*dark*/ true);
eng.Apply("Button", "primary", "", /*hovered*/ true);  // push matching style
// ... each frame, to pick up edits:
eng.ReloadIfChanged();
```

### Events — `unigui::events::Bus`

A thread-safe publish/subscribe bus (`<unigui/events/eventbus.h>`) for
decoupling components. Topics support wildcards; payloads are `std::any`;
publishing can be synchronous or queued to a worker thread; subscriptions can be
RAII-scoped.

```cpp
auto& bus = unigui::events::Bus::Instance();
auto sub = bus.SubscribeScoped("window.*", [](const std::any& e) { /* ... */ });
bus.Publish("window.close", std::any{});       // sync
bus.PublishAsync("data.loaded", payload);       // queued
```

---

## 5. Optional modules — `UNIGUI_MODULE_*`

Self-contained sub-systems, each compiled in only when its CMake option is set
*before* `add_subdirectory(...)`. When OFF, the headers aren't compiled and
`unigui.h` guards them with the corresponding `UNIGUI_HAS_*` macro, so the rest
of the library still builds.

| CMake option | Default | Namespace / header | What it adds |
|---|---|---|---|
| `UNIGUI_MODULE_WIDGETS` | ON | `unigui::` widgets | The extended widget library |
| `UNIGUI_MODULE_DSL` | ON | `unigui::dsl` | Declarative DSL + component framework |
| `UNIGUI_MODULE_STYLING` | ON | `unigui::styling` | CSS-like style engine |
| `UNIGUI_MODULE_FONTS` | ON | `unigui::fonts` | Font manager + fallback chains |
| `UNIGUI_MODULE_EVENTS` | ON | `unigui::events` | EventBus |
| `UNIGUI_MODULE_PLUGIN` | ON | `unigui::plugin` | DLL plugin manager |
| `UNIGUI_MODULE_SQLITE` | OFF | `<unigui/sqlite/database.h>` | SQLite wrapper (dep: sqlite3) |
| `UNIGUI_MODULE_CONFIG` | OFF | `<unigui/config/config.h>` | TOML/JSON config (cpptoml, nlohmann-json) |
| `UNIGUI_MODULE_IPC` | OFF | `<unigui/ipc/ipc.h>`, `shmem.h` | Shared-memory IPC (zeromq) |
| `UNIGUI_MODULE_NETWORK` | OFF | `<unigui/network/network.h>` | HTTP/WebSocket (cpp-httplib, ixwebsocket) |
| `UNIGUI_MODULE_TRADING` | OFF | `unigui::trading` | Trading models + widgets |

The **plugin** module (`<unigui/plugin/plugin_interface.h>`) defines the contract
DLLs implement:

```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual PluginInfo GetInfo() const = 0;
    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float dt) {}   // optional per-frame
    virtual void Render() {}           // optional per-frame
};
// each plugin DLL exports CreatePlugin / DestroyPlugin
```

Backends are gated the same way, with `UNIGUI_BACKEND_*` options
(`DX11` ON by default on Windows, `DX12`/`VULKAN`/`SDL3` opt-in, `GLFW3` ON).

> Module overview, dependencies, and the minimal-consumer recipe:
> **[docs/MODULES.md](MODULES.md)**. Trading toolkit:
> **[docs/TRADING.md](TRADING.md)**.

---

## Which layer should I use?

All four UI layers coexist; this is a decision guide, not a hierarchy you must
obey.

| If you are building… | Use… | Why |
|---|---|---|
| **A whole application** with screens, shared state, navigation | **Component framework** (`dsl::Component` + `Store` + `Navigator`) | Dirty-tracked rebuilds, lifecycle, composition; the "framework" spine |
| **A static or data-driven view** (forms, lists, conditional UI) | **Declarative DSL** (`dsl::Window/VBox/...`) | Most concise; binds or holds state; `If`/`For` as data |
| **Stateful controls** — tables, validated forms, trees, wizards | **Retained widgets** (`unigui::Button`, `unigui::Table`, …) | Persistent state, validation, undo/redo, serialization, a11y |
| **A one-off or custom control** / inline drawing | **Immediate layer** (`unigui::im::*`) | Stateless one-liners, themed, no naming ceremony |
| **Something the layers don't cover** — raw ImGui | **`dsl::Custom`** (in a DSL tree) or call ImGui directly | The escape hatch: arbitrary draw code inside any tree |

Rules of thumb:

- **Framework for apps, widgets as building blocks.** A `Component::Build()`
  returns a DSL tree that can `Host()` other components and embed retained
  widgets via `dsl::Custom`.
- **`im` for the simple stuff.** If a control has no state worth persisting,
  the immediate one-liner is usually the right call — even inside a retained
  widget's `Render()`.
- **`dsl::Custom` is the universal escape hatch.** Because it runs arbitrary
  code each frame, you are never boxed in: raw ImGui, a third-party widget, or a
  nested Component all drop in through it.
- **Reach across layers freely.** Reactive state, layout, styling, and events
  apply regardless of which UI layer you chose.

---

## Putting it together

A typical UniGUI app:

1. Calls `RunApp(cfg, frame)` (or `Init`/`Run`) — the **app loop** owns a
   **backend** pair chosen by `cfg.backend` and clears to the **theme** backdrop.
2. Applies a **theme** (palette + surface material) once at startup.
3. Renders, each frame, a **Component**/**Navigator** whose `Build()` returns a
   **DSL** tree; leaf controls are DSL nodes (rendered through **`im`**),
   **retained widgets**, or raw drawing via **`dsl::Custom`**.
4. Drives reactivity with **`State`/`Store`/`Computed`**, arranges with the
   **flex** layout, optionally restyles via the **CSS engine**, and decouples
   subsystems through the **EventBus**.
5. Pulls in whichever **optional modules** (SQLite, config, IPC, network,
   trading, plugins) the build enabled.

### Where to go next

| Topic | Doc |
|---|---|
| Build & first app | [docs/GETTING_STARTED.md](GETTING_STARTED.md) |
| Component framework, Store, Navigator | [docs/FRAMEWORK.md](FRAMEWORK.md) |
| Optional modules & backend matrix | [docs/MODULES.md](MODULES.md) |
| Full widget API (all widgets) | [docs/WIDGET_API.md](WIDGET_API.md) |
| One example per widget | [docs/WIDGET_EXAMPLES.md](WIDGET_EXAMPLES.md) |
| Master API index (widgets + `im` + DSL + core) | [docs/API_INDEX.md](API_INDEX.md) |
| Trading toolkit | [docs/TRADING.md](TRADING.md) |
| Public-API stability contract | [docs/API_STABILITY.md](API_STABILITY.md) |

*Per-layer deep-dives: [REACTIVE.md](REACTIVE.md), [LAYOUT.md](LAYOUT.md),
[IM_API.md](IM_API.md), [DSL.md](DSL.md), [THEMING.md](THEMING.md),
[BACKENDS.md](BACKENDS.md), [ACCESSIBILITY.md](ACCESSIBILITY.md), and
[PRESETS.md](PRESETS.md). Each is header-verified; when in doubt, the documented
header itself remains authoritative.*
