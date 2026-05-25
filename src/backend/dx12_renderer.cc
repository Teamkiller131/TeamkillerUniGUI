#include <unigui/backend/dx12_renderer.h>
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <cstdio>
#include <memory>
#include <vector>
#ifdef _WIN32
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#endif

namespace unigui {

#ifdef _WIN32

// ── Static SRV descriptor allocator for ImGui DX12 backend ────────────────────
namespace {
struct SrvAllocState {
    ID3D12DescriptorHeap* heap = nullptr;
    UINT descriptorSize = 0;
    UINT nextIndex = 1; // 0 reserved for font
    D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = {};
};

static SrvAllocState g_srvState;

static void SrvDescriptorAlloc(ImGui_ImplDX12_InitInfo* info,
    D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu) {
    (void)info;
    if (g_srvState.nextIndex >= 256) { *outCpu = {}; *outGpu = {}; return; }
    UINT offset = g_srvState.nextIndex++ * g_srvState.descriptorSize;
    outCpu->ptr = g_srvState.cpuStart.ptr + offset;
    outGpu->ptr = g_srvState.gpuStart.ptr + offset;
}
static void SrvDescriptorFree(ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE) {
    // Simple linear allocator — no free
}
} // anonymous namespace

// ── Device + SwapChain creation ───────────────────────────────────────────────
bool CreateDX12DeviceAndSwapChain(void* hwnd, int w, int h,
    ID3D12Device** outDevice, ID3D12CommandQueue** outQueue,
    ID3D12GraphicsCommandList** outCmdList, IDXGISwapChain3** outSwap,
    ID3D12DescriptorHeap** outRtvHeap, ID3D12DescriptorHeap** outSrvHeap) {
    HWND window = static_cast<HWND>(hwnd);
    UINT width = w > 0 ? static_cast<UINT>(w) : 800;
    UINT height = h > 0 ? static_cast<UINT>(h) : 600;

    // Create DXGI factory
    ComPtr<IDXGIFactory4> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) { std::fprintf(stderr, "[unigui] DX12: CreateDXGIFactory1 failed: 0x%lx\n", (unsigned long)hr); return false; }

    // Create D3D12 device
    ComPtr<ID3D12Device> device;
    hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    if (FAILED(hr)) { std::fprintf(stderr, "[unigui] DX12: D3D12CreateDevice failed: 0x%lx\n", (unsigned long)hr); return false; }

    // Create command queue
    D3D12_COMMAND_QUEUE_DESC qd{};
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    qd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ComPtr<ID3D12CommandQueue> queue;
    hr = device->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue));
    if (FAILED(hr)) { std::fprintf(stderr, "[unigui] DX12: CreateCommandQueue failed: 0x%lx\n", (unsigned long)hr); return false; }

    // Create swapchain
    DXGI_SWAP_CHAIN_DESC1 sd{};
    sd.BufferCount = 2;
    sd.Width = width;
    sd.Height = height;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    ComPtr<IDXGISwapChain1> swap1;
    hr = factory->CreateSwapChainForHwnd(queue.Get(), window, &sd, nullptr, nullptr, &swap1);
    if (FAILED(hr)) { std::fprintf(stderr, "[unigui] DX12: CreateSwapChainForHwnd failed: 0x%lx\n", (unsigned long)hr); return false; }
    factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

    ComPtr<IDXGISwapChain3> swap;
    swap1.As(&swap);

    // Create RTV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
    if (FAILED(hr)) { std::fprintf(stderr, "[unigui] DX12: CreateDescriptorHeap(RTV) failed: 0x%lx\n", (unsigned long)hr); return false; }

    // Create SRV descriptor heap for ImGui
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
    srvHeapDesc.NumDescriptors = 256;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    hr = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
    if (FAILED(hr)) { std::fprintf(stderr, "[unigui] DX12: CreateDescriptorHeap(SRV) failed: 0x%lx\n", (unsigned long)hr); return false; }

    // Init SRV allocator state
    g_srvState.heap = srvHeap.Get();
    g_srvState.descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    g_srvState.nextIndex = 1;
    g_srvState.cpuStart = srvHeap->GetCPUDescriptorHandleForHeapStart();
    g_srvState.gpuStart = srvHeap->GetGPUDescriptorHandleForHeapStart();

    // Create command allocator
    ComPtr<ID3D12CommandAllocator> allocator;
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
    if (FAILED(hr)) { std::fprintf(stderr, "[unigui] DX12: CreateCommandAllocator failed: 0x%lx\n", (unsigned long)hr); return false; }

    // Create command list
    ComPtr<ID3D12GraphicsCommandList> cmdList;
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&cmdList));
    if (FAILED(hr)) { std::fprintf(stderr, "[unigui] DX12: CreateCommandList failed: 0x%lx\n", (unsigned long)hr); return false; }
    cmdList->Close();

    // Init ImGui DX12 backend
    ImGui_ImplDX12_InitInfo initInfo{};
    initInfo.Device = device.Get();
    initInfo.CommandQueue = queue.Get();
    initInfo.NumFramesInFlight = 2;
    initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
    initInfo.SrvDescriptorHeap = srvHeap.Get();
    initInfo.SrvDescriptorAllocFn = SrvDescriptorAlloc;
    initInfo.SrvDescriptorFreeFn = SrvDescriptorFree;
    // Legacy handles for compat
    initInfo.LegacySingleSrvCpuDescriptor = g_srvState.cpuStart;
    initInfo.LegacySingleSrvGpuDescriptor = g_srvState.gpuStart;

    if (!ImGui_ImplDX12_Init(&initInfo)) {
        std::fprintf(stderr, "[unigui] DX12: ImGui_ImplDX12_Init failed\n");
        return false;
    }

    // Transfer ownership
    *outDevice = device.Detach();
    *outQueue = queue.Detach();
    *outCmdList = cmdList.Detach();
    *outSwap = swap.Detach();
    *outRtvHeap = rtvHeap.Detach();
    *outSrvHeap = srvHeap.Detach();
    return true;
}

// ── DX12Renderer methods ─────────────────────────────────────────────────────
bool DX12Renderer::Init(ImGuiContext* context) {
    if (!context && !ImGui::GetCurrentContext()) { IMGUI_CHECKVERSION(); ImGui::CreateContext(); }
    initialized_ = (device_ != nullptr);
    return initialized_;
}

void DX12Renderer::Shutdown() {
    if (!initialized_) return;
    ImGui_ImplDX12_Shutdown();
    if (srvHeap_) { srvHeap_->Release(); srvHeap_ = nullptr; }
    if (rtvHeap_) { rtvHeap_->Release(); rtvHeap_ = nullptr; }
    if (cmdList_) { cmdList_->Release(); cmdList_ = nullptr; }
    if (cmdQueue_) { cmdQueue_->Release(); cmdQueue_ = nullptr; }
    if (swapchain_) { swapchain_->Release(); swapchain_ = nullptr; }
    if (device_) { device_->Release(); device_ = nullptr; }
    g_srvState = {};
    initialized_ = false;
}

void DX12Renderer::RenderDrawData(ImDrawData* dd) {
    if (!initialized_ || !dd) return;
    // Transition back buffer, render, present — backend user manages frame orchestration
    ImGui_ImplDX12_RenderDrawData(dd, cmdList_);
    swapchain_->Present(1, 0);
}

void DX12Renderer::SetClearColor(float r, float g, float b, float a) {
    clearR_ = r; clearG_ = g; clearB_ = b; clearA_ = a;
}

std::unique_ptr<RendererBackend> CreateDX12Renderer() { return std::make_unique<DX12Renderer>(); }

#else // !_WIN32
bool CreateDX12DeviceAndSwapChain(void*, int, int, ID3D12Device**, ID3D12CommandQueue**,
    ID3D12GraphicsCommandList**, IDXGISwapChain3**, ID3D12DescriptorHeap**, ID3D12DescriptorHeap**) {
    std::fprintf(stderr, "[unigui] DX12 is Windows-only\n");
    return false;
}
std::unique_ptr<RendererBackend> CreateDX12Renderer() { return nullptr; }
#endif // _WIN32

} // namespace unigui
