#include <unigui/app/app.h>
#include <unigui/backend/backend_factory.h>
#include <unigui/theme/theme.h>
#include <unigui/core/log.h>
#include <glad/glad.h>
#include <imgui.h>
#include <cstdio>

#ifdef UNIGUI_HAS_SDL3_VULKAN
#include <unigui/backend/vulkan_context.h>
#include <SDL3/SDL.h>
#endif

#ifdef UNIGUI_HAS_DX11
#include <unigui/backend/dx11_renderer.h>
#endif

namespace unigui {

static bool g_initialized = false;
static std::unique_ptr<PlatformBackend> g_platform;
static std::unique_ptr<RendererBackend> g_renderer;
#ifdef UNIGUI_HAS_SDL3_VULKAN
static VulkanContext g_vulkanCtx;
#endif

bool Init(const AppConfig& config) {
    if (g_initialized) {
        UNIGUI_LOG_WARN("Init called but already initialized");
        return false;
    }

    InitLogging("debug");
    UNIGUI_LOG_INFO("Init: backend={}, {}x{}, title='{}'",
        (int)config.backend, config.width, config.height, config.title);

    // Create ImGui context FIRST (required before any backend Init)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    UNIGUI_LOG_DEBUG("ImGui context created");

    auto backend = CreateBackend(config.backend);
    g_platform = std::move(backend.platform);
    g_renderer = std::move(backend.renderer);

    if (!g_platform || !g_platform->Init(nullptr)) {
        UNIGUI_LOG_ERROR("Platform backend init failed (backend={})", (int)config.backend);
        return false;
    }
    UNIGUI_LOG_DEBUG("Platform backend initialized");

#ifdef UNIGUI_HAS_SDL3_VULKAN
    if (config.backend == BackendType::SDL3_Vulkan) {
        auto* window = static_cast<SDL_Window*>(g_platform->GetWindowHandle());
        if (!window) { std::fprintf(stderr, "[unigui] SDL window is null\n"); return false; }
        try { g_vulkanCtx = InitVulkanContext(window, config.width, config.height); }
        catch (const std::exception& e) { std::fprintf(stderr, "[unigui] Vulkan init failed: %s\n", e.what()); g_platform->Shutdown(); return false; }
    }
#endif

#ifdef UNIGUI_HAS_DX11
    if (config.backend == BackendType::DX11) {
        auto* hwnd = g_platform->GetWindowHandle();
        if (!hwnd) { std::fprintf(stderr, "[unigui] HWND is null\n"); return false; }
        ID3D11Device* dev = nullptr; ID3D11DeviceContext* ctx = nullptr; IDXGISwapChain* swap = nullptr;
        if (!unigui::CreateDX11DeviceAndSwapChain(hwnd, config.width, config.height, &dev, &ctx, &swap)) {
            std::fprintf(stderr, "[unigui] DX11 device creation failed\n"); g_platform->Shutdown(); return false;
        }
        auto* dxr = static_cast<unigui::DX11Renderer*>(g_renderer.get());
        dxr->device_ = dev; dxr->ctx_ = ctx; dxr->swapchain_ = swap;
    }
#endif

    if (!g_renderer || !g_renderer->Init(ImGui::GetCurrentContext())) {
        UNIGUI_LOG_ERROR("Renderer backend init failed");
        g_platform->Shutdown(); return false;
    }
    UNIGUI_LOG_DEBUG("Renderer initialized");

    g_platform->SetTitle(config.title);
    g_platform->SetSize(config.width, config.height);

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2((float)config.width, (float)config.height);
    ApplyTheme(config.theme);

    g_initialized = true;
    UNIGUI_LOG_INFO("Init complete — {}x{} docking={} viewports={}",
        config.width, config.height,
        (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) ? 1 : 0,
        (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) ? 1 : 0);
    return true;
}

void Shutdown() {
    if (!g_initialized) return;
    UNIGUI_LOG_INFO("Shutdown: releasing renderer + platform");
    if (g_renderer) { g_renderer->Shutdown(); g_renderer.reset(); }
#ifdef UNIGUI_HAS_SDL3_VULKAN
    if (g_vulkanCtx.device.device) { DestroyVulkanContext(g_vulkanCtx); g_vulkanCtx = VulkanContext{}; }
#endif
    if (g_platform) { g_platform->Shutdown(); g_platform.reset(); }
    g_initialized = false;
    UNIGUI_LOG_DEBUG("Shutdown complete");
}

bool NewFrame() {
    if (!g_initialized) return false;
    g_platform->PollEvents();
    g_platform->NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
        ImGuiDockNodeFlags_PassthruCentralNode);

    return true;
}

void Render() {
    if (!g_initialized) return;
    ImGui::Render();
    ImDrawData* dd = ImGui::GetDrawData();
    if (dd && dd->CmdListsCount > 0) {
        UNIGUI_LOG_TRACE("Render: {} CmdLists, {} Vtx, {} Idx",
            dd->CmdListsCount, dd->TotalVtxCount, dd->TotalIdxCount);
    }
    g_renderer->SetClearColor(1.0f, 0.0f, 0.0f, 1.0f); // RED diagnostic
    glClear(GL_COLOR_BUFFER_BIT);

    // Flush any GL errors before rendering
    while (glGetError() != GL_NO_ERROR) {}

    g_renderer->RenderDrawData(dd);

    GLenum err = glGetError();
    while (err != GL_NO_ERROR) {
        UNIGUI_LOG_WARN("GL error after RenderDrawData: 0x{:04x}", (unsigned)err);
        err = glGetError();
    }

    g_platform->SwapBuffers();
}

bool ShouldClose() { return g_platform ? g_platform->ShouldClose() : true; }

void Run(const std::function<void()>& callback) {
    while (!ShouldClose()) {
        g_platform->PollEvents();
        NewFrame();
        callback();
        Render();
    }
    Shutdown();
}

} // namespace unigui
