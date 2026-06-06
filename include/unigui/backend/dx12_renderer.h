#pragma once
#include <unigui/backend/renderer_backend.h>
#include <cstddef>
#include <cstdint>

struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;
struct ID3D12CommandAllocator;
struct ID3D12Resource;
struct ID3D12Fence;
struct IDXGISwapChain3;

namespace unigui {

bool CreateDX12DeviceAndSwapChain(void* hwnd, int w, int h,
    ID3D12Device** dev, ID3D12CommandQueue** queue,
    ID3D12GraphicsCommandList** cmdList, IDXGISwapChain3** swap,
    ID3D12DescriptorHeap** rtvHeap, ID3D12DescriptorHeap** srvHeap);

class DX12Renderer : public RendererBackend {
public:
    // Populated by app.cc from CreateDX12DeviceAndSwapChain() before Init().
    ID3D12Device* device_ = nullptr;
    ID3D12CommandQueue* cmdQueue_ = nullptr;
    ID3D12GraphicsCommandList* cmdList_ = nullptr;
    IDXGISwapChain3* swapchain_ = nullptr;
    ID3D12DescriptorHeap* rtvHeap_ = nullptr;
    ID3D12DescriptorHeap* srvHeap_ = nullptr;

    bool Init(ImGuiContext*) override;
    void Shutdown() override;
    void RenderDrawData(ImDrawData* dd) override;
    void SetClearColor(float r, float g, float b, float a) override;

    // Recreate swap-chain buffers after a window resize (called by app.cc NewFrame).
    bool ResizeSwapChain(int w, int h);

private:
    static constexpr int kNumFrames = 2;       // command-allocator ring (frames in flight)
    static constexpr int kNumBackBuffers = 2;  // swap-chain buffer count

    struct FrameContext {
        ID3D12CommandAllocator* alloc = nullptr;
        std::uint64_t fenceValue = 0;
    };

    FrameContext frames_[kNumFrames]{};
    unsigned int frameIndex_ = 0;

    ID3D12Fence* fence_ = nullptr;
    void* fenceEvent_ = nullptr;               // HANDLE
    std::uint64_t fenceLastSignaled_ = 0;

    ID3D12Resource* backBuffers_[kNumBackBuffers]{};
    std::size_t rtvHandlePtr_[kNumBackBuffers]{}; // D3D12_CPU_DESCRIPTOR_HANDLE::ptr

    bool initialized_ = false;
    float clearR_ = 0.10f, clearG_ = 0.10f, clearB_ = 0.12f, clearA_ = 1.00f;

    bool CreateRenderTargets();
    void CleanupRenderTargets();
    void FlushGpu();
    FrameContext* WaitForNextFrameResources();
};

} // namespace unigui
