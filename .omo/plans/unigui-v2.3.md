# TeamkillerUniGUI v2.3 — Completion + Engineering + New Components

## TL;DR

> **Quick Summary**: Complete v2.2 remaining work (DX11 HWND, vulkan_triangle, docs), add engineering infrastructure (Doxygen, CI, examples), implement 5 new components (DatePicker, Image, LoadingIndicator, Notification, Hyperlink), finish Metal backend groundwork.
> 
> **Estimated Effort**: Medium (15 tasks)
> **Test Target**: ≥150 tests (138 + ≥12 new)

---

## Phase 1: Completion (3 tasks)

- [ ] 1. DX11 HWND Runtime Integration
  **What**: Add `CreateDX11DeviceAndSwapChain(HWND, width, height)` to `dx11_renderer.cc`. Wire up GLFW `glfwGetWin32Window()` for DX11 init path in `app.cc` when `BackendType::DX11`.
  **Agent**: `deep`
  **QA**: `cmake --preset windows-msvc-debug && cmake --build` → DX11 compiles, `UNIGUI_HAS_DX11` verified

- [ ] 2. vulkan_triangle Example
  **What**: Create `examples/vulkan_triangle/main.cc` — minimal Vulkan demo. SDL3 window → clear → triangle via ImDrawList → present 10 frames. `--frames N` flag.
  **Agent**: `deep`
  **QA**: `cmake --preset windows-msvc-sdl3-vulkan-debug && cmake --build` → example compiles

- [ ] 3. README + CHANGELOG v2.2
  **What**: Update README with 32 widget table, v2.2 features, CHANGELOG v2.2 entry.
  **Agent**: `writing`
  **QA**: `Select-String -Path "README.md" -Pattern "32 widgets"` → found

## Phase 2: Engineering (4 tasks)

- [ ] 4. Doxygen API Documentation
  **What**: Add `Doxyfile`, document all public APIs (32 widget classes, BackendType, AppConfig, ThemeConfig). Generate `docs/` HTML.
  **Agent**: `writing`
  **QA**: `doxygen Doxyfile` → exit 0, docs/index.html exists

- [ ] 5. GitHub Actions CI
  **What**: `.github/workflows/build.yml` — Windows MSVC build + test on push/PR. Matrix: GLFW_GL3 debug/release.
  **Agent**: `quick`
  **QA**: Push triggers CI run, verify build passes

- [ ] 6. More Examples
  **What**: `examples/form_demo` (Form with all field types), `examples/widget_gallery` (all 32 widgets showcase).
  **Agent**: `unspecified-high`
  **QA**: Both examples compile and render with `--frames 10 --headless`

- [ ] 7. vcpkg Port Preparation
  **What**: Create `ports/unigui/vcpkg.json` and `portfile.cmake` for vcpkg registry submission.
  **Agent**: `quick`
  **QA**: `vcpkg install unigui` from local overlay works

## Phase 3: New Components (5 tasks, parallel)

- [ ] 8. DatePicker Widget
  **What**: `DatePicker` — year/month/day selection. Wraps ImGui date widgets or custom calendar popup.
  **QA**: `ctest -R DatePicker` → 2 tests pass

- [ ] 9. Image Widget
  **What**: `Image` — texture display with optional scaling. `SetTextureID()`, `SetSize()`.
  **QA**: `ctest -R ImageTest` → 2 tests

- [ ] 10. LoadingIndicator Widget
  **What**: `LoadingIndicator` — spinning animation or progress circle. `SetActive(bool)`, `SetRadius()`.
  **QA**: `ctest -R LoadingIndicator` → 2 tests

- [ ] 11. Notification/Toast Widget
  **What**: `Notification` — popup toast message. `Show(title, message, duration)`, auto-dismiss, queue.
  **QA**: `ctest -R Notification` → 2 tests

- [ ] 12. Hyperlink Widget
  **What**: `Hyperlink` — clickable link text. `SetURL()`, `SetLabel()`. Opens browser on click.
  **QA**: `ctest -R Hyperlink` → 2 tests

## Phase 4: Backend Completion (2 tasks)

- [ ] 13. Metal Backend Structure
  **What**: `src/backend/metal_renderer.mm` — Objective-C++ skeleton with `#ifdef __APPLE__`. MTKView + MTLDevice setup. Compiles only on macOS.
  **QA**: macOS: `cmake --build` compiles metal_renderer.mm

- [ ] 14. BackendType Documentation
  **What**: Document all 4 backend types, their platform requirements, and selection guide in README.
  **QA**: `Select-String -Path "README.md" -Pattern "Backend Selection Guide"` → found

## Wave FINAL

- [ ] F1-F4: Review agents (Oracle, Code Quality, QA, Scope)

---

## Scope Boundaries

### INCLUDE (v2.3)
- DX11 HWND runtime, vulkan_triangle example, README/CHANGELOG
- Doxygen, GitHub Actions CI, 2 new examples, vcpkg port prep
- 5 new widgets, Metal .mm structure

### EXCLUDE (v3+)
- Multi-viewport, WebGPU, Emscripten, DX12, ImPlot, NodeEditor, Hot-reload
