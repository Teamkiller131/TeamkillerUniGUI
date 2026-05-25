# Changelog

## v0.3.0 (2026-05-25)

### Added
- **widget_gallery**: Showcases all 45 widgets in organized tabs
- **form_demo**: Complete registration form with all field types
- **vcpkg port**: `ports/unigui/vcpkg.json` + `portfile.cmake`
- **Multi-platform CI**: Windows + Linux GitHub Actions matrix
- **Version bump**: 0.2.0 → 0.3.0

## v0.2.3 (2026-05-25)

### Added
- **DX11 HWND Runtime Integration**: `CreateDX11DeviceAndSwapChain()` with D3D11 device/swapchain creation, GLFW Win32 native window handle, App Bootstrap DX11 init path
- **Doxygen config**: `Doxyfile` for API documentation generation
- **GitHub Actions CI**: Windows build + test workflow on push/PR

## v0.2.2 (2026-05-25)

### Added
- **SDL3 + Vulkan backend**: Full Vulkan 1.3 boilerplate encapsulation (Instance, Device, Swapchain, Pipeline, Frame sync, Present loop)
- **SDL3 platform backend**: Window creation, event polling, ImGui SDL3 binding
- **BackendType selection**: `AppConfig::backend` enum (GLFW_GL3 | SDL3_Vulkan)
- **CMake UNIGUI_BACKEND option**: Conditional compilation for GLFW_GL3 vs SDL3_VULKAN
- **8 new widgets**:
  - `CheckBox` — bool toggle with onChange callback
  - `Slider<T>` — Float/Int slider with min/max/format (header-only template)
  - `ProgressBar` — Fraction bar with Normal/Warning/Error state colors
  - `RadioGroup` — Single-select from string options
  - `ComboBox` — Dropdown with BeginCombo/EndCombo + items vector
  - `LineEdit` — Text input with placeholder, validator callback, error state
  - `GroupBox` — Titled frame with child content
  - `TabWidget` — RAII TabBar/TabItem with closable tabs
- `VulkanContext` helper for one-shot Vulkan resource init/destroy
- `PlatformBackend::GetWindowHandle()` virtual method for native window access
- `windows-msvc-sdl3-vulkan-debug/release` CMake presets
- v2 widget demos in hello_unigui example
- 30 new Google Test test cases (104 total)

### Changed
- vcpkg.json: v0.1.0 → v0.2.0, added `sdl3`, `vulkan`, `imgui[sdl3-binding,vulkan-binding]`
- CMakeLists.txt: version bump to 0.2.0, added `UNIGUI_BACKEND` cache variable
- `AppConfig` now includes `BackendType backend` field (defaults to GLFW_GL3)
- `CreateBackend(BackendType)` replaces `CreateDefaultBackend()` for backend routing
- `app.cc` supports both GLFW_GL3 and SDL3_Vulkan init/shutdown paths
- README: updated with v2.0 features, backend selection docs, new widget table

### Fixed
- StyleScope: separate color/var push counters for correct ImGui PopStyleColor/PopStyleVar pairing
- StyleScope move constructor: zero all counters in source object
- GLFWPlatform/OpenGL3Renderer: auto-create ImGui context when needed
- Vulkan backend: use `PipelineInfoMain` struct for ImGui Vulkan Init (API changed 2025/09/26)
- Vulkan device: SDL3 `SDL_Vulkan_GetInstanceExtensions` API signature change
- Widget tests: added `DisplaySize` + `Fonts->Build()` to prevent "font atlas not built" crashes

## v0.1.0 (2026-05-25)

### Added
- **GLFW + OpenGL3 backend**: Full window/input + rendering encapsulation
- **6 widgets**: Window, Panel, Form, Button, Label, WidgetBase
- **Dark theme engine**: 53-color Discord/Linear-inspired preset, `StyleScope` RAII, DPI scaling
- **App Bootstrap**: `Init/Shutdown/NewFrame/Render/Run` dual API
- **Core types**: `Context` (singleton), `ErrorCode` enum, `Result<T>` 
- **Backend abstraction**: `PlatformBackend` / `RendererBackend` pure virtual interfaces, PIMPL for backend code
- **Docking** enabled (ImGui docking branch + multi-viewport config)
- CMake 3.31+, Ninja, vcpkg manifest mode, 4 platform presets
- Google Test TDD with 74 test cases
- `hello_unigui` example application
- Dependencies: `imgui` v1.92.8 (docking, freetype, glfw+opengl3 bindings), `glfw3`, `glad`, `freetype`, `gtest`
