#include <unigui/backend/dx11_renderer.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>
#include <cstdio>
#include <memory>
#ifdef _WIN32
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace unigui {

bool CreateDX11DeviceAndSwapChain(void* hwnd, int width, int height,
    ID3D11Device** outDevice, ID3D11DeviceContext** outCtx, IDXGISwapChain** outSwap) {
#ifdef _WIN32
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = (UINT)width;
    sd.BufferDesc.Height = (UINT)height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = (HWND)hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featLevel;
    const D3D_FEATURE_LEVEL featLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        featLevels, 1, D3D11_SDK_VERSION, &sd,
        outSwap, outDevice, &featLevel, outCtx);
    if (FAILED(hr)) { std::fprintf(stderr, "[unigui] DX11 device creation failed: 0x%lx\n", (unsigned long)hr); return false; }

    // Create render target view
    ID3D11Texture2D* backBuffer = nullptr;
    (*outSwap)->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    ID3D11RenderTargetView* rtv = nullptr;
    (*outDevice)->CreateRenderTargetView(backBuffer, nullptr, &rtv);
    backBuffer->Release();
    (*outCtx)->OMSetRenderTargets(1, &rtv, nullptr);

    D3D11_VIEWPORT vp{};
    vp.Width = (FLOAT)width; vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
    (*outCtx)->RSSetViewports(1, &vp);

    ImGui_ImplDX11_Init(*outDevice, *outCtx);
    return true;
#else
    (void)hwnd; (void)width; (void)height; (void)outDevice; (void)outCtx; (void)outSwap;
    return false;
#endif
}

bool DX11Renderer::Init(ImGuiContext* context) {
    if (!context && !ImGui::GetCurrentContext()) { IMGUI_CHECKVERSION(); ImGui::CreateContext(); }
    initialized_ = (device_ != nullptr);
    return initialized_;
}
void DX11Renderer::Shutdown() {
    if (!initialized_) return;
    ImGui_ImplDX11_Shutdown();
    if (rtv_) { rtv_->Release(); rtv_ = nullptr; }
    if (ctx_) { ctx_->Release(); ctx_ = nullptr; }
    if (device_) { device_->Release(); device_ = nullptr; }
    if (swapchain_) { swapchain_->Release(); swapchain_ = nullptr; }
    initialized_ = false;
}
void DX11Renderer::RenderDrawData(ImDrawData* dd) {
    if (!initialized_ || !dd) return;
    float c[4] = {clearR_, clearG_, clearB_, clearA_};
    ctx_->ClearRenderTargetView(rtv_, c);
    ImGui_ImplDX11_RenderDrawData(dd);
    swapchain_->Present(1, 0);
}
void DX11Renderer::SetClearColor(float r, float g, float b, float a) { clearR_=r; clearG_=g; clearB_=b; clearA_=a; }

std::unique_ptr<RendererBackend> CreateDX11Renderer() { return std::make_unique<DX11Renderer>(); }

} // namespace unigui
