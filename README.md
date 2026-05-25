# TeamkillerUniGUI — Modern Dear ImGui C++ Wrapper

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![CMake](https://img.shields.io/badge/CMake-3.31%2B-green)](https://cmake.org/)
[![vcpkg](https://img.shields.io/badge/vcpkg-managed-orange)](https://vcpkg.io/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)]()
[![Version](https://img.shields.io/badge/version-0.2.0-blueviolet)]()

A C++23 Dear ImGui wrapper library providing a unified dark theme engine and high-level widget components. Supports GLFW+OpenGL3 and SDL3+Vulkan backends.

## Quick Start

```bash
git clone https://xbw-nas.iepose.cn/Teamkiller131/TeamkillerUniGUI.git
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
    ├── Theme Engine (53-color dark theme, StyleScope RAII)
    ├── Widget Library (14 widgets: v1 + v2)
    ├── Backend Abstraction (PlatformBackend / RendererBackend interfaces)
    │   ├── GLFW + OpenGL 3.3 (default)
    │   └── SDL3 + Vulkan 1.3 (v2.0)
    └── App Bootstrap (Init / Run / NewFrame / Render)
    ↓
ImGui (v1.92.8, docking)
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

### v1 Widgets
| Widget | Description |
|--------|-------------|
| `Window` | Top-level window with docking, menu bar, child panels |
| `Panel` | Collapsible content panel with callback |
| `Form` | Input form with text/checkbox fields and validation |
| `Button` | Styled button with enabled/disabled states |
| `Label` | Static/updatable text label |

### v2.0 Widgets (new!)
| Widget | Description |
|--------|-------------|
| `CheckBox` | Boolean toggle with onChange callback |
| `Slider<T>` | Float/Int slider with min/max/format (header-only) |
| `ProgressBar` | Fraction bar with Normal/Warning/Error states |
| `RadioGroup` | Single-select from string options |
| `ComboBox` | Dropdown with BeginCombo/EndCombo + Selectable |
| `LineEdit` | Text input with placeholder, validator, error state |
| `GroupBox` | Titled frame with child content callback |
| `TabWidget` | RAII TabBar/TabItem with closable tabs |

## Backend Selection

```cpp
// CMake option
cmake -DUNIGUI_BACKEND=SDL3_VULKAN ...

// Runtime
cfg.backend = BackendType::GLFW_GL3;     // GLFW + OpenGL 3.3
cfg.backend = BackendType::SDL3_Vulkan; // SDL3 + Vulkan 1.3
```

## Platform Notes

- **Windows**: Primary target. MSVC 19.40+ via Visual Studio 2022.
- **Linux**: X11/Wayland via GLFW or SDL3. GCC 14+ or Clang 18+.
- **macOS**: OpenGL deprecated by Apple (capped at 4.1). Vulkan via MoltenVK.

## Dependencies (vcpkg)

- `imgui` (v1.92.8, docking+f reetype+glfw+sdl3+vulkan)
- `glfw3`, `sdl3`, `vulkan`, `glad`, `freetype`, `gtest`

## License

MIT
