// WebGPU renderer backend.
//
// Real implementation on Emscripten (built with `-sUSE_WEBGPU=1` + UNIGUI_HAS_WEBGPU):
// renders to the HTML5 `#canvas` through the browser's WebGPU and drives
// imgui_impl_wgpu. WebGPU device acquisition is asynchronous, so BringUp() starts the
// adapter/device request and returns immediately; the per-frame calls stay inert until
// the device callback fires (Ready()), which fits the browser RAF main loop. Off the
// web (or without UNIGUI_HAS_WEBGPU) this stays a clean {nullptr} stub.
#if defined(__EMSCRIPTEN__) && defined(UNIGUI_HAS_WEBGPU)

#include <unigui/backend/webgpu_renderer.h>
#include <unigui/core/log.h>

#include <imgui.h>
#include <imgui_impl_wgpu.h>

#include <webgpu/webgpu.h>

namespace unigui {
namespace {

// Namespace-level so the C-API request callbacks (which receive a void* userdata) can
// name and access it — a private nested type couldn't be referenced from free functions.
struct WgpuState {
    WGPUInstance instance = nullptr;
    WGPUSurface surface = nullptr;
    WGPUAdapter adapter = nullptr;
    WGPUDevice device = nullptr;
    WGPUQueue queue = nullptr;
    WGPUSwapChain swapchain = nullptr;
    WGPUTextureFormat format = WGPUTextureFormat_BGRA8Unorm;
    WGPUTextureView currentView = nullptr;

    int width = 1280, height = 720;
    bool ready = false;
    bool imguiInited = false;
    double clearR = 0.10, clearG = 0.10, clearB = 0.12, clearA = 1.00;
};

void ConfigureSwapChain(WgpuState* s, int w, int h) {
    if (s->swapchain) {
        wgpuSwapChainRelease(s->swapchain);
        s->swapchain = nullptr;
    }
    WGPUSwapChainDescriptor d = {};
    d.usage = WGPUTextureUsage_RenderAttachment;
    d.format = s->format;
    d.width = static_cast<uint32_t>(w);
    d.height = static_cast<uint32_t>(h);
    d.presentMode = WGPUPresentMode_Fifo;
    s->swapchain = wgpuDeviceCreateSwapChain(s->device, s->surface, &d);
    s->width = w;
    s->height = h;
}

void OnDeviceReady(WGPURequestDeviceStatus status, WGPUDevice device, const char* message,
                   void* userdata) {
    auto* s = static_cast<WgpuState*>(userdata);
    if (status != WGPURequestDeviceStatus_Success || !device) {
        UNIGUI_LOG_ERROR("WebGPU: requestDevice failed: {}", message ? message : "(no message)");
        return;
    }
    s->device = device;
    s->queue = wgpuDeviceGetQueue(device);
    ConfigureSwapChain(s, s->width, s->height);

    ImGui_ImplWGPU_InitInfo info = {};
    info.Device = s->device;
    info.NumFramesInFlight = 3;
    info.RenderTargetFormat = s->format;
    info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    if (!ImGui_ImplWGPU_Init(&info)) {
        UNIGUI_LOG_ERROR("WebGPU: ImGui_ImplWGPU_Init failed");
        return;
    }
    s->imguiInited = true;
    s->ready = true;
    UNIGUI_LOG_INFO("WebGPU: device ready, swap chain configured ({}x{})", s->width, s->height);
}

void OnAdapterReady(WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message,
                    void* userdata) {
    auto* s = static_cast<WgpuState*>(userdata);
    if (status != WGPURequestAdapterStatus_Success || !adapter) {
        UNIGUI_LOG_ERROR("WebGPU: requestAdapter failed: {}", message ? message : "(no message)");
        return;
    }
    s->adapter = adapter;
    wgpuAdapterRequestDevice(adapter, nullptr, OnDeviceReady, s);
}

} // anonymous namespace

struct WebGPURenderer::Impl : WgpuState {};

WebGPURenderer::WebGPURenderer()
        : p_(std::make_unique<Impl>()) {}
WebGPURenderer::~WebGPURenderer() {
    Shutdown();
}

bool WebGPURenderer::BringUp(int width, int height) {
    p_->width = width > 0 ? width : 1280;
    p_->height = height > 0 ? height : 720;

    WGPUInstanceDescriptor idesc = {};
    p_->instance = wgpuCreateInstance(&idesc);
    if (!p_->instance) {
        UNIGUI_LOG_ERROR("WebGPU: wgpuCreateInstance returned null (no WebGPU in this browser?)");
        return false;
    }

    // Surface bound to the Emscripten HTML5 canvas.
    WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc = {};
    canvasDesc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
    canvasDesc.selector = "#canvas";
    WGPUSurfaceDescriptor sdesc = {};
    sdesc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&canvasDesc);
    p_->surface = wgpuInstanceCreateSurface(p_->instance, &sdesc);
    if (!p_->surface) {
        UNIGUI_LOG_ERROR("WebGPU: failed to create surface for #canvas");
        return false;
    }

    // Kick off async adapter -> device. Renderer is not Ready() until OnDeviceReady fires.
    WGPURequestAdapterOptions opts = {};
    opts.compatibleSurface = p_->surface;
    wgpuInstanceRequestAdapter(p_->instance, &opts, OnAdapterReady,
                               static_cast<WgpuState*>(p_.get()));
    UNIGUI_LOG_INFO("WebGPU: instance + surface up, awaiting async device");
    return true;
}

bool WebGPURenderer::Init(ImGuiContext*) {
    // The real device setup happens asynchronously after BringUp(); report whether the
    // instance exists so the app's backend bring-up can proceed.
    return p_->instance != nullptr;
}

bool WebGPURenderer::Ready() const {
    return p_->ready;
}

void WebGPURenderer::SetClearColor(float r, float g, float b, float a) {
    p_->clearR = r;
    p_->clearG = g;
    p_->clearB = b;
    p_->clearA = a;
}

void WebGPURenderer::NewFrameWGPU(int width, int height) {
    if (!p_->ready)
        return; // device still pending — draw nothing this frame
    if (width > 0 && height > 0 && (width != p_->width || height != p_->height))
        ConfigureSwapChain(p_.get(), width, height);
    p_->currentView = wgpuSwapChainGetCurrentTextureView(p_->swapchain);
    ImGui_ImplWGPU_NewFrame();
}

void WebGPURenderer::RenderDrawData(ImDrawData* draw_data) {
    if (!p_->ready || !p_->currentView)
        return;

    WGPURenderPassColorAttachment color = {};
    color.view = p_->currentView;
    color.loadOp = WGPULoadOp_Clear;
    color.storeOp = WGPUStoreOp_Store;
    color.clearValue = WGPUColor{p_->clearR * p_->clearA, p_->clearG * p_->clearA,
                                 p_->clearB * p_->clearA, p_->clearA};

    WGPURenderPassDescriptor pass = {};
    pass.colorAttachmentCount = 1;
    pass.colorAttachments = &color;

    WGPUCommandEncoderDescriptor encDesc = {};
    WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder(p_->device, &encDesc);
    WGPURenderPassEncoder rp = wgpuCommandEncoderBeginRenderPass(enc, &pass);
    if (draw_data)
        ImGui_ImplWGPU_RenderDrawData(draw_data, rp);
    wgpuRenderPassEncoderEnd(rp);

    WGPUCommandBufferDescriptor cmdDesc = {};
    WGPUCommandBuffer cmd = wgpuCommandEncoderFinish(enc, &cmdDesc);
    wgpuQueueSubmit(p_->queue, 1, &cmd);

    // The browser presents the canvas at the next RAF tick; no explicit present needed.
    wgpuCommandBufferRelease(cmd);
    wgpuRenderPassEncoderRelease(rp);
    wgpuCommandEncoderRelease(enc);
    wgpuTextureViewRelease(p_->currentView);
    p_->currentView = nullptr;
}

void WebGPURenderer::Shutdown() {
    if (p_->imguiInited) {
        ImGui_ImplWGPU_Shutdown();
        p_->imguiInited = false;
    }
    if (p_->currentView) {
        wgpuTextureViewRelease(p_->currentView);
        p_->currentView = nullptr;
    }
    if (p_->swapchain) {
        wgpuSwapChainRelease(p_->swapchain);
        p_->swapchain = nullptr;
    }
    p_->ready = false;
}

std::unique_ptr<RendererBackend> CreateWebGPURenderer() {
    return std::make_unique<WebGPURenderer>();
}

} // namespace unigui

#else // !(__EMSCRIPTEN__ && UNIGUI_HAS_WEBGPU)

#include <unigui/backend/backend_factory.h>

#include <cstdio>
namespace unigui {
// WebGPU is only wired up for the Emscripten/browser build (UNIGUI_HAS_WEBGPU). On every
// other target the factory keeps the {nullptr, nullptr} contract for BackendType::WebGPU.
std::unique_ptr<RendererBackend> CreateWebGPURenderer() {
    std::fprintf(stderr, "[unigui] WebGPU backend is only available on the Emscripten build\n");
    return nullptr;
}
} // namespace unigui

#endif
