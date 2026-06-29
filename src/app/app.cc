#include <unigui/app/app.h>
#include <unigui/backend/backend_factory.h>
#include <unigui/core/dpi.h>
#include <unigui/core/accessibility.h>
#include <unigui/core/main_thread.h>
#include <unigui/theme/presets/registry.h>
#include <unigui/theme/theme.h>
#ifdef UNIGUI_HAS_WIDGETS
#include <unigui/widgets/toast.h>
#endif
#include <unigui/core/log.h>
#include <unigui/core/settings.h>
#ifdef UNIGUI_HAS_EVENTS
#include <unigui/events/eventbus.h>
#endif
#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif
#include <imgui.h>
#include <implot.h>

#include <cstdio>
#include <cstdlib>
#include <vector>
#ifdef UNIGUI_HAS_DX11
#include <unigui/backend/dx11_renderer.h>

#include <imgui_impl_dx11.h>
#endif
#ifdef UNIGUI_HAS_DX12
#include <unigui/backend/dx12_renderer.h>

#include <imgui_impl_dx12.h>
#endif
#ifdef UNIGUI_HAS_VULKAN
#include <unigui/backend/vulkan_renderer.h>
#endif
#ifdef UNIGUI_HAS_METAL
#include <unigui/backend/metal_renderer.h>
#endif
#ifdef UNIGUI_HAS_WEBGPU
#include <unigui/backend/webgpu_renderer.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

namespace unigui {
static bool g_initialized = false;
static BackendType g_backend = BackendType::GLFW_GL3;
static std::unique_ptr<PlatformBackend> g_platform;
static std::unique_ptr<RendererBackend> g_renderer;

static void CleanupAppResources(bool destroy_imgui_context) {
    if (g_renderer) {
        g_renderer->Shutdown();
        g_renderer.reset();
    }
    if (g_platform) {
        g_platform->Shutdown();
        g_platform.reset();
    }
    if (ImPlot::GetCurrentContext())
        ImPlot::DestroyContext();
    if (destroy_imgui_context && ImGui::GetCurrentContext())
        ImGui::DestroyContext();
}

// Tear down ONLY the platform + renderer (keep the ImGui/ImPlot contexts alive) so
// a failed backend bring-up can be retried with a different backend without losing
// the shared GUI context.
static void ResetBackendOnly() {
    if (g_renderer) {
        g_renderer->Shutdown();
        g_renderer.reset();
    }
    if (g_platform) {
        g_platform->Shutdown();
        g_platform.reset();
    }
}

// Bring up a single backend (platform window + renderer + fonts + theme) on the
// already-created ImGui/ImPlot contexts. Returns false (and resets the backend) if
// the platform, GPU device/swapchain, or renderer fails to initialise — letting the
// caller fall back to another backend.
static bool BringUpBackend(BackendType type, const AppConfig& config, float& dpiOut) {
    auto be = CreateBackend(type);
    g_backend = type;
    g_platform = std::move(be.platform);
    g_renderer = std::move(be.renderer);

    if (!g_platform || !g_platform->Init(nullptr)) {
        UNIGUI_LOG_ERROR("Platform init failed (backend={})", (int) type);
        ResetBackendOnly();
        return false;
    }
    g_platform->SetTitle(config.title);
    g_platform->SetSize(config.width, config.height);

    float dpi = config.theme.dpi_scale; // 0 = auto-detect once the window is up
    if (dpi <= 0) {
#ifdef _WIN32
        dpi = DetectDPIScale(g_platform->GetWindowHandle());
#else
        // macOS/Linux: the GLFW/SDL window content scale is the reliable HiDPI factor
        // (DetectDPIScale only implements the Win32 path, returning 1.0 elsewhere —
        // which rasterized the font atlas at 1x and blurred retina/HiDPI text).
        dpi = g_platform->GetContentScale();
#endif
        if (dpi < 0.5f)
            dpi = 1.0f;
    }
    UNIGUI_LOG_INFO("DPI scale: {:.2f}", dpi);
    LoadDefaultFont(config.theme.font_size * dpi, config.theme.font_path);

#ifdef UNIGUI_HAS_DX11
    if (type == BackendType::DX11) {
        auto hwnd = g_platform->GetWindowHandle();
        RECT rc;
        GetClientRect((HWND) hwnd, &rc);
        int pw = rc.right - rc.left, ph = rc.bottom - rc.top;
        if (pw <= 0) {
            pw = config.width;
            ph = config.height;
        }
        ID3D11Device* dev = nullptr;
        ID3D11DeviceContext* ctx = nullptr;
        IDXGISwapChain* swap = nullptr;
        ID3D11RenderTargetView* rtv = nullptr;
        if (!CreateDX11DeviceAndSwapChain(hwnd, pw, ph, &dev, &ctx, &swap, &rtv)) {
            UNIGUI_LOG_ERROR("DX11 device/swapchain creation failed");
            ResetBackendOnly();
            return false;
        }
        auto* dxr = static_cast<DX11Renderer*>(g_renderer.get());
        dxr->device_ = dev;
        dxr->ctx_ = ctx;
        dxr->swapchain_ = swap;
        dxr->rtv_ = rtv;
    }
#endif

#ifdef UNIGUI_HAS_DX12
    if (type == BackendType::DX12) {
        auto hwnd = g_platform->GetWindowHandle();
        int pw = 0, ph = 0;
        g_platform->GetClientSize(&pw, &ph);
        if (pw <= 0) {
            pw = config.width;
            ph = config.height;
        }
        ID3D12Device* dev = nullptr;
        ID3D12CommandQueue* queue = nullptr;
        ID3D12GraphicsCommandList* cmdList = nullptr;
        IDXGISwapChain3* swap = nullptr;
        ID3D12DescriptorHeap* rtvHeap = nullptr;
        ID3D12DescriptorHeap* srvHeap = nullptr;
        if (!CreateDX12DeviceAndSwapChain(hwnd, pw, ph, &dev, &queue, &cmdList, &swap, &rtvHeap,
                                          &srvHeap)) {
            UNIGUI_LOG_ERROR("DX12 device/swapchain creation failed");
            ResetBackendOnly();
            return false;
        }
        auto* dxr = static_cast<DX12Renderer*>(g_renderer.get());
        dxr->device_ = dev;
        dxr->cmdQueue_ = queue;
        dxr->cmdList_ = cmdList;
        dxr->swapchain_ = swap;
        dxr->rtvHeap_ = rtvHeap;
        dxr->srvHeap_ = srvHeap;
    }
#endif

#ifdef UNIGUI_HAS_VULKAN
    if (type == BackendType::Vulkan
#ifdef UNIGUI_HAS_SDL3
        || type == BackendType::SDL3_Vulkan
#endif
    ) {
        int pw = 0, ph = 0;
        g_platform->GetClientSize(&pw, &ph);
        if (pw <= 0) {
            pw = config.width;
            ph = config.height;
        }
        auto* vr = static_cast<VulkanRenderer*>(g_renderer.get());
        // Surface + instance extensions come from the active platform (GLFW/SDL3).
        if (!vr->BringUp(g_platform.get(), pw, ph)) {
            UNIGUI_LOG_ERROR("Vulkan device/swapchain creation failed");
            ResetBackendOnly();
            return false;
        }
    }
#endif

#ifdef UNIGUI_HAS_METAL
    if (type == BackendType::Metal) {
        int pw = 0, ph = 0;
        g_platform->GetClientSize(&pw, &ph);
        if (pw <= 0) {
            pw = config.width;
            ph = config.height;
        }
        // The Metal renderer needs the NSWindow (from the platform) to attach its
        // CAMetalLayer; like Vulkan, the heavy lifting is in BringUp, not Init.
        auto* mr = static_cast<MetalRenderer*>(g_renderer.get());
        if (!mr->BringUp(g_platform->GetNativeWindowHandle(), pw, ph)) {
            UNIGUI_LOG_ERROR("Metal device/layer creation failed");
            ResetBackendOnly();
            return false;
        }
    }
#endif

#ifdef UNIGUI_HAS_WEBGPU
    if (type == BackendType::WebGPU) {
        int pw = 0, ph = 0;
        g_platform->GetClientSize(&pw, &ph);
        if (pw <= 0) {
            pw = config.width;
            ph = config.height;
        }
        // WebGPU device acquisition is async; BringUp kicks it off and returns true.
        // The renderer stays inert (Ready()==false) until the device callback fires,
        // so early frames simply draw nothing — fine under the browser RAF loop.
        auto* wr = static_cast<WebGPURenderer*>(g_renderer.get());
        if (!wr->BringUp(pw, ph)) {
            UNIGUI_LOG_ERROR("WebGPU instance/surface creation failed");
            ResetBackendOnly();
            return false;
        }
    }
#endif

    if (!g_renderer || !g_renderer->Init(ImGui::GetCurrentContext())) {
        UNIGUI_LOG_ERROR("Renderer init failed (backend={})", (int) type);
        ResetBackendOnly();
        return false;
    }
    {
        const char* rn = ImGui::GetIO().BackendRendererName;
        const char* pn = ImGui::GetIO().BackendPlatformName;
        UNIGUI_LOG_INFO("Active backend: renderer={} platform={}", rn ? rn : "(none)",
                        pn ? pn : "(none)");
    }

    // Build font atlas AFTER renderer Init (RendererHasTextures flag is set).
    ImGui::GetIO().Fonts->Build();

    auto& io = ImGui::GetIO();
    ThemeConfig tc = config.theme;
    tc.dpi_scale = dpi;
    ApplyTheme(tc);
    if (g_backend != BackendType::DX11)
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Keyboard navigation (Tab / Shift-Tab / arrows / Space-Enter to activate) — the
    // foundation for keyboard-only operation, and what the a11y focus tracker rides on.
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.DisplaySize = ImVec2((float) config.width, (float) config.height);

    dpiOut = dpi;
    return true;
}

bool Init(const AppConfig& config) {
    if (g_initialized)
        return false;
    InitLogging("debug");
    ImPlot::CreateContext();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // HiDPI: opt into Dear ImGui's dynamic per-monitor font DPI scaling (≥1.92).
    if (config.dpiScaleFonts)
        ImGui::GetIO().ConfigDpiScaleFonts = true;
    unigui::theme::RegisterAllThemes();

    // Try the requested backend; if a hardware backend (DX11/DX12/Vulkan/Metal)
    // can't bring up a GPU device/swapchain (old/virtual GPU, RDP, missing driver),
    // automatically fall back to the portable GLFW + OpenGL3 backend so the client
    // still starts instead of dying at launch.
    float dpi = 0.0f;
    bool ok = BringUpBackend(config.backend, config, dpi);
    if (!ok && config.backend != BackendType::GLFW_GL3) {
        UNIGUI_LOG_WARN("Backend {} unavailable — falling back to GLFW/OpenGL3",
                        (int) config.backend);
        ok = BringUpBackend(BackendType::GLFW_GL3, config, dpi);
    }
    if (!ok) {
        UNIGUI_LOG_ERROR("No usable rendering backend (including GLFW/OpenGL3 fallback)");
        CleanupAppResources(true);
        return false;
    }

    g_initialized = true;
    UNIGUI_LOG_INFO("Init complete: backend={} {}x{} DPI={:.1f}", (int) g_backend, config.width,
                    config.height, dpi);
#ifdef UNIGUI_HAS_EVENTS
    events::Bus::Instance().Publish("app.init", std::make_pair(config.width, config.height));
#endif
    return true;
}

void Shutdown() {
    if (!g_initialized)
        return;
#ifdef UNIGUI_HAS_EVENTS
    events::Bus::Instance().Publish("app.shutdown", int{0});
    events::Bus::Instance().Shutdown();
#endif
    CleanupAppResources(true);
    Settings::Shutdown();
    g_initialized = false;
}

bool NewFrame() {
    if (!g_initialized)
        return false;
    g_platform->PollEvents();
    ApplyPendingFontRebuild();
#ifdef UNIGUI_HAS_DX11
    if (g_backend == BackendType::DX11)
        ImGui_ImplDX11_NewFrame();
#endif
#ifdef UNIGUI_HAS_DX12
    if (g_backend == BackendType::DX12)
        ImGui_ImplDX12_NewFrame();
#endif
#ifdef UNIGUI_HAS_VULKAN
    if (g_backend == BackendType::Vulkan
#ifdef UNIGUI_HAS_SDL3
        || g_backend == BackendType::SDL3_Vulkan
#endif
    ) {
        int cw = 0, ch = 0;
        g_platform->GetClientSize(&cw, &ch);
        auto* vr = static_cast<VulkanRenderer*>(g_renderer.get());
        if (cw > 0 && ch > 0)
            vr->RequestResize(cw, ch);
        vr->NewFrameVk();
    }
#endif
#ifdef UNIGUI_HAS_METAL
    if (g_backend == BackendType::Metal) {
        int cw = 0, ch = 0;
        g_platform->GetClientSize(&cw, &ch);
        // Acquire the drawable + start the command buffer/encoder BEFORE ImGui::NewFrame.
        static_cast<MetalRenderer*>(g_renderer.get())->NewFrameMetal(cw, ch);
    }
#endif
#ifdef UNIGUI_HAS_WEBGPU
    if (g_backend == BackendType::WebGPU) {
        int cw = 0, ch = 0;
        g_platform->GetClientSize(&cw, &ch);
        // (Re)configure the swap chain on resize + acquire this frame's texture view.
        static_cast<WebGPURenderer*>(g_renderer.get())->NewFrameWGPU(cw, ch);
    }
#endif
    // Renderer per-frame setup before ImGui::NewFrame(). For the OpenGL3/WebGL backend
    // this is ImGui_ImplOpenGL3_NewFrame(), which lazily builds the shader program +
    // buffers; without it the GL path draws nothing. No-op for backends that drive their
    // per-frame setup through the typed calls above (DX11/DX12/Vulkan/Metal/WebGPU).
    g_renderer->NewFrame();
    g_platform->NewFrame();
    ImGui::NewFrame();
    a11y::BeginFrame(); // reset the per-frame accessibility tree before widgets render
    ProcessMainThreadTasks();
    // Check for window resize (DX11 needs swapchain resize)
#ifdef UNIGUI_HAS_DX11
    if (g_backend == BackendType::DX11) {
        int cw = 0, ch = 0;
        g_platform->GetClientSize(&cw, &ch);
        static int lastW = 0, lastH = 0;
        if (cw > 0 && ch > 0 && (cw != lastW || ch != lastH)) {
            lastW = cw;
            lastH = ch;
            auto* dxr = static_cast<DX11Renderer*>(g_renderer.get());
            if (dxr->ResizeSwapChain(cw, ch)) {
                ImGui::GetIO().DisplaySize = ImVec2((float) cw, (float) ch);
            }
        }
    }
#endif
#ifdef UNIGUI_HAS_DX12
    if (g_backend == BackendType::DX12) {
        int cw = 0, ch = 0;
        g_platform->GetClientSize(&cw, &ch);
        static int lastW12 = 0, lastH12 = 0;
        if (cw > 0 && ch > 0 && (cw != lastW12 || ch != lastH12)) {
            lastW12 = cw;
            lastH12 = ch;
            auto* dxr = static_cast<DX12Renderer*>(g_renderer.get());
            if (dxr->ResizeSwapChain(cw, ch)) {
                ImGui::GetIO().DisplaySize = ImVec2((float) cw, (float) ch);
            }
        }
    }
#endif
    if (g_backend != BackendType::DX11)
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);
    return true;
}

// Optional render verification for CI. When UNIGUI_RENDER_VERIFY=1, read the GL
// framebuffer back after RenderDrawData and report whether the UI actually drew pixels
// (vs. just a clear). This catches "renders nothing" regressions a log-only smoke can't
// see — e.g. a missing ImGui_ImplOpenGL3_NewFrame() leaves no shader/buffers bound, so
// the screen stays the clear colour (the 4.3.1 black-screen bug). GL backends only; inert
// unless the env var is set, so zero cost in normal runs. Must be called after
// RenderDrawData and before SwapBuffers (it reads the rendered back buffer).
static void VerifyRenderIfEnabled() {
    static const bool verify = [] {
        const char* e = std::getenv("UNIGUI_RENDER_VERIFY");
        return e && e[0] == '1';
    }();
    if (!verify)
        return;

    const GLenum err = glGetError();

    GLint vp[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, vp);
    const int w = vp[2], h = vp[3];
    if (w <= 0 || h <= 0) {
        UNIGUI_LOG_INFO("[render-verify] glError=0x{:X} viewport=0x0 — skipped", (unsigned) err);
        return;
    }

    std::vector<unsigned char> buf((size_t) w * (size_t) h * 4u);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());

    const ImVec4 bg = GetBackdropColor();
    auto to8 = [](float v) { return (int) (v * 255.0f + 0.5f); };
    const int cr = to8(bg.x), cg = to8(bg.y), cb = to8(bg.z);

    // Sample a coarse grid (cheap) and count pixels that differ from the clear colour.
    const int step = 16;
    int total = 0, nonClear = 0;
    for (int y = 0; y < h; y += step) {
        for (int x = 0; x < w; x += step) {
            const size_t i = ((size_t) y * (size_t) w + (size_t) x) * 4u;
            ++total;
            if (std::abs((int) buf[i] - cr) > 8 || std::abs((int) buf[i + 1] - cg) > 8 ||
                std::abs((int) buf[i + 2] - cb) > 8)
                ++nonClear;
        }
    }
    const bool drawn = nonClear >= 4;
    UNIGUI_LOG_INFO("[render-verify] glError=0x{:X} nonClear={}/{} drawn={}", (unsigned) err,
                    nonClear, total, drawn ? "true" : "false");
}

void Render() {
    if (!g_initialized)
        return;
#ifdef UNIGUI_HAS_WIDGETS
    unigui::Toast::Instance().Render();
#endif
    ImGui::Render();
    ImDrawData* dd = ImGui::GetDrawData();
    // Clear to the theme-derived backdrop so translucent (glass) surfaces read
    // against a tinted background. Applies to every backend; GLFW additionally
    // issues the GL clear here (other backends clear inside RenderDrawData).
    {
        ImVec4 bg = GetBackdropColor();
        g_renderer->SetClearColor(bg.x, bg.y, bg.z, bg.w);
    }
    // The Emscripten/WebGL backend renders through the same OpenGL3 renderer as the
    // desktop GLFW_GL3 backend, so it needs the identical GL clear + buffer swap (the
    // ImGui GL impl doesn't clear; on the web glfwSwapBuffers is a harmless no-op that
    // mirrors the reference imgui emscripten loop).
    const bool gl_backend =
        (g_backend == BackendType::GLFW_GL3 || g_backend == BackendType::Emscripten);
    if (gl_backend) {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    g_renderer->RenderDrawData(dd);
    if (gl_backend) {
        VerifyRenderIfEnabled(); // reads the back buffer; no-op unless UNIGUI_RENDER_VERIFY=1
        g_platform->SwapBuffers();
    }
}

bool ShouldClose() {
    return g_platform ? g_platform->ShouldClose() : true;
}
void* GetNativeWindowHandle() {
    return g_platform ? g_platform->GetNativeWindowHandle() : nullptr;
}
void SetContentScale(float scale) {
    if (ImGui::GetCurrentContext())
        ImGui::GetStyle().FontScaleDpi = scale > 0.f ? scale : 1.0f;
}
float GetContentScale() {
    return ImGui::GetCurrentContext() ? ImGui::GetStyle().FontScaleDpi : 1.0f;
}
void SetContentScaleFromMonitor(float rawScale, bool snap) {
    SetContentScale(snap ? dpi::NormalizeContentScale(rawScale) : rawScale);
}
#ifdef __EMSCRIPTEN__
namespace {
std::function<void()> g_emFrameCb;
void EmscriptenFrame() {
    if (!NewFrame())
        return;
    if (g_emFrameCb)
        g_emFrameCb();
    Render();
}
} // namespace
void Run(const std::function<void()>& cb, int /*maxFrames*/) {
    // The browser owns the event loop — register a per-frame callback and hand control
    // back to it. A blocking while-loop would freeze the page. `maxFrames` is ignored
    // on the web (the page runs until the tab closes), and this never returns, so the
    // app lives for the process lifetime (no Shutdown).
    g_emFrameCb = cb;
    emscripten_set_main_loop(EmscriptenFrame, 0, /*simulate_infinite_loop=*/true);
}
#else
void Run(const std::function<void()>& cb, int maxFrames) {
    int frame = 0;
    // NewFrame() already polls platform events; do not poll again here.
    while (!ShouldClose()) {
        if (!NewFrame())
            break;
        if (cb)
            cb();
        Render();
        if (maxFrames > 0 && ++frame >= maxFrames)
            break;
    }
    Shutdown();
}
#endif
int RunApp(const AppConfig& config, const std::function<void()>& cb, int maxFrames) {
    if (!Init(config)) {
        UNIGUI_LOG_ERROR("App init failed; cannot start main loop");
        return 1;
    }
    Run(cb, maxFrames);
    return 0;
}
} // namespace unigui
