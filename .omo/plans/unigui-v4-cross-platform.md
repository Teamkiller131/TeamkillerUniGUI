# UniGUI v4.0 — Cross-Platform

## TL;DR

> **Quick Summary**: 多平台正式支持 — Linux (X11/Wayland)、macOS (Metal/MoltenVK)、Web (Emscripten WASM)。
> v4 不做新功能，纯平台适配 + CI/CD 管道。

## Target Platforms

| Platform | Backend | Graphics | Status | Priority |
|----------|---------|----------|--------|----------|
| Windows | GLFW+DX11/GL3 | DirectX 11 / OpenGL 3.3 | ★ Production | P0 (keep) |
| Linux | GLFW+GL3 | OpenGL 3.3 (X11/Wayland) | ✓ Compiles | P1 |
| macOS | GLFW+GL3/Metal | OpenGL 3.3 / Metal 2 | ✓ Compiles | P1 |
| Web | Emscripten | WebGL 2.0 | ✗ Fails | P2 |

## Work Objectives

### Phase 1: Linux
- [ ] GCC 14 / Clang 18 编译验证
- [ ] `vcpkg` Linux triplet (x64-linux) 依赖解析
- [ ] X11 + Wayland runtime QA (GLFW dual-backend)
- [ ] CJK 字体回退 (系统 Noto Sans CJK)
- [ ] Linux release 打包 (AppImage / deb)

### Phase 2: macOS
- [ ] Clang 18 编译验证
- [ ] `vcpkg` macOS triplet (x64-osx / arm64-osx)
- [ ] Metal backend runtime QA (GPU capture)
- [ ] OpenGL 4.1 (deprecated cap) workaround
- [ ] macOS release 打包 (dmg)

### Phase 3: Web
- [ ] Emscripten 4.x toolchain
- [ ] `vcpkg` wasm32-emscripten triplet
- [ ] WebGL 2.0 backend runtime
- [ ] HTML shell + canvas sizing
- [ ] Web demo deployment

### Phase 4: CI/CD
- [ ] GitHub Actions / Gitea Actions matrices
- [ ] Cross-compile all 3 platforms from Windows host
- [ ] Automated release packaging per platform
- [ ] Integration test suite per platform

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
