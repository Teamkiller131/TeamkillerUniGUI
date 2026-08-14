#include <unigui/backend/dx11_renderer.h>
#include <unigui/core/log.h>

#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>

#include "../detail/golden_capture.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
#ifdef _WIN32
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace unigui {

bool CreateDX11DeviceAndSwapChain(void* hwnd, int width, int height, ID3D11Device** outDevice,
                                  ID3D11DeviceContext** outCtx, IDXGISwapChain** outSwap,
                                  ID3D11RenderTargetView** outRtv) {
#ifdef _WIN32
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = (UINT) width;
    sd.BufferDesc.Height = (UINT) height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = (HWND) hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featLevel;
    const D3D_FEATURE_LEVEL featLevels[] = {D3D_FEATURE_LEVEL_11_0};
    // UNIGUI_DX11_WARP=1 forces the WARP software rasterizer (Microsoft's high-fidelity
    // software adapter, present on every Windows 8.1+) instead of the hardware device —
    // the headless-CI escape hatch that lets the app-level smokes create a REAL device,
    // render, and read pixels back without a GPU. Hardware stays the default; WARP falls
    // back to hardware if WARP itself is unavailable.
    const bool wantWarp = [] {
        const char* e = std::getenv("UNIGUI_DX11_WARP");
        return e && e[0] == '1';
    }();
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, wantWarp ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            featLevels, 1, D3D11_SDK_VERSION, &sd, outSwap, outDevice, &featLevel, outCtx);
    if (FAILED(hr) && wantWarp) {
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                           featLevels, 1, D3D11_SDK_VERSION, &sd, outSwap,
                                           outDevice, &featLevel, outCtx);
    }
    if (FAILED(hr)) {
        std::fprintf(stderr, "[unigui] DX11 device creation failed: 0x%lx\n", (unsigned long) hr);
        return false;
    }
    UNIGUI_LOG_INFO("DX11 device created on {} adapter",
                    wantWarp ? "WARP (software)" : "hardware");

    // Create render target view
    ID3D11Texture2D* backBuffer = nullptr;
    (*outSwap)->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &backBuffer);
    ID3D11RenderTargetView* rtv = nullptr;
    (*outDevice)->CreateRenderTargetView(backBuffer, nullptr, &rtv);
    backBuffer->Release();
    (*outCtx)->OMSetRenderTargets(1, &rtv, nullptr);
    if (outRtv)
        *outRtv = rtv;

    D3D11_VIEWPORT vp{};
    vp.Width = (FLOAT) width;
    vp.Height = (FLOAT) height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    (*outCtx)->RSSetViewports(1, &vp);

    ImGui_ImplDX11_Init(*outDevice, *outCtx);
    return true;
#else
    (void) hwnd;
    (void) width;
    (void) height;
    (void) outDevice;
    (void) outCtx;
    (void) outSwap;
    return false;
#endif
}

bool DX11Renderer::Init(ImGuiContext* context) {
    if (!context && !ImGui::GetCurrentContext()) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
    }
    initialized_ = (device_ != nullptr);
    return initialized_;
}
void DX11Renderer::Shutdown() {
    if (!initialized_)
        return;
    ImGui_ImplDX11_Shutdown();
    if (rtv_) {
        rtv_->Release();
        rtv_ = nullptr;
    }
    if (ctx_) {
        ctx_->Release();
        ctx_ = nullptr;
    }
    if (device_) {
        device_->Release();
        device_ = nullptr;
    }
    if (swapchain_) {
        swapchain_->Release();
        swapchain_ = nullptr;
    }
    initialized_ = false;
}
void DX11Renderer::RenderDrawData(ImDrawData* dd) {
    if (!initialized_ || !dd)
        return;
    float c[4] = {clearR_, clearG_, clearB_, clearA_};
    // Bind the main render target EVERY frame, not just at device creation / resize.
    //
    // ClearRenderTargetView takes the RTV by handle, so the clear always hit the right
    // surface — but the draw goes to whatever is *bound*, and this used to rely on the
    // binding made once in CreateDeviceD3D()/resize surviving forever. Nothing else
    // rebound it, so it did… until multi-viewport: ImGui_ImplDX11_RenderWindow() binds
    // each secondary window's RTV and does not restore the previous one. From the first
    // frame a popped-out window renders, the main window would clear its own surface and
    // then draw into the secondary one — showing nothing but the backdrop colour, and
    // recovering only when a resize happened to rebind (maximising "fixed" it).
    // Binding per frame is also what the upstream imgui DX11 example does.
    ctx_->OMSetRenderTargets(1, &rtv_, nullptr);
    ctx_->ClearRenderTargetView(rtv_, c);
    ImGui_ImplDX11_RenderDrawData(dd);
    VerifyRenderIfEnabled(); // reads the swapchain back buffer; no-op unless env var set
    swapchain_->Present(1, 0);
}

// Render verification for CI, mirroring the GL path's UNIGUI_RENDER_VERIFY: read the
// swapchain back buffer back (before Present — DISCARD swap effect frees it after) and
// report whether the UI actually drew pixels vs. just a clear. This is what proves the
// multi-viewport RTV-rebind fix at runtime: after a secondary window renders, the main
// window must still contain drawn pixels, not a bare backdrop. Inert unless the env var
// is set, so zero cost in normal runs.
void DX11Renderer::VerifyRenderIfEnabled() {
    static const bool verify = [] {
        const char* e = std::getenv("UNIGUI_RENDER_VERIFY");
        return e && e[0] == '1';
    }();
    const char* golden = detail::GoldenCapturePath();
    if ((!verify && !golden) || !swapchain_ || !device_ || !ctx_) {
        if (verify)
            lastVerifyDrawn_ = -1;
        return;
    }

    lastVerifyDrawn_ = -1;
    ID3D11Texture2D* back = nullptr;
    if (FAILED(swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &back)))
        return;
    D3D11_TEXTURE2D_DESC td{};
    back->GetDesc(&td);

    D3D11_TEXTURE2D_DESC sd = td;
    sd.Usage = D3D11_USAGE_STAGING;
    sd.BindFlags = 0;
    sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    sd.MiscFlags = 0;
    ID3D11Texture2D* staging = nullptr;
    if (FAILED(device_->CreateTexture2D(&sd, nullptr, &staging))) {
        back->Release();
        return;
    }
    ctx_->CopyResource(staging, back);
    // D3D11 Map(READ) does not itself wait for the queued copy — flush so the readback
    // sees THIS frame's pixels, not whatever the GPU happened to have finished.
    ctx_->Flush();

    D3D11_MAPPED_SUBRESOURCE m{};
    const HRESULT mapHr = ctx_->Map(staging, 0, D3D11_MAP_READ, 0, &m);
    if (FAILED(mapHr)) {
        staging->Release();
        back->Release();
        return;
    }
    const uint8_t* px = static_cast<const uint8_t*>(m.pData);

    // Golden capture: dump the FULL buffer (row pitch may exceed width*4 — copy
    // tightly packed rows so the raw format is plain RGBA).
    if (golden) {
        std::vector<uint8_t> packed((size_t) td.Width * td.Height * 4u);
        for (UINT y = 0; y < td.Height; ++y)
            std::memcpy(packed.data() + (size_t) y * td.Width * 4u,
                        px + (size_t) y * m.RowPitch, (size_t) td.Width * 4u);
        if (!detail::SaveGoldenRaw(golden, (int) td.Width, (int) td.Height, packed.data()))
            UNIGUI_LOG_WARN("golden capture: failed to write {}", golden);
    }

    // Sample a coarse grid (cheap) and count pixels that differ from the clear colour.
    auto to8 = [](float v) { return (int) std::lround(v * 255.0f); };
    const int cr = to8(clearR_), cg = to8(clearG_), cb = to8(clearB_);
    const int step = 16;
    int total = 0, nonClear = 0;
    for (UINT y = 0; y < td.Height; y += step) {
        for (UINT x = 0; x < td.Width; x += step) {
            const size_t i = ((size_t) y * m.RowPitch) + (size_t) x * 4u;
            ++total;
            if (std::abs((int) px[i] - cr) > 8 || std::abs((int) px[i + 1] - cg) > 8 ||
                std::abs((int) px[i + 2] - cb) > 8)
                ++nonClear;
        }
    }
    ctx_->Unmap(staging, 0);
    staging->Release();
    back->Release();

    if (verify) {
        lastVerifyDrawn_ = (nonClear >= 4) ? 1 : 0;
        UNIGUI_LOG_INFO("[render-verify] nonClear={}/{} drawn={}", nonClear, total,
                        lastVerifyDrawn_ ? "true" : "false");
    }
}
void DX11Renderer::SetClearColor(float r, float g, float b, float a) {
    clearR_ = r;
    clearG_ = g;
    clearB_ = b;
    clearA_ = a;
}

bool DX11Renderer::ResizeSwapChain(int w, int h) {
    if (!swapchain_ || w <= 0 || h <= 0)
        return false;
    if (rtv_) {
        rtv_->Release();
        rtv_ = nullptr;
    }
    ctx_->OMSetRenderTargets(0, nullptr, nullptr);
    HRESULT hr = swapchain_->ResizeBuffers(0, (UINT) w, (UINT) h, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        UNIGUI_LOG_ERROR("DX11 ResizeBuffers failed");
        return false;
    }
    ID3D11Texture2D* bb = nullptr;
    swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &bb);
    device_->CreateRenderTargetView(bb, nullptr, &rtv_);
    bb->Release();
    ctx_->OMSetRenderTargets(1, &rtv_, nullptr);
    D3D11_VIEWPORT vp{};
    vp.Width = (FLOAT) w;
    vp.Height = (FLOAT) h;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    ctx_->RSSetViewports(1, &vp);
    return true;
}

std::unique_ptr<RendererBackend> CreateDX11Renderer() {
    return std::make_unique<DX11Renderer>();
}

} // namespace unigui
