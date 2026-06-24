# TeamkillerUniGUI — Modern Dear ImGui C++ Wrapper

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![CMake](https://img.shields.io/badge/CMake-3.31%2B-green)](https://cmake.org/)
[![vcpkg](https://img.shields.io/badge/vcpkg-managed-orange)](https://vcpkg.io/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Web-lightgrey)]()
[![Version](https://img.shields.io/badge/version-3.8.9-blueviolet)]()
[![Tests](https://img.shields.io/badge/tests-1060-brightgreen)]()
[![Widgets](https://img.shields.io/badge/widgets-95-blue)]()
[![Backends](https://img.shields.io/badge/backends-7%20%284%20runtime%29-orange)]()

A C++23 Dear ImGui wrapper providing a unified dark+light theme engine, high-level widget components, declarative DSL, CSS styling, plugin system, and EventBus. Supports 7 backends: GLFW+OpenGL3, SDL3+Vulkan, DX11, DX12, Metal, WebGPU, and Emscripten.

## Quick Start

### Windows: one command (recommended)

```powershell
git clone https://github.com/Teamkiller131/TeamkillerUniGUI.git
cd TeamkillerUniGUI

# Check the toolchain first (Visual Studio / CMake / Ninja / vcpkg):
pwsh -File scripts/check_env.ps1

# Configure + build + (optional) test in one go:
pwsh -File scripts/build.ps1                       # release build
pwsh -File scripts/build.ps1 -Preset windows-msvc-debug -Test
pwsh -File scripts/build.ps1 -Clean                # fresh build dir
```

`build.ps1` runs the environment self-check, then drives the build through
`cmake-msvc.cmd` so the MSVC toolset is always pinned correctly.

### Manual steps

```bash
git clone https://github.com/Teamkiller131/TeamkillerUniGUI.git
cd TeamkillerUniGUI

# Windows MSVC: use the cmake-msvc.cmd wrapper for correct MSVC environment
cmake-msvc.cmd --preset windows-msvc-release
cmake-msvc.cmd --build --preset windows-msvc-release
ctest --preset windows-msvc-release

# SDL3 + Vulkan
cmake --preset windows-msvc-sdl3-vulkan-release
cmake --build --preset windows-msvc-sdl3-vulkan-release

# Run demo
./build/windows-msvc-release/examples/hello_unigui/hello_unigui.exe --frames 10

# Linux (Fedora 43 / Rocky 9, GCC 14+, CMake 3.26+)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux -G Ninja -DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF
cmake --build build
ctest --test-dir build
```

## Documentation

| Doc | Description |
|-----|-------------|
| **[docs/README.md](docs/README.md)** | **Documentation hub** (index) |
| [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md) | Build & first app |
| [docs/EXAMPLES.md](docs/EXAMPLES.md) | Cookbook (composition, theme, DSL, threading) |
| [docs/WIDGET_EXAMPLES.md](docs/WIDGET_EXAMPLES.md) | One minimal example per widget (93 entries) |
| [docs/WIDGET_API.md](docs/WIDGET_API.md) | Full API (widgets + inline TreeView / CascadingCombo) |
| [docs/API_INDEX.md](docs/API_INDEX.md) | Master index (widgets + `im` + DSL + core) |
| [docs/API_STABILITY.md](docs/API_STABILITY.md) | API contract: semver, stability tiers, deprecation policy |
| [docs/TRADING.md](docs/TRADING.md) | Trading toolkit: financial formatting + models |
| [INTEGRATION.md](INTEGRATION.md) | Submodule + vcpkg embedding |
| [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | Build / CRT / CI FAQ |

## Architecture

```
User Code
    ↓
unigui:: API
    ├── Theme Engine (dark + light + 13 presets, unified style/color tokens, glass surfaces, elevation, StyleScope RAII)
    ├── Widget Library (95 widgets (100% PushID-safe), form validation, undo/redo, serialization)
    ├── Declarative DSL (unigui::dsl — Window, VBox/HBox, Button, CheckBox, SliderFloat, InputText, If/For)
    ├── EventBus (unigui::events::Bus — publish/subscribe with wildcards)
    ├── CSS Styling (unigui::styling::Engine — selector engine + variables)
    ├── Plugin System (unigui::plugin::Manager — DLL plugin hot-reload)
    ├── Font Manager (unigui::fonts::Manager — multi-font with fallback chains)
    ├── Config Layer (unigui::config::Store — TOML/JSON/INI)
    ├── SQLite (unigui::sqlite::Database — thin wrapper + migrations)
    ├── IPC (unigui::ipc — SharedMemory + ZMQ channels)
    ├── Backend Abstraction (PlatformBackend / RendererBackend interfaces)
    │   ├── GLFW + OpenGL 3.3 ★ (default, production)
    │   ├── SDL3 + Vulkan 1.3 ★ (production)
    │   ├── GLFW + DX11 ★ (production, Windows default)
    │   ├── GLFW + DX12 ★ (production)
    │   ├── Metal          (macOS, stub — not yet implemented)
    │   ├── WebGPU         (cross-platform, stub)
    │   └── Emscripten     (Web/HTML5, stub)
    └── App Bootstrap (Init / Run / NewFrame / Render)
    ↓
ImGui (v1.92.8, docking + multi-viewport)
```

## API Overview

### Core Loop

The simplest way — one call handles `Init`, the loop, and `Shutdown`:

```cpp
#include <unigui/unigui.h>

int main() {
    unigui::AppConfig cfg;
    cfg.title = "My App";
    // cfg.backend = unigui::BackendType::DX11; // default on Windows
    return unigui::RunApp(cfg, [] {
        ImGui::ShowDemoWindow(); // raw ImGui works + auto-themed
    });
}
```

`RunApp` returns `0` on success or `1` if `Init` failed.  Pass an optional
`maxFrames` argument to stop after a fixed number of frames (useful for CI):

```cpp
return unigui::RunApp(cfg, myUiCallback, /*maxFrames=*/10);
```

For manual control (e.g. when you need setup/teardown between frames):

```cpp
unigui::AppConfig cfg;
if (!unigui::Init(cfg)) return 1;

while (!unigui::ShouldClose()) {
    unigui::NewFrame();
    ImGui::ShowDemoWindow();
    unigui::Render();
}
unigui::Shutdown();
```

### Widget Fluent API

All widgets inherit chainable `With*` wrappers from the base class, enabling
one-liner configuration. Widgets that opt into the CRTP `FluentWidget<Derived>`
base (e.g. `Button`) keep the **derived type** through the whole chain, so
base helpers and widget-specific helpers mix freely:

```cpp
auto btn = std::make_shared<unigui::Button>("save", "Save");
btn->WithTooltip("Ctrl+S — Save the file")  // base helper  → Button&
   .WithEnabled(dirty)                      // base helper  → Button&
   .WithPrimary()                           // Button-only  → Button&
   .WithOnClick([]{ /* save */ });          // Button-only  → Button&

auto lbl = std::make_shared<unigui::Label>("hint", "Read-only");
lbl->WithVisible(false).WithAccessibleName("Hint label");
```

### Immediate Mode (`unigui::im`)

For the common "just draw a control" case you don't need a `shared_ptr`, a
unique name, or a manual `Render()`. The `unigui::im` namespace provides
themed, immediate-mode free functions that read like raw ImGui but stay inside
the UniGUI namespace (and avoid clashing with the retained-mode widget
*classes* of the same name):

```cpp
#include <unigui/im/im.h>
namespace im = unigui::im;

if (im::Button("Save", im::ButtonVariant::Primary)) save();
im::Checkbox("Enabled", &enabled);
im::SliderFloat("Gain", &gain, 0.f, 1.f);
im::InputText("Name", &name);          // bound to a std::string
im::Combo("Mode", &mode, {"Fast","Safe"});
im::SameLine();
im::Text("status: ok");
```

**Immediate vs retained mode** — use `unigui::im` free functions for simple,
stateless controls; use the retained-mode widget classes (`unigui::Button`,
`unigui::Form`, `unigui::DataTable`, …) when you need persistent state,
validation, undo/redo or serialization. The two layers coexist.

`unigui::im` wraps **100% of Dear ImGui's practical public surface** (201
functions) — you rarely need to drop to raw `ImGui::`, though it stays fully
supported and auto-themed when you do. The figure is tracked in CI by
[`scripts/coverage_vs_imgui.py`](scripts/coverage_vs_imgui.py).

### RAII Scopes

Move-only guards pair ImGui's `Begin*/Push*` calls with their matching
`End*/Pop*` automatically — no more forgotten or mismatched `End()`/`PopID()`:

```cpp
#include <unigui/core/scope.h>

if (unigui::WindowScope w{"Settings"}) {
    unigui::IDScope id{"row"};
    unigui::DisabledScope d{readOnly};
    im::Button("Apply");
}   // End() / PopID() / EndDisabled() run automatically, in reverse order
```

Available: `WindowScope`, `ChildScope`, `IDScope`, `DisabledScope`,
`GroupScope`, `TabBarScope`, `TabItemScope` (alongside the existing
`StyleScope`).

### Widget Factory

`unigui::Make<T>` / `MakeNamed<T>` cut the `std::make_shared` boilerplate and
can auto-generate unique widget names:

```cpp
auto btn = unigui::Make<unigui::Button>("save", "Save"); // explicit name
auto lbl = unigui::MakeNamed<unigui::Label>("Read-only"); // auto unique name
```

### Declarative DSL

Describe the UI as a tree of value-type builders, then `Render()` it each
frame. The DSL renders through the themed `unigui::im` layer, so its output
matches the rest of the toolkit. Stateful controls either **bind to a
variable** through a pointer or keep their state inside the retained node, so
re-`Render()`-ing the same tree preserves user input:

```cpp
#include <unigui/dsl/dsl.h>
using namespace unigui::dsl;

bool enabled = true;
float gain = 0.5f;

auto ui = Window("DSL Demo", VBox({
    Text("Welcome!"),
    Separator(),
    HBox({
        Button("Save", ButtonVariant::Primary, []{ /* action */ }),
        Button("Exit",                          []{ std::exit(0); })
    }),
    CheckBox("Enabled", &enabled),          // bound to an external bool
    SliderFloat("Gain", &gain, 0.f, 1.f),   // bound to an external float
    If([&]{ return enabled; }, Text("…running")),
    For(5, [](int i){ return Label("Item #" + std::to_string(i+1)); })
}));

// In render loop:
Render(ui);
```

Builders: `Window`, `VBox`, `HBox`, `Label`, `Text`, `TextWrapped`,
`TextDisabled`, `BulletText`, `Button` (with `ButtonVariant`), `CheckBox`,
`SliderFloat`, `InputText` (each bound or node-stated), `Separator`, `Spacing`,
`If`, `IfElse`, `For`.

### EventBus

```cpp
#include <unigui/events/eventbus.h>
using namespace unigui::events;

auto id = Bus::Instance().Subscribe("window.*", [](auto& e) {
    // handles window.close, window.resize, ...
});
Bus::Instance().Publish("window.close", std::string("main"));
```

### CSS Styling

```cpp
#include <unigui/styling/style_engine.h>
using namespace unigui::styling;

Engine::Instance().Parse(R"(
    Window { bg: #1a1a2e; rounding: 8 }
    Button.primary { bg: #e94560; rounding: 4 }
    Button:hover { bg: #ff6b6b }
)");
Engine::Instance().ApplyAll();
```

### Plugin System

```cpp
#include <unigui/plugin/plugin_manager.h>
using namespace unigui::plugin;

auto* p = Manager::Instance().Load("my_plugin.dll");
if (p) { p->Init(); /* each frame: p->Render(); */ }
```

### Modular CMake (conditional compilation)

```bash
# Core only (minimal, ~200 targets)
cmake -DUNIGUI_MODULE_WIDGETS=OFF -DUNIGUI_MODULE_DSL=OFF \
      -DUNIGUI_MODULE_EVENTS=OFF -DUNIGUI_MODULE_PLUGIN=OFF ...

# Full (all modules)
cmake -DUNIGUI_MODULE_SQLITE=ON -DUNIGUI_MODULE_CONFIG=ON \
      -DUNIGUI_MODULE_IPC=ON -DUNIGUI_BACKEND_DX12=ON ...
```

## ID Safety

All 95 widgets automatically scope their ImGui IDs via `PushID(name)/PopID()`.
No manual ID management needed — just give each widget instance a unique name:
```cpp
auto btn1 = std::make_shared<unigui::Button>("ok", "OK");
auto btn2 = std::make_shared<unigui::Button>("cancel", "Cancel"); // same label, no conflict!
```

## Widgets (86 total)

> **Docs hub**: [docs/README.md](docs/README.md) · API: [WIDGET_API.md](docs/WIDGET_API.md) · Per-widget: [WIDGET_EXAMPLES.md](docs/WIDGET_EXAMPLES.md) · Index: [API_INDEX.md](docs/API_INDEX.md) (TreeView & CascadingCombo merged in WIDGET_API)


| Category | Widget | Key API |
|----------|--------|---------|
| Containers | Window | `AddPanel()`, `SetMenuBarEnabled()`, `SetPosition()` |
| | Panel | `SetContentCallback(fn)`, `SetWrapEnabled(bool)` |
| | PanelBox | dark titled panel, `SetTintColor()` |
| | Card | elevated/outlined/filled surface, shadow |
| | GroupBox | `SetContentCallback(fn)`, `SetTitle()` |
| | TabWidget | `AddTab()`, `RemoveTab()` |
| | CollapsingHeader | `SetContentCallback(fn)`, `SetOpen(bool)` (v3.3) |
| Inputs | LineEdit | `SetValue()`, `SetValidator()`, `Undo()`/`Redo()` |
| | MultiLine | `SetText()`, `Undo()`/`Redo()` |
| | PasswordInput | `GetStrengthScore()` (0-4), show/hide toggle |
| | ComboBox | `SetItems()`, `SetOnChange()`, `SetSearchable()` |
| | MultiCombo | `GetSelectedIndices()`, `SetSelected()` |
| | CascadingCombo | N-level linked dropdowns, H/V layout + width ([API](docs/WIDGET_API.md#cascadingcombo)) |
| | SearchBox | `SetItems(v)`, `GetQuery()`, `SetOnSelect(fn)` |
| | CommandPalette | Ctrl+P fuzzy command launcher, `AddCommand()`, `Matches()`, `Execute()` (v3.8.5) |
| | FileDialog | in-ImGui open/save/folder picker, `NavigateInto()`, `ResolvedPath()`, `Confirm()` (v3.8.6) |
| | Slider\<T\> | `SetMin()`, `SetMax()`, `SetValue()` |
| | MultiHandleSlider | multi-draggable tick handles (v3.2) |
| | InputInt/InputFloat | `GetValue()`, `SetValue()` |
| | SpinBox\<T\> | `GetValue()`, `SetRange()` |
| | DatePicker | `GetDate()`, `SetDate()` |
| | ColorPicker | `GetColor()`, `SetColor()` |
| | FilePath | `SetPath()`, file picker |
| | DirPath | directory picker |
| | DragFloat\<T\>/DragInt\<T\> | `GetValue()`, `SetRange()`, `SetSpeed()` (v3.3) |
| | ColorEdit | `GetColor()`, `SetColor()`, `SetFormat()` (v3.3) |
| Display | Label | `GetText()`, `SetText()` |
| | Button | `WasClicked()`, `SetEnabled()` |
| | ImageButton | `SetImage(texID, w, h)`, `SetLabel()` |
| | IconButton | `WasClicked()` |
| | Hyperlink | `WasClicked()` |
| | RichText | `SetSpans()`, `AddSpan()` |
| | Markdown | `SetMarkdown()`, supports # ** * - [links] |
| | Image | `SetTexture(texID)`, scale modes |
| | ProgressBar | `SetFraction()`, state colors |
| | Gauge | radial/ring progress dial, `SetSweepDegrees()`, centre label |
| | Sparkline | inline Line/Area/Bar trend, `PushValue()`, trend colour |
| | MetricCard | KPI/pod tile: accent rail + status dot + value/delta/body (v3.7) |
| | ToggleButton | bistate run/stop button, enabled-predicate + tooltip (v3.7) |
| | ButtonGroup | aligned (Left/Right/Fill) button cluster (v3.7) |
| | PnlText / TagList | polarity-aware P&L text + inline flag chips (v3.7) |
| | StatusLamp | named states + glow (`SetGlowEnabled`) |
| | RiskBar | thresholds, animated ratio bar |
| | FuturesRiskBar | actual/estimated/overnight markers |
| | Badge / Tag | dot/count/label badges, removable tags |
| | HeroSection | gradient banner + action button |
| | LoadingIndicator | `SetActive(bool)`, spinner animation |
| Lists | VirtualList | `SetItemCount(n)`, `SetItemGetter(fn)` — 100k+ |
| | DataTable\<T\> | virtual scroll, sort, row color, group rows, inline edit, checkbox columns, filter |
| | EditableDataGrid\<T\> | typed per-column cell editors (no per-row cache), frozen-when-running (v3.7) |
| | BasketTicket\<T\> | editable basket grid: add/remove/import/validate/submit (v3.7) |
| | GroupedRiskTree | risk tree with Worst/Mean/Sum rollup + threshold colour (v3.7) |
| | MultiSplitter | N-panel H/V resizable layout (v3.2) |
| | ListView | `SetItems()`, `SetOnSelect()` |
| | Table | `AddRow()`, sortable, cell embedding, `ExportCSV()`/`ImportCSV()` |
| | TreeView | `SetRoot()`, `TextSpan`/`spans`, `SetRowRenderer()` ([API](docs/WIDGET_API.md#treeview)) |
| Selection | Selectable | `SetLabel()`, `SetSelected()`, `SetOnClick()` (v3.3) |
| | ListBox | `SetItems()`, `GetSelectedIndex()`, `SetOnChange()` (v3.3) |
| | SegmentedControl | single-select button group (1D/1W/1M), `SetOnChange()` (v3.6) |
| Layout | Splitter | `SetOrientation()`, drag to resize |
| | ScrollArea | `SetContentCallback(fn)` |
| | Separator | horizontal/vertical dividers |
| | Space | `DockSpace()` docking layout |
| Navigation | MenuBar | `SetMenus()`, nested submenus |
| | Breadcrumb | `SetItems()`, path navigation |
| | Wizard | `AddStep()`, `Next()`, `Previous()` |
| Dialogs | Dialog | `Open()`, `Close()`, modal/ non-modal |
| | ConfirmDialog | confirm popup, danger styling |
| | AlertBar | persistent animated banner |
| | Tooltip | `Show(text)`, hover tooltips |
| | ContextMenu | `Show()`, right-click popup |
| | Toast | `Toast::Info()`, `Success()`, `Warn()`, `Error()` |
| Forms | Form | `AddTextField()`, `Validate()`, `Serialize()` |
| | PropertyGrid | `AddProperty({name, type, val})` |
| | CheckBox | `SetChecked()`, `OnChange()` |
| | RadioGroup | `SetSelected()`, option groups |
| | ToggleSwitch | `SetOn(bool)`, toggle with label |
| Misc | ConnectionStatusBar | link-health strip: lamp + latency + sparkline + reconnect (v3.7) |
| | DragDrop | `BeginDragSource<T>()`, `AcceptDragDrop<T>()` |
| | TimeSeriesChart | real-time implot chart, sliding window |
| | PriceTicker | scrolling symbol/price marquee, ▲/▼ tint, `SetSpeed()` (v3.6) |
| | SliderBar | futures/price tick bar with confirm/rollback |
| | ShortcutManager | `Register()`, global shortcuts |
| | Notification | `Show()`, pending count |
| | TrayIcon | `Show()`, `Hide()`, `SetMenu()`, `ShowNotification()` |
| | Tag | colored tag badges |
| | ContextMenu | right-click popup menus |

## Theming & Surface Materials

The theme engine layers four token passes on top of any palette, so all 13 presets
(plus Dark/Light) share one consistent look. The frosted-glass / glassmorphism
(毛玻璃) aesthetic is the default.

```cpp
unigui::AppConfig cfg;
cfg.theme.theme   = "dark";                              // or any registry preset
cfg.theme.surface = unigui::theme::SurfaceStyle::Glass;  // default; see options below
unigui::Init(cfg);
```

**Surface materials** (`<unigui/theme/surface_style.h>`) — a translucency/"material"
pass applied over the colour palette. `ThemeConfig::surface` selects one:

| `SurfaceStyle` | Look |
|----------------|------|
| `Solid` | Flat, fully opaque — classic. |
| `Glass` *(default)* | Frosted glass — translucent surfaces + bright rim. |
| `Frosted` | Heavier translucency, stronger rim. |
| `Acrylic` | Fluent-style acrylic — firmer tint + border. |
| `Minimal` | Near-opaque, borderless, quiet. |

`SurfaceStyleName()` / `AllSurfaceStyles()` drive theme pickers. Translucent
materials read against a tinted backdrop: the app loop clears every backend to
`unigui::GetBackdropColor()` so glass surfaces don't render against black.

**Accent & semantic colours** (`<unigui/theme/color_tokens.h>`) — each theme derives
its full interactive palette (accent → hover → active, plus `Success`/`Warning`/
`Danger`/`Info`) from one base accent. Query the active palette:

```cpp
ImVec4 ok = unigui::GetSemanticColor(unigui::theme::Semantic::Success);
const auto& tokens = unigui::GetColorTokens();   // accent/hover/active/success/...
```

**Elevation** (`<unigui/fx/elevation.h>`) — a semantic shadow scale tied to the active
surface material. Glass gets a soft, diffuse shadow plus a rim; solid gets a firmer one.

```cpp
unigui::Button("save", "Save").WithElevation(unigui::fx::Elevation::Medium); // None/Low/Medium/High
```

## Backend Selection

```cpp
// Runtime (select from 8 backends)
cfg.backend = BackendType::GLFW_GL3;      // GLFW + OpenGL 3.3 ★
cfg.backend = BackendType::Vulkan;        // GLFW + Vulkan 1.3 (cross-platform) ★
cfg.backend = BackendType::SDL3_Vulkan;   // SDL3 + Vulkan 1.3 (opt-in, see below)
cfg.backend = BackendType::DX11;          // GLFW + DirectX 11 ★
cfg.backend = BackendType::DX12;          // GLFW + DirectX 12 ★
cfg.backend = BackendType::Metal;         // macOS Metal (stub, not yet implemented)
cfg.backend = BackendType::WebGPU;        // Dawn/WGPU (stub)
cfg.backend = BackendType::Emscripten;    // Web/HTML5 (stub)
```

| Backend | Platform | Graphics API | Status | MSAA |
|---------|----------|-------------|--------|------|
| GLFW+GL3 | Win/Lin/Mac | OpenGL 3.3 | ★ Production | 4x |
| GLFW+Vulkan | Win/Lin/Mac | Vulkan 1.3 | ★ Production | Config |
| SDL3+Vulkan | Win/Lin/Mac | Vulkan 1.3 | Opt-in (needs SDL3) | Config |
| GLFW+DX11 | Windows | DirectX 11 | ★ Production | 4x |
| GLFW+DX12 | Windows | DirectX 12 | ★ Production | Config |
| Metal | macOS | Metal 2 | Stub | Native |
| WebGPU | Cross | Dawn/WGPU | Stub | Native |
| Emscripten | Web | WebGL/WebGPU | Stub | Browser |

**Vulkan is backend-agnostic.** There is a single, platform-independent `VulkanRenderer`
(built on Dear ImGui's `imgui_impl_vulkan`). The one OS-specific step — creating the
`VkSurfaceKHR` and reporting the instance extensions it needs — is delegated to the active
`PlatformBackend`. GLFW backs it with `glfwCreateWindowSurface` (all OSes); SDL3 backs it with
`SDL_Vulkan_CreateSurface`. So `BackendType::Vulkan` works on Windows/Linux/macOS out of the box.

**Enabling SDL3** (`BackendType::SDL3_Vulkan`) is opt-in by design — UniGUI exists to abstract
backends, so the SDL3 code stays in-tree but is only compiled when you ask for it *and* its
dependencies are present:

1. Add the deps to your vcpkg manifest: `"sdl3"` and `imgui` feature `"sdl3-binding"`.
2. Configure with `-DUNIGUI_BACKEND_SDL3=ON` (it also pulls in the shared Vulkan renderer).

With the option off (the default), no SDL3 / `imgui_impl_sdl3` symbols are compiled or linked.

## Sub-Modules

| Module | Namespace | Header |
|--------|-----------|--------|
| Declarative DSL | `unigui::dsl` | `<unigui/dsl/dsl.h>` |
| EventBus | `unigui::events` | `<unigui/events/eventbus.h>` |
| CSS Styling | `unigui::styling` | `<unigui/styling/style_engine.h>` |
| Font Manager | `unigui::fonts` | `<unigui/fonts/font_manager.h>` |
| Plugin System | `unigui::plugin` | `<unigui/plugin/plugin_manager.h>` |
| Config (TOML/JSON/INI) | `unigui::config` | `<unigui/config/config.h>` |
| SQLite Database | `unigui::sqlite` | `<unigui/sqlite/database.h>` |
| IPC (Shared Memory + ZMQ) | `unigui::ipc` | `<unigui/ipc/shmem.h>`, `<unigui/ipc/ipc.h>` |
| HTTP / WebSocket | `unigui::network` | `<unigui/network/network.h>` |

All sub-module headers are also pulled in by `<unigui/unigui.h>` for convenience.

## Platform Notes

- **Windows**: Primary target. MSVC 19.40+ via Visual Studio 2022. DX11 is default. Full test suite (895) passes.
- **Linux**: GCC 14+/Clang 18+ via GLFW+OpenGL3. X11/Wayland. A small number of GL-context tests are skipped in headless runs. See [vcpkg.json](vcpkg.json) for x64-linux triplet deps.
- **macOS**: OpenGL deprecated by Apple (capped at 4.1). Vulkan via MoltenVK.

## Fonts

UniGUI embeds **JetBrains Mono Nerd Font** directly in the library binary. No system font installation required. The bundled font is licensed under the SIL Open Font License 1.1 — see [fonts/LICENSE](fonts/LICENSE).

```cpp
unigui::AppConfig cfg;
cfg.theme.font_path = "C:/path/to/my-font.ttf"; // custom font
cfg.theme.font_size = 20.0f;                     // logical px at 96 DPI
```

**CJK Support**: On Windows, the library automatically merges Microsoft YaHei (msyh.ttc) for Chinese/Japanese/Korean glyphs. On Linux/macOS, CJK merge is skipped — users can provide a custom CJK font via `ThemeConfig::font_path`.

**Nerd Font Icons**: The embedded font includes Nerd Font glyphs (`` `` `` `` etc.), usable in any ImGui text.

## Dependencies (vcpkg)

```
imgui (1.92.8, docking+freetype+all bindings)
implot (1.0), imgui-node-editor (0.9.3)
glfw3, sdl3, vulkan, glad, freetype, gtest, spdlog
Optional: sqlite3, cpptoml, nlohmann-json, zeromq, cpp-httplib, ixwebsocket
Windows: d3d11, d3d12, d3dcompiler, dxgi, dxguid
```

Optional vcpkg features: `sqlite`, `config`, `ipc`, `network`

## Development Tools

| Tool | Command |
|------|---------|
| Build (MSVC) | `cmake-msvc.cmd --preset windows-msvc-debug` |
| Build (Clang) | `cmake --preset windows-clang-coverage` |
| Test | `ctest --test-dir build/windows-msvc-debug --output-on-failure -j4` |
| Coverage | `cmake --build build/windows-clang-coverage --target coverage` |
| Lint | `cmake --preset windows-clang-tidy` |
| Format | `clang-format -i src/widgets/*.cc` |

See **[docs/README.md](docs/README.md)** for the full documentation index.

## License

The source code is licensed under the **MIT License** — see [LICENSE](LICENSE).

The bundled font (`fonts/JetBrainsMonoNerdFont-Regular.ttf`) is a Nerd Fonts
build of JetBrains Mono, licensed separately under the **SIL Open Font License
1.1** — see [fonts/LICENSE](fonts/LICENSE).
