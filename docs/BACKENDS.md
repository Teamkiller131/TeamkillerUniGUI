# Backends & the Application Loop

TeamkillerUniGUI (v4.9.0) runs Dear ImGui on top of a **pluggable backend
abstraction**: every window/input system is a `PlatformBackend` and every GPU
API is a `RendererBackend`, and the two are combined into a backend *pair* by a
single factory. The high-level application loop in `unigui::Init` / `Run` /
`RunApp` owns one such pair, drives ImGui through it, and clears every frame to
the theme's backdrop color so translucent (glass) surfaces read correctly.

This document covers:

- The two-interface abstraction (`PlatformBackend` × `RendererBackend`).
- Which **platforms** and **renderers** are available (all are functional;
  WebGPU is the newest and build-verified in CI).
- How to **select** a backend at runtime (`AppConfig::backend` /
  `BackendType`) and at build time (CMake `UNIGUI_BACKEND_*` options + the named
  presets).
- The application loop: `AppConfig`, `Init`, `NewFrame`/`Render`,
  `Run(callback, maxFrames)`, `RunApp`, and the `--frames` headless pattern.
- The **backdrop-clear contract** every backend honours.

All signatures below are quoted from the public headers under
`include/unigui/app/` and `include/unigui/backend/`.

---

## 1. The abstraction

UniGUI splits "drawing a UI" into two orthogonal responsibilities:

| Interface | Header | Responsibility |
|-----------|--------|----------------|
| `PlatformBackend` | `include/unigui/backend/platform_backend.h` | Window creation, event polling, input, buffer swap, native handle, Vulkan surface seam |
| `RendererBackend` | `include/unigui/backend/renderer_backend.h` | Initialise the GPU API, render `ImDrawData`, set the clear color |

A concrete app uses one of each, combined into a **pair**. The pairing is done
by `CreateBackend(BackendType)` (see §4), which returns:

```cpp
// include/unigui/backend/backend_factory.h
struct DefaultBackend {
    std::unique_ptr<PlatformBackend> platform;
    std::unique_ptr<RendererBackend> renderer;
};
```

You rarely instantiate these yourself — `unigui::Init()` does it for you based on
`AppConfig::backend`. But understanding the seam is useful when embedding UniGUI
in an existing window or porting to a new GPU API.

### 1.1 `PlatformBackend`

```cpp
class PlatformBackend {
public:
    virtual ~PlatformBackend() = default;

    virtual bool Init(void* native_window_handle = nullptr) = 0;
    virtual void Shutdown() = 0;
    virtual void NewFrame() = 0;
    virtual void PollEvents() = 0;
    virtual bool ShouldClose() const = 0;

    virtual void* GetWindowHandle() const { return nullptr; }
    virtual void* GetNativeWindowHandle() const { return GetWindowHandle(); }
    virtual void GetClientSize(int* w, int* h);   // writes 0,0 by default
    virtual void SetTitle(const char*) {}
    virtual void SetSize(int, int) {}
    virtual void SwapBuffers() {}                 // OpenGL/Vulkan present

    // Vulkan seam — see §3.2
    virtual void GetVulkanInstanceExtensions(std::vector<const char*>& out) const {}
    virtual bool CreateVulkanSurface(void* instance, void* out_surface) { return false; }
};
```

Notes:

- `Init(native_window_handle)` — pass `nullptr` (the common case) and the
  backend creates and owns its own window. Pass a native handle to adopt an
  existing one. `unigui::Init` always passes `nullptr`.
- The Vulkan methods are the *only* platform-specific seam of the cross-platform
  Vulkan renderer. GLFW backs `CreateVulkanSurface` with
  `glfwCreateWindowSurface`, SDL3 with `SDL_Vulkan_CreateSurface`. They take
  `void*` so the generic interface stays free of Vulkan headers. Non-Vulkan
  platforms leave the defaults (no extensions, surface creation returns
  `false`).

### 1.2 `RendererBackend`

```cpp
class RendererBackend {
public:
    virtual ~RendererBackend() = default;

    virtual bool Init(ImGuiContext* context) = 0;
    virtual void Shutdown() = 0;
    virtual void RenderDrawData(ImDrawData* draw_data) = 0;   // may be nullptr
    virtual void SetClearColor(float r, float g, float b, float a) = 0;
};
```

`SetClearColor` is the hook the application loop uses for the
backdrop-clear contract (§7). `RenderDrawData` may receive `nullptr` for an
empty frame and must tolerate it.

---

## 2. Platforms

A *platform* owns the window and input. Two are shipped:

| Platform | `BackendType` it backs | Status | CMake gate |
|----------|------------------------|--------|------------|
| **GLFW** (default) | `GLFW_GL3`, `DX11`, `DX12`, `Vulkan`, `Metal`, `WebGPU` | Fully functional | `UNIGUI_BACKEND_GLFW3` (ON) |
| **SDL3** (opt-in) | `SDL3_Vulkan` | Functional when built with SDL3 | `UNIGUI_BACKEND_SDL3` (OFF) |
| Emscripten/Web | `Emscripten` | **Functional** (WebAssembly + WebGL2; delegates to GLFW) | (compiled in core, no gate) |

GLFW is the workhorse platform: it creates the window for every renderer except
SDL3's. The factory's `CreateGLFWPlatform(BackendType type)` takes the backend
type so it knows whether to create an **OpenGL context** (`GLFW_GL3`) or an
**API-agnostic window** (`GLFW_NO_API`) that a DX/Metal/WebGPU renderer can own
the swapchain on:

```cpp
// include/unigui/backend/backend_factory.h
std::unique_ptr<PlatformBackend> CreateGLFWPlatform(BackendType type = BackendType::GLFW_GL3);
std::unique_ptr<PlatformBackend> CreateSDL3Platform();
std::unique_ptr<PlatformBackend> CreateEmscriptenPlatform();
```

---

## 3. Renderers

A *renderer* owns the GPU API. Every renderer is now real — OpenGL3 (incl.
WebGL2 on the Web), Vulkan, DX11, DX12, Metal, and WebGPU.

| Renderer | `BackendType` | Status | OS | CMake gate |
|----------|---------------|--------|----|------------|
| **OpenGL 3** (default) | `GLFW_GL3`, `Emscripten` | **Functional** (GLES3/WebGL2 on the Web) | All + Web | `UNIGUI_BACKEND_GLFW3` (ON) |
| **Vulkan** | `Vulkan`, `SDL3_Vulkan` | **Functional** | All (cross-platform) | `UNIGUI_BACKEND_VULKAN` / `UNIGUI_BACKEND_SDL3` (OFF) |
| **DirectX 11** | `DX11` | **Functional** | **Windows only** | `UNIGUI_BACKEND_DX11` (ON) |
| **DirectX 12** | `DX12` | **Functional** | **Windows only** | `UNIGUI_BACKEND_DX12` (OFF) |
| **Metal** | `Metal` | **Functional** (`imgui_impl_metal` on a `CAMetalLayer`) | macOS | (auto on `APPLE`) |
| **WebGPU** | `WebGPU` | **Functional** (`imgui_impl_wgpu`, emdawnwebgpu) | Web | `UNIGUI_WEB_WEBGPU` (OFF) |

> **All backends are real.** Every renderer renders. WebGPU is the newest: it is
> build-verified in CI (emsdk 4.0.10 + `--use-port=emdawnwebgpu`); since CI has no
> GPU/browser, its in-browser runtime is validated manually with the uploaded
> `web_demo` artifact. The default GLFW+OpenGL3 path is the most battle-tested.

The factory exposes one creator per renderer. Note that `CreateMetalRenderer`
and `CreateWebGPURenderer` are always *declared* (unguarded), while
`CreateDX12Renderer` is only declared when `UNIGUI_HAS_DX12` is defined:

```cpp
std::unique_ptr<RendererBackend> CreateOpenGL3Renderer();
std::unique_ptr<RendererBackend> CreateVulkanRenderer();
std::unique_ptr<RendererBackend> CreateDX11Renderer();
std::unique_ptr<RendererBackend> CreateMetalRenderer();   // macOS (imgui_impl_metal)
#ifdef UNIGUI_HAS_DX12
std::unique_ptr<RendererBackend> CreateDX12Renderer();
#endif
std::unique_ptr<RendererBackend> CreateWebGPURenderer();  // Web (imgui_impl_wgpu)
```

### 3.1 DirectX 11 / 12 are Windows-only

`CreateDX11Renderer` / `CreateDX12Renderer` link Direct3D and are guarded so
they only compile on Windows. **On Linux and macOS you must disable them** or
configuration will fail:

```bash
-DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF
```

The Linux and macOS presets (`linux-gcc-debug`, `linux-gcc-debug-asan`,
`macos-clang-debug`) already set both to `OFF` for you.

The DX renderers expose extra non-virtual surface for the app loop to populate
the device/swapchain (created in `app.cc` before `Init`) and to resize. The DX11
header also declares the free function `CreateDX11DeviceAndSwapChain(...)` that
`app.cc` calls to build those objects:

```cpp
// include/unigui/backend/dx11_renderer.h
bool CreateDX11DeviceAndSwapChain(void* hwnd, int w, int h, ID3D11Device** dev,
                                  ID3D11DeviceContext** ctx, IDXGISwapChain** swap,
                                  ID3D11RenderTargetView** rtv = nullptr);

class DX11Renderer : public RendererBackend {
public:
    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* ctx_ = nullptr;
    IDXGISwapChain* swapchain_ = nullptr;
    ID3D11RenderTargetView* rtv_ = nullptr;
    bool Init(ImGuiContext*) override;
    void Shutdown() override;
    void RenderDrawData(ImDrawData* dd) override;
    void SetClearColor(float r, float g, float b, float a) override;
    bool ResizeSwapChain(int w, int h);
};
```

DX12 (`include/unigui/backend/dx12_renderer.h`) similarly exposes
`device_`/`cmdQueue_`/`cmdList_`/`swapchain_`/`rtvHeap_`/`srvHeap_` plus
`ResizeSwapChain(int, int)`, and runs a 2-frame command-allocator ring
(`kNumFrames = 2`) with a double-buffered swap chain (`kNumBackBuffers = 2`). Its
free-function builder is `CreateDX12DeviceAndSwapChain(...)`.

### 3.2 The cross-platform Vulkan renderer

`VulkanRenderer` (`include/unigui/backend/vulkan_renderer.h`) is **platform
agnostic**: it owns the `VkInstance` / `VkDevice` / surface / descriptor pool and
drives ImGui's own `ImGui_ImplVulkanH_Window` helper for swap-chain, render
pass, framebuffers, per-frame command buffers and synchronisation. The *only*
platform-specific step — creating the window surface and listing the instance
extensions it needs — is delegated to the active `PlatformBackend` via
`GetVulkanInstanceExtensions` / `CreateVulkanSurface`. That's why the same
renderer serves both the GLFW (`Vulkan`) and SDL3 (`SDL3_Vulkan`) backends with
no Win32/SDL dependency of its own.

Its extra methods (called by `app.cc`, not by user code):

```cpp
class VulkanRenderer : public RendererBackend {
public:
    bool BringUp(PlatformBackend* platform, int w, int h); // instance/device/surface/swapchain
    void NewFrameVk();        // rebuild swapchain on resize, then ImGui_ImplVulkan_NewFrame()
    void RequestResize(int w, int h);
};
```

---

## 4. The `BackendType` enum & the factory

Runtime backend selection is a single enum value carried in `AppConfig`:

```cpp
// include/unigui/backend/backend_types.h
enum class BackendType {
    GLFW_GL3,    ///< GLFW platform + OpenGL 3 renderer (default)
    SDL3_Vulkan, ///< SDL3 platform + shared Vulkan renderer (opt-in; needs SDL3)
    DX11,        ///< DirectX 11 renderer (Windows only)
    DX12,        ///< DirectX 12 renderer (Windows only)
    Metal,       ///< Metal renderer (macOS only)
    WebGPU,      ///< WebGPU renderer (cross-platform via Dawn/WGPU)
    Emscripten,  ///< Emscripten/Web platform
    Vulkan,      ///< GLFW platform + shared Vulkan renderer (cross-platform)
};
```

`CreateBackend(BackendType)` (inline in `backend_factory.h`) maps each enum to a
platform+renderer pair, guarded by the `UNIGUI_HAS_*` compile definitions so a
disabled backend resolves to `{nullptr, nullptr}`:

```cpp
inline DefaultBackend CreateBackend(BackendType type) {
    switch (type) {
    case BackendType::GLFW_GL3:
        return {CreateGLFWPlatform(BackendType::GLFW_GL3), CreateOpenGL3Renderer()};
    case BackendType::SDL3_Vulkan:
#if defined(UNIGUI_HAS_SDL3) && defined(UNIGUI_HAS_VULKAN)
        return {CreateSDL3Platform(), CreateVulkanRenderer()};
#else
        return {nullptr, nullptr};
#endif
    case BackendType::DX11:
#ifdef UNIGUI_HAS_DX11
        return {CreateGLFWPlatform(BackendType::DX11), CreateDX11Renderer()};
#else
        return {nullptr, nullptr};
#endif
    case BackendType::Metal:
#ifdef __APPLE__
        return {CreateGLFWPlatform(BackendType::Metal), CreateMetalRenderer()};
#else
        return {nullptr, nullptr};
#endif
    case BackendType::DX12:
#ifdef UNIGUI_HAS_DX12
        return {CreateGLFWPlatform(BackendType::DX12), CreateDX12Renderer()};
#else
        return {nullptr, nullptr};
#endif
    case BackendType::Vulkan:
#ifdef UNIGUI_HAS_VULKAN
        return {CreateGLFWPlatform(BackendType::Vulkan), CreateVulkanRenderer()};
#else
        return {nullptr, nullptr};
#endif
    case BackendType::WebGPU:
        return {CreateGLFWPlatform(BackendType::WebGPU), CreateWebGPURenderer()};
    case BackendType::Emscripten:
        return {CreateEmscriptenPlatform(), CreateWebGPURenderer()};
    }
    return {nullptr, nullptr};
}

inline DefaultBackend CreateDefaultBackend() {
    return CreateBackend(BackendType::GLFW_GL3); // GLFW + OpenGL3
}
```

Note that `Metal` is guarded by `__APPLE__`, and `WebGPU`/`Emscripten` by
`__EMSCRIPTEN__` (+ `UNIGUI_HAS_WEBGPU` for WebGPU). So the pairings are:

| `BackendType` | Platform | Renderer |
|---------------|----------|----------|
| `GLFW_GL3` | GLFW (GL context) | OpenGL 3 |
| `Vulkan` | GLFW (NO_API) | Vulkan |
| `SDL3_Vulkan` | SDL3 | Vulkan |
| `DX11` | GLFW (NO_API) | DirectX 11 |
| `DX12` | GLFW (NO_API) | DirectX 12 |
| `Metal` | GLFW (NO_API) | Metal (`imgui_impl_metal`) |
| `WebGPU` | GLFW (NO_API) | WebGPU (`imgui_impl_wgpu`, emdawnwebgpu) |
| `Emscripten` | Emscripten → GLFW (WebGL2 context) | OpenGL 3 (GLES3/WebGL2) |

### 4.1 Automatic fallback

If a hardware backend (DX11/DX12/Vulkan/Metal) can't bring up a GPU
device/swapchain — old/virtual GPU, RDP session, missing driver —
`unigui::Init` **automatically falls back to `GLFW_GL3`** so the app still
starts instead of dying at launch. You don't need to handle this yourself; it's
logged as a warning. Only if even the GLFW/OpenGL3 fallback fails does `Init`
return `false`.

---

## 5. Selecting a backend

There are two selection layers, and **both must agree**: the build must *compile
in* the backend (CMake), and the runtime must *request* it (`AppConfig`).

### 5.1 Build-time: CMake options

From the top-level `CMakeLists.txt`:

| Option | Default | What it enables |
|--------|:-------:|-----------------|
| `UNIGUI_BACKEND_GLFW3` | `ON` | GLFW + OpenGL3 (the default backend) |
| `UNIGUI_BACKEND_DX11` | `ON` | DirectX 11 renderer (**Windows only**) |
| `UNIGUI_BACKEND_DX12` | `OFF` | DirectX 12 renderer (**Windows only**) |
| `UNIGUI_BACKEND_VULKAN` | `OFF` | Vulkan renderer via GLFW (cross-platform) |
| `UNIGUI_BACKEND_SDL3` | `OFF` | SDL3 platform backend (needs `sdl3` + `imgui[sdl3-binding]` + `vulkan`) |

These map to the `UNIGUI_HAS_*` compile definitions consumed by the factory and
`app.cc` (all defined via `target_compile_definitions(unigui PUBLIC …)` in
`src/CMakeLists.txt`):

- `UNIGUI_BACKEND_DX11` → `UNIGUI_HAS_DX11` (Windows only; links
  `d3d11 d3dcompiler dxgi dxguid`).
- `UNIGUI_BACKEND_DX12` → `UNIGUI_HAS_DX12` (Windows only; links
  `d3d12 dxgi dxguid`).
- `UNIGUI_BACKEND_VULKAN` **or** `UNIGUI_BACKEND_SDL3` → `UNIGUI_HAS_VULKAN`
  (`find_package(Vulkan REQUIRED)`; the shared Vulkan renderer is compiled).
- `UNIGUI_BACKEND_SDL3` → `UNIGUI_HAS_SDL3`
  (`find_package(SDL3 CONFIG REQUIRED)`).
- On `APPLE`, the Metal renderer is compiled and `UNIGUI_HAS_METAL` defined
  automatically (a real `imgui_impl_metal` renderer on a `CAMetalLayer`).
- Under an Emscripten toolchain, `cmake/Emscripten.cmake` provides imgui/implot/
  spdlog via FetchContent and GLFW/WebGL2/freetype via Emscripten ports; the lib
  cross-compiles to WebAssembly and renders through the OpenGL3 (WebGL2) backend.

The DX11/DX12 link steps are inside an `if(WIN32)` guard, so requesting them on
Linux/macOS has no effect at link time — but you should still pass
`-DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF` on those platforms for a
clean, predictable configure (the non-Windows presets do this for you).

Example: a Linux build with the Vulkan renderer compiled in:

```bash
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF \
  -DUNIGUI_BACKEND_VULKAN=ON
cmake --build build
```

### 5.2 Named presets

The SDL3+Vulkan combination has dedicated presets in `CMakePresets.json`:

| Preset | What it configures |
|--------|--------------------|
| `windows-msvc-sdl3-vulkan-debug` | MSVC Debug, SDL3 platform + Vulkan renderer |
| `windows-msvc-sdl3-vulkan-release` | MSVC Release, SDL3 platform + Vulkan renderer |

```powershell
cmake-msvc.cmd --preset windows-msvc-sdl3-vulkan-debug
cmake-msvc.cmd --build --preset windows-msvc-sdl3-vulkan-debug
```

> **Heads-up on the SDL3 presets.** These presets set a cache variable
> `UNIGUI_BACKEND=SDL3_VULKAN`. That meta-variable is **not** consumed by the
> current `CMakeLists.txt` — only the per-backend `UNIGUI_BACKEND_*` options are.
> If you need the SDL3+Vulkan stack guaranteed compiled in, add the explicit
> per-backend flags as well:
>
> ```powershell
> cmake-msvc.cmd --preset windows-msvc-sdl3-vulkan-debug `
>   -DUNIGUI_BACKEND_SDL3=ON -DUNIGUI_BACKEND_VULKAN=ON
> ```
>
> (`UNIGUI_BACKEND_SDL3=ON` alone is enough, since it pulls in `UNIGUI_HAS_VULKAN`
> via the shared-renderer rule `if(UNIGUI_BACKEND_VULKAN OR UNIGUI_BACKEND_SDL3)`.)

### 5.3 Runtime: `AppConfig::backend`

Once a backend is compiled in, request it by setting `AppConfig::backend` to the
matching `BackendType`. The default differs per OS (see §6.1): Windows defaults
to `DX11`, everything else to `GLFW_GL3`.

```cpp
unigui::AppConfig cfg;
#ifdef _WIN32
cfg.backend = unigui::BackendType::DX11;       // default on Windows anyway
#else
cfg.backend = unigui::BackendType::GLFW_GL3;    // portable default
#endif
// Opt into the Vulkan renderer (requires UNIGUI_BACKEND_VULKAN=ON at build time):
// cfg.backend = unigui::BackendType::Vulkan;
```

If you request a backend that wasn't compiled in, `CreateBackend` returns a null
pair and `Init` falls back to GLFW/OpenGL3 (§4.1).

---

## 6. The application loop

The whole loop lives in `include/unigui/app/app.h`. The lifecycle is
**Init → (NewFrame → your UI → Render)\* → Shutdown**, and `Run`/`RunApp` wrap
it for you.

### 6.1 `AppConfig`

```cpp
struct AppConfig {
    int width  = 1280;
    int height = 720;
    const char* title = "UniGUI Application";
    ThemeConfig theme = {ThemePreset::Dark, 0.0f, 16.0f}; // auto-DPI, 16px logical
#ifdef _WIN32
    BackendType backend = BackendType::DX11;        // DX11 is stable on Windows
#else
    BackendType backend = BackendType::GLFW_GL3;
#endif
    bool dpiScaleFonts = false;
    bool multiViewport = false;                    // opt-in: windows can be dragged out of the main window
};
```

| Field | Default | Meaning |
|-------|---------|---------|
| `width`, `height` | `1280 × 720` | Initial client size in pixels |
| `title` | `"UniGUI Application"` | Window title |
| `theme` | Dark, auto-DPI, 16px | A `ThemeConfig` (see `theme/theme.h`): `preset`, `dpi_scale` (0 = auto-detect), `font_size`, `font_path`, `emoji_fallback`, `surface` |
| `backend` | `DX11` (Win) / `GLFW_GL3` (other) | Which `BackendType` to bring up |
| `dpiScaleFonts` | `false` | Opt into Dear ImGui ≥1.92 dynamic per-monitor font DPI scaling (`io.ConfigDpiScaleFonts`) — recommended for fractional/multi-monitor DPI |

The default `theme` uses positional aggregate initialisation of the first three
`ThemeConfig` fields (`preset = ThemePreset::Dark`, `dpi_scale = 0.0f`,
`font_size = 16.0f`); the remaining fields (`font_path`, `emoji_fallback`,
`surface`) take their own in-class defaults — `font_path = nullptr` (auto-detect
CJK font), `emoji_fallback = true`, and `surface = theme::SurfaceStyle::Glass`.
`ThemePreset` has exactly two values: `Dark` and `Light`.

### 6.2 Lifecycle functions

```cpp
bool Init(const AppConfig& config);   // create context + bring up backend (with fallback)
void Shutdown();                      // tear everything down
bool NewFrame();                      // poll events, ImGui::NewFrame(); false if not running
void Render();                        // ImGui::Render() + clear-to-backdrop + present
bool ShouldClose();                   // window close requested?

void Run(const std::function<void()>& callback, int maxFrames = 0);
int  RunApp(const AppConfig& config, const std::function<void()>& callback, int maxFrames = 0);

void* GetNativeWindowHandle();        // HWND on Windows, GLFWwindow* elsewhere

// HiDPI content scale (ImGuiStyle::FontScaleDpi, Dear ImGui ≥1.92)
void  SetContentScale(float scale);              // 1.0 = 100%, 1.5 = 150%, …
float GetContentScale();                         // 1.0 if no context
void  SetContentScaleFromMonitor(float rawScale, bool snap = true);
```

- `Init` creates the ImPlot context and the ImGui context, registers themes
  (`unigui::theme::RegisterAllThemes()`), then brings up `config.backend` —
  falling back to `GLFW_GL3` if a hardware backend fails (§4.1). Returns `false`
  only if no backend (including the fallback) works.
- `NewFrame` polls platform events, runs any pending font rebuild, issues the
  per-backend `NewFrame`, then `ImGui::NewFrame()`. Returns `false` if the app
  isn't initialised. **Do not poll events again yourself** — `NewFrame` already
  does it.
- `Render` calls `ImGui::Render()`, applies the backdrop clear (§7), and
  presents. For the GLFW/OpenGL3 path it issues the GL clear and
  `SwapBuffers()` here; other backends clear/present inside their
  `RenderDrawData`.
- `Shutdown` releases the renderer and platform, destroys the ImGui/ImPlot
  contexts, and shuts down `Settings`. `Run`/`RunApp` call it automatically.
- `SetContentScaleFromMonitor` runs the raw platform scale (e.g. GLFW's
  `glfwGetWindowContentScale`) through `dpi::NormalizeContentScale` when
  `snap` is `true`, so a fractional `1.4583` becomes a crisp `1.5`.

### 6.3 `Run` and `RunApp`

```cpp
void Run(const std::function<void()>& callback, int maxFrames = 0);
int  RunApp(const AppConfig& config, const std::function<void()>& callback, int maxFrames = 0);
```

`Run` is the main loop. It invokes `callback` once per frame between
`NewFrame()` and `Render()`, and calls `Shutdown()` automatically when the loop
ends. The loop terminates when the window is closed **or** when `maxFrames > 0`
frames have been rendered:

```cpp
// Effective loop body (from src/app/app.cc):
void Run(const std::function<void()>& cb, int maxFrames) {
    int frame = 0;
    // NewFrame() already polls platform events; do not poll again here.
    while (!ShouldClose()) {
        if (!NewFrame()) break;
        if (cb) cb();
        Render();
        if (maxFrames > 0 && ++frame >= maxFrames) break;
    }
    Shutdown();
}
```

`RunApp` is the one-call entry point — `Init(config)` followed by
`Run(callback, maxFrames)`, with init-failure handling. It returns `0` on
success, `1` if initialisation failed:

```cpp
int main() {
    unigui::AppConfig cfg;
    cfg.title = "My App";
    return unigui::RunApp(cfg, [] {
        ImGui::Text("Hello, UniGUI");
    });
}
```

`maxFrames = 0` (the default) runs until the window closes; any positive value
renders that many frames and exits — the basis of the headless pattern below.

### 6.4 Manual loop (when you need control)

If you can't use `Run`, drive the lifecycle yourself:

```cpp
unigui::AppConfig cfg;
cfg.title = "Manual loop";
if (!unigui::Init(cfg))
    return 1;

while (!unigui::ShouldClose()) {
    if (!unigui::NewFrame())
        break;

    ImGui::Begin("Panel");
    ImGui::Text("Frame-driven UI");
    ImGui::End();

    unigui::Render();
}
unigui::Shutdown();
```

### 6.5 The `--frames` headless pattern

Every example accepts `--frames N` to render `N` frames and exit, which makes
them usable as CI smoke tests and for screenshots. The idiom (from
`examples/hello_unigui/main.cc`) is to parse the count into `maxFrames` and pass
it straight through to `RunApp`:

```cpp
int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++) {
        std::string_view arg = argv[i];
        if (arg == "--frames" && i + 1 < argc)
            max_frames = std::atoi(argv[++i]);
    }

    unigui::AppConfig config;
    config.title = "Hello UniGUI";
#ifdef _WIN32
    config.backend = unigui::BackendType::DX11;
#endif

    return unigui::RunApp(config, [&] { BuildDemoWindow(); }, max_frames);
}
```

Run it headlessly:

```bash
./build/examples/hello_unigui/hello_unigui --frames 10
```

---

## 7. The backdrop-clear contract

This is a contract **every** backend honours, and it is enforced centrally by
the application loop — you don't implement it per backend, but you must not
break it.

UniGUI's translucent surface materials (Glass / Frosted / Acrylic — the default
`SurfaceStyle::Glass`) blend against whatever is *behind* the ImGui windows. For
the glass effect to read correctly, the framebuffer must be cleared to the
**theme-derived backdrop color**, not to black or an arbitrary opaque color.

The contract: **on every frame, the renderer's clear color is set to
`GetBackdropColor()`**. From `src/app/app.cc`'s `Render()`:

```cpp
void Render() {
    // …
    ImGui::Render();
    ImDrawData* dd = ImGui::GetDrawData();
    // Clear to the theme-derived backdrop so translucent (glass) surfaces read
    // against a tinted background. Applies to every backend; GLFW additionally
    // issues the GL clear here (other backends clear inside RenderDrawData).
    {
        ImVec4 bg = GetBackdropColor();
        g_renderer->SetClearColor(bg.x, bg.y, bg.z, bg.w);
    }
    if (g_backend == BackendType::GLFW_GL3) {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    g_renderer->RenderDrawData(dd);
    if (g_backend == BackendType::GLFW_GL3)
        g_platform->SwapBuffers();
}
```

`GetBackdropColor()` is declared in `include/unigui/theme/theme.h`:

```cpp
/// Opaque framebuffer clear colour for the active theme + surface material.
/// Translucent surface materials (Glass/Frosted/Acrylic) reveal whatever is drawn
/// behind ImGui windows, so backends should clear to this tinted backdrop instead
/// of black for the glass effect to read correctly. Updated on every ApplyTheme();
/// defaults to the Dark window background before the first ApplyTheme() call.
ImVec4 GetBackdropColor();
```

It's refreshed on every `ApplyTheme()` call, so switching theme presets updates
the backdrop automatically.

**Implications when writing or porting a backend:**

- Implement `SetClearColor(r,g,b,a)` to store the color and actually use it when
  clearing the framebuffer (the GL3, DX11, DX12, and Vulkan renderers all do).
- Never hard-code a black/opaque clear — that defeats the glass surfaces.
- The GLFW/OpenGL3 path clears + swaps in `Render()`; the DX/Vulkan renderers do
  it inside their own `RenderDrawData`. Either is fine as long as the stored
  backdrop color is the one used.

### 7.1 Secondary viewports (multi-viewport)

With `AppConfig::multiViewport`, windows dragged out of the main window become
real OS windows. Upstream's per-backend `Renderer_RenderWindow` functions clear
those secondary viewports to a **hardcoded black** (`imgui_impl_dx11.cpp` and
`imgui_impl_opengl3.cpp` both do), which would make translucent materials on a
popped-out window render against garbage.

The app loop therefore extends the contract to every secondary viewport itself:
before `ImGui::Render()` it paints a full-viewport backdrop rect into each
secondary viewport's background draw list (flattened before all windows):

```cpp
if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    const ImU32 bgCol = ImGui::GetColorU32(GetBackdropColor());
    for (ImGuiViewport* vp : ImGui::GetPlatformIO().Viewports) {
        if (vp == nullptr || vp == ImGui::GetMainViewport() ||
            (vp->Flags & ImGuiViewportFlags_IsMinimized))
            continue;
        ImGui::GetBackgroundDrawList(vp)->AddRectFilled(
            vp->Pos, vp->Pos + vp->Size, bgCol);
    }
}
```

This covers **every** renderer with viewport support (GL3, DX11, Vulkan, …) with
no per-backend hooks, so a popped-out window honours the same backdrop contract
as the main one. The black upstream clear underneath is simply never visible.

**Multi-viewport capability matrix** (runtime-verified = pixels asserted in CI):

| Backend | Viewport support | Runtime-verified |
|---------|------------------|------------------|
| GLFW + OpenGL3 | ✅ (GLFW viewport windows) | ✅ Linux headless smoke (main window) |
| GLFW + DX11 | ✅ | ✅ `DXMultiViewportSmoke` (pop-out → main still drawn → merge-back; WARP/GPU) |
| GLFW + DX12 | ❌ upstream `imgui_impl_dx12` has no multi-viewport support — the flag is dropped by the capability self-check | — |
| SDL3 + Vulkan | ✅ (SDL3 viewport windows) | build-only |
| Metal | ✅ (build-only) | build-only |
| Emscripten | ignored (browser page has no secondary OS windows) | — |

The capability self-check in `BringUpBackend()` reports
`ImGuiBackendFlags_PlatformHasViewports`/`RendererHasViewports` after init and
drops back to single-viewport with a warning when a pair can't support it — a
half-installed viewport setup does not degrade gracefully.

---

## 8. Quick reference

| Task | How |
|------|-----|
| Default backend | GLFW + OpenGL3 (`BackendType::GLFW_GL3`) |
| Default on Windows | DX11 (`AppConfig::backend` defaults to `DX11`) |
| Pick a backend at runtime | Set `AppConfig::backend = BackendType::…` |
| Compile in Vulkan | `-DUNIGUI_BACKEND_VULKAN=ON` (any OS) |
| Compile in SDL3+Vulkan | `-DUNIGUI_BACKEND_SDL3=ON` (pulls in Vulkan) or the `windows-msvc-sdl3-vulkan-*` presets (+ explicit flags) |
| Disable DX on Linux/macOS | `-DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF` |
| One-call app | `unigui::RunApp(cfg, callback)` |
| Headless / CI | `RunApp(cfg, cb, N)` or `--frames N` on any example |
| Backdrop clear | Automatic — backends clear to `GetBackdropColor()` every frame |
| Native handle | `unigui::GetNativeWindowHandle()` (HWND on Windows, `GLFWwindow*` elsewhere) |

> **All backends are real.** OpenGL3, Vulkan, DX11, DX12, Metal, the Emscripten
> (WebGL2) path, and WebGPU all render. WebGPU is the newest — build-verified in
> CI, with its in-browser runtime validated manually via the `web_demo` artifact.
