# UniGUI v4.0 — Cross-Platform

## TL;DR

> **Quick Summary**: 多平台正式支持 — Linux (X11/Wayland)、macOS (Metal/MoltenVK)、Web (Emscripten WASM)。
> v4 不做新功能，纯平台适配 + CI/CD 管道。

## Target Platforms

| Platform | Backend | Graphics | Status | Priority |
|----------|---------|----------|--------|----------|
| Windows | GLFW+DX11/GL3 | DirectX 11 / OpenGL 3.3 | ★ Production | P0 (keep) |
| Linux | GLFW+GL3 | OpenGL 3.3 (X11/Wayland) | ★ Compiles (225/225) | P1 ✅ |
| macOS | GLFW+GL3/Metal | OpenGL 3.3 / Metal 2 | ✓ Code ready, untested | P1 |
| Web | Emscripten | WebGL 2.0 | ✓ Code ready, untested | P2 |

---

## Progress Log

### 2026-05-26 — Linux ✅
- Fedora 43, GCC 15.2, CMake 4.3, Ninja 1.13
- 225/225 targets compile, 236/244 tests pass (8 GL-headless expected)
- CMake 3.31→3.26, vcpkg DX split, `UNIGUI_HAS_DX11` guards, `<algorithm>` includes
- Python `embed_font.py` for non-Windows font embedding
- CJK font paths: Noto Sans CJK on Linux

### 2026-05-26 — macOS code ✅
- Metal renderer: full ObjC++ implementation with MTLDevice/CommandQueue/ImGui_ImplMetal
- CJK font paths: PingFang.ttc / STHeiti / AppleSDGothicNeo
- CMake: `-x objective-c++ -fobjc-arc` for Metal source, Metal.framework + QuartzCore.framework linkage
- `UNIGUI_HAS_METAL` compile definition on __APPLE__
- *Not tested — no macOS hardware available*

## Work Objectives

### Phase 1: Linux ✅
- [x] GCC 14 / Clang 18 编译验证 (Fedora 43, GCC 15.2)
- [x] `vcpkg` Linux triplet (x64-linux) 依赖解析
- [x] CJK 字体回退 (Noto Sans CJK)
- [ ] X11 + Wayland runtime QA (GLFW dual-backend) — 需要 GUI 环境
- [ ] Linux release 打包 (AppImage / deb) — 延迟

### Phase 2: macOS — Code Ready
- [x] Metal backend ObjC++ 实现 (MTLDevice/CmdQueue/ImGui_ImplMetal)
- [x] CMake Apple 编译选项 (-fobjc-arc, Metal.framework)
- [x] CJK 字体路径 (PingFang/STHeiti/AppleSDGothic)
- [ ] Clang 18 编译验证 — 需要 macOS 硬件
- [ ] Metal backend runtime QA — 需要 macOS 硬件
- [ ] macOS release 打包 (dmg) — 延迟

### Phase 3: Web — Code Ready
- [x] Emscripten platform 实现 (canvas sizing, input loop, emscripten_set_main_loop)
- [x] HTML shell 模板 (spinner, canvas sizing, Module bridge)
- [ ] Emscripten 4.x toolchain — 需要 emsdk
- [ ] `vcpkg` wasm32-emscripten triplet — 需要 emsdk
- [ ] WebGL 2.0 runtime QA — 需要 emsdk
- [ ] Web demo deployment — 延迟

### Phase 4: CI/CD ✅
- [x] GitHub Actions: Windows (MSVC), Linux (Ubuntu+GCC), macOS (Clang)
- [x] Linux: apt install X11/GL dev libs + cmake inline (no preset needed)
- [x] macOS: brew ninja + cmake inline x64-osx triplet
- [ ] Cross-compile all 3 platforms from Windows host — 延迟
- [ ] Automated release packaging per platform — 延迟

## Estimated Effort

| Phase | Tasks | Weeks |
|-------|-------|-------|
| Linux | ~6  | 2-3   |
| macOS | ~6  | 2-3   |
| Web   | ~4  | 1-2   |
| CI/CD | ~4  | 1-2   |
| **Total** | **~20** | **6-10** |

## Key Risks

- **GLFW Wayland**: GLFW 3.4 supports Wayland natively but DPI/input quirks exist. X11 fallback available.
- **macOS OpenGL**: Apple deprecated OpenGL 4.1. Metal backend is preferred but stub-only currently.
- **Emscripten GLFW**: Emscripten GLFW port has limited windowing. May need SDL3 fallback.
- **vcpkg cross-compile**: Not all packages support Emscripten triplet (GLAD, spdlog — need alternatives).

## Dependency

- v4.0 depends on v3.1 stability baseline
- No blocking v3.1 tasks — can start immediately after v3.1 tag

## Decision Needed

> **Should v4.0 target ALL 3 platforms simultaneously, or one platform per minor version?**
> - **Option A**: v4.0 = Linux only, v4.1 = macOS, v4.2 = Web (smaller scope, faster delivery)
> - **Option B**: v4.0 = all 3 + CI/CD from day one (larger scope, complete solution)
> - **Recommendation**: Option A — validate Linux first, then macOS, then Web.
