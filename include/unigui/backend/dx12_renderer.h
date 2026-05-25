#pragma once
#include <unigui/backend/renderer_backend.h>
struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;
struct IDXGISwapChain3;
namespace unigui {
bool CreateDX12DeviceAndSwapChain(void* hwnd, int w, int h,
    ID3D12Device** dev, ID3D12CommandQueue** queue,
    ID3D12GraphicsCommandList** cmdList, IDXGISwapChain3** swap,
    ID3D12DescriptorHeap** rtvHeap, ID3D12DescriptorHeap** srvHeap);
class DX12Renderer : public RendererBackend {
public:
    ID3D12Device* device_ = nullptr;
    ID3D12CommandQueue* cmdQueue_ = nullptr;
    ID3D12GraphicsCommandList* cmdList_ = nullptr;
    IDXGISwapChain3* swapchain_ = nullptr;
    ID3D12DescriptorHeap* rtvHeap_ = nullptr;
    ID3D12DescriptorHeap* srvHeap_ = nullptr;
    bool Init(ImGuiContext*) override;
    void Shutdown() override;
    void RenderDrawData(ImDrawData* dd) override;
    void SetClearColor(float r,float g,float b,float a) override;
private:
    bool initialized_ = false;
    float clearR_=0.10f, clearG_=0.10f, clearB_=0.12f, clearA_=1.00f;
};
} // namespace unigui
