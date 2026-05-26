# UniGUI v1.0.0 — Window Resize Support

## TL;DR

> **Quick Summary**: Add dynamic window resize for DX11 backend. GLFW platform detects size changes, DX11 swapchain resizes, RTV recreates. GLFW_GL3 works natively.
> 
> **Deliverables**: Resizable windows with correct mouse coordinates and font rendering at any size.
> **Estimated Effort**: Quick (~6 tasks)
> **Test Target**: 186 测试不回归，4 sample 可缩放

---

## Context

DX11 swapchain is fixed-size at creation. When window resizes:
- Swapchain stays at old size → mouse coordinates mismatch
- `GLFW_RESIZABLE=FALSE` was a workaround — now removed
- Need proper swapchain resize via `IDXGISwapChain::ResizeBuffers`

GLFW_GL3 works naturally — `glfwSwapBuffers` handles any framebuffer size.

---

## Work Objectives

### Core Objective
DX11 窗口可自由缩放，swapchain 自动重建，鼠标坐标始终正确。

### Must Have
- DX11 swapchain resize on window size change
- RTV recreation for new back buffer
- Correct io.DisplaySize update
- GLFW_RESIZABLE=TRUE restored

### Must NOT Have
- Mini map / drag-ghost effects (pure functional)
- Rebuild entire pipeline (only swapchain + RTV)

---

## TODOs

- [ ] 1. DX11Renderer: add ResizeSwapChain(w, h) method
  **What**: In dx11_renderer.h/cc, add `void ResizeSwapChain(int w, int h)`.
  Steps: `swapchain_->ResizeBuffers(2, w, h, DXGI_FORMAT_R8G8B8A8_UNORM, 0)`,
  recreate RTV from new back buffer, update viewport.
  **QA**: Compiles, method callable

- [ ] 2. glfw_platform: re-enable GLFW_RESIZABLE + expose GetClientSize
  **What**: Set `GLFW_RESIZABLE=GLFW_TRUE`. Add `GetClientSize(int* w, int* h)` to PlatformBackend
  that returns current framebuffer/client size. GLFW impl: `glfwGetWindowSize`.
  DX11 path: can also use `GetClientRect` on HWND.
  **QA**: Window resizable, GetClientSize returns actual dimensions

- [ ] 3. app.cc: detect resize in NewFrame + trigger swapchain resize
  **What**: In `NewFrame()`, compare current client size with last known size.
  If changed, call `dxr->ResizeSwapChain(w, h)` and update `io.DisplaySize`.
  Use `g_platform->GetClientSize(&w, &h)`.
  **QA**: Resizing window triggers swapchain resize (verify via log)

- [ ] 4. app.cc: update io.DisplaySize on resize
  **What**: After swapchain resize, set `io.DisplaySize = ImVec2(w, h)`.
  This ensures mouse coordinates match new window size.
  **QA**: Mouse clicks align with UI elements after resize

- [ ] 5. Remove GLFW_RESIZABLE=FALSE workaround
  **What**: Remove the flag from glfw_platform.cc.
  **QA**: Window starts resizable

- [ ] 6. Build + test: 186 tests pass, all 4 samples resize correctly
  **What**: Full build, ctest, manual sample verification.
  **QA**: Window resizes, UI scales correctly, no mouse offset

---

## Scope

### INCLUDE
- DX11 swapchain resize
- GLFW_RESIZABLE=true
- Correct mouse coordinates after resize

### EXCLUDE
- Animated resize transitions
- Vulkan/SDL3 resize (Vulkan needs surface recreation)
- Full pipeline rebuild on resize
