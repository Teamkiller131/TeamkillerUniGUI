# Changelog

## v3.2.0 (2026-05-26) — Cross-Platform + Polish

### Added
- **Linux**: Full compilation support (Fedora 43, GCC 15.2, 225/225 targets, 236/244 tests). `cmake/embed_font.py` cross-platform font embedding, platform-aware CJK font paths (Windows/MSYH, macOS/PingFang, Linux/NotoSansCJK).
- **macOS**: Metal backend ObjC++ implementation (MTLDevice, CommandQueue, ImGui_ImplMetal_Init/Shutdown/Render). CMake `-fobjc-arc` + Metal.framework/QuartzCore.framework linkage. Platform-aware CJK fallback.
- **Emscripten/Web**: Full platform backend (canvas sizing, emscripten_set_main_loop, HTML shell template with spinner and Module bridge).
- **CI/CD**: GitHub Actions cross-platform matrix — Windows (MSVC), Linux (Ubuntu+GCC), macOS (Clang).
- **Card**: `SetBorderColor(ImU32)`, `SetBorderRadius(float)`, proper padding via WindowPadding.
- **SkeletonScreen**: built-in shimmer animation via `SetShimmer(bool, speed)`.
- **ThemeRegistry**: `SetOnChange(std::function<void(std::string)>)` callback for theme switch notifications.

### Fixed
- CMake minimum lowered 3.31→3.26 for Rocky/Fedora compatibility.
- `vcpkg.json`: DX11/DX12 bindings split to Windows-only platform.
- `app.cc::NewFrame()`: DX11 resize code wrapped in `#ifdef UNIGUI_HAS_DX11`.
- `std::find` → `#include <algorithm>` added (4 files) for GCC 15 strictness.
- `webgpu/emscripten/metal` backend stubs compiled on all platforms (not just WIN32).
- `config/database/ipc` headers guarded by `UNIGUI_HAS_*` preprocessor defines in `unigui.h`.

### Changed
- **AnimationState** docs: `progress` = target, `Update(dt)` return = eased current value.
- **FontManager::Build()** doc warning: rebuilding atlas invalidates ImFont* pointers.

## v3.1.0 (2026-05-26) — Stability

### Fixed
- **EventBus exit crash**: `~Bus()` destructor now calls `Shutdown()` to join worker thread (244/244 tests pass)
- **Toast triple-bug**: double-animation, `Hide()` deadlock, z-order burying via per-message windows + `BringWindowToDisplayFront`
- **CSS @media** evaluates `min-width`/`max-width`/`min-height`/`prefers-color-scheme`

## v3.0.0 (2026-05-26) — UI Beautification

### Added
- **fx/easing**: 10 easing curves (linear, quad, cubic, expo, elastic, bounce) + CSS aliases + `ParseEasing()`
- **fx/effects**: `ShadowEffect` (multi-pass blur), `GlowEffect` (radial rings), `BlurEffect` (glass morphism), `GradientBrush` (horizontal/vertical/multi-stop), `Effects` factory
- **fx/animation**: `AnimationState` with Play/Stop/Loop/PingPong/`onComplete`, `AnimationManager` singleton
- **fx/transition**: `Fade`, `SlideIn`, `Scale`, `CrossFade`, `PageSwitch`, `Appear`, `Disappear` (header-only)
- **CSS Engine v2**: 16 → 70 properties, `linear-gradient()` parser, `transition` shorthand, `:active`/`:focus`/`:disabled`/`:first-child` pseudo-classes, `@media` block detection
- **10 built-in themes**: Material Dark/Light, Fluent Dark/Light, Dracula, Nord, Gruvbox, Catppuccin Mocha, Solarized Dark/Light, TokyoNight, OneDark, Everforest
- **ThemeRegistry**: `Register`/`Get`/`List`/`Apply` + `RegisterAllThemes()` auto-init
- **New widgets**: `Card` (Elevated/Outlined/Filled), `Shimmer` (animated sweep), `Badge` (Dot/Count/Label), `SkeletonScreen` (placeholder blocks), `HeroSection` (gradient banner + CTA), `GradientText` (per-char interpolation)
- **Widget polish**: Button animated hover, Toast eased fade-in, ProgressBar animated fill, ToggleSwitch alpha pulse, TabWidget crossfade, Panel shadow via `WidgetBase::SetShadow()`
- **Demos**: `v3_overview` (all features), `theme_demo` (auto-cycle), `widget_gallery` updated

### Changed
- `Toast` now renders in `Render()` (on top of all windows), per-message independent ImGui windows

## v2.9.0 (2026-05-26) — De-v2 Namespace

### Breaking
- **All `v2::` namespace removed**: `unigui::v2::EventBus` → `unigui::events::Bus`, etc.
- Headers moved: `include/unigui/v2/*` → `include/unigui/<module>/*`

## v2.8.0 (2026-05-26) — Modular CMake

### Added
- 20 configurable modules via `-DUNIGUI_MODULE_*=ON/OFF`
- 3 presets: recommended (default), minimal (~200 targets), full (~470 targets)
- Conditional deps: SQLite3, ZeroMQ

## v1.10.0 (2026-05-26) — TrayIcon Polish

### Added
- TrayIcon::UpdateTooltip(title) — dynamic tooltip via NIM_MODIFY
- TrayMenuItem submenus — children field for recursive submenus
- TrayMenuItem::isSeparator — menu separators (MF_SEPARATOR)
- ShowNotification(title, msg, NotifyType) — Info/Warning/Error icons
- Toast dismiss callback — Show(msg, type, dur, onDismiss)
- PasswordInput demo in widget_gallery (strength indicator)

### Changed
- Version: 1.9.0 → 1.10.0
- Tests: 202/202 pass

### Fixed
- **TrayIcon**: proper right-click menu via TrackPopupMenu, icon resource ID parameter, notification title/msg param pass-through.
- **GetNativeWindowHandle()**: public API in app.h, returns HWND on Windows.
- **GLFW headers**: auto-included in unigui.h (glfw3.h + glfw3native.h), no user include needed.
- **Toast::SetPosition(anchor, x, y)**: control notification position (top-left/right, bottom-left/right).

### Changed
- vcpkg version synced with CMake version (1.9.0)
- Version: 1.8.0 → 1.9.0
- Tests: 202/202 pass

## v1.8.0 (2026-05-26) — 1.x Series Final

### Finalized
- **API Freeze**: Public API locked for 1.x → 2.0 migration. No breaking changes in 1.x series.
- **CHANGELOG unified**: All versions from v0.1.0 through v1.8.0 in a single document.
- **202 tests**: Full regression suite, 100% pass rate maintained.
- **9 releases**: v1.0.0 through v1.8.0 delivered in a single session.

### What Changed From v0.1.0
- Windows backend: OpenGL → **DX11** (AMD GPU stable)
- Font: system-dependent → **JetBrains Mono Nerd Font embedded**
- Widgets: 6 → **55**
- Tests: 74 → **202**
- Backends: 1 → **7 (4 runtime-ready)**
- Enterprise: i18n, Settings persistence, UndoStack, serialization
- Performance: VirtualList 10k items < 100ms

## v1.7.0 (2026-05-26) — Performance Optimization
- Extended benchmarks: 100 buttons, 100 labels, VirtualList 10k, Form 20 fields
- VirtualList: 10,000 items < 100ms (ImGuiListClipper)
- Tests: 200 → 202

## v1.6.0 (2026-05-26) — Advanced Features
- Layout::HBox/VBox/BeginHSplit/EndHSplit declarative helpers
- Window::SetDropCallback for file drag-drop
- Clipboard::Copy/Paste wrapping ImGui clipboard
- Animate::FadeIn/SlideIn/Lerp/FadeScope animation system

## v1.5.1 (2026-05-26) — System Tray + Widget Docs
- TrayIcon: Shell_NotifyIcon (Windows), menu + notifications
- README widget quick-reference table

## v1.5.0 (2026-05-26) — User Feedback Fixes
- ImPlot::CreateContext/DestroyContext auto-managed
- DX12 renderer conditional compile
- GLFW_EXPOSE_NATIVE_WIN32 handled internally
- GetNativeWindowHandle() unified API

## v1.4.0 (2026-05-26) — i18n & Settings
- Locale::LoadFromFile(JSON) + built-in en_US/zh_CN/ja_JP
- Settings::EnableAutoSave + MRU (AddRecentFile/GetRecentFiles)
- hello_unigui i18n switch demo

## v1.3.0 (2026-05-26) — Platform Hardening
- Render integration test (manual, GPU required)
- Fuzz test: 100 random widget iterations
- Font smoke test: embedded font loading

## v1.2.0 (2026-05-26) — Widget Ecosystem
- VirtualList: 100k+ entries with ImGuiListClipper
- MultiCombo: multi-select checkboxes + preview
- PropertyGrid: two-column property editor
- SearchBox: filtered dropdown suggestions
- Toast: singleton Info/Success/Warn/Error notifications
- PasswordInput: strength indicator (0-4)
- Wizard: multi-step with Next/Previous/Finish

## v1.1.0 (2026-05-26) — Embedded Fonts
- JetBrains Mono Nerd Font embedded via CMake pipeline
- LoadDefaultFont: AddFontFromMemoryTTF
- CJK merge: optional system font merge
- DPI update: FontGlobalScale instead of reload

## v1.0.0 (2026-05-26) — First Stable Release
- DX11 backend: Windows default, AMD-stable
- DPI auto-scaling: GetDpiForWindow
- CJK font support: Microsoft YaHei merge
- Window resize: DX11 swapchain resize
- Auto-wrap text: Panel default
- Popup input priority: Window NoInputs
- spdlog structured logging
- 48+ widgets, 186 tests

## v0.5.0 (2026-05-25) — Enterprise
- Locale i18n, Settings INI, UndoStack
- Form/Table/Theme serialization
- Widget accessibility & size constraints
- Performance benchmarks

## v0.4.0 (2026-05-25)
- NodeEditor groundwork
- RichText, ImageButton, Markdown widgets
- Undo/Redo for LineEdit/MultiLine
- Form validation, ComboBox icons, Table columns
- plot_demo example

## v0.3.2 (2026-05-25)
- DX12 renderer, WebGPU/Metal/Emscripten stubs

## v0.3.1 (2026-05-25)
- Multi-Viewport, DockSpace, ContextMenu, DragDrop, ShortcutManager

## v0.3.0 (2026-05-25)
- widget_gallery, form_demo, CI matrix, vcpkg port

## v0.2.3 (2026-05-25)
- DX11 HWND runtime, Doxygen, GitHub Actions CI

## v0.2.2 (2026-05-25)
- SDL3+Vulkan backend, 8 widgets (CheckBox through TabWidget)

## v0.1.0 (2026-05-25)
- GLFW+OpenGL3, Dark theme, 6 widgets, App Bootstrap, 74 tests
