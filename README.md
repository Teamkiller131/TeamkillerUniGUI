# TeamkillerUniGUI — Modern Dear ImGui C++ Wrapper

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![CMake](https://img.shields.io/badge/CMake-3.31%2B-green)](https://cmake.org/)
[![vcpkg](https://img.shields.io/badge/vcpkg-managed-orange)](https://vcpkg.io/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Web-lightgrey)]()
[![Version](https://img.shields.io/badge/version-3.5.0-blueviolet)]()
[![Tests](https://img.shields.io/badge/tests-637-brightgreen)]()
[![Widgets](https://img.shields.io/badge/widgets-82-blue)]()
[![Backends](https://img.shields.io/badge/backends-7%20%284%20runtime%29-orange)]()

A C++23 Dear ImGui wrapper providing a unified dark+light theme engine, high-level widget components, declarative DSL, CSS styling, plugin system, and EventBus. Supports 7 backends: GLFW+OpenGL3, SDL3+Vulkan, DX11, DX12, Metal, WebGPU, and Emscripten.

## Quick Start

### Windows: one command (recommended)

```powershell
git clone https://xbw-nas.iepose.cn/Teamkiller131/TeamkillerUniGUI.git
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
git clone https://xbw-nas.iepose.cn/Teamkiller131/TeamkillerUniGUI.git
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

## Troubleshooting / FAQ

First line of defense: run `pwsh -File scripts/check_env.ps1`. It detects a
missing C++ workload, an out-of-date CMake, a missing Ninja, an unset
`VCPKG_ROOT`, and — importantly — multiple MSVC toolsets on `PATH`.

**Q: `cl.exe`/`link.exe` "CreateProcess failed" or "系统找不到指定的路径" after a Visual Studio update.**
A: CMake cached the path of an MSVC toolset that no longer exists. Always build
through `cmake-msvc.cmd` (it re-runs `vcvars64.bat` to pin the current toolset).
If a build directory was already configured against the stale toolset, delete it
and reconfigure: `Remove-Item -Recurse -Force build/windows-msvc-release` then
re-run the preset (or use `scripts/build.ps1 -Clean`).

**Q: "Could not find toolchain file .../vcpkg.cmake" or vcpkg packages don't resolve.**
A: `VCPKG_ROOT` isn't set. Either set it to your standalone vcpkg checkout, or
rely on the copy bundled with Visual Studio (the wrapper picks it up). The
self-check script reports which vcpkg it found.

**Q: "ninja: command not found" / generator errors.**
A: Every preset uses the Ninja generator. Install it with
`winget install Ninja-build.Ninja`, or build through `cmake-msvc.cmd`, which
inherits the Ninja that ships with Visual Studio.

**Q: I'm on a non-default Visual Studio edition (Professional/Enterprise/BuildTools) and the wrapper can't find it.**
A: `cmake-msvc.cmd` locates VS via `vswhere` and works across editions. If your
install is in an unusual location, set `VS_INSTALL_DIR` to its root before
running the wrapper.

**Q: One test (`AppTest.Init_WithoutDisplay_ReturnsFalse`) hangs in CI / headless.**
A: It needs a window/graphics device. Exclude it in headless runs:
`ctest --preset windows-msvc-release -E "AppTest\.Init_WithoutDisplay_ReturnsFalse"`.

**Q: clang presets fail to link (`oldnames.lib`/`msvcrtd.lib` not found).**
A: clang-cl still needs the MSVC environment. Run clang presets through
`cmake-msvc.cmd` so `vcvars64.bat` sets the library paths.

## Architecture

```
User Code
    ↓
unigui:: API
    ├── Theme Engine (53-color dark + light theme, StyleScope RAII)
    ├── Widget Library (82 widgets (100% PushID-safe), form validation, undo/redo, serialization)
    ├── Declarative DSL (unigui::dsl — Window, VBox, HBox, Button, For, If)
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
    │   ├── Metal          (macOS, stub on Windows)
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
one-liner configuration:

```cpp
auto btn = std::make_shared<unigui::Button>("save", "Save");
btn->WithTooltip("Ctrl+S — Save the file")
   .WithEnabled(dirty)
   .WithShadow();

auto lbl = std::make_shared<unigui::Label>("hint", "Read-only");
lbl->WithVisible(false).WithAccessibleName("Hint label");
```

### Declarative DSL

```cpp
#include <unigui/dsl/dsl.h>
using namespace unigui::dsl;

auto ui = Window("DSL Demo", VBox({
    Text("Welcome!"),
    Separator(),
    HBox({
        Button("Click Me", []{ /* action */ }),
        Button("Exit",     []{ std::exit(0); })
    }),
    For(5, [](int i){ return Label("Item #" + std::to_string(i+1)); })
}));

// In render loop:
Render(ui);
```

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

All 82 widgets automatically scope their ImGui IDs via `PushID(name)/PopID()`.
No manual ID management needed — just give each widget instance a unique name:
```cpp
auto btn1 = std::make_shared<unigui::Button>("ok", "OK");
auto btn2 = std::make_shared<unigui::Button>("cancel", "Cancel"); // same label, no conflict!
```

## Widgets (82 total)

> Full, detailed API reference with examples: **[docs/WIDGET_API.md](docs/WIDGET_API.md)**.
> In-depth component guides: **[TreeView](docs/TREEVIEW.md)**, **[CascadingCombo](docs/CASCADINGCOMBO.md)**.


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
| | CascadingCombo | N-level linked dropdowns, H/V layout + width control ([guide](docs/CASCADINGCOMBO.md)) |
| | SearchBox | `SetItems(v)`, `GetQuery()`, `SetOnSelect(fn)` |
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
| | StatusLamp | named states + glow (`SetGlowEnabled`) |
| | RiskBar | thresholds, animated ratio bar |
| | FuturesRiskBar | actual/estimated/overnight markers |
| | Badge / Tag | dot/count/label badges, removable tags |
| | HeroSection | gradient banner + action button |
| | LoadingIndicator | `SetActive(bool)`, spinner animation |
| Lists | VirtualList | `SetItemCount(n)`, `SetItemGetter(fn)` — 100k+ |
| | DataTable\<T\> | virtual scroll, sort, row color, group rows, inline edit, checkbox columns, filter |
| | MultiSplitter | N-panel H/V resizable layout (v3.2) |
| | ListView | `SetItems()`, `SetOnSelect()` |
| | Table | `AddRow()`, sortable, cell embedding, `ExportCSV()`/`ImportCSV()` |
| | TreeView | `SetRoot()`, multi-select, composite/custom rows ([guide](docs/TREEVIEW.md)) |
| Selection | Selectable | `SetLabel()`, `SetSelected()`, `SetOnClick()` (v3.3) |
| | ListBox | `SetItems()`, `GetSelectedIndex()`, `SetOnChange()` (v3.3) |
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
| Misc | DragDrop | `BeginDragSource<T>()`, `AcceptDragDrop<T>()` |
| | TimeSeriesChart | real-time implot chart, sliding window |
| | SliderBar | futures/price tick bar with confirm/rollback |
| | ShortcutManager | `Register()`, global shortcuts |
| | Notification | `Show()`, pending count |
| | TrayIcon | `Show()`, `Hide()`, `SetMenu()`, `ShowNotification()` |
| | Tag | colored tag badges |
| | ContextMenu | right-click popup menus |

## Backend Selection

```cpp
// Runtime (select from 7 backends)
cfg.backend = BackendType::GLFW_GL3;      // GLFW + OpenGL 3.3 ★
cfg.backend = BackendType::SDL3_Vulkan;   // SDL3 + Vulkan 1.3 ★
cfg.backend = BackendType::DX11;          // GLFW + DirectX 11 ★
cfg.backend = BackendType::DX12;          // GLFW + DirectX 12
cfg.backend = BackendType::Metal;         // macOS Metal (stub on Win)
cfg.backend = BackendType::WebGPU;        // Dawn/WGPU (stub)
cfg.backend = BackendType::Emscripten;    // Web/HTML5 (stub)
```

| Backend | Platform | Graphics API | Status | MSAA |
|---------|----------|-------------|--------|------|
| GLFW+GL3 | Win/Lin/Mac | OpenGL 3.3 | ★ Production | 4x |
| SDL3+Vulkan | Win/Lin/Mac | Vulkan 1.3 | ★ Production | Config |
| GLFW+DX11 | Windows | DirectX 11 | ★ Production | 4x |
| GLFW+DX12 | Windows | DirectX 12 | ★ Production | Config |
| Metal | macOS | Metal 2 | Stub | Native |
| WebGPU | Cross | Dawn/WGPU | Stub | Native |
| Emscripten | Web | WebGL/WebGPU | Stub | Browser |

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

- **Windows**: Primary target. MSVC 19.40+ via Visual Studio 2022. DX11 is default. Full test suite (637) passes.
- **Linux**: GCC 14+/Clang 18+ via GLFW+OpenGL3. X11/Wayland. A small number of GL-context tests are skipped in headless runs. See [vcpkg.json](vcpkg.json) for x64-linux triplet deps.
- **macOS**: OpenGL deprecated by Apple (capped at 4.1). Vulkan via MoltenVK.

## Fonts

UniGUI embeds **JetBrains Mono Nerd Font** directly in the library binary. No system font installation required.

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

Full API documentation: [docs/WIDGET_API.md](docs/WIDGET_API.md)
Build workflow guide: [cpp-build-workflow skill](/.hermes/skills/cpp-build-workflow/SKILL.md)

## License

MIT
