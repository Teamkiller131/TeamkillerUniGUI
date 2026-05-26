# TeamkillerUniGUI — Modern Dear ImGui C++ Wrapper

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![CMake](https://img.shields.io/badge/CMake-3.31%2B-green)](https://cmake.org/)
[![vcpkg](https://img.shields.io/badge/vcpkg-managed-orange)](https://vcpkg.io/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Web-lightgrey)]()
[![Version](https://img.shields.io/badge/version-3.2.3-blueviolet)]()
[![Tests](https://img.shields.io/badge/tests-244%20(236%2F244%20Linux)-brightgreen)]()
[![Widgets](https://img.shields.io/badge/widgets-66-blue)]()
[![Backends](https://img.shields.io/badge/backends-7%20%284%20runtime%29-orange)]()

A C++23 Dear ImGui wrapper providing a unified dark+light theme engine, high-level widget components, declarative DSL, CSS styling, plugin system, and EventBus. Supports 7 backends: GLFW+OpenGL3, SDL3+Vulkan, DX11, DX12, Metal, WebGPU, and Emscripten.

## Quick Start

```bash
git clone https://github.com/Teamkiller131/TeamkillerUniGUI.git
cd TeamkillerUniGUI

# Default: GLFW + OpenGL3
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
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

## Architecture

```
User Code
    ↓
unigui:: API
    ├── Theme Engine (53-color dark + light theme, StyleScope RAII)
    ├── Widget Library (66 widgets, form validation, undo/redo, serialization)
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

```cpp
#include <unigui/unigui.h>

unigui::AppConfig cfg;
cfg.backend = unigui::BackendType::DX11; // or GLFW_GL3, SDL3_Vulkan, DX12
unigui::Init(cfg);

while (!unigui::ShouldClose()) {
    unigui::NewFrame();
    ImGui::ShowDemoWindow(); // raw ImGui works + auto-themed
    unigui::Render();
}
unigui::Shutdown();
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

## Widgets (66 total)

| Category | Widget | Key API |
|----------|--------|---------|
| Containers | Window | `AddPanel()`, `SetMenuBarEnabled()`, `SetPosition()` |
| | Panel | `SetContentCallback(fn)`, `SetWrapEnabled(bool)` |
| | GroupBox | `SetContentCallback(fn)`, `SetTitle()` |
| | TabWidget | `AddTab()`, `RemoveTab()` |
| Inputs | LineEdit | `SetValue()`, `SetValidator()`, `Undo()`/`Redo()` |
| | MultiLine | `SetText()`, `Undo()`/`Redo()` |
| | PasswordInput | `GetStrengthScore()` (0-4), show/hide toggle |
| | ComboBox | `SetItems()`, `SetOnChange()`, `SetSearchable()` |
| | MultiCombo | `GetSelectedIndices()`, `SetSelected()` |
| | SearchBox | `SetItems(v)`, `GetQuery()`, `SetOnSelect(fn)` |
| | Slider\<T\> | `SetMin()`, `SetMax()`, `SetValue()` |
| | MultiHandleSlider | multi-draggable tick handles (v3.2) |
| | InputInt/InputFloat | `GetValue()`, `SetValue()` |
| | SpinBox\<T\> | `GetValue()`, `SetRange()` |
| | DatePicker | `GetDate()`, `SetDate()` |
| | ColorPicker | `GetColor()`, `SetColor()` |
| | FilePath | `SetPath()`, file picker |
| | DirPath | directory picker |
| Display | Label | `GetText()`, `SetText()` |
| | Button | `WasClicked()`, `SetEnabled()` |
| | ImageButton | `SetImage(texID, w, h)`, `SetLabel()` |
| | IconButton | `WasClicked()` |
| | Hyperlink | `WasClicked()` |
| | RichText | `SetSpans()`, `AddSpan()` |
| | Markdown | `SetMarkdown()`, supports # ** * - [links] |
| | Image | `SetTexture(texID)`, scale modes |
| | ProgressBar | `SetFraction()`, state colors |
| | LoadingIndicator | `SetActive(bool)`, spinner animation |
| Lists | VirtualList | `SetItemCount(n)`, `SetItemGetter(fn)` — 100k+ |
| | DataTable\<T\> | virtual scroll, sort, row color, inline edit, filter (v3.2) |
| | ListView | `SetItems()`, `SetOnSelect()` |
| | Table | `AddRow()`, `ExportCSV()`, `ImportCSV()` |
| | TreeView | `SetRoot()`, multi-select support |
| Layout | Splitter | `SetOrientation()`, drag to resize |
| | ScrollArea | `SetContentCallback(fn)` |
| | Separator | horizontal/vertical dividers |
| | Space | `DockSpace()` docking layout |
| Navigation | MenuBar | `SetMenus()`, nested submenus |
| | Breadcrumb | `SetItems()`, path navigation |
| | Wizard | `AddStep()`, `Next()`, `Previous()` |
| Dialogs | Dialog | `Open()`, `Close()`, modal/ non-modal |
| | Tooltip | `Show(text)`, hover tooltips |
| | ContextMenu | `Show()`, right-click popup |
| | Toast | `Toast::Info()`, `Success()`, `Warn()`, `Error()` |
| Forms | Form | `AddTextField()`, `Validate()`, `Serialize()` |
| | PropertyGrid | `AddProperty({name, type, val})` |
| | CheckBox | `SetChecked()`, `OnChange()` |
| | RadioGroup | `SetSelected()`, option groups |
| | ToggleSwitch | `SetOn(bool)`, toggle with label |
| Misc | DragDrop | `BeginDragSource<T>()`, `AcceptDragDrop<T>()` |
| | TimeSeriesChart | real-time implot chart, sliding window (v3.2) |
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

- **Windows**: Primary target. MSVC 19.40+ via Visual Studio 2022. DX11 is default. 244/244 tests pass.
- **Linux**: GCC 14+/Clang 18+ via GLFW+OpenGL3. X11/Wayland. 236/244 tests pass (8 GL-context failures expected in headless). See [vcpkg.json](vcpkg.json) for x64-linux triplet deps.
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
Optional: sqlite3, cpptoml, nlohmann-json, cppzmq, cpp-httplib, ixwebsocket
Windows: d3d11, d3d12, d3dcompiler, dxgi, dxguid
```

## License

MIT
