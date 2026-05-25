# TeamkillerUniGUI v2.0 — SDL3 + Vulkan + 8 New Widgets

## TL;DR

> **Quick Summary**: Upgrade unigui from GLFW+OpenGL3 to SDL3+Vulkan full backends, add 8 high-value widgets (TabWidget, LineEdit, CheckBox, RadioGroup, ComboBox, Slider, ProgressBar, GroupBox), introduce `BackendType` selection in `AppConfig`, and add conditional CMake compilation for Vulkan.
> 
> **Deliverables**:
> - SDL3 platform backend (`src/backend/sdl3_platform.cc`)
> - Complete Vulkan renderer backend with full boilerplate encapsulation (`src/backend/vulkan_renderer.cc`)
> - 8 new widgets with unit tests
> - `AppConfig::backend` enum for backend selection
> - CMake `UNIGUI_BACKEND` option (GLFW_GL3 | SDL3_VULKAN)
> - `vulkan_triangle` example (Vulkan demo)
> - Updated README with v2.0 API
> 
> **Estimated Effort**: Large (20 implementation tasks)
> **Parallel Execution**: YES — 4 waves, max 6 concurrent
> **Critical Path**: Task 1 → Task 6 → Task 7 → Task 16 → FINAL

---

## Context

### v1 Baseline (delivered)
- GLFW+OpenGL3 backend, 6 widgets (Window/Panel/Form/Button/Label/WidgetBase), 74 tests
- C++23, CMake+Ninja+vcpkg, TDD with Google Test
- Dark theme (53 colors), StyleScope RAII, App Bootstrap

### v2.0 Goals
- **Backend upgrade**: Replace GLFW+OpenGL3 with SDL3+Vulkan as primary backend; keep GLFW+OpenGL3 as fallback
- **Complete Vulkan encapsulation**: Users call `unigui::Init({.backend=BackendType::SDL3_Vulkan})` — all instance/device/swapchain/pipeline/sync handled internally
- **8 new widgets**: TabWidget (RAII tabs), LineEdit (validation + std::string), CheckBox, RadioGroup, ComboBox (vector data binding), Slider (format + range), ProgressBar (state colors), GroupBox (titled frame)
- **Backend selection**: `AppConfig::backend` enum, `UNIGUI_BACKEND` CMake option
- **Deferred to v2.1+**: TreeView, ListView, Dialog, MenuBar, StatusBar, ToolBar, Table, ColorPicker, DirectX backends, Metal backend, Light theme

---

## Work Objectives

### Core Objective
Upgrade unigui to support SDL3+Vulkan as a full backend with complete Vulkan boilerplate encapsulation, add 8 Tier 1 widgets, and introduce compile-time/runtime backend selection.

### Definition of Done
- [ ] `cmake --preset windows-msvc-release -DUNIGUI_BACKEND=SDL3_VULKAN` → exit 0, produces `unigui.lib`
- [ ] `ctest --preset windows-msvc-release` → ≥100 tests pass (74 existing + ≥26 new)
- [ ] `vulkan_triangle.exe --frames 10` → renders 10 frames with clear triangle, no validation errors
- [ ] `AppConfig::backend = BackendType::GLFW_GL3` still works (backward compatible)
- [ ] All 8 new widgets have unit tests

### Must Have
- SDL3 platform backend working on Windows (Linux deferred to v2.1)
- Full Vulkan renderer backend (instance/device/swapchain/pipeline/sync/frame loop)
- 8 new widgets: TabWidget, LineEdit, CheckBox, RadioGroup, ComboBox, Slider, ProgressBar, GroupBox
- `BackendType` enum in `AppConfig` (GLFW_GL3 | SDL3_VULKAN)
- CMake `UNIGUI_BACKEND` option
- Backward compatibility with v1 API
- Azure-style Vulkan boilerplate (DRIVE naming convention for clarity)

### Must NOT Have
- NO Light theme (v3)
- NO DirectX backends (v2.1)
- NO Metal backend (v2.1)
- NO TreeView/ListView/Dialog/MenuBar/StatusBar/ToolBar/Table/ColorPicker (v2.1)
- NO breaking changes to v1 public API
- NO multi-viewport support
- NO god-files: max 300 lines per Vulkan source file

---

## Verification Strategy

### Test Decision
- **Infrastructure exists**: YES (Google Test from v1, 74 tests)
- **Automated tests**: TDD with Google Test
- **Framework**: Google Test (gtest via vcpkg)
- **Testable**: Widget state logic, validation, data binding, backend factory selection
- **Agent QA only**: Visual rendering, Vulkan validation layer errors

### QA Policy
Every task includes agent-executed QA scenarios. Evidence in `.omo/evidence/v2-task-{N}-*.log`.

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Foundation — 6 tasks):
├── Task 1: vcpkg + CMake — add SDL3 & Vulkan features [quick]
├── Task 2: BackendType enum + AppConfig refactor [quick]
├── Task 3: SDL3 platform backend [deep]
├── Task 4: Vulkan backend — Instance + Device + Queues [deep]
├── Task 5: Vulkan backend — Swapchain + RenderPass + Framebuffers [deep]
└── Task 6: Vulkan backend — Pipeline + CommandPool + Sync + Frame Loop [deep]

Wave 2 (Widgets — 8 parallel tasks, depends on Wave 1):
├── Task 7:  CheckBox widget [quick]
├── Task 8:  Slider widget [quick]
├── Task 9:  ProgressBar widget [quick]
├── Task 10: RadioGroup widget [quick]
├── Task 11: ComboBox widget [unspecified-high]
├── Task 12: LineEdit widget [unspecified-high]
├── Task 13: GroupBox widget [unspecified-high]
└── Task 14: TabWidget widget [unspecified-high]

Wave 3 (Integration — 4 tasks, depends on Waves 1+2):
├── Task 15: BackendFactory refactor — BackendType routing [deep]
├── Task 16: App Bootstrap update for Vulkan init path [deep]
├── Task 17: Update hello_unigui for v2 widgets [unspecified-high]
└── Task 18: vulkan_triangle example (Vulkan demo) [deep]

Wave 4 (Docs — 2 parallel tasks):
├── Task 19: Update README for v2.0 [writing]
└── Task 20: v2.0 CHANGELOG / migration notes [writing]

Wave FINAL (4 parallel reviews):
├── F1: Plan Compliance Audit [oracle]
├── F2: Code Quality Review [unspecified-high]
├── F3: Real Manual QA [unspecified-high]
└── F4: Scope Fidelity Check [deep]
```

### Dependency Matrix

| Task | Depends On | Blocks | Wave |
|------|-----------|--------|------|
| 1 | - | 2, 3, 4 | 1 |
| 2 | 1 | 15 | 1 |
| 3 | 1 | 15, 16 | 1 |
| 4 | 1 | 5 | 1 |
| 5 | 4 | 6 | 1 |
| 6 | 5 | 15, 16, 18 | 1 |
| 7-14 | 1, 2 | - | 2 |
| 15 | 3, 6, 2 | 16 | 3 |
| 16 | 15, 14 | 17, 18 | 3 |
| 17 | 16 | - | 3 |
| 18 | 6, 16 | - | 3 |
| 19-20 | - | - | 4 |
| F1-F4 | ALL | - | FINAL |

**Critical Path**: 1 → 4 → 5 → 6 → 15 → 16 → 17 → FINAL

---

## TODOs

> Implementation + Test = ONE Task. TDD: RED test first, then GREEN implementation.

- [ ] 1. vcpkg + CMake — SDL3 & Vulkan Features

  **What to do**:
  - Update `vcpkg.json`: add `imgui[sdl3-binding,vulkan-binding]`, `sdl3`, `vulkan`, `vulkan-headers`, `vulkan-loader`
  - Remove `imgui[glfw-binding]` if SDL3 replaces GLFW as primary (GLFW kept for backward compat)
  - Add `UNIGUI_BACKEND` CMake option: `set(UNIGUI_BACKEND "GLFW_GL3" CACHE STRING "Backend: GLFW_GL3 or SDL3_VULKAN")`
  - Add `find_package(SDL3 CONFIG)` and `find_package(Vulkan REQUIRED)` conditional on `UNIGUI_BACKEND`
  - Add `src/backend/sdl3_platform.cc` and `src/backend/vulkan_renderer.cc` to target_sources conditionally
  - Add SDL3 and Vulkan to `target_link_libraries` conditionally
  - Update `CMakePresets.json`: add `windows-msvc-sdl3-vulkan-debug` preset

  **Must NOT do**: Remove GLFW backend — keep backward compat

  **Recommended Agent Profile**: `quick`

  **Acceptance Criteria**:
  - [ ] `cmake --preset windows-msvc-debug -DUNIGUI_BACKEND=SDL3_VULKAN` configures with SDL3+Vulkan vcpkg packages
  - [ ] `cmake --preset windows-msvc-debug` (no arg, default=GLFW_GL3) still works

  **QA**:
  ```
  Scenario: SDL3+Vulkan configure
    Steps: cmake --preset windows-msvc-debug -DUNIGUI_BACKEND=SDL3_VULKAN
    Expected: exit 0, vcpkg installs sdl3+vulkan+imgui[sdl3-binding,vulkan-binding]
    Evidence: .omo/evidence/v2-task-1-configure.log
  ```

- [ ] 2. BackendType Enum + AppConfig Refactor

  **What to do**:
  - Add `enum class BackendType { GLFW_GL3, SDL3_Vulkan }` to `backend_types.h`
  - Add `BackendType backend = BackendType::GLFW_GL3` to `AppConfig`
  - Backward compat: `Init({})` defaults to GLFW_GL3
  - Update `CreateDefaultBackend()` to `CreateBackend(BackendType)`
  - Write RED test: `AppConfig_DefaultBackend_IsGLFW_GL3`
  - Write GREEN: update `app.cc` to route based on `config.backend`

  **Must NOT do**: Break v1 `Init(AppConfig{})` behavior

  **Recommended Agent Profile**: `quick`

  **Acceptance Criteria**:
  - [ ] `BackendType::GLFW_GL3` and `BackendType::SDL3_Vulkan` compile
  - [ ] `AppConfig{}.backend == BackendType::GLFW_GL3`
  - [ ] Existing tests still pass with default backend

  **QA**:
  ```
  Scenario: Default backend unchanged
    Steps: ctest --preset windows-msvc-debug -R "AppConfig"
    Expected: all tests pass, default backend is GLFW_GL3
  ```

- [ ] 3. SDL3 Platform Backend

  **What to do**:
  - RED: Write `tests/backend/sdl3_platform_test.cc` — Init, Shutdown, NewFrame, PollEvents, ShouldClose
  - GREEN: `src/backend/sdl3_platform.cc`:
    - `SDL3Platform : PlatformBackend`
    - Init: `SDL_Init(SDL_INIT_VIDEO)`, create window, `ImGui_ImplSDL3_InitForVulkan(window)`
    - NewFrame: `ImGui_ImplSDL3_NewFrame()`
    - PollEvents: `SDL_PollEvent()` loop
    - ShouldClose: window close event flag
    - Shutdown: `ImGui_ImplSDL3_Shutdown()`, `SDL_DestroyWindow()`, `SDL_Quit()`
  - Add `CreateSDL3Platform()` to `backend_factory.h`

  **Must NOT do**: Mix SDL2 code — SDL3 only

  **Recommended Agent Profile**: `deep`

  **Acceptance Criteria**:
  - [ ] `tests/backend/sdl3_platform_test.cc` — 4 tests pass (Init/Shutdown/PollEvents/ShouldClose)
  - [ ] SDL3 creates hidden window for testing

  **QA**:
  ```
  Scenario: SDL3 platform lifecycle
    Steps: ctest --preset windows-msvc-debug -R "SDL3Platform"
    Expected: 4 tests pass
  ```

- [ ] 4. Vulkan Backend — Instance + Device + Queues

  **What to do**:
  - RED: `tests/backend/vulkan_instance_test.cc` — create instance, pick GPU, create device
  - GREEN: `src/backend/vulkan_device.cc`:
    ```cpp
    struct VulkanDevice {
        VkInstance instance;
        VkPhysicalDevice physical_device;
        VkDevice device;
        uint32_t graphics_family;
        uint32_t present_family;
        VkQueue graphics_queue;
        VkQueue present_queue;
    };
    VulkanDevice CreateVulkanDevice(SDL_Window* window);
    void DestroyVulkanDevice(VulkanDevice& vd);
    ```
  - Create Vulkan instance with validation layers (debug), portable extensions
  - Select suitable physical device (discrete GPU preferred)
  - Create logical device with graphics + present queues
  - Set up `VK_EXT_debug_utils` for validation error callbacks

  **Must NOT do**: Create swapchain or pipeline — Task 5

  **Recommended Agent Profile**: `deep`

  **Acceptance Criteria**:
  - [ ] `CreateVulkanDevice()` returns valid instance/device/queues
  - [ ] Validation layers enabled in debug builds
  - [ ] Test runs without Vulkan validation errors

  **QA**:
  ```
  Scenario: Vulkan device creation
    Steps: ctest --preset windows-msvc-debug -R "VulkanDevice"
    Expected: test passes, no VK_ERROR reported
  ```

- [ ] 5. Vulkan Backend — Swapchain + RenderPass + Framebuffers

  **What to do**:
  - RED: `tests/backend/vulkan_swapchain_test.cc`
  - GREEN: `src/backend/vulkan_swapchain.cc`:
    - `VulkanSwapchain` struct (swapchain handle, images, image views, extent, format)
    - `CreateSwapchain(VkPhysicalDevice, VkDevice, VkSurfaceKHR, extent)` 
    - `RecreateSwapchain()` for window resize
    - `DestroySwapchain()`
    - `VkRenderPass CreateRenderPass(VkDevice, VkFormat)`
    - `std::vector<VkFramebuffer> CreateFramebuffers(VkDevice, renderPass, imageViews, extent)`
    - `DestroyFramebuffers()` / `DestroyRenderPass()`

  **Must NOT do**: Pipeline creation — Task 6

  **Recommended Agent Profile**: `deep`

  **Acceptance Criteria**:
  - [ ] Swapchain created with ≥2 images, correct format
  - [ ] RenderPass with single subpass for ImGui output
  - [ ] Framebuffers created per swapchain image

  **QA**:
  ```
  Scenario: Swapchain lifecycle
    Steps: ctest --preset windows-msvc-debug -R "VulkanSwapchain"
    Expected: create/recreate/destroy without validation errors
  ```

- [ ] 6. Vulkan Backend — Pipeline + CommandPool + Sync + Frame Loop

  **What to do**:
  - GREEN: `src/backend/vulkan_pipeline.cc`:
    - Compile GLSL→SPIR-V shaders (embedded as byte arrays from `shaders/imgui.vert.spv` and `shaders/imgui.frag.spv`)
    - Create descriptor pool + descriptor sets for font texture
    - Create graphics pipeline (blend enabled, no depth, dynamic viewport/scissor)
  - `src/backend/vulkan_frame.cc`:
    - Per-frame resources: command pool, command buffers (2-3), fences, semaphores
    - `BeginFrame()`: acquire next swapchain image, begin command buffer, begin render pass
    - `EndFrame()`: end render pass, end command buffer, submit, present
    - Frame-in-flight synchronization (MAX_FRAMES_IN_FLIGHT = 2)
  - `src/backend/vulkan_renderer.cc`:
    - `VulkanRenderer : RendererBackend`
    - Init: creates all Vulkan resources via SDL3 window
    - RenderDrawData: records ImGui draw commands, submits frame
    - Shutdown: `vkDeviceWaitIdle()`, destroy all resources in reverse order
    - SetClearColor: stores clear value for render pass

  **Must NOT do**: Exceed 300 lines per source file

  **Recommended Agent Profile**: `deep`

  **Acceptance Criteria**:
  - [ ] Pipeline compiles and links SPIR-V shaders
  - [ ] Frame loop renders without validation errors
  - [ ] `vkDestroyInstance` called on shutdown (no leaks via validation)

  **QA**:
  ```
  Scenario: Vulkan frame loop
    Steps: run vulkan_triangle --frames 10
    Expected: 10 frames rendered, no validation layer errors
    Evidence: .omo/evidence/v2-task-6-vulkan-frames.log
  ```

- [ ] 7. CheckBox Widget

  **What to do**: RED tests + GREEN `CheckBox` class. Wraps `ImGui::Checkbox()`. State: bool checked, label, onChange callback. Style: dark theme.

  **Agent**: `quick`

- [ ] 8. Slider Widget

  **What to do**: RED tests + GREEN `Slider` class. Wraps `ImGui::SliderFloat/Int()`. State: value, min, max, format string. Style: dark theme.

  **Agent**: `quick`

- [ ] 9. ProgressBar Widget

  **What to do**: RED tests + GREEN `ProgressBar` class. Wraps `ImGui::ProgressBar()`. State: fraction (0.0-1.0), color by state (normal/warning/error), overlay text.

  **Agent**: `quick`

- [ ] 10. RadioGroup Widget

  **What to do**: RED tests + GREEN `RadioGroup` class. Manages N radio buttons, tracks selection index, onChange callback. Wraps `ImGui::RadioButton()`.

  **Agent**: `quick`

- [ ] 11. ComboBox Widget

  **What to do**: RED tests + GREEN `ComboBox` class. Wraps `BeginCombo/EndCombo`. State: `std::vector<std::string>` items, selected index, preview label, searchable flag.

  **Agent**: `unspecified-high`

- [ ] 12. LineEdit Widget

  **What to do**: RED tests + GREEN `LineEdit` class. Wraps `ImGui::InputText()`. State: `std::string` value, placeholder text, validation callback (`std::function<bool(const std::string&)>`), error state, max length.

  **Agent**: `unspecified-high`

- [ ] 13. GroupBox Widget

  **What to do**: RED tests + GREEN `GroupBox` class. Renders titled frame around child content. Wraps `ImGui::BeginGroup/EndGroup` + manual border drawing + `ImGui::GetCursorScreenPos()` for positioning.

  **Agent**: `unspecified-high`

- [ ] 14. TabWidget Widget

  **What to do**: RED tests + GREEN `TabWidget` class. RAII-managed tabs: `BeginTabBar/EndTabBar` + `BeginTabItem/EndTabItem`. State: vector of `TabPage` structs (name, content callback, closable flag), active tab index.

  **Agent**: `unspecified-high`

- [ ] 15. BackendFactory Refactor

  **What to do**: Update `backend_factory.h`: `CreateBackend(BackendType)` routing to GLFW+GL3 or SDL3+Vulkan factory functions. Inject platform-specific window handle when needed.

  **Agent**: `deep`

- [ ] 16. App Bootstrap Update for Vulkan

  **What to do**: Update `app.cc` Init/Render to handle SDL3+Vulkan init path. Ensure `SDLVulkanInit()` sequence: SDL window → Vulkan surface → device → swapchain → pipeline → frame resources. Keep GLFW+GL3 as fallback.

  **Agent**: `deep`

- [ ] 17. Update hello_unigui for v2 widgets

  **What to do**: Add v2 widget demo to example: TabWidget with 2 tabs (Widgets + Form), GroupBox demo, ComboBox demo, Slider demo.

  **Agent**: `unspecified-high`

- [ ] 18. vulkan_triangle Example

  **What to do**: Standalone Vulkan demo: clear color → triangle via ImGui draw list → present. Validates full Vulkan pipeline without widget dependency.

  **Agent**: `deep`

- [ ] 19. Update README for v2.0

  **What to do**: Add SDL3+Vulkan setup, BackendType docs, new widget API docs, v1→v2 migration notes.

  **Agent**: `writing`

- [ ] 20. v2.0 CHANGELOG

  **What to do**: Document all changes: new backend, new widgets, API additions, breaking changes (none expected).

  **Agent**: `writing`

---

## Final Verification Wave

- [ ] F1. **Plan Compliance Audit** — `oracle`
- [ ] F2. **Code Quality Review** — `unspecified-high`
- [ ] F3. **Real Manual QA** — `unspecified-high`
- [ ] F4. **Scope Fidelity Check** — `deep`

---

## Commit Strategy

All commits: `type(scope): description` format

- **1-6**: Backend infrastructure commits
- **7-14**: Widget commits (one per task)
- **15-18**: Integration commits
- **19-20**: Documentation commits

## Success Criteria

```bash
# Vulkan backend build
cmake --preset windows-msvc-debug -DUNIGUI_BACKEND=SDL3_VULKAN
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug  # ≥100 tests

# GLFW backend still works
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug  # ≥74 tests

# Vulkan demo
./build/.../examples/vulkan_triangle/vulkan_triangle.exe --frames 10
```
