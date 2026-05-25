#include <unigui/app/app.h>
#include <unigui/backend/backend_factory.h>
#include <unigui/theme/theme.h>
#include <glad/glad.h>
#include <imgui.h>
#include <cstdio>

#ifdef UNIGUI_HAS_SDL3_VULKAN
#include <unigui/backend/vulkan_context.h>
#include <SDL3/SDL.h>
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
        std::fprintf(stderr, "[unigui] Already initialized\n");
        return false;
    }

    auto backend = CreateBackend(config.backend);
    g_platform = std::move(backend.platform);
    g_renderer = std::move(backend.renderer);

    if (!g_platform || !g_platform->Init(nullptr)) {
        std::fprintf(stderr, "[unigui] Platform backend init failed\n");
        return false;
    }

#ifdef UNIGUI_HAS_SDL3_VULKAN
    if (config.backend == BackendType::SDL3_Vulkan) {
        auto* window = static_cast<SDL_Window*>(g_platform->GetWindowHandle());
        if (!window) { std::fprintf(stderr, "[unigui] SDL window is null\n"); return false; }
        try {
            g_vulkanCtx = InitVulkanContext(window, config.width, config.height);
        } catch (const std::exception& e) {
            std::fprintf(stderr, "[unigui] Vulkan init failed: %s\n", e.what());
            g_platform->Shutdown(); return false;
        }
    }
#endif

    if (!g_renderer || !g_renderer->Init(ImGui::GetCurrentContext())) {
        std::fprintf(stderr, "[unigui] Renderer backend init failed\n");
        g_platform->Shutdown(); return false;
    }

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2((float)config.width, (float)config.height);
    ApplyTheme(config.theme);

    g_initialized = true;
    return true;
}

void Shutdown() {
    if (!g_initialized) return;
    if (g_renderer) { g_renderer->Shutdown(); g_renderer.reset(); }
#ifdef UNIGUI_HAS_SDL3_VULKAN
    if (g_vulkanCtx.device.device) { DestroyVulkanContext(g_vulkanCtx); g_vulkanCtx = VulkanContext{}; }
#endif
    if (g_platform) { g_platform->Shutdown(); g_platform.reset(); }
    g_initialized = false;
}

bool NewFrame() {
    if (!g_initialized) return false;
    g_platform->NewFrame();
    ImGui::NewFrame();
    return true;
}

void Render() {
    if (!g_initialized) return;
    ImGui::Render();
    g_renderer->SetClearColor(0.10f, 0.10f, 0.12f, 1.00f);
    g_renderer->RenderDrawData(ImGui::GetDrawData());
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
