# Getting Started

TeamkillerUniGUI (**v4.9.0**) is a C++23 [Dear ImGui](https://github.com/ocornut/imgui)
framework. It layers a unified dark/light theme engine, ~95 retained-mode
widgets, a themed immediate-mode helper layer (`unigui::im`), a declarative
DSL + reactive component framework (`unigui::dsl`), a flexbox layout solver, a
CSS-like styling engine, an EventBus, a plugin system, and optional sub-modules
(SQLite, config, IPC, networking, trading) on top of ImGui — all behind a
pluggable backend abstraction (GLFW/SDL3 platforms × OpenGL3/Vulkan/DX11/DX12
renderers).

This guide takes you from a clean checkout to a running window, then through
three "hello world" apps that each demonstrate a different layer of the
framework. By the end you'll know which layer to reach for and where to read
next.

---

## 1. Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| **C++ compiler** | C++23 | MSVC 19.40+ (VS 2022 17.10+), GCC 14+, or Clang 18+ |
| **CMake** | 3.31+ | The presets declare `cmakeMinimumRequired` 3.31 |
| **Ninja** | any recent | Every preset uses the Ninja generator |
| **vcpkg** | recent | Manifest mode; dependencies are declared in `vcpkg.json` |

Dependencies (Dear ImGui, GLFW, GoogleTest, …) are resolved automatically by
**vcpkg in manifest mode** — there is no manual `vcpkg install` step. CMake reads
the `vcpkg.json` manifest at the repo root during configure and builds exactly
the versions it pins.

### vcpkg setup

Clone and bootstrap vcpkg once, then point `VCPKG_ROOT` at it so the presets can
find the toolchain file:

```bash
git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh          # Windows: .\vcpkg\bootstrap-vcpkg.bat
export VCPKG_ROOT=$PWD/vcpkg        # PowerShell: $env:VCPKG_ROOT = "$PWD\vcpkg"
```

Every preset references `$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake` as
its `CMAKE_TOOLCHAIN_FILE`, so `VCPKG_ROOT` **must** be set in the environment
before you configure.

### Platform toolchains

- **Windows:** Visual Studio 2022 with the **"Desktop development with C++"**
  workload (this provides MSVC, and bundles a recent CMake + Ninja). DX11/DX12
  backends are Windows-only and enabled by default there.
- **Linux:** GCC 14+ or Clang 18+, plus the usual GL / X11 / Wayland dev
  packages your distro needs for GLFW.
- **macOS:** a recent Xcode / Apple Clang toolchain.

On Linux and macOS, disable the Windows-only DX backends when configuring
(`-DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF`); the cross-platform
presets already do this for you.

### Clone

```bash
git clone https://github.com/Teamkiller131/TeamkillerUniGUI.git
cd TeamkillerUniGUI
```

---

## 2. Building

Builds are driven by **CMake presets** (`CMakePresets.json`). Pick the preset for
your platform; the named presets pin the C++ standard (23), the vcpkg triplet,
and the toolchain file for you.

### Linux

```bash
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
```

The `linux-gcc-debug` preset already sets `VCPKG_TARGET_TRIPLET=x64-linux` and
turns the DX backends off. If you prefer a manual invocation (e.g. to tweak
options) it expands to:

```bash
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux \
  -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF
cmake --build build
```

### macOS

```bash
cmake --preset macos-clang-debug
cmake --build --preset macos-clang-debug
```

The `macos-clang-debug` preset uses the `arm64-osx` triplet and disables the DX
backends. (On an Intel Mac, override with `-DVCPKG_TARGET_TRIPLET=x64-osx`.)

### Windows (MSVC)

On Windows, **always configure through the `cmake-msvc.cmd` wrapper** (not bare
`cmake`). The wrapper re-runs `vcvars64.bat` so the MSVC toolset and Ninja are
pinned correctly — this avoids the "stale toolset cached by CMake" failure that
bare `cmake` is prone to after a Visual Studio update.

The smoothest path is the `build.ps1` helper, which wires the whole flow
together (environment self-check → configure → build → optional tests), all
through `cmake-msvc.cmd`:

```powershell
pwsh -File scripts/check_env.ps1                              # toolchain self-check
pwsh -File scripts/build.ps1 -Preset windows-msvc-debug -Test # configure + build + ctest
```

`check_env.ps1` is read-only — it inspects your machine and prints a
PASS/WARN/FAIL summary, flagging the common pitfalls (missing C++ workload,
multiple MSVC toolsets on `PATH`, CMake older than 3.31, missing `VCPKG_ROOT`).
`build.ps1` accepts `-Preset <name>`, `-Test` (run ctest after building),
`-Clean` (wipe the build dir first), and `-SkipCheck`.

Or drive the wrapper directly:

```powershell
cmake-msvc.cmd --preset windows-msvc-release
cmake-msvc.cmd --build --preset windows-msvc-release
ctest --preset windows-msvc-release --output-on-failure
```

### Key presets

| Purpose | Preset |
|---------|--------|
| MSVC debug / release | `windows-msvc-debug`, `windows-msvc-release` |
| Linux GCC debug | `linux-gcc-debug` |
| macOS Clang debug | `macos-clang-debug` |
| AddressSanitizer | `windows-msvc-debug-asan`, `linux-gcc-debug-asan` |
| Warnings-as-errors | `windows-msvc-debug-werror`, `linux-gcc-debug-werror` |
| clang-tidy lint | `windows-clang-tidy` |
| Coverage (target `coverage`) | `windows-clang-coverage` |
| SDL3 + Vulkan | `windows-msvc-sdl3-vulkan-debug`, `windows-msvc-sdl3-vulkan-release` |

There are matching `buildPresets` and `testPresets` of the same names, so
`cmake --build --preset <name>` and `ctest --preset <name>` work for each.

---

## 3. Running the tests and an example

### Tests

The suite (~110 GoogleTest files) runs under **ctest**. With a preset:

```bash
ctest --preset linux-gcc-debug --output-on-failure
```

or against a build directory directly:

```bash
ctest --test-dir build --output-on-failure
```

Some GL-context tests are skipped in headless Linux CI — that's expected.

### Run an example headlessly

Every example accepts `--frames N`: it renders N frames and exits, which makes it
safe for CI/smoke runs and screenshots (no window stays open waiting for input).

```bash
# Linux/macOS
./build/linux-gcc-debug/examples/hello_unigui/hello_unigui --frames 10
```

```powershell
# Windows
.\build\windows-msvc-release\examples\hello_unigui\hello_unigui.exe --frames 300
```

Other examples worth poking at: `widget_gallery`, `theme_demo`, `form_demo`,
`plot_demo`, `plugin_example`, and `v3_overview`.

---

## 4. The application entry point

Everything below builds on one small API from `<unigui/app/app.h>`. The umbrella
header `<unigui/unigui.h>` pulls in the whole library (app, widgets, `im`, `dsl`,
theme, …), so a single include is enough to start.

### `AppConfig`

`AppConfig` configures the window, theme, and backend:

```cpp
struct AppConfig {
    int width = 1280;
    int height = 720;
    const char* title = "UniGUI Application";
    ThemeConfig theme = {ThemePreset::Dark, 0.0f, 16.0f}; // auto-DPI, 16px logical
#ifdef _WIN32
    BackendType backend = BackendType::DX11;   // DX11 is stable on Windows
#else
    BackendType backend = BackendType::GLFW_GL3;
#endif
    bool dpiScaleFonts = false;                 // re-rasterise fonts as DPI changes
};
```

`ThemeConfig` (from `<unigui/theme/theme.h>`) controls the look:

```cpp
struct ThemeConfig {
    ThemePreset preset = ThemePreset::Dark;     // Dark | Light
    float dpi_scale = 0.0f;                      // 0 = auto-detect system DPI
    float font_size = 16.0f;                     // logical px at 96 DPI
    const char* font_path = nullptr;             // nullptr = auto-detect CJK font
    bool emoji_fallback = true;                  // auto-load system emoji font
    theme::SurfaceStyle surface = theme::SurfaceStyle::Glass; // Glass material default
};
```

`BackendType` (from `<unigui/backend/backend_types.h>`) selects platform +
renderer: `GLFW_GL3` (default off-Windows), `DX11`/`DX12` (Windows only),
`SDL3_Vulkan`, `Vulkan`, `Metal`, `WebGPU`, `Emscripten`. The Metal/WebGPU/
Emscripten renderers are stubs today — don't assume they're functional.

### `RunApp` — the one-call entry point

The simplest possible app: `RunApp` does `Init(config)` → run loop → `Shutdown()`
in one call, with initialization-failure handling. The callback runs once per
frame between `NewFrame()` and `Render()`.

```cpp
#include <unigui/unigui.h>

int main() {
    unigui::AppConfig cfg;
    cfg.title  = "My App";
    cfg.width  = 1280;
    cfg.height = 720;
    return unigui::RunApp(cfg, [] {
        ImGui::Text("Hello, UniGUI %s", UNIGUI_VERSION_STRING);
    });
}
```

```cpp
/// @return 0 on success, 1 if initialization failed.
int RunApp(const AppConfig& config, const std::function<void()>& callback, int maxFrames = 0);
```

Pass `maxFrames` to stop after N frames — the same headless hook the examples
expose via `--frames`:

```cpp
return unigui::RunApp(cfg, uiCallback, /*maxFrames=*/60);
```

### Manual render loop

When you integrate with your own engine or event loop, drive the four lifecycle
calls yourself instead of `RunApp`/`Run`:

```cpp
unigui::AppConfig cfg;
if (!unigui::Init(cfg)) return 1;
while (!unigui::ShouldClose()) {
    unigui::NewFrame();
    drawUi();
    unigui::Render();
}
unigui::Shutdown();
```

`unigui::Run(callback, maxFrames)` is the loop half of `RunApp`; call it after a
successful `Init` if you want the framework loop but need to run setup code
between `Init` and the first frame.

---

## 5. The four layers

UniGUI gives you four ways to put pixels on screen — they coexist deliberately,
and you mix them freely:

| Layer | Namespace | Style | Use for |
|-------|-----------|-------|---------|
| Raw ImGui | `ImGui::` | immediate | anything ImGui can do; the floor everything sits on |
| Immediate helpers | `unigui::im` | immediate, stateless | quick themed controls without `shared_ptr`/IDs |
| Retained widgets | `unigui::` | retained, stateful | persistent state, validation, undo/redo, serialization |
| Framework | `unigui::dsl` | declarative + reactive | whole applications built from stateful components |

The **golden path** for a real application is the framework layer
(`unigui::dsl`): describe the UI as **Components** whose views are a function of
reactive **State**, and let the framework re-render only what changed. The other
layers are how you drop down when you need a custom leaf or custom drawing.

The next three sections are complete, runnable `main()`s — one per ease-of-use
layer.

---

### 5a. Immediate mode — `unigui::im`

The immediate layer is a thin, allocation-light wrapper over Dear ImGui that
keeps the common "just draw a control" case a one-liner — no `shared_ptr`, no
unique names, no `ImGui::` prefix, and it's themed to match the rest of the
toolkit. Controls live in `unigui::im` (not `unigui`) so their names don't
collide with the same-named retained-mode widget *classes*.

State is yours: `im::` controls bind to your own variables and return `true` on
the frame they change.

```cpp
#include <unigui/unigui.h>

namespace im = unigui::im;

int main() {
    unigui::AppConfig cfg;
    cfg.title = "Immediate mode";

    // Caller-owned state (lives across frames because it's static/local to main).
    static bool  enabled = true;
    static float gain    = 0.5f;
    static int   clicks  = 0;

    return unigui::RunApp(cfg, [] {
        // Raw ImGui still owns windows; im:: fills them with themed controls.
        ImGui::Begin("Controls");

        im::Text("UniGUI immediate layer");
        im::Separator();

        im::Checkbox("Enabled", &enabled);

        {
            // RAII disabled scope from <unigui/core/scope.h>: grey out when !enabled.
            unigui::DisabledScope d{!enabled};
            im::SliderFloat("Gain", &gain, 0.0f, 1.0f);
            if (im::Button("Click me", im::ButtonVariant::Primary))
                ++clicks;
        }

        im::TextDisabled("Clicked " + std::to_string(clicks) + " time(s)");

        ImGui::End();
    });
}
```

Notes anchored to the real API:

- `im::Button` has an overload taking a `ButtonVariant`
  (`Default`, `Primary`, `Danger`, `Success`, `Warning`) and returns `true` on
  the clicked frame.
- `im::Checkbox(label, bool*)`, `im::SliderFloat(label, float*, min, max)` bind by
  pointer and return `true` when edited.
- `unigui::DisabledScope` (from `<unigui/core/scope.h>`) greys out and disables
  every control in its scope until it goes out of scope.

---

### 5b. A retained widget — `unigui::`

Retained-mode widgets are classes that persist between frames. You construct one
once (typically a `std::shared_ptr` held in stable storage), configure it with a
fluent `With*` chain, and call `Render()` each frame. Reach for these when you
want persistent state, validation, undo/redo, or serialization baked into the
widget rather than managed by hand.

This example uses two retained widgets: a `Window` containing a `Panel`, and a
`Button` configured with the CRTP fluent chain.

```cpp
#include <unigui/unigui.h>

#include <memory>

int main() {
    unigui::AppConfig cfg;
    cfg.title = "Retained widget";

    // Build the widget tree once, in stable storage, so it survives every frame.
    static auto window = std::make_shared<unigui::Window>("main", "Retained Demo");
    static auto saveBtn = std::make_shared<unigui::Button>("save", "Save");
    static bool built = false;
    static int  saves = 0;

    if (!built) {
        auto panel = std::make_shared<unigui::Panel>("panel", "Document");
        panel->SetContentCallback([] {
            // Panel content is drawn each frame; mix in im:: / raw ImGui freely.
            ImGui::TextWrapped("This panel and button are retained widgets — "
                               "constructed once, rendered every frame.");

            // Fluent chain: base helpers + Button-specific helpers compose because
            // each With* returns Button& (CRTP FluentWidget<Button>).
            saveBtn->WithLabel("Save")
                    .WithPrimary()
                    .WithSize(unigui::Button::Large);
            saveBtn->Render();
            if (saveBtn->WasClicked())
                ++saves;

            ImGui::Text("Saved %d time(s)", saves);
        });
        window->AddPanel(panel);
        built = true;
    }

    return unigui::RunApp(cfg, [] { window->Render(); });
}
```

Notes anchored to the real API:

- `unigui::Window(name, title)` + `AddPanel(std::shared_ptr<Panel>)`;
  `unigui::Panel(name, title)` + `SetContentCallback(std::function<void()>)`.
  The first argument is a stable **ID name** (used for ImGui ID scoping), distinct
  from the human-readable title/label.
- `unigui::Button(name, label)` exposes `WasClicked()` and a fluent chain:
  `WithLabel`, `WithVariant`/`WithPrimary`/`WithDanger`/`WithSuccess`/`WithMuted`,
  `WithSize` (`Button::Small | Medium | Large`), `WithOnClick`. Its color variants
  are `Default, Primary, Danger, Success, Muted`.

---

### 5c. The framework — a `dsl::Component`

The framework layer turns "widgets you call every frame" into "an app built from
self-contained, stateful, composable units". A **Component** owns reactive
**State**, declares its view as a DSL node tree in `Build()`, and the framework
re-`Build()`s it **only when state changes** (dirty tracking), rendering the
cached tree every frame. This is the rebuild-on-`setState` model, adapted to
immediate mode.

```cpp
#include <unigui/unigui.h>

#include <string>

using namespace unigui::dsl;

// A reactive component: its view is a function of its State.
class Counter : public Component {
    State<int> count_{this, 0};   // reactive cell, owned by this component

public:
    const char* InspectorName() const override { return "Counter"; }

    NodePtr Build() override {     // view = f(state); re-runs only after a change
        return VBox({
            Text("Count: " + std::to_string(count_())),
            Separator(),
            HBox({
                Button("Increment", ButtonVariant::Primary,
                       [this] { count_ = count_() + 1; }),
                Button("Reset", ButtonVariant::Danger,
                       [this] { count_ = 0; }),
            }),
        });
    }
};

int main() {
    unigui::AppConfig cfg;
    cfg.title = "Framework (DSL component)";

    // The component is address-sensitive (State holds a back-pointer to it) — it
    // is neither copyable nor movable, so hold it in stable storage.
    static Counter app;

    return unigui::RunApp(cfg, [] {
        // Components render into the current ImGui context. Wrap in a window so
        // the VBox/HBox tree has somewhere to live.
        ImGui::Begin("Counter");
        app.Render();   // mount on first call, rebuild on state change, draw cached tree
        ImGui::End();
    });
}
```

Key pieces from `<unigui/dsl/component.h>` and `<unigui/dsl/dsl.h>`:

- `State<T>{this, initial}` — a reactive cell. `count_()` reads; `count_ = v`
  (or `.Set(v)` / `.Mutate(fn)`) writes. A write change-detects (`operator==`)
  and, on a real change, marks the component dirty so it rebuilds next frame.
- `Build()` returns a `NodePtr` tree from builders like `VBox`, `HBox`, `Flex`,
  `Text`, `Separator`, `Button(label, variant, onClick)`, `CheckBox`,
  `SliderFloat`, `InputText`, `If`/`IfElse`, `For`, and `Custom` (an escape hatch
  that runs any `im::`/raw-ImGui drawing inside the tree).
- `Component::Render()` mounts the component on first call (`OnMount()`),
  rebuilds the cached tree only when dirty, then draws it.

#### Composition, shared state, and navigation

Beyond a single component, the framework gives an application structure (from
`<unigui/dsl/app.h>`):

- **Composition** — embed a child component in a parent's `Build()` tree with
  `Host(child)`; each child keeps its own state and dirty tracking. The parent
  must own the child so it outlives the rendered tree.
- **Shared state** — `Store<T>` lives outside the component tree (app-wide).
  A component subscribes with `Watch(store)` in `OnMount()` and re-renders when
  the store changes; read it with `store()` / `store.Get()`, write it with
  `store.Set(v)` / `store.Update(fn)`.
- **Navigation** — a `Navigator` is a stack of screens (components) you
  `Push` / `Pop` / `Replace`; it renders the top one each frame, unmounting a
  screen (running its `OnCleanup` teardowns + `OnUnmount`) when it leaves.

```cpp
unigui::dsl::Store<int> cartCount{0};

class Header : public unigui::dsl::Component {
public:
    void OnMount() override { Watch(cartCount); }   // re-render on store change
    unigui::dsl::NodePtr Build() override {
        return unigui::dsl::Text("Cart: " + std::to_string(cartCount()));
    }
};
```

See [FRAMEWORK.md](FRAMEWORK.md) for the full component/store/navigator guide.

---

## 6. Theming at startup

The theme is part of `AppConfig`, so it's applied during `Init`:

```cpp
unigui::AppConfig cfg;
cfg.theme.preset    = unigui::ThemePreset::Light;          // Dark | Light
cfg.theme.font_size = 16.0f;                                // logical px at 96 DPI
cfg.theme.surface   = unigui::theme::SurfaceStyle::Glass;   // surface material
unigui::RunApp(cfg, drawUi);
```

A few things to know:

- **Translucent surfaces** (`Glass`/`Frosted`/`Acrylic`) render against the theme
  backdrop. The app loop already clears every backend to
  `unigui::GetBackdropColor()` so the glass effect reads correctly — don't
  hard-code a black/opaque clear in custom backend code.
- **HiDPI** is automatic (`dpi_scale = 0` auto-detects). For multi-monitor /
  fractional-DPI setups, set `cfg.dpiScaleFonts = true` to let ImGui re-rasterise
  fonts on the fly, or call `unigui::SetContentScale(scale)` after `Init`.
- **Named presets** (Dracula, Nord, Catppuccin, …) are applied at runtime through
  the theme registry. `Init`/`RunApp` already calls `RegisterAllThemes()`, so the
  built-in presets are available from the first frame — apply one by its
  registered (case-sensitive) name through the singleton instance, e.g.
  `unigui::theme::ThemeRegistry::Instance().Apply("Dracula")`.

---

## 7. Embedding UniGUI in your own project

Link the library target in your CMake project:

```cmake
target_link_libraries(my_app PRIVATE unigui::unigui)
```

Optional functionality is gated by CMake options so you only pay for what you
use. Modules default **on**: `UNIGUI_MODULE_WIDGETS`, `UNIGUI_MODULE_DSL`,
`UNIGUI_MODULE_STYLING`, `UNIGUI_MODULE_FONTS`, `UNIGUI_MODULE_EVENTS`,
`UNIGUI_MODULE_PLUGIN`. Default **off**: `UNIGUI_MODULE_SQLITE`,
`UNIGUI_MODULE_CONFIG`, `UNIGUI_MODULE_IPC`, `UNIGUI_MODULE_NETWORK`,
`UNIGUI_MODULE_TRADING`. Backends are gated by `UNIGUI_BACKEND_*` (DX11 on by
default on Windows; GLFW3 on by default; Vulkan/SDL3/DX12 opt-in).

Embedding as a git submodule and the single-triplet vcpkg flow are covered in
[INTEGRATION.md](../INTEGRATION.md).

---

## 8. Where to go next

| Doc | What it covers |
|-----|----------------|
| [FRAMEWORK.md](FRAMEWORK.md) | The golden path: components, reactive `State`/`Store`, `Computed`, effects, `Navigator` |
| [WIDGET_API.md](WIDGET_API.md) | Full reference for every retained widget |
| [WIDGET_EXAMPLES.md](WIDGET_EXAMPLES.md) | One minimal example per widget |
| [API_INDEX.md](API_INDEX.md) | Master index — widgets + `im` + DSL + core |
| [EXAMPLES.md](EXAMPLES.md) | Common end-to-end patterns |
| [MODULES.md](MODULES.md) | Optional sub-modules (SQLite, config, IPC, network, trading) |
| [TRADING.md](TRADING.md) | Trading toolkit (financial formatting, models, widgets) |
| [API_STABILITY.md](API_STABILITY.md) | The public-API contract (semver, tiers, deprecation lifecycle) |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | Build / CRT / CI FAQ |
| [INTEGRATION.md](../INTEGRATION.md) | Embedding as a submodule + vcpkg |
