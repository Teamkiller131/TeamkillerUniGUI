# Changelog

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
