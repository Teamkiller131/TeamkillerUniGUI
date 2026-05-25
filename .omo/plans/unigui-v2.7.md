# TeamkillerUniGUI v2.7 — Backend Completion

## TL;DR

> **Quick Summary**: Complete all remaining backends — DX12 renderer, WebGPU renderer, full Metal implementation (macOS), Emscripten/Web support. BackendType entries for all.
> 
> **Test Target**: ≥165 tests (no regression, backend-specific tests)
> **Tasks**: ~8 tasks

---

## Backend Tasks

- [ ] 1. DX12 Renderer Backend
  **What**: `src/backend/dx12_renderer.cc` — ImGui DX12 backend wrapper. DXGI factory, device, command queue, swapchain, descriptor heaps. ~400 lines.
  **QA**: Compiles on Windows with DX12 SDK

- [ ] 2. DX12 vcpkg Integration
  **What**: Add `imgui[dx12-binding]` to vcpkg.json. CMake conditional for DX12. `UNIGUI_HAS_DX12` define.
  **QA**: `cmake --preset windows-msvc-debug` configures with DX12

- [ ] 3. WebGPU Renderer Backend
  **What**: `src/backend/webgpu_renderer.cc` — Dawn/WGPU backend. Wraps `imgui_impl_wgpu`. Cross-platform modern graphics.
  **QA**: Compiles with webgpu headers

- [ ] 4. Full Metal Implementation
  **What**: Convert `metal_renderer.cc` stub to `metal_renderer.mm` — Objective-C++ with MTKView, MTLDevice, CAMetalLayer. Full render loop.
  **QA**: Compiles on macOS

- [ ] 5. Emscripten Platform Backend
  **What**: `src/backend/emscripten_platform.cc` — Web platform using Emscripten HTML5 API. Canvas, input, gamepad.
  **QA**: `emcmake cmake` configures for Emscripten

- [ ] 6. BackendType Entries
  **What**: Add `DX12`, `WebGPU`, `Emscripten` to BackendType enum. BackendFactory routing.
  **QA**: All 7 backend types compile

- [ ] 7. Backend Comparison Table (README)
  **What**: Matrix: GL3/Vulkan/DX11/DX12/Metal/WebGPU/Emscripten × Platform/Anti-aliasing/Performance/Complexity.
  **QA**: `Select-String -Path "README.md" -Pattern "DX12"` → found

- [ ] 8. CHANGELOG v2.7
  **What**: Document all new backends.
  **QA**: CHANGELOG updated

---

## Scope

### INCLUDE: DX12, WebGPU, Metal, Emscripten, BackendType docs
### EXCLUDE: Backend-specific UI features, performance tuning
