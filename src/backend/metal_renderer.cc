// Metal renderer backend (macOS) — real implementation.
//
// Compiled as Objective-C++ with ARC (see src/CMakeLists.txt). Drives imgui_impl_metal
// against a CAMetalLayer attached to the GLFW window's NSView. The platform creates the
// window with GLFW_NO_API (BackendType::Metal -> needGL_==false), so Metal owns the
// drawable; the app loop calls NewFrameMetal() before ImGui::NewFrame() and
// RenderDrawData() encodes + presents.
#ifdef __APPLE__
#include <unigui/backend/metal_renderer.h>
#include <unigui/core/log.h>

#include <imgui.h>
#include <imgui_impl_metal.h>

#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace unigui {

struct MetalRenderer::Impl {
    id<MTLDevice> device = nil;
    id<MTLCommandQueue> queue = nil;
    CAMetalLayer* layer = nil;
    MTLRenderPassDescriptor* pass = nil;

    // Per-frame state: built in NewFrameMetal, consumed/cleared in RenderDrawData.
    id<CAMetalDrawable> drawable = nil;
    id<MTLCommandBuffer> commandBuffer = nil;
    id<MTLRenderCommandEncoder> encoder = nil;

    bool imguiInited = false;
    double clearR = 0.10, clearG = 0.10, clearB = 0.12, clearA = 1.00;
};

MetalRenderer::MetalRenderer()
        : p_(std::make_unique<Impl>()) {}
MetalRenderer::~MetalRenderer() {
    Shutdown();
}

bool MetalRenderer::BringUp(void* nsWindowPtr, int width, int height) {
    p_->device = MTLCreateSystemDefaultDevice();
    if (!p_->device) {
        UNIGUI_LOG_ERROR("Metal: MTLCreateSystemDefaultDevice() returned nil (no Metal GPU)");
        return false;
    }
    p_->queue = [p_->device newCommandQueue];

    NSWindow* nswin = (__bridge NSWindow*) nsWindowPtr;
    if (!nswin) {
        UNIGUI_LOG_ERROR("Metal: no NSWindow from the platform backend");
        return false;
    }

    const int w = width > 0 ? width : 1280;
    const int h = height > 0 ? height : 720;
    p_->layer = [CAMetalLayer layer];
    p_->layer.device = p_->device;
    p_->layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    p_->layer.drawableSize = CGSizeMake(w, h);
    nswin.contentView.layer = p_->layer;
    nswin.contentView.wantsLayer = YES;

    p_->pass = [MTLRenderPassDescriptor new];

    if (!ImGui_ImplMetal_Init(p_->device)) {
        UNIGUI_LOG_ERROR("Metal: ImGui_ImplMetal_Init failed");
        return false;
    }
    p_->imguiInited = true;
    UNIGUI_LOG_INFO("Metal: device + CAMetalLayer ready ({}x{})", w, h);
    return true;
}

bool MetalRenderer::Init(ImGuiContext*) {
    // BringUp() does the real setup (it needs the NSWindow); Init just reports
    // readiness, mirroring the Vulkan renderer.
    return p_->imguiInited;
}

void MetalRenderer::SetClearColor(float r, float g, float b, float a) {
    p_->clearR = r;
    p_->clearG = g;
    p_->clearB = b;
    p_->clearA = a;
}

void MetalRenderer::RunFrameInScope(const std::function<void()>& body) {
    // Drain the frame's autoreleased drawable/command-buffer/encoder here. Without this,
    // on a manual render loop (no NSApplication run loop draining a pool each iteration)
    // the CAMetalLayer's drawable pool is exhausted after a few frames and [layer
    // nextDrawable] blocks/returns nil — the app stalls. Mirrors the Dear ImGui
    // GLFW+Metal example, which wraps each frame body in @autoreleasepool.
    @autoreleasepool {
        if (body)
            body();
    }
}

void MetalRenderer::NewFrameMetal(int width, int height) {
    if (!p_->layer)
        return;
    const int w = width > 0 ? width : 1280;
    const int h = height > 0 ? height : 720;
    p_->layer.drawableSize = CGSizeMake(w, h);
    p_->drawable = [p_->layer nextDrawable];
    if (!p_->drawable)
        return; // no drawable available this frame — skip rendering
    p_->commandBuffer = [p_->queue commandBuffer];
    p_->pass.colorAttachments[0].clearColor = MTLClearColorMake(
        p_->clearR * p_->clearA, p_->clearG * p_->clearA, p_->clearB * p_->clearA, p_->clearA);
    p_->pass.colorAttachments[0].texture = p_->drawable.texture;
    p_->pass.colorAttachments[0].loadAction = MTLLoadActionClear;
    p_->pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    p_->encoder = [p_->commandBuffer renderCommandEncoderWithDescriptor:p_->pass];
    ImGui_ImplMetal_NewFrame(p_->pass);
}

void MetalRenderer::RenderDrawData(ImDrawData* drawData) {
    if (!p_->encoder || !p_->commandBuffer)
        return; // NewFrameMetal couldn't acquire a drawable this frame
    if (drawData)
        ImGui_ImplMetal_RenderDrawData(drawData, p_->commandBuffer, p_->encoder);
    [p_->encoder endEncoding];
    if (p_->drawable)
        [p_->commandBuffer presentDrawable:p_->drawable];
    [p_->commandBuffer commit];
    p_->encoder = nil;
    p_->commandBuffer = nil;
    p_->drawable = nil;
    // Release the render pass's strong reference to the just-presented drawable's texture so
    // it isn't pinned across the inter-frame gap (the descriptor is reused next frame).
    p_->pass.colorAttachments[0].texture = nil;
}

void MetalRenderer::Shutdown() {
    if (p_->imguiInited) {
        ImGui_ImplMetal_Shutdown();
        p_->imguiInited = false;
    }
    p_->encoder = nil;
    p_->commandBuffer = nil;
    p_->drawable = nil;
    p_->pass = nil;
    p_->layer = nil;
    p_->queue = nil;
    p_->device = nil;
}

std::unique_ptr<RendererBackend> CreateMetalRenderer() {
    return std::make_unique<MetalRenderer>();
}

} // namespace unigui

#else // !__APPLE__
// Non-macOS stub — the symbol must exist (it is declared unconditionally in
// backend_factory.h) but Metal is macOS-only.
#include <unigui/backend/backend_factory.h>

#include <cstdio>
namespace unigui {
std::unique_ptr<RendererBackend> CreateMetalRenderer() {
    std::fprintf(stderr, "[unigui] Metal backend is only available on macOS\n");
    return nullptr;
}
} // namespace unigui
#endif // __APPLE__
