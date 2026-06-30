// WebGPU renderer backend.
//
// Real implementation on Emscripten (built with `--use-port=emdawnwebgpu` +
// UNIGUI_HAS_WEBGPU): renders to the HTML5 `#canvas` through the browser's WebGPU and
// drives imgui_impl_wgpu (imgui 1.92 targets the modern webgpu.h — WGPUStringView, the
// surface API, CallbackInfo structs — which the emdawnwebgpu port provides). WebGPU
// device acquisition is asynchronous; BringUp() kicks off the adapter/device request
// with AllowSpontaneous callbacks (which fire from the browser event loop) and returns
// immediately. The per-frame calls stay inert until OnDeviceReady configures the surface
// and runs ImGui_ImplWGPU_Init — early frames simply draw nothing, which fits the browser
// RAF loop. Off the web (or without UNIGUI_HAS_WEBGPU) this stays a clean {nullptr} stub.
#if defined(__EMSCRIPTEN__) && defined(UNIGUI_HAS_WEBGPU)

#include <unigui/backend/webgpu_renderer.h>
#include <unigui/core/log.h>

#include <imgui.h>
#include <imgui_impl_wgpu.h>

#include <webgpu/webgpu.h>

namespace unigui {
namespace {

// Namespace-level so the C-API request callbacks (which receive void* userdata) can name
// and access it — a private nested type couldn't be referenced from free functions.
struct WgpuState {
    WGPUInstance instance = nullptr;
    WGPUSurface surface = nullptr;
    WGPUAdapter adapter = nullptr;
    WGPUDevice device = nullptr;
    WGPUQueue queue = nullptr;
    WGPUTextureFormat format = WGPUTextureFormat_BGRA8Unorm;
    WGPUTextureView currentView = nullptr;

    int width = 1280, height = 720;
    bool ready = false;
    bool imguiInited = false;
    double clearR = 0.10, clearG = 0.10, clearB = 0.12, clearA = 1.00;
};

void ConfigureSurface(WgpuState* s, int w, int h) {
    WGPUSurfaceConfiguration cfg = {};
    cfg.device = s->device;
    cfg.format = s->format;
    cfg.usage = WGPUTextureUsage_RenderAttachment;
    cfg.width = static_cast<uint32_t>(w);
    cfg.height = static_cast<uint32_t>(h);
    cfg.presentMode = WGPUPresentMode_Fifo;
    cfg.alphaMode = WGPUCompositeAlphaMode_Auto;
    wgpuSurfaceConfigure(s->surface, &cfg);
    s->width = w;
    s->height = h;
}

void OnDeviceReady(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView /*message*/,
                   void* userdata1, void* /*userdata2*/) {
    auto* s = static_cast<WgpuState*>(userdata1);
    if (status != WGPURequestDeviceStatus_Success || !device) {
        UNIGUI_LOG_ERROR("WebGPU: requestDevice failed");
        return;
    }
    s->device = device;
    s->queue = wgpuDeviceGetQueue(device);

    // Pick a surface format from the adapter capabilities (fall back to BGRA8Unorm).
    WGPUSurfaceCapabilities caps = {};
    if (wgpuSurfaceGetCapabilities(s->surface, s->adapter, &caps) == WGPUStatus_Success &&
        caps.formatCount > 0)
        s->format = caps.formats[0];
    ConfigureSurface(s, s->width, s->height);

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
    UNIGUI_LOG_INFO("WebGPU: device ready, surface configured ({}x{})", s->width, s->height);
}

void OnAdapterReady(WGPURequestAdapterStatus status, WGPUAdapter adapter,
                    WGPUStringView /*message*/, void* userdata1, void* /*userdata2*/) {
    auto* s = static_cast<WgpuState*>(userdata1);
    if (status != WGPURequestAdapterStatus_Success || !adapter) {
        UNIGUI_LOG_ERROR("WebGPU: requestAdapter failed");
        return;
    }
    s->adapter = adapter;

    WGPURequestDeviceCallbackInfo cbInfo = {};
    cbInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    cbInfo.callback = OnDeviceReady;
    cbInfo.userdata1 = s;
    WGPUDeviceDescriptor devDesc = {};
    wgpuAdapterRequestDevice(adapter, &devDesc, cbInfo);
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

    p_->instance = wgpuCreateInstance(nullptr);
    if (!p_->instance) {
        UNIGUI_LOG_ERROR("WebGPU: wgpuCreateInstance returned null (no WebGPU in this browser?)");
        return false;
    }

    // Surface bound to the Emscripten HTML5 canvas.
    WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvasSource = {};
    canvasSource.chain.sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
    canvasSource.selector = WGPUStringView{"#canvas", WGPU_STRLEN};
    WGPUSurfaceDescriptor sdesc = {};
    sdesc.nextInChain = &canvasSource.chain;
    p_->surface = wgpuInstanceCreateSurface(p_->instance, &sdesc);
    if (!p_->surface) {
        UNIGUI_LOG_ERROR("WebGPU: failed to create surface for #canvas");
        return false;
    }

    // Kick off async adapter -> device. Renderer is not Ready() until OnDeviceReady fires.
    WGPURequestAdapterOptions opts = {};
    opts.compatibleSurface = p_->surface;
    WGPURequestAdapterCallbackInfo cbInfo = {};
    cbInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    cbInfo.callback = OnAdapterReady;
    cbInfo.userdata1 = static_cast<WgpuState*>(p_.get());
    wgpuInstanceRequestAdapter(p_->instance, &opts, cbInfo);
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
        ConfigureSurface(p_.get(), width, height);

    WGPUSurfaceTexture st = {};
    wgpuSurfaceGetCurrentTexture(p_->surface, &st);

    // Only SuccessOptimal/SuccessSuboptimal yield a texture we should draw into. On
    // Timeout/Outdated/Lost (typically right after a resize) reconfigure the surface and skip
    // the frame rather than render into a stale/expired texture. Any texture handed back must
    // be released either way (see below).
    const bool usable = st.texture != nullptr &&
                        (st.status == WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal ||
                         st.status == WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal);
    if (!usable) {
        if (st.texture)
            wgpuTextureRelease(st.texture);
        if (width > 0 && height > 0)
            ConfigureSurface(p_.get(), width, height);
        return;
    }

    p_->currentView = wgpuTextureCreateView(st.texture, nullptr);
    // wgpuSurfaceGetCurrentTexture hands the caller an owned reference to st.texture; the view
    // we just created holds its own independent reference, so release ours now. Without this
    // the renderer leaks one WGPUTexture per frame (~60/s under the browser RAF loop).
    wgpuTextureRelease(st.texture);
    ImGui_ImplWGPU_NewFrame();
}

void WebGPURenderer::RenderDrawData(ImDrawData* draw_data) {
    if (!p_->ready || !p_->currentView)
        return;

    WGPURenderPassColorAttachment color = {};
    color.view = p_->currentView;
    color.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    color.loadOp = WGPULoadOp_Clear;
    color.storeOp = WGPUStoreOp_Store;
    color.clearValue = WGPUColor{p_->clearR, p_->clearG, p_->clearB, p_->clearA};

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
    // Release the owned WebGPU object chain in reverse acquisition order. Shutdown can run more
    // than once per process (backend teardown/recreate via ResetBackendOnly, plus the
    // destructor), so null each handle to keep a second pass a no-op. Without this every
    // bring-up/teardown cycle leaks the device/queue/adapter/surface/instance.
    if (p_->queue) {
        wgpuQueueRelease(p_->queue);
        p_->queue = nullptr;
    }
    if (p_->device) {
        wgpuDeviceRelease(p_->device);
        p_->device = nullptr;
    }
    if (p_->adapter) {
        wgpuAdapterRelease(p_->adapter);
        p_->adapter = nullptr;
    }
    if (p_->surface) {
        wgpuSurfaceRelease(p_->surface);
        p_->surface = nullptr;
    }
    if (p_->instance) {
        wgpuInstanceRelease(p_->instance);
        p_->instance = nullptr;
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
