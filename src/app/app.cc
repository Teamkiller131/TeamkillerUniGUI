#include <unigui/app/app.h>
#include <unigui/backend/backend_factory.h>
#include <unigui/core/dpi.h>
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
#include <glad/glad.h>
#include <imgui.h>
#include <implot.h>

#include <cstdio>
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
        dpi = DetectDPIScale(g_platform->GetWindowHandle());
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
    g_platform->NewFrame();
    ImGui::NewFrame();
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
    if (g_backend == BackendType::GLFW_GL3) {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    g_renderer->RenderDrawData(dd);
    if (g_backend == BackendType::GLFW_GL3)
        g_platform->SwapBuffers();
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
int RunApp(const AppConfig& config, const std::function<void()>& cb, int maxFrames) {
    if (!Init(config)) {
        UNIGUI_LOG_ERROR("App init failed; cannot start main loop");
        return 1;
    }
    Run(cb, maxFrames);
    return 0;
}
} // namespace unigui
