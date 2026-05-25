# TeamkillerUniGUI — Dear ImGui Modern C++ Wrapper Library

## TL;DR

> **Quick Summary**: Build a C++23 Dear ImGui wrapper library that provides a modern dark theme engine (auto-styling raw ImGui calls) + a widget component library (Window, Panel, Form, Button, Label) with GLFW+OpenGL3 backend abstraction. Cross-platform: Windows/Linux/macOS.
> 
> **Deliverables**:
> - `unigui` static library (`unigui.lib`/`libunigui.a`) with CMake install targets
> - Theme engine: Dark theme preset, `StyleScope` RAII scoping, DPI-aware font scaling
> - Widget library: 5 core widgets (Window, Panel, Form, Button, Label) with docking support
> - GLFW+OpenGL3 platform backend (PIMPL-abstracted for future backends)
> - Google Test test suite (TDD), `hello_unigui` example app
> 
> **Estimated Effort**: Medium-Large (14 implementation + 4 review tasks)
> **Parallel Execution**: YES — 4 waves, max 3 concurrent
> **Critical Path**: Task 1 → Task 2 → Task 6 → Task 9 → Task 11 → Task 12 → Task 13 → FINAL

---

## Context

### Original Request
Build a Dear ImGui C++ wrapper with unified modern UI styling and backend encapsulation supporting Windows, Linux, macOS. Tech stack: C++, CMake, Ninja, vcpkg (at `D:\vcpkg`).

### Interview Summary
**Key Discussions**:
- **Abstraction level**: Both theme engine (auto-styles raw `ImGui::` calls) AND widget component library
- **ImGui visibility**: PUBLIC — users `#include <imgui.h>` and call `ImGui::Button()` directly; PIMPL only for backend internals
- **Backend (v1)**: GLFW+OpenGL3 only; backend abstraction interface designed for future Vulkan/SDL3
- **Vulkan**: DEFERRED to v2 (1000+ LOC Vulkan boilerplate would derail v1)
- **v1 Widgets**: Window, Panel, Form, Button, Label (5 widgets; Table/TreeView/Dialog/Menu deferred)
- **v1 Theme**: Single dark theme (Discord/Linear-inspired); light theme deferred
- **Main loop**: Dual API — `unigui::Run(callback)` and manual `NewFrame()/Render()`
- **Library type**: Static by default, shared optional via `UNIGUI_BUILD_SHARED`
- **Docking**: Enabled (vcpkg feature); multi-viewport = NO
- **C++23** with MSVC 19.40+, GCC 14+, Clang 18+
- **TDD** with Google Test (gtest from vcpkg)

**Research Findings**:
- vcpkg at `D:\vcpkg`: imgui v1.92.8 confirmed with `docking-experimental`, `freetype`, `glfw-binding`, `opengl3-binding` features
- Dear ImGui M×N backend matrix: Platform (window/input) × Renderer (GPU draw)
- Production patterns: PIMPL for platform abstraction, RAII scope wrappers, theme push/pop
- CMake 3.31+: generator expressions, vcpkg manifest mode, `GenerateExportHeader`

### Metis Review
**Identified Gaps** (addressed):
- **PIMPL vs public ImGui**: Resolved — ImGui PUBLIC, PIMPL for backend only
- **Vulkan scope explosion**: Resolved — deferred to v2
- **Widget scope (9→5)**: Locked to Window/Panel/Form/Button/Label
- **Theme subjectivity**: Locked to ONE dark theme with named reference
- **TDD realism**: Unit-testable = logic/state/validation; not-testable = pixel output. Agent-QA for visual.
- **Who owns main loop?**: Both `Run(callback)` and manual API provided
- **Missing consumability criterion**: Added `find_package(unigui)` integration test
- **No definition of done**: Added explicit DoD checklist

---

## Work Objectives

### Core Objective
Create `unigui` — a C++23 static library wrapping Dear ImGui with a unified dark theme engine (Discord/Linear-inspired) and a set of 5 high-level widgets (Window, Panel, Form, Button, Label), built on a PIMPL-abstracted GLFW+OpenGL3 backend, consumable via CMake `find_package(unigui CONFIG)`.

### Concrete Deliverables
- `include/unigui/unigui.h` — single include header
- `include/unigui/core/context.h`, `include/unigui/core/error.h` — core types
- `include/unigui/backend/platform_backend.h`, `include/unigui/backend/renderer_backend.h` — interfaces
- `include/unigui/theme/theme.h`, `include/unigui/theme/style_scope.h` — theme engine
- `include/unigui/widgets/window.h`, `panel.h`, `form.h`, `button.h`, `label.h` — widget API
- `src/` — all implementation files
- `tests/` — Google Test suite with ≥30 test cases
- `examples/hello_unigui/` — working demo application
- `CMakeLists.txt`, `CMakePresets.json`, `vcpkg.json` — build system

### Definition of Done
- [ ] `cmake --preset windows-msvc-release` → exit 0, produces `build/windows-msvc-release/unigui.lib`
- [ ] `ctest --preset windows-msvc-release` → 100% tests pass, ≥30 test cases
- [ ] `build/windows-msvc-release/examples/hello_unigui/hello_unigui.exe --frames 10` → exit 0, renders 10 frames
- [ ] Downstream project does `find_package(unigui CONFIG REQUIRED)` and links `unigui::unigui` successfully
- [ ] All 14 implementation tasks have QA evidence files in `.omo/evidence/`

### Must Have
- GLFW+OpenGL3 backend working on Windows (primary), Linux, macOS
- Dark theme auto-applied to all raw `ImGui::` calls
- 5 widgets (Window, Panel, Form, Button, Label) with styled defaults
- C++23, CMake+Ninja+vcpkg build, Google Test TDD
- Docking support enabled
- `unigui::Run()` and manual `NewFrame()/Render()` APIs both functional
- `.gitignore` with `build/`, `vcpkg_installed/`, `.vs/`, `out/`

### Must NOT Have (Guardrails)
- NO Vulkan, SDL3, DirectX, Metal, or WebGPU backends in v1
- NO multi-viewport support
- NO light theme
- NO Table, TreeView, ListView, Dialog, MenuBar (standalone widget class), StatusBar, ButtonBar widgets
- NO "future-proof" virtual interfaces for single-implementation things
- NO async/threading infrastructure
- NO plugin system, scripting bindings, serialization, or hot-reload
- NO `[[nodiscard]]`, `noexcept`, `constexpr` sprinkled reflexively — justify each
- NO Doxygen on private members; public API documentation only
- NO custom logging/assertion/string/container utilities — use C++23 `<format>`, `<source_location>`
- NO god-files — one translation unit per widget/concern
- NO files in `docs/` or `plans/` — only `.omo/plans/`
- NO task labels with prefixes like "T1.", "Phase 1:", "Task-1." — use "1.", "2.", "F1.", "F2." only

---

## Verification Strategy (MANDATORY)

> **ZERO HUMAN INTERVENTION** - ALL verification is agent-executed. No exceptions.

### Test Decision
- **Infrastructure exists**: NO (greenfield — created in Task 1)
- **Automated tests**: TDD (Google Test)
- **Framework**: Google Test (gtest via vcpkg)
- **TDD workflow**: Each task includes RED (failing test) → GREEN (minimal impl) → REFACTOR cycle
- **What is unit-testable**: Theme color values, style stack push/pop counts, form validation logic, widget state transitions, backend factory selection, error propagation, context lifecycle
- **What is integration-testable (Agent QA)**: Window opens, ImGui renders without errors, valid framerate, font loading, backend init/shutdown ordering
- **What is NOT unit-testable**: Pixel-perfect rendering output (snapshot testing territory — defer to v2)

### QA Policy
Every task MUST include agent-executed QA scenarios.
Evidence saved to `.omo/evidence/task-{N}-{scenario-slug}.{ext}`.

- **Build verification**: `cmake --preset <name> && cmake --build --preset <name>` → exits 0
- **Test verification**: `ctest --preset <name> --output-on-failure` → exit 0
- **Example app**: `--frames N --headless` → exits 0, logs expected output
- **API/Backend**: Bash (curl not applicable for C++ libs; use compiled test runner)
- **Module verification**: `bun` not applicable; use `ctest` test runner

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Foundation):
├── Step 1: Task 1 — Project Scaffolding & Build System [quick]
├── Step 2 (parallel after Task 1):
│   ├── Task 2: Core Types, Context & Error Handling [quick]
│   └── Task 3: Backend Abstraction Interfaces [quick]
└── Step 3 (parallel after Tasks 1, 3):
    ├── Task 4: GLFW + OpenGL3 Backend Implementation [deep]
    └── Task 5: Theme Engine [deep]

Wave 2 (Widgets — depends on Wave 1 complete):
├── Step 1: Task 6 — Widget Base Infrastructure [quick]
├── Step 2 (parallel after Task 6):
│   ├── Task 7: Label Widget [quick]
│   ├── Task 8: Button Widget [quick]
│   └── Task 9: Panel Widget [unspecified-high]
└── Step 3 (after Tasks 6, 8):
    └── Task 10: Form Widget [unspecified-high]

Wave 3 (Integration — depends on Waves 1+2):
├── Task 11: Window Widget (depends: 4, 5, 6, 9) [unspecified-high]
└── Task 12: Application Bootstrap & Main Loop (depends: 4, 5, 11) [deep]
    Note: Task 12 runs after Task 11 (sequential chain: 11 → 12)

Wave 4 (Examples & Docs — depends on Wave 3):
├── Task 13: hello_unigui Example Application (depends: 12) [unspecified-high]
└── Task 14: README Documentation [writing]
    Note: Tasks 13 and 14 are fully parallel

Wave FINAL (4 parallel reviews — AFTER all tasks):
├── Task F1: Plan Compliance Audit [oracle]
├── Task F2: Code Quality Review [unspecified-high]
├── Task F3: Real Manual QA [unspecified-high]
└── Task F4: Scope Fidelity Check [deep]
→ Present results → Get explicit user okay
```

### Dependency Matrix

| Task | Depends On | Blocks | Wave |
|------|-----------|--------|------|
| 1 | - | 2, 3, 4, 5 | 1 (Step 1) |
| 2 | 1 | 6 | 1 (Step 2) |
| 3 | 1 | 4 | 1 (Step 2) |
| 4 | 1, 3 | 11, 12 | 1 (Step 3) |
| 5 | 1 | 6, 7, 8, 9, 10, 11, 12 | 1 (Step 3) |
| 6 | 2, 5 | 7, 8, 9, 10, 11 | 2 (Step 1) |
| 7 | 6 | - | 2 (Step 2) |
| 8 | 6 | 10 | 2 (Step 2) |
| 9 | 6 | 11 | 2 (Step 2) |
| 10 | 6, 8 | - | 2 (Step 3) |
| 11 | 4, 5, 6, 9 | 12, 13 | 3 |
| 12 | 4, 5, 11 | 13 | 3 |
| 13 | 12 | - | 4 |
| 14 | - | - | 4 |
| F1-F4 | ALL | - | FINAL |

**Critical Path**: 1 → 2 → 6 → 9 → 11 → 12 → 13 → FINAL
**Parallel Speedup**: ~50% faster than sequential (inner-wave parallelism limited by interface→impl dependencies)
**Max Concurrent**: 3 (Wave 2 Step 2: Tasks 7, 8, 9)

### Agent Dispatch Summary
- **Wave 1 Step 1 (1)**: T1 → `quick`
- **Wave 1 Step 2 (2)**: T2 → `quick`, T3 → `quick`
- **Wave 1 Step 3 (2)**: T4 → `deep`, T5 → `deep`
- **Wave 2 Step 1 (1)**: T6 → `quick`
- **Wave 2 Step 2 (3)**: T7 → `quick`, T8 → `quick`, T9 → `unspecified-high`
- **Wave 2 Step 3 (1)**: T10 → `unspecified-high`
- **Wave 3 (2)**: T11 → `unspecified-high`, T12 → `deep` (sequential: 11 → 12)
- **Wave 4 (2)**: T13 → `unspecified-high`, T14 → `writing` (parallel)

---

## TODOs

> Implementation + Test = ONE Task. Never separate.
> EVERY task MUST have: Recommended Agent Profile + QA Scenarios.
> **TDD means each task writes failing test FIRST, then implementation.**
> Task labels: bare numbers ("1.", "2.", NOT "T1.", "Task 1.")
> Final Wave labels: "F1.", "F2.", NOT "F-1.", "Final 1."

- [ ] 1. Project Scaffolding & Build System

  **What to do** (TDD — test infrastructure itself is the deliverable):
  - Create `CMakeLists.txt` (top-level): `cmake_minimum_required(VERSION 3.31)`, `project(unigui VERSION 0.1.0 LANGUAGES CXX)`, C++23 requirement, option `UNIGUI_BUILD_SHARED`, option `UNIGUI_BUILD_TESTS`, `add_subdirectory(src)`, install rules, `CMakePackageConfigHelpers` for `find_package` support
  - Create `src/CMakeLists.txt`: `add_library(unigui)`, `add_library(unigui::unigui ALIAS unigui)`, `target_sources` with generator expressions for platform files, `target_include_directories` with `$<BUILD_INTERFACE:>` / `$<INSTALL_INTERFACE:>`, `target_link_libraries` to imgui/glfw3/glad/freetype, `GenerateExportHeader` for `UNIGUI_API` macro
  - Create `vcpkg.json` (manifest mode): name=`unigui`, version=`0.1.0`, dependencies=`imgui[core,docking-experimental,freetype,glfw-binding,opengl3-binding]`, `glfw3`, `freetype`, `glad`, `gtest`. Pin `builtin-baseline` to current vcpkg commit.
  - Create `CMakePresets.json` (version 10): 4 presets — `windows-msvc-debug`, `windows-msvc-release`, `linux-gcc-debug`, `macos-clang-debug`. Each sets `CMAKE_TOOLCHAIN_FILE=$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake`, `CMAKE_CXX_STANDARD=23`, Ninja generator. Windows presets set `VCPKG_TARGET_TRIPLET=x64-windows`.
  - Create `cmake/unigui-config.cmake.in` for `find_package` consumption
  - Create `.gitignore`: `build/`, `vcpkg_installed/`, `.vs/`, `out/`, `*.user`, `CMakeUserPresets.json`
  - Create `include/unigui/unigui_export.h` (placeholder — `GenerateExportHeader` will overwrite at build time)
  - Create `tests/CMakeLists.txt`: `enable_testing()`, `find_package(GTest CONFIG REQUIRED)`, `add_executable` per test file, `target_link_libraries(... GTest::gtest GTest::gtest_main unigui)`, `gtest_discover_tests`
  - Create `tests/test_main.cc` — minimal gtest main (or use `GTest::gtest_main`)
  - **RED**: Write `tests/test_smoke.cc` — one test `BuildSystemWorks` that `#include <unigui/unigui.h>` and asserts `UNIGUI_VERSION_MAJOR == 0`
  - **GREEN**: Create `include/unigui/unigui.h`, `include/unigui/core/version.h` with `UNIGUI_VERSION_MAJOR/MINOR/PATCH` defines. `unigui.h` includes version.h. `src/unigui.cc` with empty impl.
  - Verify: `cmake --preset windows-msvc-debug && cmake --build --preset windows-msvc-debug && ctest --preset windows-msvc-debug` → all pass (1 test)

  **Must NOT do**:
  - Do NOT create unnecessary directory nesting (no `src/core/subdir/deep/`)
  - Do NOT add `install(TARGETS ...)` until library target actually exists
  - Do NOT hardcode paths — use `CMAKE_CURRENT_SOURCE_DIR`, `PROJECT_SOURCE_DIR`
  - Do NOT add `set(CMAKE_CXX_STANDARD 23)` globally — use `target_compile_features(unigui PUBLIC cxx_std_23)`

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: CMake boilerplate setup — well-defined patterns, no algorithmic complexity
  - **Skills**: None required beyond default tooling

  **Parallelization**:
  - **Can Run In Parallel**: NO (must complete first — creates directory tree and CMakeLists.txt that all other tasks depend on)
  - **Parallel Group**: Wave 1 Step 1 (standalone)
  - **Blocks**: Tasks 2, 3, 4, 5 (all need build system)
  - **Blocked By**: None (can start immediately)

  **References**:
  - `D:\vcpkg\ports\imgui\vcpkg.json` — reference for imgui feature names and vcpkg.json structure
  - `D:\vcpkg\ports\imgui\portfile.cmake` — reference for how vcpkg builds imgui with feature flags
  - CMake docs: `https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html` — DLL export macro generation
  - CMake docs: `https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html` — CMakePresets.json schema v10

  **Acceptance Criteria**:
  - [ ] `Test-Path "D:\TeamkillerUniGUI\CMakeLists.txt"` → True
  - [ ] `Test-Path "D:\TeamkillerUniGUI\vcpkg.json"` → True
  - [ ] `Test-Path "D:\TeamkillerUniGUI\CMakePresets.json"` → True
  - [ ] `cmake --preset windows-msvc-debug` → exit 0, configures successfully with vcpkg toolchain
  - [ ] `cmake --build --preset windows-msvc-debug` → exit 0, produces `build/windows-msvc-debug/unigui.lib`
  - [ ] `ctest --preset windows-msvc-debug` → exit 0, 1 test passes: `BuildSystemWorks`

  **QA Scenarios**:

  ```
  Scenario: Clean configure and build on Windows
    Tool: Bash (pwsh)
    Preconditions: D:\vcpkg exists and is bootstrapped, VCPKG_ROOT env var set
    Steps:
      1. cmake --preset windows-msvc-debug -B build/windows-msvc-debug
      2. Assert exit code 0, stdout contains "Configuring done", "Generating done"
      3. cmake --build --preset windows-msvc-debug
      4. Assert exit code 0, build/windows-msvc-debug/unigui.lib exists
    Expected Result: Build succeeds, static library produced
    Failure Indicators: cmake configure fails (missing vcpkg, wrong toolchain), build fails (compiler errors)
    Evidence: .omo/evidence/task-1-build-success.log

  Scenario: Smoke test passes
    Tool: Bash (pwsh)
    Preconditions: Build succeeded
    Steps:
      1. ctest --preset windows-msvc-debug --output-on-failure
      2. Assert exit code 0
      3. Assert stdout contains "100% tests passed" or "1/1 Test #1: BuildSystemWorks ... Passed"
    Expected Result: 1 test passes, version macro defined correctly
    Failure Indicators: Test fails (version macro missing or wrong value)
    Evidence: .omo/evidence/task-1-ctest-pass.log
  ```

  **Evidence to Capture**:
  - [x] `task-1-build-success.log` — cmake configure + build output
  - [x] `task-1-ctest-pass.log` — ctest output

  **Commit**: YES
  - Message: `feat(build): add CMake, vcpkg manifest, and presets`
  - Files: `CMakeLists.txt`, `src/CMakeLists.txt`, `tests/CMakeLists.txt`, `vcpkg.json`, `CMakePresets.json`, `cmake/unigui-config.cmake.in`, `.gitignore`, `include/unigui/unigui.h`, `include/unigui/core/version.h`, `src/unigui.cc`, `tests/test_smoke.cc`

- [ ] 2. Core Types, Context & Error Handling

  **What to do** (TDD):
  - **RED**: Write tests in `tests/core/context_test.cc`:
    - `CreateContext_ReturnsValidPointer` — calls `unigui::CreateContext()`, asserts non-null, calls `unigui::DestroyContext()`
    - `DoubleCreate_ReturnsSameContext` — two `CreateContext()` calls return same pointer (singleton or ref-counted)
    - `DestroyThenCreate_ReturnsNewContext` — destroy, create, verify it's usable
    - `GetContext_WithoutCreate_ReturnsNull` — `GetContext()` returns nullptr if no context exists
  - Write tests in `tests/core/error_test.cc`:
    - `ErrorCode_ToMessage_ReturnsNonEmpty` — each `unigui::ErrorCode` enum value maps to a `std::string_view` message
    - `Result_Success_HoldsValue` — `unigui::Result<int>` with success, `.has_value()` true, `.value()` returns correct int
    - `Result_Error_HoldsErrorCode` — `unigui::Result<int>` with error, `.has_value()` false, `.error()` returns correct code
  - **GREEN**: Implement:
    - `include/unigui/core/context.h` — `unigui::CreateContext()`, `DestroyContext()`, `GetContext()` (singleton pattern, stores `ImGuiContext*` + internal state)
    - `include/unigui/core/error.h` — `enum class ErrorCode`, `class Result<T>` (using `std::expected`-like API, since C++23 `<expected>` may not be fully available on MSVC — fall back to simple `std::optional` + error code pair)
    - `src/core/context.cc` — `CreateContext` calls `ImGui::CreateContext()`, stores in static `std::unique_ptr`, `DestroyContext` calls `ImGui::DestroyContext()`
    - `src/core/error.cc` — `ErrorCode` → string mapping
  - Update `include/unigui/unigui.h` to include `core/context.h` and `core/error.h`

  **Must NOT do**:
  - Do NOT create a custom Result type if `std::expected` works on MSVC 19.40+ — prefer standard library
  - Do NOT add thread safety to context (single-threaded ImGui rule)
  - Do NOT add allocator customization (v2 concern)

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Core types are straightforward — singleton pattern, enum class, Result wrapper. No algorithmic complexity.
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 Step 2 (with Task 3)
  - **Blocks**: Task 6 (Widget Base needs Context)
  - **Blocked By**: Task 1 (needs build system)

  **References**:
  - Dear ImGui source: `https://github.com/ocornut/imgui/blob/master/imgui.h` — `ImGui::CreateContext()`, `ImGui::DestroyContext()` API
  - cppreference: `https://en.cppreference.com/w/cpp/utility/expected` — C++23 `std::expected` API (reference for Result class)
  - `include/unigui/core/version.h` (from Task 1) — version defines pattern to follow

  **Acceptance Criteria**:
  - [ ] `tests/core/context_test.cc` compiles, all 4 tests pass
  - [ ] `tests/core/error_test.cc` compiles, all 3 tests pass
  - [ ] `#include <unigui/unigui.h>` provides `unigui::CreateContext()`, `DestroyContext()`, `GetContext()`, `ErrorCode`, `Result<T>`

  **QA Scenarios**:

  ```
  Scenario: Context lifecycle
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "ContextTest" --output-on-failure
      2. Assert exit code 0, 4 tests pass
    Expected Result: All context tests pass — create/destroy/recreate/null-get all correct
    Failure Indicators: Test failure (nullptr from Create, double-create bug, use-after-destroy)
    Evidence: .omo/evidence/task-2-context-tests.log

  Scenario: Error handling types
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "ErrorTest" --output-on-failure
      2. Assert exit code 0, 3 tests pass
    Expected Result: All error tests pass — mapping, Result success/error paths
    Failure Indicators: Missing enum value strings, Result type logic bug
    Evidence: .omo/evidence/task-2-error-tests.log
  ```

  **Evidence to Capture**:
  - [x] `task-2-context-tests.log` — context test output
  - [x] `task-2-error-tests.log` — error test output

  **Commit**: YES
  - Message: `feat(core): add unigui context, error handling, and version header`
  - Files: `include/unigui/core/context.h`, `include/unigui/core/error.h`, `src/core/context.cc`, `src/core/error.cc`, `tests/core/context_test.cc`, `tests/core/error_test.cc`, updated `include/unigui/unigui.h`

- [ ] 3. Backend Abstraction Interfaces

  **What to do** (TDD):
  - **RED**: Write tests in `tests/backend/backend_interface_test.cc`:
    - `PlatformBackend_Virtuals_ArePureVirtual` — cannot instantiate `PlatformBackend` directly (compile-time assertion via `std::is_abstract`)
    - `RendererBackend_Virtuals_ArePureVirtual` — same for `RendererBackend`
    - `MockPlatform_CallsInit_ReturnsTrue` — mock implementation, `Init()` returns true
    - `MockRenderer_RenderDrawData_DoesNotCrash` — mock `RenderDrawData(nullptr)` doesn't crash
  - **GREEN**: Implement:
    - `include/unigui/backend/platform_backend.h` — pure virtual interface:
      ```cpp
      class UNIGUI_API PlatformBackend {
      public:
          virtual ~PlatformBackend() = default;
          virtual bool Init(void* native_window_handle) = 0;
          virtual void Shutdown() = 0;
          virtual void NewFrame() = 0;
          virtual void PollEvents() = 0;
          virtual bool ShouldClose() const = 0;
      };
      ```
    - `include/unigui/backend/renderer_backend.h` — pure virtual interface:
      ```cpp
      class UNIGUI_API RendererBackend {
      public:
          virtual ~RendererBackend() = default;
          virtual bool Init(ImGuiContext* context) = 0;
          virtual void Shutdown() = 0;
          virtual void RenderDrawData(ImDrawData* draw_data) = 0;
          virtual void SetClearColor(float r, float g, float b, float a) = 0;
      };
      ```
    - `include/unigui/backend/backend_types.h` — `struct BackendConfig { int width; int height; const char* title; };`
    - Test mocks in `tests/backend/mock_backends.h` — concrete mock classes for testing

  **Must NOT do**:
  - Do NOT add virtual methods that are speculative (Vulkan-specific, multi-viewport, etc.) — only what GLFW+OpenGL3 needs
  - Do NOT couple `PlatformBackend` and `RendererBackend` — they are independent interfaces (M×N matrix)
  - Do NOT add `noexcept` or `[[nodiscard]]` without a concrete reason

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Pure virtual interfaces with mock implementations — straightforward OOP design, no complex logic
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 Step 2 (with Task 2)
  - **Blocks**: Task 4 (needs interfaces to implement)
  - **Blocked By**: Task 1 (needs build system)

  **References**:
  - Dear ImGui backend examples: `imgui_impl_glfw.h`, `imgui_impl_opengl3.h` — patterns for `Init`, `NewFrame`, `Shutdown`, `RenderDrawData`
  - C++ Core Guidelines: `https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-abstract` — interface class design
  - Dear ImGui BACKENDS.md: `https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md` — backend responsibilities

  **Acceptance Criteria**:
  - [ ] `tests/backend/backend_interface_test.cc` compiles and all 4 tests pass
  - [ ] `PlatformBackend` and `RendererBackend` are pure virtual (cannot be instantiated)
  - [ ] Mock implementations compile and pass tests
  - [ ] `BackendConfig` struct is available via `#include <unigui/backend/backend_types.h>`

  **QA Scenarios**:

  ```
  Scenario: Backend interfaces compile and are abstract
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "BackendInterface" --output-on-failure
      2. Assert exit code 0, 4 tests pass
    Expected Result: Interfaces are abstract, mocks implement correctly, no crashes on mock operations
    Failure Indicators: Interface is not abstract (can instantiate), mock test crashes
    Evidence: .omo/evidence/task-3-backend-interface-tests.log

  Scenario: Interfaces compile without ImGui dependency in header
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. grep -r "ImDrawData" include/unigui/backend/renderer_backend.h
      2. Assert the forward declaration or include exists and compiles cleanly
    Expected Result: RendererBackend references ImDrawData* (forward-declared or included correctly)
    Failure Indicators: Missing ImDrawData forward declaration, compilation error
    Evidence: .omo/evidence/task-3-interface-headers.txt
  ```

  **Evidence to Capture**:
  - [x] `task-3-backend-interface-tests.log` — test output
  - [x] `task-3-interface-headers.txt` — header file contents summary

  **Commit**: YES
  - Message: `feat(backend): add platform and renderer backend interfaces`
  - Files: `include/unigui/backend/platform_backend.h`, `include/unigui/backend/renderer_backend.h`, `include/unigui/backend/backend_types.h`, `tests/backend/backend_interface_test.cc`, `tests/backend/mock_backends.h`

- [ ] 4. GLFW + OpenGL3 Backend Implementation

  **What to do** (TDD):
  - **RED**: Write tests in `tests/backend/glfw_gl3_backend_test.cc`:
    - `GLFWPlatform_Init_Default_Succeeds` — `Init(nullptr)` creates hidden window, returns true (nullptr = create default hidden window for testing)
    - `GLFWPlatform_Init_WithNullHandle_Succeeds` — same as above, explicit documentation that nullptr means "create internal window"
    - `GLFWPlatform_Init_WithValidConfig_Succeeds` — creates hidden GLFW window (no display needed for test), `Init()` returns true, `ShouldClose()` returns false, `PollEvents()` doesn't crash
    - `GLFWPlatform_NewFrame_BeforeInit_Asserts` — calling `NewFrame()` before `Init()` triggers assertion (use death test `EXPECT_DEATH` if available, otherwise skip)
    - `GLFWPlatform_Shutdown_AfterInit_CleansUp` — `Shutdown()` after `Init()`, calling `NewFrame()` after `Shutdown()` returns gracefully
    - `OpenGL3Renderer_Init_WithoutContext_ReturnsFalse` — `Init(nullptr)` returns false
    - `OpenGL3Renderer_Init_WithValidContext_Succeeds` — `Init(context)` returns true
    - `OpenGL3Renderer_RenderDrawData_Null_DrawData_DoesNotCrash` — calling `RenderDrawData(nullptr)` doesn't crash
  - **GREEN**: Implement:
    - `src/backend/glfw_platform.cc` — `class GLFWPlatform : public PlatformBackend`:
      - `Init(void* native_window_handle = nullptr)`: When handle is nullptr, creates internal hidden GLFW window (800×600, `GLFW_VISIBLE=GLFW_FALSE`). When handle is provided, attaches to existing window. Sets up ImGui GLFW backend and callbacks.
      - `Shutdown()`: calls `ImGui_ImplGlfw_Shutdown()`, `glfwDestroyWindow()`, `glfwTerminate()`
      - `NewFrame()`: calls `ImGui_ImplGlfw_NewFrame()`
      - `PollEvents()`: calls `glfwPollEvents()`
      - `ShouldClose()`: calls `glfwWindowShouldClose()`
    - `src/backend/opengl3_renderer.cc` — `class OpenGL3Renderer : public RendererBackend`:
      - `Init(context)`: stores context, calls `ImGui_ImplOpenGL3_Init("#version 130")` (GLSL 1.30 for max compatibility)
      - `Shutdown()`: calls `ImGui_ImplOpenGL3_Shutdown()`
      - `RenderDrawData(draw_data)`: calls `ImGui_ImplOpenGL3_RenderDrawData(draw_data)`. Does NOT call `glfwSwapBuffers()` — swap ownership belongs to the application layer (Task 12).
      - `SetClearColor(r,g,b,a)`: calls `glClearColor()`
    - `src/backend/backend_registry.cc` — factory function `CreateDefaultBackend()` returns `GLFWPlatform` + `OpenGL3Renderer`
    - Update `src/CMakeLists.txt`: add `src/backend/glfw_platform.cc`, `src/backend/opengl3_renderer.cc`, `src/backend/backend_registry.cc`; link `glfw3`, `glad`, `freetype`; use `find_package(glfw3 CONFIG REQUIRED)`, `find_package(glad CONFIG REQUIRED)`, `find_package(freetype CONFIG REQUIRED)`

  **Must NOT do**:
  - Do NOT create a visible GLFW window during tests (use `GLFW_VISIBLE=GLFW_FALSE`)
  - Do NOT add Vulkan, SDL, or any other backend code — this is GLFW+OpenGL3 ONLY
  - Do NOT hardcode OpenGL version 4.6 — use 3.3 for maximum compatibility (macOS supports only up to 4.1)

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: Complex integration — GLFW window management, OpenGL context creation, ImGui backend wiring. Requires understanding of GPU pipeline, GLFW lifecycle, and ImGui internals.
  - **Skills**: None (standard C++/OpenGL knowledge)

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 Step 3 (with Task 5)
  - **Blocks**: Tasks 11, 12 (Window Widget, App Bootstrap need backend)
  - **Blocked By**: Tasks 1 (build system), 3 (backend interfaces)

  **References**:
  - ImGui GLFW backend: `https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_glfw.cpp` — reference implementation for GLFW integration
  - ImGui OpenGL3 backend: `https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_opengl3.cpp` — reference implementation for OpenGL3 renderer
  - GLFW docs: `https://www.glfw.org/docs/latest/window_guide.html` — window creation, hints, callbacks
  - `include/unigui/backend/platform_backend.h` (Task 3) — interface to implement
  - `include/unigui/backend/renderer_backend.h` (Task 3) — interface to implement

  **Acceptance Criteria**:
  - [ ] `tests/backend/glfw_gl3_backend_test.cc` compiles and all 7 tests pass (or skip death tests if unsupported)
  - [ ] `GLFWPlatform` compiles and links against glfw3
  - [ ] `OpenGL3Renderer` compiles and links against glad + OpenGL
  - [ ] `CreateDefaultBackend()` returns valid platform + renderer pair
  - [ ] Headless GLFW window created without display server (works on CI)

  **QA Scenarios**:

  ```
  Scenario: GLFW platform backend init and shutdown
    Tool: Bash (pwsh)
    Preconditions: Build succeeds (with GLFW installed via vcpkg)
    Steps:
      1. ctest --preset windows-msvc-debug -R "GLFWPlatform" --output-on-failure
      2. Assert exit code 0, all GLFW-related tests pass
      3. Assert no "GLFW error" messages in stderr
    Expected Result: Platform init succeeds (creates hidden GLFW window), shutdown cleans up without leak warnings
    Failure Indicators: GLFW init fails (missing DLL, wrong triplet), test crashes on window creation
    Evidence: .omo/evidence/task-4-glfw-platform-tests.log

  Scenario: OpenGL3 renderer backend init and render
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "OpenGL3Renderer" --output-on-failure
      2. Assert exit code 0, all OpenGL3 renderer tests pass
      3. Assert no OpenGL errors in stderr (no "GL_INVALID_OPERATION", "GL_INVALID_ENUM")
    Expected Result: Renderer init succeeds, null draw data render doesn't crash
    Failure Indicators: OpenGL context creation fails (no GPU, wrong driver), ImGui assertion triggered
    Evidence: .omo/evidence/task-4-opengl3-renderer-tests.log

  Scenario: Headless smoke test runs without display
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. Create temp test: GLFWPlatform platform; platform.Init(nullptr); platform.NewFrame(); platform.Shutdown();
      2. Assert exit code 0 throughout — no display server required
    Expected Result: GLFW headless window works without monitor attached
    Failure Indicators: GLFW fails if no display (Wayland/X11 issue on Linux CI)
    Evidence: .omo/evidence/task-4-headless-smoke.log
  ```

  **Evidence to Capture**:
  - [x] `task-4-glfw-platform-tests.log` — platform test output
  - [x] `task-4-opengl3-renderer-tests.log` — renderer test output
  - [x] `task-4-headless-smoke.log` — headless smoke test

  **Commit**: YES
  - Message: `feat(backend): implement GLFW+OpenGL3 backend`
  - Files: `src/backend/glfw_platform.cc`, `src/backend/opengl3_renderer.cc`, `src/backend/backend_registry.cc`, `tests/backend/glfw_gl3_backend_test.cc`, updated `src/CMakeLists.txt`

- [ ] 5. Theme Engine

  **What to do** (TDD):
  - **RED**: Write tests in `tests/theme/theme_test.cc`:
    - `DarkTheme_Apply_SetsExpectedColors` — apply dark theme, assert `ImGuiStyle` values: `Colors[ImGuiCol_WindowBg]` ≈ `(0.10f, 0.10f, 0.12f, 1.0f)`, `Colors[ImGuiCol_Text]` ≈ `(0.90f, 0.90f, 0.92f, 1.0f)`, `WindowRounding` == `6.0f`, `FrameRounding` == `4.0f`, `GrabRounding` == `4.0f`, `FramePadding` == `(8.0f, 6.0f)`, `ItemSpacing` == `(8.0f, 6.0f)`
    - `DarkTheme_Apply_SetsAll53Colors` — verify ALL 53 `ImGuiCol_` enum values are explicitly set (not left as default). Use `EXPECT_NE` comparing against a freshly default-initialized style.
    - `StyleScope_PushPop_RestoresPrevious` — push a custom `ImGuiCol_Button` color to red, create StyleScope, within scope change to blue, verify blue, exit scope, verify restored to red
    - `StyleScope_MoveSemantics_TransfersOwnership` — create scope A, move-construct scope B from A, verify B is active and A is not
    - `Theme_ApplyWithDPI_ScalesFontCorrectly` — apply theme with DPI 2.0, verify `ImGui::GetFont()->FontSize` is scaled accordingly
  - Write tests in `tests/theme/font_test.cc`:
    - `LoadDefaultFont_CreatesAtlas` — `ImGui::GetIO().Fonts->Fonts.Size` > 0 after init
    - `FontConfig_Size_ProducesCorrectPixelHeight` — load font at 16px, verify font glyph height is approximately 16px
  - **GREEN**: Implement:
    - `include/unigui/theme/theme.h` — `enum class ThemePreset { Dark }`, `struct ThemeConfig { ThemePreset preset; float dpi_scale; float font_size; }`, `class Theme { public: static void Apply(const ThemeConfig& config); }`
    - `include/unigui/theme/style_scope.h` — `class StyleScope { public: StyleScope(); ~StyleScope(); StyleScope(StyleScope&&) noexcept; StyleScope& operator=(StyleScope&&) noexcept; StyleScope(const StyleScope&) = delete; void PushColor(ImGuiCol_ idx, ImVec4 color); void PushVar(ImGuiStyleVar idx, float val); // ... more Push overloads };`
    - `src/theme/theme.cc` — `Theme::Apply()`: sets all 53 `ImGuiCol_` values for dark theme (Discord/Linear-inspired palette), sets style vars (rounding, padding, spacing, scrollbar size), calls `ImGui::GetIO().FontGlobalScale = config.dpi_scale;` for DPI
    - Dark palette specification (exact values documented as ADR):
      - Background hierarchy: WindowBg=0.10/0.10/0.12, ChildBg=0.12/0.12/0.14, PopupBg=0.14/0.14/0.16
      - Surface hierarchy: FrameBg=0.16/0.16/0.18, TitleBg=0.10/0.10/0.12, TitleBgActive=0.16/0.16/0.18
      - Accent: Button=0.24/0.24/0.28, ButtonHovered=0.30/0.30/0.35, ButtonActive=0.22/0.22/0.26, CheckMark=0.40/0.58/0.93 (blue accent)
      - Text: Text=0.90/0.90/0.92, TextDisabled=0.50/0.50/0.55
      - Borders: Border=0.20/0.20/0.24, BorderShadow=0.00/0.00/0.00
      - Scrollbar: ScrollbarBg=0.12/0.12/0.14, ScrollbarGrab=0.24/0.24/0.28, ScrollbarGrabHovered=0.30/0.30/0.35
      - Headers: Header=0.18/0.18/0.22, HeaderHovered=0.22/0.22/0.26, HeaderActive=0.20/0.20/0.24
      - ResizeGrip=0.24/0.24/0.28, ResizeGripHovered=0.40/0.58/0.93, ResizeGripActive=0.40/0.58/0.93
      - Plot colors: PlotLines=0.40/0.58/0.93, PlotLinesHovered=1.00/0.43/0.35, PlotHistogram=0.40/0.58/0.93, PlotHistogramHovered=1.00/0.43/0.35
      - Table colors: TableHeaderBg=0.14/0.14/0.16, TableBorderStrong=0.24/0.24/0.28, TableBorderLight=0.18/0.18/0.22, TableRowBg=0.10/0.10/0.12, TableRowBgAlt=0.12/0.12/0.14
      - Nav: NavHighlight=0.40/0.58/0.93, NavWindowingHighlight=0.90/0.90/0.92
      - Modal: ModalWindowDimBg=(0.00f, 0.00f, 0.00f, 0.50f)
      - Docking: DockingPreview=0.40/0.58/0.93, DockingEmptyBg=0.10/0.10/0.12
      - DragDrop: DragDropTarget=0.40/0.58/0.93
      - Separator=0.20/0.20/0.24
      - Tab: Tab=0.12/0.12/0.14, TabHovered=0.30/0.30/0.35, TabActive=0.18/0.18/0.22, TabUnfocused=0.12/0.12/0.14, TabUnfocusedActive=0.12/0.12/0.14
    - `src/theme/style_scope.cc` — `StyleScope` uses `ImGui::PushStyleColor()` / `PopStyleColor()` with RAII; move semantics transfer ownership counter; destructor calls `PopStyleColor()` only if scope is active (not moved-from)
    - Update `include/unigui/unigui.h` to include theme headers

  **Must NOT do**:
  - Do NOT add a second theme preset (Light) — explicitly out of scope
  - Do NOT create a custom font loader — use ImGui's default font + freetype (vcpkg feature handles this)
  - Do NOT add icon font support (Font Awesome, etc.) — v2 concern
  - Do NOT expose raw `ImGuiStyle` pointer that users could corrupt

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: Theming requires careful coordination of 53 color values, DPI scaling math, RAII scope management with move semantics. Subtle ImGui style push/pop stack bugs are easy to introduce.
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 Step 3 (with Task 4)
  - **Blocks**: Tasks 6, 7, 8, 9, 10, 11, 12 (all widgets and app need theme)
  - **Blocked By**: Task 1 (needs build system, ImGui linked)

  **References**:
  - ImGui style reference: `https://github.com/ocornut/imgui/blob/master/imgui.h` — `ImGuiStyle` struct definition, all `ImGuiCol_` and `ImGuiStyleVar_` enum values
  - ImGui style API: `ImGui::PushStyleColor()`, `ImGui::PopStyleColor()`, `ImGui::PushStyleVar()`, `ImGui::PopStyleVar()`
  - Discord color palette (design reference): Dark theme with blue accent, layered grays for depth hierarchy
  - `include/unigui/core/context.h` (Task 2) — theme application requires a valid ImGui context

  **Acceptance Criteria**:
  - [ ] `tests/theme/theme_test.cc` compiles and all 5 tests pass
  - [ ] `tests/theme/font_test.cc` compiles and all 2 tests pass
  - [ ] All 53 `ImGuiCol_` values explicitly set by dark theme (verified by test)
  - [ ] `StyleScope` correctly pushes/pops colors and style vars
  - [ ] DPI scaling multiplies `FontGlobalScale` correctly

  **QA Scenarios**:

  ```
  Scenario: Dark theme applies all 53 colors
    Tool: Bash (pwsh)
    Preconditions: Build succeeds, ImGui context created in test
    Steps:
      1. ctest --preset windows-msvc-debug -R "DarkTheme_Apply" --output-on-failure
      2. Assert exit code 0
      3. Assert test output shows "53/53 colors explicitly set"
    Expected Result: Every ImGuiCol_ value differs from default — no color left at default
    Failure Indicators: Some colors still at default value (theme missed entries), color value assertion fails
    Evidence: .omo/evidence/task-5-theme-colors.log

  Scenario: StyleScope push/pop restores state
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "StyleScope_PushPop" --output-on-failure
      2. Assert exit code 0, push/pop test passes
    Expected Result: After StyleScope destructor, original color is restored exactly
    Failure Indicators: Color not restored, double-pop assertion in ImGui, move semantics broken
    Evidence: .omo/evidence/task-5-stylescope-tests.log

  Scenario: DPI scaling works
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "DPI" --output-on-failure
      2. Assert exit code 0, font scale test passes
    Expected Result: FontGlobalScale == dpi_scale value applied
    Failure Indicators: DPI ignored, font scale stuck at 1.0
    Evidence: .omo/evidence/task-5-dpi-tests.log
  ```

  **Evidence to Capture**:
  - [x] `task-5-theme-colors.log` — theme color test output
  - [x] `task-5-stylescope-tests.log` — StyleScope test output
  - [x] `task-5-dpi-tests.log` — DPI/font test output

  **Commit**: YES
  - Message: `feat(theme): add dark theme engine and style scope RAII`
  - Files: `include/unigui/theme/theme.h`, `include/unigui/theme/style_scope.h`, `src/theme/theme.cc`, `src/theme/style_scope.cc`, `tests/theme/theme_test.cc`, `tests/theme/font_test.cc`, updated `include/unigui/unigui.h`

- [ ] 6. Widget Base Infrastructure

  **What to do** (TDD):
  - **RED**: Write tests in `tests/widgets/widget_base_test.cc`:
    - `Widget_GetID_ReturnsConsistentValue` — create widget with string ID, `GetID()` returns same `ImGuiID` for same string, different IDs for different strings
    - `Widget_IsVisible_DefaultsToTrue` — new widget `IsVisible()` returns true
    - `Widget_Show_Hide_TogglesVisibility` — `Hide()` → `IsVisible()` false, `Show()` → `IsVisible()` true
    - `Widget_GetName_ReturnsGivenName` — widget constructed with "MyPanel" returns "MyPanel" from `GetName()`
  - **GREEN**: Implement:
    - `include/unigui/widgets/widget_base.h` — `class Widget { public: Widget(std::string name); virtual ~Widget() = default; virtual void Render() = 0; void Show(); void Hide(); bool IsVisible() const; const std::string& GetName() const; ImGuiID GetID() const; }`
    - `src/widgets/widget_base.cc` — `Widget::Widget()` stores name, computes `ImGuiID` via `ImHashStr(name.c_str())`, `Show()`/`Hide()` toggle a `bool visible_` member, `Render()` checks `visible_` before executing
    - This is a light base class — minimal state, no complex inheritance hierarchy
    - Update `include/unigui/unigui.h` to include `widgets/widget_base.h`

  **Must NOT do**:
  - Do NOT create a full MVC framework — Widget is minimal state holder + render hook
  - Do NOT add virtual methods for event handling, focus management, or layout in v1
  - Do NOT create `WidgetManager`, `WidgetRegistry`, or container classes — v2 concern

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Simple base class — string storage, hash computation, bool toggle. No algorithmic complexity.
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: NO (must complete first in Wave 2 — provides Widget base class header that Tasks 7-10 inherit from)
  - **Parallel Group**: Wave 2 Step 1 (standalone)
  - **Blocks**: Tasks 7, 8, 9, 10, 11 (all widgets inherit from base)
  - **Blocked By**: Tasks 2 (needs Context for ImGuiID), 5 (theme should be applied before widgets rendered)

  **References**:
  - ImGui ID system: `https://github.com/ocornut/imgui/blob/master/imgui.h` — `ImHashStr()` for stable ID generation from strings
  - `include/unigui/core/context.h` (Task 2) — `GetContext()` for accessing ImGui context
  - `include/unigui/theme/theme.h` (Task 5) — theme application before widget rendering

  **Acceptance Criteria**:
  - [ ] `tests/widgets/widget_base_test.cc` compiles and all 4 tests pass
  - [ ] `Widget::GetID()` returns consistent ImGuiID for same string name
  - [ ] `Widget::Show()`/`Hide()`/`IsVisible()` works correctly

  **QA Scenarios**:

  ```
  Scenario: Widget base class lifecycle
    Tool: Bash (pwsh)
    Preconditions: Build succeeds, ImGui context created in test
    Steps:
      1. ctest --preset windows-msvc-debug -R "Widget_GetID|Widget_IsVisible|Widget_Show_Hide|Widget_GetName" --output-on-failure
      2. Assert exit code 0, 4 tests pass
    Expected Result: All widget base operations work — ID consistency, visibility toggle, name storage
    Failure Indicators: ImHashStr produces different IDs for same string, Show/Hide broken
    Evidence: .omo/evidence/task-6-widget-base-tests.log
  ```

  **Evidence to Capture**:
  - [x] `task-6-widget-base-tests.log` — test output

  **Commit**: YES
  - Message: `feat(widgets): add widget base infrastructure`
  - Files: `include/unigui/widgets/widget_base.h`, `src/widgets/widget_base.cc`, `tests/widgets/widget_base_test.cc`, updated `include/unigui/unigui.h`

- [ ] 7. Label Widget

  **What to do** (TDD):
  - **RED**: Write tests in `tests/widgets/label_test.cc`:
    - `Label_Render_DoesNotCrash` — create Label("test"), call `Render()`, no crash, no ImGui assertion
    - `Label_GetText_ReturnsGivenText` — `Label("Hello").GetText()` == "Hello"
    - `Label_SetText_UpdatesText` — `SetText("World")`, then `GetText()` == "World"
    - `Label_Hidden_DoesNotRender` — `Hide()`, call `Render()`, verify no crash and it doesn't produce draw calls (check `ImGui::GetDrawData()` is null or empty for this window)
  - **GREEN**: Implement:
    - `include/unigui/widgets/label.h` — `class Label : public Widget { public: Label(std::string name, std::string text = ""); void Render() override; void SetText(std::string text); const std::string& GetText() const; }`
    - `src/widgets/label.cc` — `Render()`: if visible, calls `ImGui::TextUnformatted(text_.c_str())` (safe — doesn't interpret format specifiers), uses theme-applied colors
    - Styling: Label uses current theme's `ImGuiCol_Text` automatically (no explicit push needed since Theme::Apply already set global style)

  **Must NOT do**:
  - Do NOT use `ImGui::Text()` with format strings — use `TextUnformatted()` for safety
  - Do NOT add text wrapping, alignment, or color override API in v1

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Trivial widget — text storage + single ImGui::TextUnformatted call. 1:1 mapping from API to ImGui call.
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 Step 2 (with Tasks 8, 9)
  - **Blocks**: None
  - **Blocked By**: Task 6 (widget base)

  **References**:
  - `include/unigui/widgets/widget_base.h` (Task 6) — base class to inherit from
  - ImGui Text API: `ImGui::TextUnformatted()` — safe text rendering without format string interpretation
  - `include/unigui/theme/theme.h` (Task 5) — theme auto-applies text color

  **Acceptance Criteria**:
  - [ ] `tests/widgets/label_test.cc` compiles and all 4 tests pass
  - [ ] `Label::Render()` calls `ImGui::TextUnformatted()` with stored text
  - [ ] `SetText()` updates text immediately (not deferred)

  **QA Scenarios**:

  ```
  Scenario: Label renders text
    Tool: Bash (pwsh)
    Preconditions: Build succeeds, ImGui context + GLFW backend initialized
    Steps:
      1. ctest --preset windows-msvc-debug -R "LabelTest" --output-on-failure
      2. Assert exit code 0, all 4 tests pass
    Expected Result: Label renders without crash, text get/set works, hidden label skips rendering
    Failure Indicators: Crash in TextUnformatted, text not updated after SetText, hidden label still renders
    Evidence: .omo/evidence/task-7-label-tests.log
  ```

  **Evidence to Capture**:
  - [x] `task-7-label-tests.log` — test output

  **Commit**: YES
  - Message: `feat(widgets): add Label widget`
  - Files: `include/unigui/widgets/label.h`, `src/widgets/label.cc`, `tests/widgets/label_test.cc`

- [ ] 8. Button Widget

  **What to do** (TDD):
  - **RED**: Write tests in `tests/widgets/button_test.cc`:
    - `Button_Render_DoesNotCrash` — create Button("btn", "Click Me"), call `Render()`, no crash
    - `Button_Clicked_ReturnsTrue` — simulate click (set `ImGui::GetIO().MouseDown[0]` and position over button rect), `WasClicked()` returns true
    - `Button_NotClicked_ReturnsFalse` — render without click, `WasClicked()` returns false
    - `Button_Disabled_DoesNotRespondToClick` — `SetEnabled(false)`, simulate click, `WasClicked()` still false
    - `Button_GetLabel_ReturnsGivenLabel` — `Button("btn", "Submit").GetLabel()` == "Submit"
    - `Button_Hidden_DoesNotRender` — `Hide()`, render, no crash
  - **GREEN**: Implement:
    - `include/unigui/widgets/button.h` — `class Button : public Widget { public: Button(std::string name, std::string label); void Render() override; bool WasClicked() const; void SetEnabled(bool enabled); bool IsEnabled() const; const std::string& GetLabel() const; void SetLabel(std::string label); }`
    - `src/widgets/button.cc` — `Render()`: if visible, calls `ImGui::BeginDisabled(!enabled_)`, then `ImGui::Button(label_.c_str(), ImVec2(0, 0))` (auto-sized width), then `ImGui::EndDisabled()`. Stores `ImGui::IsItemClicked()` result in `clicked_this_frame_`.
    - Returns `ImGui::Button()` boolean for `WasClicked()` — resets each frame via `Render()` call
    - Styling: Uses theme's `ImGuiCol_Button`, `ImGuiCol_ButtonHovered`, `ImGuiCol_ButtonActive`, `ImGuiCol_Text` (already set by Theme::Apply)
    - Size: Default `ImVec2(0, 0)` for auto-width; `FramePadding` from theme controls height

  **Must NOT do**:
  - Do NOT add icon support (icon + text button) in v1
  - Do NOT add `OnClick` callback pattern — use immediate-mode `WasClicked()` query
  - Do NOT add toggle button, radio button, or button group variants

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Straightforward widget — ImGui::Button wrapper with enabled/disabled state. No complex layout logic.
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 Step 2 (with Tasks 7, 9)
  - **Blocks**: Task 10 (Form uses Button for submit)
  - **Blocked By**: Task 6 (widget base)

  **References**:
  - `include/unigui/widgets/widget_base.h` (Task 6) — base class
  - ImGui Button API: `ImGui::Button()`, `ImGui::IsItemClicked()`, `ImGui::BeginDisabled()` / `ImGui::EndDisabled()`
  - `include/unigui/theme/theme.h` (Task 5) — button colors already in theme

  **Acceptance Criteria**:
  - [ ] `tests/widgets/button_test.cc` compiles and all 6 tests pass
  - [ ] `WasClicked()` returns true only on the frame the button is clicked
  - [ ] Disabled button does not register clicks
  - [ ] Button renders with theme colors (no hardcoded style pushes)

  **QA Scenarios**:

  ```
  Scenario: Button click detection
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "ButtonTest" --output-on-failure
      2. Assert exit code 0, 6 tests pass
      3. Specifically verify "Button_Clicked_ReturnsTrue" and "Button_NotClicked_ReturnsFalse" pass
    Expected Result: Click detection works in immediate-mode style — per-frame boolean
    Failure Indicators: WasClicked returns true without click, disabled button still clickable
    Evidence: .omo/evidence/task-8-button-tests.log

  Scenario: Button disabled state
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "Button_Disabled" --output-on-failure
      2. Assert exit code 0
    Expected Result: Disabled button ignores mouse input
    Failure Indicators: SetEnabled(false) has no effect, ImGui::BeginDisabled not called
    Evidence: .omo/evidence/task-8-button-disabled.log
  ```

  **Evidence to Capture**:
  - [x] `task-8-button-tests.log` — test output
  - [x] `task-8-button-disabled.log` — disabled state test output

  **Commit**: YES
  - Message: `feat(widgets): add Button widget`
  - Files: `include/unigui/widgets/button.h`, `src/widgets/button.cc`, `tests/widgets/button_test.cc`

- [ ] 9. Panel Widget

  **What to do** (TDD):
  - **RED**: Write tests in `tests/widgets/panel_test.cc`:
    - `Panel_Render_DoesNotCrash` — create Panel("panel", "My Panel"), call `Render()`, no crash
    - `Panel_IsCollapsed_DefaultsToFalse` — new panel starts expanded (`IsCollapsed()` false)
    - `Panel_ToggleCollapsed_FlipsState` — call `ToggleCollapsed()`, `IsCollapsed()` becomes true; call again, becomes false
    - `Panel_SetTitle_UpdatesTitle` — `SetTitle("New Title")`, then `GetTitle()` == "New Title"
    - `Panel_Hidden_DoesNotRender` — `Hide()`, render, no crash
    - `Panel_RenderCallback_IsCalled` — set a render callback (via lambda), verify it's invoked during `Render()` when panel is expanded
    - `Panel_Collapsed_RenderCallback_NotCalled` — collapse panel, verify callback is NOT called during `Render()`
  - **GREEN**: Implement:
    - `include/unigui/widgets/panel.h`:
      ```cpp
      class Panel : public Widget {
      public:
          Panel(std::string name, std::string title);
          void Render() override;
          void SetTitle(std::string title);
          const std::string& GetTitle() const;
          bool IsCollapsed() const;
          void ToggleCollapsed();
          void SetContentCallback(std::function<void()> callback);
      private:
          std::string title_;
          bool collapsed_ = false;
          std::function<void()> content_callback_;
      };
      ```
    - `src/widgets/panel.cc` — `Render()`: if visible, calls `ImGui::Begin(name_.c_str(), nullptr, ImGuiWindowFlags_None)` (no close button initially since we manage via Hide/Show; can add `&visible_` pointer for close button behavior). Renders title bar with collapse/expand triangle. If not collapsed, calls `content_callback_()` if set. Then `ImGui::End()`.
    - Uses `ImGui::Begin()` with `ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse` by default for clean panel appearance
    - Collapse state stored in `collapsed_` member and reflected in the Begin/End pair — when collapsed, `ImGui::Begin()` returns false (content skipped)

  **Must NOT do**:
  - Do NOT make Panel dockable by default (docking is on via theme but windows must opt-in)
  - Do NOT add drag-to-resize or splitter behavior
  - Do NOT add header customization (icon, custom buttons) in v1

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: Panel is the first compound widget — manages ImGui window begin/end pair, content callback, collapse state. Moderate complexity with multiple interacting features.
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 Step 2 (with Tasks 7, 8)
  - **Blocks**: Task 11 (Window uses Panel internally)
  - **Blocked By**: Task 6 (widget base)

  **References**:
  - ImGui Window API: `ImGui::Begin()`, `ImGui::End()`, `ImGuiWindowFlags` — window creation and flags
  - `include/unigui/widgets/widget_base.h` (Task 6) — base class
  - `include/unigui/theme/theme.h` (Task 5) — window styling from theme

  **Acceptance Criteria**:
  - [ ] `tests/widgets/panel_test.cc` compiles and all 7 tests pass
  - [ ] Panel renders via `ImGui::Begin/End` pair
  - [ ] Content callback invoked when expanded, skipped when collapsed
  - [ ] `SetTitle()` updates the ImGui window title on next frame

  **QA Scenarios**:

  ```
  Scenario: Panel lifecycle (create, collapse, expand, hide)
    Tool: Bash (pwsh)
    Preconditions: Build succeeds, ImGui context + GLFW backend initialized
    Steps:
      1. ctest --preset windows-msvc-debug -R "PanelTest" --output-on-failure
      2. Assert exit code 0, all 7 tests pass
    Expected Result: Panel renders with ImGui::Begin/End, collapse toggles content visibility
    Failure Indicators: ImGui::Begin/End mismatch (assertion), content callback called when collapsed
    Evidence: .omo/evidence/task-9-panel-tests.log

  Scenario: Panel content callback invoked correctly
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "Panel_RenderCallback" --output-on-failure
      2. Assert exit code 0
    Expected Result: Callback invoked exactly when panel is expanded and visible
    Failure Indicators: Callback not called, called twice, called when hidden/collapsed
    Evidence: .omo/evidence/task-9-panel-callback.log
  ```

  **Evidence to Capture**:
  - [x] `task-9-panel-tests.log` — test output
  - [x] `task-9-panel-callback.log` — callback test output

  **Commit**: YES
  - Message: `feat(widgets): add Panel widget`
  - Files: `include/unigui/widgets/panel.h`, `src/widgets/panel.cc`, `tests/widgets/panel_test.cc`

- [ ] 10. Form Widget

  **What to do** (TDD):
  - Per interview: "Form with labeled inputs, validation, error display, submit" — field types limited to Text + Checkbox for v1 (combo/dropdown deferred to v2), validation is required-field-only (no regex/minmax in v1).
  - **RED**: Write tests in `tests/widgets/form_test.cc`:
    - `Form_Render_DoesNotCrash` — create Form("form", "Settings"), call `Render()`, no crash
    - `Form_AddField_TextField_GetValue` — `AddTextField("username", "Enter name")`, set value "Alice", `GetFieldValue("username")` returns "Alice"
    - `Form_AddField_CheckboxField_GetValue` — `AddCheckbox("agree", "I agree")`, check it, `GetFieldValue("agree")` returns "1" (string representation of bool)
    - `Form_Validate_EmptyRequired_ReturnsError` — add required text field "email" with empty value, `Validate()` returns non-empty error list
    - `Form_Validate_AllValid_ReturnsNoErrors` — fill all required fields, `Validate()` returns empty error list
    - `Form_OnSubmit_Callback_IsCalled` — set submit callback, render with clicked submit button (simulate via test), verify callback invoked
    - `Form_Hidden_DoesNotRender` — `Hide()`, render, no crash
  - **GREEN**: Implement:
    - `include/unigui/widgets/form.h`:
      ```cpp
      struct FormField {
          std::string name;
          std::string label;
          enum class Type { Text, Checkbox } type;
          bool required = false;
          std::string value;  // string for all types (checkbox = "0"/"1")
      };

      struct FormError {
          std::string field_name;
          std::string message;
      };

      class Form : public Widget {
      public:
          Form(std::string name, std::string title);
          void Render() override;

          void AddTextField(std::string name, std::string label, bool required = false);
          void AddCheckbox(std::string name, std::string label);
          std::string GetFieldValue(const std::string& name) const;
          void SetFieldValue(const std::string& name, std::string value);

          std::vector<FormError> Validate() const;
          void SetOnSubmit(std::function<void()> callback);
      private:
          std::string title_;
          std::vector<FormField> fields_;
          std::function<void()> on_submit_;
          std::vector<FormError> last_errors_;
      };
      ```
    - `src/widgets/form.cc` — `Render()`: if visible, uses `ImGui::Begin(title_.c_str())`. Iterates `fields_`, for each type:
      - Text: `ImGui::InputText(label, &value)` — uses `ImGuiInputTextFlags_EnterReturnsTrue` for submit-on-enter
      - Checkbox: `ImGui::Checkbox(label, &bool_value)` converting to/from "0"/"1" string
    - After all fields, renders a "Submit" Button (uses Task 8's Button internally or raw `ImGui::Button`). If Submit clicked, calls `Validate()`, if errors empty calls `on_submit_()`, else stores errors for display.
    - `Validate()`: checks all `required` fields have non-empty `value`, returns `vector<FormError>` with field name and "Required field" message
    - If `last_errors_` non-empty, renders them in red below the form using `ImGui::TextColored(red, error.message)`

  **Must NOT do**:
  - Do NOT add combo/dropdown, slider, color picker, or date picker field types in v1
  - Do NOT add custom validation rules (regex, min/max, etc.) — only "required" check
  - Do NOT add form layout options (horizontal, grid, etc.) — vertical stack only

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: Form is the most complex v1 widget — field management, validation logic, submit flow, error display. Multiple interacting states.
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: NO (depends on Task 8 — Button widget used for submit)
  - **Parallel Group**: Wave 2 Step 3 (standalone)
  - **Blocks**: None
  - **Blocked By**: Tasks 6 (widget base), 8 (uses Button for submit)

  **References**:
  - `include/unigui/widgets/widget_base.h` (Task 6) — base class
  - `include/unigui/widgets/button.h` (Task 8) — Button widget for submit
  - `include/unigui/widgets/label.h` (Task 7) — may use Label for field labels
  - ImGui Widgets API: `ImGui::InputText()`, `ImGui::Checkbox()`, `ImGuiInputTextFlags`
  - `include/unigui/theme/theme.h` (Task 5) — text input and checkbox colors
  - `include/unigui/core/error.h` (Task 2) — `ErrorCode` enum for validation errors

  **Acceptance Criteria**:
  - [ ] `tests/widgets/form_test.cc` compiles and all 7 tests pass
  - [ ] `Validate()` correctly identifies empty required fields
  - [ ] `OnSubmit` callback fires only when validation passes
  - [ ] Text and checkbox fields render and accept input
  - [ ] Errors displayed in red below form when validation fails

  **QA Scenarios**:

  ```
  Scenario: Form field add and get values
    Tool: Bash (pwsh)
    Preconditions: Build succeeds, ImGui context initialized
    Steps:
      1. ctest --preset windows-msvc-debug -R "Form_AddField" --output-on-failure
      2. Assert exit code 0, text field and checkbox tests pass
    Expected Result: AddField creates accessible fields, GetFieldValue returns correct values
    Failure Indicators: Field value not stored/retrieved, checkbox value not "0"/"1"
    Evidence: .omo/evidence/task-10-form-fields.log

  Scenario: Form validation
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "Form_Validate" --output-on-failure
      2. Assert exit code 0
      3. Assert "empty required" test yields at least 1 error
      4. Assert "all valid" test yields 0 errors
    Expected Result: Required empty field produces error; all-filled produces no errors
    Failure Indicators: Empty required field passes validation, filled field wrongly flagged
    Evidence: .omo/evidence/task-10-form-validation.log

  Scenario: Form submit callback
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "Form_OnSubmit" --output-on-failure
      2. Assert exit code 0
    Expected Result: Submit callback invoked only when form validates successfully
    Failure Indicators: Callback not called, called despite validation errors
    Evidence: .omo/evidence/task-10-form-submit.log
  ```

  **Evidence to Capture**:
  - [x] `task-10-form-fields.log` — field tests
  - [x] `task-10-form-validation.log` — validation tests
  - [x] `task-10-form-submit.log` — submit callback test

  **Commit**: YES
  - Message: `feat(widgets): add Form widget with validation`
  - Files: `include/unigui/widgets/form.h`, `src/widgets/form.cc`, `tests/widgets/form_test.cc`

- [ ] 11. Window Widget

  **What to do** (TDD):
  - **RED**: Write tests in `tests/widgets/window_test.cc`:
    - `Window_Render_DoesNotCrash` — create Window("win", "Main Window"), call `Render()`, no crash
    - `Window_AddPanel_PanelRendered` — add a Panel child, render Window, verify Panel's content callback is invoked
    - `Window_HasMenuBar_DefaultsToFalse` — new window `HasMenuBar()` returns false
    - `Window_EnableMenuBar_RendersMenuBar` — `SetMenuBarEnabled(true)`, render, verify `ImGui::BeginMenuBar()` is called (via test spy or callback)
    - `Window_SetSize_SetsWindowSize` — `SetSize(800, 600)`, verify `ImGui::SetNextWindowSize()` is called with correct values
    - `Window_DockingEnabled_DefaultsToTrue` — new Window has docking flag set (can verify via `ImGuiWindowFlags_DockNodeHost` or similar)
    - `Window_Close_CallsOnCloseCallback` — set `SetOnClose(callback)`, close window (simulate clicking X), verify callback fires
  - **GREEN**: Implement:
    - `include/unigui/widgets/window.h`:
      ```cpp
      class Window : public Widget {
      public:
          Window(std::string name, std::string title);

          void Render() override;
          void AddPanel(std::shared_ptr<Panel> panel);
          void RemovePanel(const std::string& panel_name);

          void SetSize(float width, float height);
          void SetMenuBarEnabled(bool enabled);
          bool HasMenuBar() const;
          void SetOnClose(std::function<void()> callback);

      private:
          std::string title_;
          std::vector<std::shared_ptr<Panel>> panels_;
          bool menu_bar_enabled_ = false;
          float width_ = 0, height_ = 0;  // 0 = auto-size
          ImGuiWindowFlags flags_ = ImGuiWindowFlags_None;
          std::function<void()> on_close_;
      };
      ```
    - `src/widgets/window.cc` — `Render()`: if not visible, return. Sets `ImGui::SetNextWindowSize(ImVec2(width_, height_), ImGuiCond_FirstUseEver)`. Calls `ImGui::Begin(title_.c_str(), &visible_, flags_)`. If `Begin()` returns true:
      - If `menu_bar_enabled_`, calls `ImGui::BeginMenuBar()`/`ImGui::EndMenuBar()` pair with placeholder "File" menu
      - Iterates `panels_` and calls `panel->Render()` for each
      - Calls `ImGui::End()`
    - If window was closed (X button clicked → `visible_` becomes false), calls `on_close_()` if set
    - Docking: Window uses `ImGuiWindowFlags_NoDocking` by default? NO — user chose docking enabled. Set `ImGuiConfigFlags_DockingEnable` in context creation (Task 12 will handle this, Window just needs to not opt-out).
    - Menu bar: placeholder with "File > Exit" for demo purposes

  **Must NOT do**:
  - Do NOT implement full menu system — placeholder "File > Exit" only (this is ImGui BeginMenuBar usage, NOT a standalone MenuBar widget)
  - Do NOT add viewport/docking configuration API — use ImGui defaults (docking only, no multi-viewport)
  - Do NOT add status bar — v2 widget

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: Window is the top-level container — manages Begin/End lifecycle, child panels, menu bar integration, close callback. Moderate complexity.
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: NO (runs before Task 12 in Wave 3 — Window is needed by App Bootstrap)
  - **Parallel Group**: Wave 3 (runs before Task 12)
  - **Blocks**: Task 12 (App Bootstrap creates Window), Task 13 (example uses Window)
  - **Blocked By**: Tasks 4 (backend), 5 (theme), 6 (widget base), 9 (Panel)

  **References**:
  - ImGui Window API: `ImGui::Begin()`, `ImGui::End()`, `ImGui::SetNextWindowSize()`, `ImGuiWindowFlags`
  - ImGui Docking: `ImGuiConfigFlags_DockingEnable`, `ImGui::DockSpace()`
  - `include/unigui/widgets/panel.h` (Task 9) — Panel child widget
  - `include/unigui/widgets/widget_base.h` (Task 6) — base class
  - `include/unigui/theme/theme.h` (Task 5) — window styling

  **Acceptance Criteria**:
  - [ ] `tests/widgets/window_test.cc` compiles and all 7 tests pass
  - [ ] Window renders child Panels
  - [ ] Menu bar renders when enabled
  - [ ] Close callback fires on window close
  - [ ] Docking flag set (windows can be docked)

  **QA Scenarios**:

  ```
  Scenario: Window with child panels
    Tool: Bash (pwsh)
    Preconditions: Build succeeds, ImGui context + GLFW backend initialized
    Steps:
      1. ctest --preset windows-msvc-debug -R "Window_AddPanel|Window_Render" --output-on-failure
      2. Assert exit code 0
    Expected Result: Window renders, child panel callbacks are invoked
    Failure Indicators: Panel callback not called, ImGui::Begin/End mismatch
    Evidence: .omo/evidence/task-11-window-panels.log

  Scenario: Window close callback
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "Window_Close" --output-on-failure
      2. Assert exit code 0
    Expected Result: Close button triggers callback
    Failure Indicators: Callback not fired, fired multiple times
    Evidence: .omo/evidence/task-11-window-close.log

  Scenario: Window menu bar
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "Window_EnableMenuBar|Window_HasMenuBar" --output-on-failure
      2. Assert exit code 0
    Expected Result: Menu bar renders when enabled, does not crash
    Failure Indicators: Menu bar assertion, BeginMenuBar/EndMenuBar mismatch
    Evidence: .omo/evidence/task-11-window-menubar.log
  ```

  **Evidence to Capture**:
  - [x] `task-11-window-panels.log` — window + panels test
  - [x] `task-11-window-close.log` — close callback test
  - [x] `task-11-window-menubar.log` — menu bar test

  **Commit**: YES
  - Message: `feat(widgets): add Window widget with docking and menu bar`
  - Files: `include/unigui/widgets/window.h`, `src/widgets/window.cc`, `tests/widgets/window_test.cc`

- [ ] 12. Application Bootstrap & Main Loop

  **What to do** (TDD):
  - **RED**: Write tests in `tests/app/app_test.cc`:
    - `Init_ValidConfig_ReturnsTrue` — call `unigui::Init({.width=800, .height=600, .title="Test"})`, assert returns true
    - `Init_Twice_ReturnsFalse` — second `Init()` without `Shutdown()` returns false (logs warning)
    - `Shutdown_BeforeInit_ReturnsGracefully` — `Shutdown()` before `Init()` doesn't crash
    - `Shutdown_AfterInit_CleansUp` — `Init()`, `Shutdown()`, then `Init()` again works (can re-init)
    - `NewFrame_BeforeInit_ReturnsFalse` — `NewFrame()` before `Init()` returns false
    - `NewFrame_AfterInit_ReturnsTrue` — `Init()`, `NewFrame()` returns true
    - `Run_Callback_IsCalled` — provide a callback, `Run()` calls it and exits when `ShouldClose` signals
    - `Render_AfterNewFrame_DoesNotCrash` — `NewFrame()`, `Render()` doesn't crash
  - **GREEN**: Implement:
    - `include/unigui/app/app.h`:
      ```cpp
      namespace unigui {
      struct AppConfig {
          int width = 1280;
          int height = 720;
          const char* title = "UniGUI Application";
          ThemeConfig theme = { ThemePreset::Dark, 1.0f, 16.0f };
      };

      bool Init(const AppConfig& config);
      void Shutdown();
      bool NewFrame();
      void Render();
      bool ShouldClose();
      void Run(const std::function<void()>& callback);
      } // namespace unigui
      ```
    - `src/app/app.cc`:
      - `Init()`: Creates `ImGuiContext` via `ImGui::CreateContext()`. Enables docking config flag ONLY (`ImGuiConfigFlags_DockingEnable` — NOT viewports, which is deferred to v2). Calls `Theme::Apply(config.theme)`. Creates GLFW window (visible, sized per config). Calls `GLFWPlatform::Init()` and `OpenGL3Renderer::Init()`. Stores state in module-level static (single-instance in v1).
      - `Shutdown()`: Calls `RendererBackend::Shutdown()`, `PlatformBackend::Shutdown()`, `ImGui::DestroyContext()`.
      - `NewFrame()`: Calls `PlatformBackend::NewFrame()`, `ImGui::NewFrame()`.
      - `Render()`: Calls `ImGui::Render()`, `RendererBackend::SetClearColor(theme_clear_color)`, `glClear(GL_COLOR_BUFFER_BIT)`, `RendererBackend::RenderDrawData(ImGui::GetDrawData())`, `glfwSwapBuffers()`.
      - `ShouldClose()`: Delegates to `PlatformBackend::ShouldClose()`.
      - `Run(callback)`: Loop: `while(!ShouldClose()) { PollEvents(); NewFrame(); callback(); Render(); }`. Then `Shutdown()`.
    - Update `include/unigui/unigui.h` to include `app/app.h`

  **Must NOT do**:
  - Do NOT add event system, input hooks, or callback registries — just the bare loop
  - Do NOT add multi-window management — v1 is single-window
  - Do NOT add command-line argument parsing (defer to example app)
  - Do NOT add config file loading (.ini is handled by ImGui default)

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: App bootstrap is the central integration point — wires together backend, theme, context, and render loop. Must handle init/shutdown ordering, error states, and re-entrancy correctly.
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: NO (runs after Task 11 in Wave 3 — needs Window widget)
  - **Parallel Group**: Wave 3 (runs after Task 11)
  - **Blocks**: Task 13 (example uses app bootstrap)
  - **Blocked By**: Tasks 4 (backend), 5 (theme), 11 (Window widget)

  **References**:
  - Dear ImGui init pattern: `ImGui::CreateContext()`, `ImGui::DestroyContext()`, `ImGui::NewFrame()`, `ImGui::Render()`, `ImGui::GetIO().ConfigFlags`
  - `include/unigui/backend/platform_backend.h` (Task 3) — platform interface
  - `include/unigui/backend/renderer_backend.h` (Task 3) — renderer interface
  - `include/unigui/theme/theme.h` (Task 5) — ThemeConfig, Theme::Apply
  - `include/unigui/core/context.h` (Task 2) — Context management

  **Acceptance Criteria**:
  - [ ] `tests/app/app_test.cc` compiles and all 8 tests pass
  - [ ] `Init()` creates ImGui context, GLFW window, OpenGL context, applies theme
  - [ ] `Run()` loop calls callback each frame, exits on ShouldClose
  - [ ] `Shutdown()` cleans up in correct order (renderer → platform → ImGui)
  - [ ] Re-entrant init works (Shutdown then Init again)

  **QA Scenarios**:

  ```
  Scenario: Full init / newframe / render / shutdown cycle
    Tool: Bash (pwsh)
    Preconditions: Build succeeds, GLFW DLLs available in PATH
    Steps:
      1. ctest --preset windows-msvc-debug -R "AppTest" --output-on-failure
      2. Assert exit code 0, 8 tests pass
      3. Verify Init returns true, NewFrame returns true, Render doesn't crash
    Expected Result: Complete app lifecycle works without crash or ImGui assertion
    Failure Indicators: GLFW init failure (no GPU driver), OpenGL context creation failure, ImGui assertion on NewFrame before Init
    Evidence: .omo/evidence/task-12-app-tests.log

  Scenario: Run loop with callback
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. ctest --preset windows-msvc-debug -R "Run_Callback" --output-on-failure
      2. Assert exit code 0
    Expected Result: Callback invoked at least once, loop exits cleanly
    Failure Indicators: Callback never called, infinite loop, crash on exit
    Evidence: .omo/evidence/task-12-run-loop.log
  ```

  **Evidence to Capture**:
  - [x] `task-12-app-tests.log` — app lifecycle test output
  - [x] `task-12-run-loop.log` — run loop test output

  **Commit**: YES
  - Message: `feat(app): add application bootstrap and main loop API`
  - Files: `include/unigui/app/app.h`, `src/app/app.cc`, `tests/app/app_test.cc`, updated `include/unigui/unigui.h`

- [ ] 13. hello_unigui Example Application

  **What to do**:
  - Create `examples/hello_unigui/CMakeLists.txt`:
    ```cmake
    add_executable(hello_unigui main.cc)
    target_link_libraries(hello_unigui PRIVATE unigui::unigui)
    target_compile_features(hello_unigui PRIVATE cxx_std_23)
    ```
  - Update top-level `CMakeLists.txt`: add `option(UNIGUI_BUILD_EXAMPLES "Build example applications" ON)` and `if(UNIGUI_BUILD_EXAMPLES) add_subdirectory(examples/hello_unigui) endif()`
  - Create `examples/hello_unigui/main.cc`:
    ```cpp
    #include <unigui/unigui.h>
    #include <cstdio>

    int main(int argc, char** argv) {
        int max_frames = 0;
        for (int i = 1; i < argc; i++) {
            std::string_view arg = argv[i];
            if (arg == "--frames" && i+1 < argc) max_frames = std::atoi(argv[++i]);
        }

        unigui::AppConfig config;
        config.width = 1280;
        config.height = 720;
        config.title = "Hello UniGUI";

        if (!unigui::Init(config)) {
            std::fprintf(stderr, "[unigui] Failed to initialize\n");
            return 1;
        }
        std::printf("[unigui] Initialized backend=GLFW+OpenGL3\n");

        int frame_count = 0;
        bool done = false;

        while (!done && !unigui::ShouldClose()) {
            unigui::NewFrame();

            // Demo UI: a simple window with a panel and form
            {
                static auto window = std::make_shared<unigui::Window>("demo", "UniGUI Demo");
                static bool first = true;
                if (first) {
                    auto panel = std::make_shared<unigui::Panel>("info", "Information");
                    panel->SetContentCallback([]() {
                        ImGui::TextWrapped("Welcome to UniGUI! This is a demo application showing the dark theme and widget library.");
                    });
                    window->AddPanel(panel);
                    window->SetMenuBarEnabled(true);
                    first = false;
                }
                window->Render();
            }

            unigui::Render();
            frame_count++;

            if (max_frames > 0 && frame_count >= max_frames) {
                std::printf("[unigui] frame %d/%d rendered\n", frame_count, max_frames);
                done = true;
            }
        }

        unigui::Shutdown();
        std::printf("[unigui] Shutdown complete\n");
        return 0;
    }
    ```
  - The example demonstrates: Window creation, Panel embedding, menu bar, dark theme applied, ImGui docking enabled

  **Must NOT do**:
  - Do NOT add complex demo UI — keep it simple: one window, one panel, one text
  - Do NOT add more example apps (v2: form_demo, panel_demo, etc.)
  - Do NOT add screenshot capability in v1

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: Example application — needs to integrate all library components correctly. Must compile, link, and run successfully.
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4 (with Task 14)
  - **Blocks**: None
  - **Blocked By**: Task 12 (app bootstrap)

  **References**:
  - `include/unigui/app/app.h` (Task 12) — Init/Shutdown/NewFrame/Render API
  - `include/unigui/widgets/window.h` (Task 11) — Window widget
  - `include/unigui/widgets/panel.h` (Task 9) — Panel widget

  **Acceptance Criteria**:
  - [ ] `examples/hello_unigui/main.cc` compiles without errors or warnings
  - [ ] `examples/hello_unigui/build/.../hello_unigui.exe --frames 10 --headless` → exit 0, prints:
    - `[unigui] Initialized backend=GLFW+OpenGL3`
    - `[unigui] frame 10/10 rendered`
    - `[unigui] Shutdown complete`
  - [ ] No GLFW errors, OpenGL errors, or ImGui assertion failures in output

  **QA Scenarios**:

  ```
  Scenario: Example runs 10 frames
    Tool: Bash (pwsh)
    Preconditions: Build succeeds, GLFW DLLs in PATH (display required)
    Steps:
      1. & "build/windows-msvc-release/examples/hello_unigui/hello_unigui.exe" --frames 10
      2. Assert exit code 0
      3. Assert stdout contains "[unigui] Initialized backend=GLFW+OpenGL3"
      4. Assert stdout contains "[unigui] frame 10/10 rendered"
      5. Assert stdout contains "[unigui] Shutdown complete"
    Expected Result: 10 frames rendered, clean shutdown, no errors
    Failure Indicators: Exit code non-zero, missing log lines, GLFW error messages
    Evidence: .omo/evidence/task-13-hello-unigui.log

  Scenario: Example build verification
    Tool: Bash (pwsh)
    Preconditions: Build succeeds
    Steps:
      1. Test-Path "build/windows-msvc-release/examples/hello_unigui/hello_unigui.exe"
      2. Assert file exists
    Expected Result: hello_unigui.exe built successfully
    Failure Indicators: Executable not found, build failed
    Evidence: .omo/evidence/task-13-build-exists.txt
  ```

  **Evidence to Capture**:
  - [x] `task-13-hello-unigui.log` — headless run output
  - [x] `task-13-build-exists.txt` — build confirmation

  **Commit**: YES
  - Message: `feat(examples): add hello_unigui demo application`
  - Files: `examples/hello_unigui/main.cc`, `examples/hello_unigui/CMakeLists.txt`, updated top-level `CMakeLists.txt`

- [ ] 14. README Documentation

  **What to do**:
  - Create `README.md` at repository root with:
    - **Title**: "TeamkillerUniGUI — Modern Dear ImGui C++ Wrapper"
    - **Badges**: C++23, CMake, vcpkg, platforms (Windows/Linux/macOS)
    - **Overview**: 2-3 sentences describing the library — dark theme engine + widget library + GLFW/OpenGL3 backend
    - **Quick Start**: 
      ```bash
      git clone https://xbw-nas.iepose.cn/Teamkiller131/TeamkillerUniGUI.git
      cd TeamkillerUniGUI
      cmake --preset windows-msvc-release
      cmake --build --preset windows-msvc-release
      ctest --preset windows-msvc-release
      ./build/windows-msvc-release/examples/hello_unigui/hello_unigui.exe
      ```
    - **Prerequisites**: CMake 3.31+, Ninja, vcpkg at `D:\vcpkg` (or `$VCPKG_ROOT`), MSVC 19.40+ / GCC 14+ / Clang 18+
    - **Architecture** (diagram as ASCII art): `User Code → unigui:: API → Widgets / Theme → ImGui → GLFW+OpenGL3 Backend → OS`
    - **API Overview**: code snippets for `unigui::Init()`, `unigui::Run()`, creating a Window, adding a Panel, using a Form
    - **Theme**: description of dark theme, screenshot placeholder
    - **Widgets**: list of 5 v1 widgets with brief descriptions
    - **Platform Notes**: macOS OpenGL deprecated (capped at 4.1), Linux X11/Wayland support, Windows primary target
    - **License**: MIT (add `LICENSE` file)
    - **Contributing**: basic guidelines (submit issues, PRs welcome)

  **Must NOT do**:
  - Do NOT create a docs/ directory or Doxygen site (README only for v1)
  - Do NOT add a CONTRIBUTING.md, CODE_OF_CONDUCT.md, or CHANGELOG.md
  - Do NOT add GitHub Actions CI configuration (v2)

  **Recommended Agent Profile**:
  - **Category**: `writing`
    - Reason: Documentation — clear prose, code snippets, ASCII diagrams. No code changes.
  - **Skills**: None

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4 (with Task 13)
  - **Blocks**: None
  - **Blocked By**: None (can start anytime, but best after all APIs are stable per Wave 3)

  **References**:
  - `include/unigui/unigui.h` — public API surface to document
  - `include/unigui/app/app.h` (Task 12) — Init/Run API
  - `include/unigui/widgets/window.h` (Task 11) — Window API
  - `include/unigui/theme/theme.h` (Task 5) — Theme API

  **Acceptance Criteria**:
  - [ ] `README.md` exists at repository root with all sections listed above
  - [ ] Quick Start commands are copy-paste runnable on Windows
  - [ ] API code snippets compile against the actual API
  - [ ] `LICENSE` file exists with MIT license text

  **QA Scenarios**:

  ```
  Scenario: README exists and has required sections
    Tool: Bash (pwsh)
    Preconditions: Repo exists
    Steps:
      1. Test-Path "README.md" → True
      2. Select-String -Path "README.md" -Pattern "Quick Start" → found
      3. Select-String -Path "README.md" -Pattern "Architecture" → found
      4. Select-String -Path "README.md" -Pattern "API Overview" → found
      5. Test-Path "LICENSE" → True
    Expected Result: README has all required sections, LICENSE exists
    Failure Indicators: Missing sections, broken markdown
    Evidence: .omo/evidence/task-14-readme-sections.txt
  ```

  **Evidence to Capture**:
  - [x] `task-14-readme-sections.txt` — verification of README sections

  **Commit**: YES
  - Message: `docs: add README with build instructions and API overview`
  - Files: `README.md`, `LICENSE`, `LICENSE`

---

## Final Verification Wave (MANDATORY — after ALL implementation tasks)

> 4 review agents run in PARALLEL. ALL must APPROVE. Present consolidated results to user and get explicit "okay" before completing.

- [ ] F1. **Plan Compliance Audit** — `oracle`
  Read the plan end-to-end. For each "Must Have": verify implementation exists (read file, run command). For each "Must NOT Have": search codebase for forbidden patterns — reject with file:line if found. Check evidence files exist in `.omo/evidence/`. Compare deliverables against plan.
  Output: `Must Have [N/N] | Must NOT Have [N/N] | Tasks [N/N] | VERDICT: APPROVE/REJECT`

- [ ] F2. **Code Quality Review** — `unspecified-high`
  Run build + tests: `cmake --preset windows-msvc-release && cmake --build --preset windows-msvc-release && ctest --preset windows-msvc-release --output-on-failure`. Review all changed files for: C-style casts, raw `new`/`delete` (should use `std::unique_ptr`), uninitialized variables, missing `#include` guards, `using namespace` in headers, AI slop (excessive comments, over-abstraction, generic names like `data`/`result`/`item`/`temp`).
  Output: `Build [PASS/FAIL] | Tests [N pass/N fail] | Files [N clean/N issues] | VERDICT`

- [ ] F3. **Real Manual QA** — `unspecified-high`
  Start from clean build. Run `hello_unigui --frames 100 --headless` — verify exit 0, ≥90fps average. Run full test suite. Verify: `find_package(unigui)` from separate project. Cross-task integration: Window opens Panel, Panel contains Form, Form uses Button for submit. Edge cases: empty context init, double shutdown, missing font file.
  Output: `Scenarios [N/N pass] | Integration [N/N] | Edge Cases [N tested] | VERDICT`

- [ ] F4. **Scope Fidelity Check** — `deep`
  For each task: read "What to do", read actual diff (git log/diff). Verify 1:1 — everything in spec was built (no missing), nothing beyond spec was built (no creep). Check "Must NOT do" compliance. Detect cross-task contamination. Flag unaccounted changes.
  Output: `Tasks [N/N compliant] | Contamination [CLEAN/N issues] | Unaccounted [CLEAN/N files] | VERDICT`

---

## Commit Strategy

All commits use Conventional Commits format: `type(scope): description`

- **1**: `feat(build): add CMake, vcpkg manifest, and presets` — all build files
- **2**: `feat(core): add unigui context, error handling, and version header` — include/unigui/core/, src/core/, tests/core/
- **3**: `feat(backend): add platform and renderer backend interfaces` — include/unigui/backend/, tests/backend/
- **4**: `feat(backend): implement GLFW+OpenGL3 backend` — src/backend/, tests/backend/
- **5**: `feat(theme): add dark theme engine and style scope RAII` — include/unigui/theme/, src/theme/, tests/theme/
- **6**: `feat(widgets): add widget base infrastructure` — include/unigui/widgets/base.h, src/widgets/, tests/widgets/
- **7**: `feat(widgets): add Label widget` — widget files + tests
- **8**: `feat(widgets): add Button widget` — widget files + tests
- **9**: `feat(widgets): add Panel widget` — widget files + tests
- **10**: `feat(widgets): add Form widget` — widget files + tests
- **11**: `feat(widgets): add Window widget with docking` — widget files + tests
- **12**: `feat(app): add application bootstrap and main loop API` — src/app/, tests/app/
- **13**: `feat(examples): add hello_unigui demo application` — examples/hello_unigui/
- **14**: `docs: add README with build instructions and API overview` — README.md

## Success Criteria

### Verification Commands
```bash
# Windows (primary target)
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release --output-on-failure
build/windows-msvc-release/examples/hello_unigui/hello_unigui.exe --frames 50
# Expected: exit 0, logs "[unigui] backend=GLFW+OpenGL3", "[unigui] frame 50/50 rendered"

# Linux
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug --output-on-failure

# macOS
cmake --preset macos-clang-debug
cmake --build --preset macos-clang-debug
ctest --preset macos-clang-debug --output-on-failure
```

### Final Checklist
- [ ] All "Must Have" present
- [ ] All "Must NOT Have" absent
- [ ] `cmake --build` exits 0 on all 3 platforms
- [ ] `ctest` passes 100% with ≥30 test cases
- [ ] `hello_unigui --headless` renders successfully
- [ ] Downstream `find_package(unigui CONFIG)` works
- [ ] All 14 tasks have QA evidence in `.omo/evidence/`
- [ ] F1-F4 all return APPROVE
