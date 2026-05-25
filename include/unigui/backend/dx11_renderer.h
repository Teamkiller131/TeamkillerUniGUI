#pragma once
#include <unigui/backend/renderer_backend.h>
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
namespace unigui {
bool CreateDX11DeviceAndSwapChain(void* hwnd, int w, int h,
    ID3D11Device** dev, ID3D11DeviceContext** ctx, IDXGISwapChain** swap);
class DX11Renderer : public RendererBackend {
public:
    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* ctx_ = nullptr;
    IDXGISwapChain* swapchain_ = nullptr;
    ID3D11RenderTargetView* rtv_ = nullptr;
    bool Init(ImGuiContext*) override;
    void Shutdown() override;
    void RenderDrawData(ImDrawData* dd) override;
    void SetClearColor(float r,float g,float b,float a) override;
private:
    bool initialized_ = false;
    float clearR_=0.10f, clearG_=0.10f, clearB_=0.12f, clearA_=1.00f;
};
}
