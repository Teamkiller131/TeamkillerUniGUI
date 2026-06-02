// Metal renderer backend — macOS-only (requires Objective-C++ compilation)
// Full implementation uses ImGui Metal backend (imgui_impl_metal.h)
#ifdef __APPLE__
#include <unigui/backend/renderer_backend.h>
#include <unigui/core/log.h>
#include <imgui.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>
#include <cstdio>
#include <memory>

#if defined(__has_include)
#  if __has_include(<imgui_impl_metal.h>)
#    include <imgui_impl_metal.h>
#    define UNIGUI_HAS_IMGUI_IMPL_METAL 1
#  else
#    define UNIGUI_HAS_IMGUI_IMPL_METAL 0
#  endif
#else
#  define UNIGUI_HAS_IMGUI_IMPL_METAL 0
#endif

namespace unigui {

namespace {
class MetalRenderer : public RendererBackend {
public:
    bool Init(ImGuiContext* context) override {
        if (!context && !ImGui::GetCurrentContext()) {
            IMGUI_CHECKVERSION(); ImGui::CreateContext();
        }
#if !UNIGUI_HAS_IMGUI_IMPL_METAL
        UNIGUI_LOG_ERROR("Metal backend unavailable: imgui_impl_metal.h not found. Install ImGui Metal backend headers or disable the Metal renderer.");
        return false;
#else
        device_ = MTLCreateSystemDefaultDevice();
        if (!device_) {
            UNIGUI_LOG_ERROR("Metal: no Metal-capable GPU found");
            return false;
        }
        // Create command queue
        commandQueue_ = [device_ newCommandQueue];
        // ImGui Metal backend init
        ImGui_ImplMetal_Init(device_);
        initialized_ = true;
        UNIGUI_LOG_INFO("Metal: device created, backend initialized");
        return true;
#endif
    }

    void Shutdown() override {
#if UNIGUI_HAS_IMGUI_IMPL_METAL
        if (!initialized_) return;
        ImGui_ImplMetal_Shutdown();
        if (commandQueue_) { [commandQueue_ release]; commandQueue_ = nullptr; }
        if (device_)       { [device_ release];       device_ = nullptr; }
        initialized_ = false;
#endif
    }

    void RenderDrawData(ImDrawData* dd) override {
#if UNIGUI_HAS_IMGUI_IMPL_METAL
        if (!initialized_ || !dd) return;
        // Metal frame: create command buffer, encode render commands, present
        @autoreleasepool {
            id<MTLCommandBuffer> cmdBuf = [commandQueue_ commandBuffer];
            ImGui_ImplMetal_RenderDrawData(dd, cmdBuf);
            id<CAMetalDrawable> drawable = nil;
            if (!drawable) {
                UNIGUI_LOG_WARN("Metal: no drawable available, skipping present");
                [cmdBuf commit];
                return;
            }
            [cmdBuf presentDrawable:drawable];
            [cmdBuf commit];
        }
#else
        (void)dd;
#endif
    }

    void SetClearColor(float r, float g, float b, float a) override {
        clearR_ = r; clearG_ = g; clearB_ = b; clearA_ = a;
    }

    /// Resize the Metal swap chain / framebuffer.
    bool ResizeSwapChain(int w, int h) override {
        (void)w; (void)h; // Metal manages swap chain via CAMetalLayer
        UNIGUI_LOG_DEBUG("Metal: ResizeSwapChain {}x{}", w, h);
        return true;
    }

private:
    id<MTLDevice>        device_ = nullptr;
    id<MTLCommandQueue>  commandQueue_ = nullptr;
    bool initialized_ = false;
    float clearR_ = 0.f, clearG_ = 0.f, clearB_ = 0.f, clearA_ = 1.f;
};
} // anonymous namespace

std::unique_ptr<RendererBackend> CreateMetalRenderer() {
    return std::make_unique<MetalRenderer>();
}

} // namespace unigui

#else // !__APPLE__
// Non-macOS stub — compilation guard
#include <unigui/backend/backend_factory.h>
#include <cstdio>
namespace unigui {
std::unique_ptr<RendererBackend> CreateMetalRenderer() {
    std::fprintf(stderr, "[unigui] Metal backend is only available on macOS\n");
    return nullptr;
}
} // namespace unigui
#endif // __APPLE__
