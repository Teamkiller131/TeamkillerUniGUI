# TeamkillerUniGUI — Modern Dear ImGui C++ Wrapper

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![CMake](https://img.shields.io/badge/CMake-3.31%2B-green)](https://cmake.org/)
[![vcpkg](https://img.shields.io/badge/vcpkg-managed-orange)](https://vcpkg.io/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Web-lightgrey)]()
[![Version](https://img.shields.io/badge/version-2.9.0-blueviolet)]()
[![Tests](https://img.shields.io/badge/tests-200-brightgreen)]()
[![Widgets](https://img.shields.io/badge/widgets-55-blue)]()
[![Backends](https://img.shields.io/badge/backends-7%20%284%20runtime%29-orange)]()

A C++23 Dear ImGui wrapper library providing a unified dark+light theme engine and high-level widget components. Supports 7 backends: GLFW+OpenGL3, SDL3+Vulkan, DX11, DX12, Metal, WebGPU, and Emscripten.

## Quick Start

```bash
git clone https://github.com/Teamkiller131/TeamkillerUniGUI.git
cd TeamkillerUniGUI

# GLFW + OpenGL3 (default, backward compatible)
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release

# SDL3 + Vulkan (v2.0)
cmake --preset windows-msvc-sdl3-vulkan-release
cmake --build --preset windows-msvc-sdl3-vulkan-release

# Run demo (GLFW_GL3)
./build/windows-msvc-release/examples/hello_unigui/hello_unigui.exe --frames 10
```

## Architecture

```
User Code
    ↓
unigui:: API
    ├── Theme Engine (53-color dark + light theme, StyleScope RAII)
    ├── Widget Library (45 widgets across 6 releases)
    ├── Backend Abstraction (PlatformBackend / RendererBackend interfaces)
    │   ├── GLFW + OpenGL 3.3 ★ (default, production)
    │   ├── SDL3 + Vulkan 1.3 ★ (v2.0, production)
    │   ├── GLFW + DX11 ★ (v2.3, runtime-ready)
    │   ├── GLFW + DX12   (v2.7, runtime-ready)
    │   ├── Metal          (macOS, stub on Windows)
    │   ├── WebGPU         (cross-platform, stub)
    │   └── Emscripten     (Web/HTML5, stub)
    └── App Bootstrap (Init / Run / NewFrame / Render)
    ↓
ImGui (v1.92.8, docking + multi-viewport)
```

## API Overview

```cpp
#include <unigui/unigui.h>

// Backend selection (v2.0)
unigui::AppConfig cfg;
cfg.backend = unigui::BackendType::GLFW_GL3;      // default
// cfg.backend = unigui::BackendType::SDL3_Vulkan; // v2.0
unigui::Init(cfg);

// Manual loop
while (!unigui::ShouldClose()) {
    unigui::NewFrame();
    ImGui::ShowDemoWindow(); // raw ImGui works + auto-themed
    unigui::Render();
}
unigui::Shutdown();
```

## Widgets

### All Widgets (45 total)

| Tier | Widgets |
|------|---------|
| **v1** | Window, Panel, Form, Button, Label, WidgetBase (6) |
| **v2.0** | CheckBox, Slider\<T\>, ProgressBar, RadioGroup, ComboBox, LineEdit, GroupBox, TabWidget (8) |
| **v2.1** | TreeView, ListView, Dialog, MenuBar, StatusBar, ToolBar, Table, ColorPicker (8) |
| **v2.2** | FilePath, DirPath, SpinBox\<T\>, ToggleSwitch, InputInt, InputFloat, Splitter, Separator, ScrollArea, Tooltip (10) |
| **v2.3** | DatePicker, Image, LoadingIndicator, Notification, Hyperlink (5) |
| **v2.4** | Tag, Breadcrumb, MultiLine, IconButton (4) |
| **v2.5-6** | DockSpace, ContextMenu, DragDrop, ShortcutManager (4) |
| **v2.8** | RichText, ImageButton, Markdown (3) |

### Widget Quick Reference

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
| | InputInt/InputFloat | `GetValue()`, `SetValue()` |
| | SpinBox\<T\> | `GetValue()`, `SetRange()` |
| | DatePicker | `GetDate()`, `SetDate()` |
| | ColorPicker | `GetColor()`, `SetColor()` |
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
| | ListView | `SetItems()`, `SetOnSelect()` |
| | Table | `AddRow()`, `ExportCSV()`, `ImportCSV()` |
| | TreeView | `SetRoot()`, multi-select support |
| Layout | Splitter | `SetOrientation()`, drag to resize |
| | ScrollArea | `SetContentCallback(fn)` |
| | Separator | horizontal/vertical dividers |
| | Space | `DockSpace()` docking layout |
| Navigation | MenuBar | `SetMenus()`, nested submenus |
| | TabWidget | tabbed content panels |
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
| Files | FilePath | `SetPath()`, file picker |
| | DirPath | directory picker |
| Misc | DragDrop | `BeginDragSource<T>()`, `AcceptDragDrop<T>()` |
| | ShortcutManager | `Register()`, global shortcuts |
| | Notification | `Show()`, pending count |
| | TrayIcon | `Show()`, `Hide()`, `SetMenu()`, `ShowNotification()` |

## Backend Selection

```cpp
// CMake option (binary backends)
cmake -DUNIGUI_BACKEND=SDL3_VULKAN ...

// Runtime (select from 7 backends)
cfg.backend = BackendType::GLFW_GL3;      // GLFW + OpenGL 3.3 ★
cfg.backend = BackendType::SDL3_Vulkan;  // SDL3 + Vulkan 1.3 ★
cfg.backend = BackendType::DX11;          // GLFW + DirectX 11
cfg.backend = BackendType::DX12;          // GLFW + DirectX 12
cfg.backend = BackendType::Metal;         // macOS Metal (stub on Win)
cfg.backend = BackendType::WebGPU;        // Dawn/WGPU (stub)
cfg.backend = BackendType::Emscripten;    // Web/HTML5 (stub)
```

### Backend Comparison

| Backend | Platform | Graphics API | Status | Anti-aliasing | Complexity |
|---------|----------|-------------|--------|--------------|------------|
| GLFW+GL3 | Win/Lin/Mac | OpenGL 3.3 | ★ Production | 4x MSAA | Low |
| SDL3+Vulkan | Win/Lin/Mac | Vulkan 1.3 | ★ Production | Configurable | High |
| GLFW+DX11 | Windows | DirectX 11 | ✓ Runtime | 4x MSAA | Medium |
| GLFW+DX12 | Windows | DirectX 12 | ✓ Runtime | Configurable | High |
| Metal | macOS | Metal 2 | Stub | Native | Medium |
| WebGPU | Cross | Dawn/WGPU | Stub | Native | High |
| Emscripten | Web | WebGL/WebGPU | Stub | Browser | Medium |

## Platform Notes

- **Windows**: Primary target. MSVC 19.40+ via Visual Studio 2022.
- **Linux**: X11/Wayland via GLFW or SDL3. GCC 14+ or Clang 18+.
- **macOS**: OpenGL deprecated by Apple (capped at 4.1). Vulkan via MoltenVK.

## Fonts

UniGUI embeds **JetBrains Mono Nerd Font** directly in the library binary. No system font installation required.

```cpp
// Default: JetBrains Mono Nerd Font at auto-DPI size
unigui::AppConfig cfg;

// Custom font via file path
cfg.theme.font_path = "C:/path/to/my-font.ttf";
cfg.theme.font_size = 20.0f;           // logical px at 96 DPI

// Disable CJK merge (faster startup)
cfg.theme.font_path = nullptr;         // use embedded font only
```

**CJK Support**: On Windows, the library automatically merges Microsoft YaHei (msyh.ttc) for Chinese/Japanese/Korean glyphs. On Linux/macOS, CJK merge is skipped — users can provide a custom CJK font via `ThemeConfig::font_path`.

**Nerd Font Icons**: The embedded font includes Nerd Font glyphs (`` `` `` `` etc.), usable in any ImGui text.

## Dependencies (vcpkg)

- `imgui` (v1.92.8, docking+freetype+glfw+opengl3+sdl3+vulkan+dx11+dx12 bindings)
- `implot` (v1.0, plot widget library)
- `imgui-node-editor` (v0.9.3, node editor groundwork)
- `glfw3`, `sdl3`, `vulkan`, `glad`, `freetype`, `gtest`
- Windows: `d3d11`, `d3d12`, `d3dcompiler`, `dxgi`, `dxguid`

## License

MIT
