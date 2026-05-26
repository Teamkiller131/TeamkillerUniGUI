# Changelog

## v1.1.0 (2026-05-26) — Embedded Fonts

### Added
- **Embedded Font**: JetBrains Mono Nerd Font (~2.4MB) embedded directly in the library binary via CMake code generation pipeline (`cmake/FontEmbed.cmake`). No system font dependency for the primary UI font. Nerd Font icons included (Powerline, Devicons, etc.).
- **CMake Font Pipeline**: `embed_font(target font_file header var_name)` — converts TTF to C array at build time using PowerShell (Windows) or `xxd` (Linux/macOS).
- **Font docs**: README section on embedded fonts, how to load custom fonts via `ThemeConfig::font_path`.

### Changed
- `LoadDefaultFont` simplified: embedded font always used as primary, CJK merge from system fonts kept as optional enhancement.
- Removed system font fallback logic for the base font (segoeui.ttf/arial.ttf no longer needed).

### Fixed
- Cross-platform font consistency: identical rendering on all platforms regardless of system fonts.
- Nerd Font icons now render correctly (e.g., `` `` ``).

## v1.0.0 (2026-05-26) — First Stable Release
- DX11 Backend default on Windows, OpenGL 3.3 on Linux/macOS
- DPI auto-scaling, CJK font support, window resize
- Auto-wrap text, popup input priority, spdlog logging
- 48+ widgets, 186 tests, 4 example apps

# Changelog

## v0.5.0 (2026-05-25)

### Added
- **Locale/i18n System**: `unigui::Locale` — translation table with `Set()`, `Tr()`, `SetCurrent("zh_CN")`. Supports multiple locale codes.
- **Settings Persistence**: `unigui::Settings` singleton — INI key-value store with `Set()/Get()`, typed helpers (`SetInt/GetInt`, `SetFloat/GetFloat`, `SetBool/GetBool`), and `Save()/Load()` file I/O.
- **Global UndoStack**: `unigui::UndoStack<Action>` — command-pattern undo/redo template. `Execute()`, `Undo()`, `Redo()`, 100-depth default.
- **Form Serialization**: `Form::Serialize()` / `Form::Deserialize(json)` — JSON string import/export of form state.
- **Table CSV Import/Export**: `Table::ExportCSV()` / `Table::ImportCSV(csv)` — full CSV with quote escaping.
- **Theme JSON Export**: `ExportThemeJSON()` / `ImportThemeJSON(json)` — save/load all 53 ImGui colors.
- **Accessibility Hints**: `Widget::SetAccessibleName()` / `SetAccessibleDescription()`.
- **Widget Size Constraints**: `Widget::SetMinSize(w,h)` / `SetMaxSize(w,h)`.
- **Performance Benchmarks**: `tests/bench/bench_test.cc` — frame time measurement for 100-button/100-label renders.
- **Version bump**: 0.4.0 → 0.5.0

### Changed
- widget_base: added `minSize_`, `maxSize_`, `accessibleName_`, `accessibleDesc_` members
- unigui.h: includes `locale.h`, `settings.h`, `undo_stack.h`
- Tests: 173 → 187 (+14 new tests)

### Added
- **NodeEditor Groundwork**: `include/unigui/ext/node_editor.h` — RAII wrapper for `ax::NodeEditor` (Begin/End/Node/Pin/Link). Requires `imgui-node-editor` vcpkg dependency.
- **RichText Widget**: `RichText` — formatted text display with bold/italic/color spans. `SetSpans()` + `AddSpan()` API.
- **ImageButton Widget**: `ImageButton` — image + label button. `SetImage(textureID, w, h)` + `SetLabel()`.
- **Markdown Widget**: `Markdown` — inline markdown renderer supporting `#` headers, `**bold**`, `*italic*`, `` `code` ``, `-` bullets, `---` hr, `[links](url)`. No external dependency.
- **Undo/Redo**: Added to LineEdit and MultiLine (`Undo()`, `Redo()`, `CanUndo()`, `CanRedo()`). 50-level undo stack.
- **Form Validation**: `SetFieldValidatorRegex(name, pattern, error)` + `SetFieldMinMax(name, min, max)`.
- **ComboBox Icons**: `SetItemIcon(index, textureID)` — per-item icon rendering in dropdown.
- **Table Column Widths**: `SaveColumnWidths()` / `RestoreColumnWidths()` for persisting user-adjusted widths.
- **plot_demo Example**: `examples/plot_demo/` — line/bar/scatter plot demo using ImPlot.
- **Version bump**: 0.3.2 → 0.4.0

### Changed
- LineEdit/MultiLine now maintain undo stacks (50-level depth cap)
- Form::Validate() checks regex and min/max validators in addition to required
- ComboBox renders icons before item text when set
- tests/CMakeLists.txt adds richtext, imagebutton, markdown test targets

### Added
- **DX12 Renderer**: Full DX12 backend — device/command-queue/swapchain/descriptor-heaps/fence via `CreateDX12DeviceAndSwapChain()`. Wraps `imgui_impl_dx12.h`. Runtime-ready on Windows.
- **WebGPU Renderer**: `webgpu_renderer.cc` — Dawn/WGPU stub (full impl requires Dawn library).
- **Metal Renderer**: Updated `metal_renderer.cc` — macOS-only stub (full `.mm` impl requires Objective-C++ on macOS).
- **Emscripten Platform**: `emscripten_platform.cc` — Web/HTML5 stub with `__EMSCRIPTEN__` guard (full impl requires Emscripten SDK).
- **BackendType**: `DX12`, `WebGPU`, `Emscripten` entries in enum + backend_factory routing.
- **`UNIGUI_HAS_DX12`** compile definition for DX12 backend conditional compilation.
- **Backend Comparison Table**: README matrix for all 7 backends (platform, API, status, features).
- **DX12 vcpkg**: `imgui[dx12-binding]` + `d3d12` link library.

## v0.3.1 (2026-05-25)

### Added
- **Multi-Viewport**: `ImGuiConfigFlags_ViewportsEnable` — OS-level window dragging
- **DockSpace**: Full docking layout widget
- **ContextMenu**: Right-click popup menu (`ContextMenu::Show`)
- **Drag-Drop API**: `BeginDragSource`/`AcceptDragDrop` template helpers
- **ShortcutManager**: Global keyboard shortcut registration (`Ctrl+S`, etc.)
- **Widget Tooltip**: `Widget::SetTooltip()` on all widgets
- **Widget Focus**: `SetFocused()`, `IsFocused()`, `SetNextFocused()`

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
