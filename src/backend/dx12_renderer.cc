#include <unigui/backend/dx12_renderer.h>

#include <imgui.h>
#include <imgui_impl_dx12.h>

#include <cstdio>
#include <memory>
#ifdef _WIN32
#include <d3d12.h>
#include <windows.h>

#include <dxgi1_6.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#endif

namespace unigui {

#ifdef _WIN32

// ── Static SRV descriptor allocator for the ImGui DX12 backend ────────────────
// ImGui 1.92's dynamic texture system requests one SRV descriptor per texture via
// these callbacks (font atlas + any user textures). We hand out slots linearly
// from the shader-visible CBV/SRV/UAV heap created below.
namespace {
struct SrvAllocState {
    UINT descriptorSize = 0;
    UINT capacity = 0;
    UINT nextIndex = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = {};
};

static SrvAllocState g_srvState;

static void SrvDescriptorAlloc(ImGui_ImplDX12_InitInfo* /*info*/,
                               D3D12_CPU_DESCRIPTOR_HANDLE* outCpu,
                               D3D12_GPU_DESCRIPTOR_HANDLE* outGpu) {
    if (g_srvState.nextIndex >= g_srvState.capacity) {
        std::fprintf(stderr, "[unigui] DX12: SRV descriptor heap exhausted (%u)\n",
                     g_srvState.capacity);
        *outCpu = {};
        *outGpu = {};
        return;
    }
    const UINT offset = g_srvState.nextIndex++ * g_srvState.descriptorSize;
    outCpu->ptr = g_srvState.cpuStart.ptr + offset;
    outGpu->ptr = g_srvState.gpuStart.ptr + offset;
}

static void SrvDescriptorFree(ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE,
                              D3D12_GPU_DESCRIPTOR_HANDLE) {
    // Linear allocator: descriptors are released wholesale at Shutdown().
}
} // anonymous namespace

// ── Device + SwapChain creation ───────────────────────────────────────────────
bool CreateDX12DeviceAndSwapChain(void* hwnd, int w, int h, ID3D12Device** outDevice,
                                  ID3D12CommandQueue** outQueue,
                                  ID3D12GraphicsCommandList** outCmdList, IDXGISwapChain3** outSwap,
                                  ID3D12DescriptorHeap** outRtvHeap,
                                  ID3D12DescriptorHeap** outSrvHeap) {
    HWND window = static_cast<HWND>(hwnd);
    UINT width = w > 0 ? static_cast<UINT>(w) : 800;
    UINT height = h > 0 ? static_cast<UINT>(h) : 600;

    UINT factoryFlags = 0;
#if defined(_DEBUG) || defined(DEBUG)
    {
        ComPtr<ID3D12Debug> dbg;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dbg)))) {
            dbg->EnableDebugLayer();
            factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        std::fprintf(stderr, "[unigui] DX12: CreateDXGIFactory2 failed: 0x%lx\n",
                     (unsigned long) hr);
        return false;
    }

    ComPtr<ID3D12Device> device;
    hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    if (FAILED(hr)) {
        std::fprintf(stderr, "[unigui] DX12: D3D12CreateDevice failed: 0x%lx\n",
                     (unsigned long) hr);
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC qd{};
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    qd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ComPtr<ID3D12CommandQueue> queue;
    hr = device->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue));
    if (FAILED(hr)) {
        std::fprintf(stderr, "[unigui] DX12: CreateCommandQueue failed: 0x%lx\n",
                     (unsigned long) hr);
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 sd{};
    sd.BufferCount = 2;
    sd.Width = width;
    sd.Height = height;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.Flags = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Scaling = DXGI_SCALING_STRETCH;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

    ComPtr<IDXGISwapChain1> swap1;
    hr = factory->CreateSwapChainForHwnd(queue.Get(), window, &sd, nullptr, nullptr, &swap1);
    if (FAILED(hr)) {
        std::fprintf(stderr, "[unigui] DX12: CreateSwapChainForHwnd failed: 0x%lx\n",
                     (unsigned long) hr);
        return false;
    }
    factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

    ComPtr<IDXGISwapChain3> swap;
    if (FAILED(swap1.As(&swap))) {
        std::fprintf(stderr, "[unigui] DX12: IDXGISwapChain3 query failed\n");
        return false;
    }

    // RTV heap: one descriptor per back buffer.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.NumDescriptors = sd.BufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
    if (FAILED(hr)) {
        std::fprintf(stderr, "[unigui] DX12: CreateDescriptorHeap(RTV) failed: 0x%lx\n",
                     (unsigned long) hr);
        return false;
    }

    // Shader-visible SRV heap for ImGui textures.
    const UINT kSrvCount = 256;
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
    srvHeapDesc.NumDescriptors = kSrvCount;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    hr = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
    if (FAILED(hr)) {
        std::fprintf(stderr, "[unigui] DX12: CreateDescriptorHeap(SRV) failed: 0x%lx\n",
                     (unsigned long) hr);
        return false;
    }

    g_srvState = {};
    g_srvState.descriptorSize =
        device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    g_srvState.capacity = kSrvCount;
    g_srvState.nextIndex = 0;
    g_srvState.cpuStart = srvHeap->GetCPUDescriptorHandleForHeapStart();
    g_srvState.gpuStart = srvHeap->GetGPUDescriptorHandleForHeapStart();

    ImGui_ImplDX12_InitInfo initInfo{};
    initInfo.Device = device.Get();
    initInfo.CommandQueue = queue.Get();
    initInfo.NumFramesInFlight = 2; // must match DX12Renderer::kNumFrames
    initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
    initInfo.SrvDescriptorHeap = srvHeap.Get();
    initInfo.SrvDescriptorAllocFn = SrvDescriptorAlloc;
    initInfo.SrvDescriptorFreeFn = SrvDescriptorFree;

    if (!ImGui_ImplDX12_Init(&initInfo)) {
        std::fprintf(stderr, "[unigui] DX12: ImGui_ImplDX12_Init failed\n");
        return false;
    }

    std::fprintf(stderr,
                 "[unigui] DX12: device + swapchain ready (%ux%u), renderer=imgui_impl_dx12\n",
                 width, height);

    // Per-frame command allocators and the command list are created by
    // DX12Renderer::Init(); the command list out-param is intentionally null here.
    *outCmdList = nullptr;
    *outDevice = device.Detach();
    *outQueue = queue.Detach();
    *outSwap = swap.Detach();
    *outRtvHeap = rtvHeap.Detach();
    *outSrvHeap = srvHeap.Detach();
    return true;
}

// ── DX12Renderer ──────────────────────────────────────────────────────────────
bool DX12Renderer::CreateRenderTargets() {
    const UINT rtvSize = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStart = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    for (int i = 0; i < kNumBackBuffers; ++i) {
        ComPtr<ID3D12Resource> bb;
        if (FAILED(swapchain_->GetBuffer((UINT) i, IID_PPV_ARGS(&bb)))) {
            std::fprintf(stderr, "[unigui] DX12: GetBuffer(%d) failed\n", i);
            return false;
        }
        D3D12_CPU_DESCRIPTOR_HANDLE h{};
        h.ptr = rtvStart.ptr + (SIZE_T) i * rtvSize;
        device_->CreateRenderTargetView(bb.Get(), nullptr, h);
        backBuffers_[i] = bb.Detach();
        rtvHandlePtr_[i] = h.ptr;
    }
    return true;
}

void DX12Renderer::CleanupRenderTargets() {
    for (int i = 0; i < kNumBackBuffers; ++i) {
        if (backBuffers_[i]) {
            backBuffers_[i]->Release();
            backBuffers_[i] = nullptr;
        }
        rtvHandlePtr_[i] = 0;
    }
}

void DX12Renderer::FlushGpu() {
    if (!fence_ || !cmdQueue_)
        return;
    const std::uint64_t v = ++fenceLastSignaled_;
    cmdQueue_->Signal(fence_, v);
    if (fence_->GetCompletedValue() < v) {
        fence_->SetEventOnCompletion(v, (HANDLE) fenceEvent_);
        WaitForSingleObject((HANDLE) fenceEvent_, INFINITE);
    }
    for (auto& f : frames_)
        f.fenceValue = 0;
}

DX12Renderer::FrameContext* DX12Renderer::WaitForNextFrameResources() {
    const unsigned int next = frameIndex_ + 1;
    frameIndex_ = next;
    FrameContext* f = &frames_[next % kNumFrames];
    const std::uint64_t v = f->fenceValue;
    if (v != 0) {
        f->fenceValue = 0;
        if (fence_->GetCompletedValue() < v) {
            fence_->SetEventOnCompletion(v, (HANDLE) fenceEvent_);
            WaitForSingleObject((HANDLE) fenceEvent_, INFINITE);
        }
    }
    return f;
}

bool DX12Renderer::Init(ImGuiContext* context) {
    if (!context && !ImGui::GetCurrentContext()) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
    }
    if (!device_ || !swapchain_ || !rtvHeap_ || !cmdQueue_) {
        std::fprintf(stderr, "[unigui] DX12: Init() missing device/swapchain handles\n");
        initialized_ = false;
        return false;
    }

    for (int i = 0; i < kNumFrames; ++i) {
        if (FAILED(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                   IID_PPV_ARGS(&frames_[i].alloc)))) {
            std::fprintf(stderr, "[unigui] DX12: CreateCommandAllocator(%d) failed\n", i);
            return false;
        }
        frames_[i].fenceValue = 0;
    }

    ComPtr<ID3D12GraphicsCommandList> cmd;
    if (FAILED(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frames_[0].alloc,
                                          nullptr, IID_PPV_ARGS(&cmd)))) {
        std::fprintf(stderr, "[unigui] DX12: CreateCommandList failed\n");
        return false;
    }
    cmd->Close();
    cmdList_ = cmd.Detach();

    if (FAILED(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)))) {
        std::fprintf(stderr, "[unigui] DX12: CreateFence failed\n");
        return false;
    }
    fenceEvent_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!fenceEvent_) {
        std::fprintf(stderr, "[unigui] DX12: CreateEvent failed\n");
        return false;
    }

    if (!CreateRenderTargets())
        return false;

    initialized_ = true;
    return true;
}

DX12Renderer::~DX12Renderer() {
    Shutdown();
}

void DX12Renderer::Shutdown() {
    // Release everything we own even when Init() never completed (initialized_ == false).
    // A failed bring-up still leaves the device/queue/swapchain/heaps owned here (they were
    // detached from CreateDX12DeviceAndSwapChain, which also ran ImGui_ImplDX12_Init), plus
    // any command allocators / command list / fence created before Init() failed. Early-
    // returning on !initialized_ leaked the entire device chain on every failed bring-up or
    // backend fallback. Each release below is individually guarded, so this is idempotent
    // (a second call after the explicit Shutdown in ResetBackendOnly is a no-op).
    FlushGpu(); // self-guards on fence_/cmdQueue_; no-op if the GPU was never set up

    // A non-null device_ means CreateDX12DeviceAndSwapChain succeeded — which is also where
    // ImGui_ImplDX12_Init ran — so the DX12 ImGui backend is live and must be shut down.
    // Calling ImGui_ImplDX12_Shutdown when it was never initialized would assert.
    if (device_)
        ImGui_ImplDX12_Shutdown();
    CleanupRenderTargets();

    if (cmdList_) {
        cmdList_->Release();
        cmdList_ = nullptr;
    }
    for (auto& f : frames_) {
        if (f.alloc) {
            f.alloc->Release();
            f.alloc = nullptr;
        }
        f.fenceValue = 0;
    }
    if (fenceEvent_) {
        CloseHandle((HANDLE) fenceEvent_);
        fenceEvent_ = nullptr;
    }
    if (fence_) {
        fence_->Release();
        fence_ = nullptr;
    }
    if (srvHeap_) {
        srvHeap_->Release();
        srvHeap_ = nullptr;
    }
    if (rtvHeap_) {
        rtvHeap_->Release();
        rtvHeap_ = nullptr;
    }
    if (swapchain_) {
        swapchain_->Release();
        swapchain_ = nullptr;
    }
    if (cmdQueue_) {
        cmdQueue_->Release();
        cmdQueue_ = nullptr;
    }
    if (device_) {
        device_->Release();
        device_ = nullptr;
    }

    g_srvState = {};
    frameIndex_ = 0;
    fenceLastSignaled_ = 0;
    initialized_ = false;
}

bool DX12Renderer::ResizeSwapChain(int w, int h) {
    if (!initialized_ || !swapchain_ || w <= 0 || h <= 0)
        return false;
    FlushGpu();
    CleanupRenderTargets();
    HRESULT hr = swapchain_->ResizeBuffers(0, (UINT) w, (UINT) h, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        std::fprintf(stderr, "[unigui] DX12: ResizeBuffers failed: 0x%lx\n", (unsigned long) hr);
        return false;
    }
    return CreateRenderTargets();
}

void DX12Renderer::RenderDrawData(ImDrawData* dd) {
    if (!initialized_ || !dd)
        return;

    FrameContext* frame = WaitForNextFrameResources();
    const UINT backIdx = swapchain_->GetCurrentBackBufferIndex();

    frame->alloc->Reset();
    cmdList_->Reset(frame->alloc, nullptr);

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = backBuffers_[backIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    cmdList_->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv{};
    rtv.ptr = rtvHandlePtr_[backIdx];
    const float clear[4] = {clearR_, clearG_, clearB_, clearA_};
    cmdList_->ClearRenderTargetView(rtv, clear, 0, nullptr);
    cmdList_->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

    ID3D12DescriptorHeap* heaps[] = {srvHeap_};
    cmdList_->SetDescriptorHeaps(1, heaps);

    ImGui_ImplDX12_RenderDrawData(dd, cmdList_);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cmdList_->ResourceBarrier(1, &barrier);

    cmdList_->Close();
    ID3D12CommandList* lists[] = {cmdList_};
    cmdQueue_->ExecuteCommandLists(1, lists);

    swapchain_->Present(1, 0);

    const std::uint64_t v = fenceLastSignaled_ + 1;
    cmdQueue_->Signal(fence_, v);
    fenceLastSignaled_ = v;
    frame->fenceValue = v;
}

void DX12Renderer::SetClearColor(float r, float g, float b, float a) {
    clearR_ = r;
    clearG_ = g;
    clearB_ = b;
    clearA_ = a;
}

std::unique_ptr<RendererBackend> CreateDX12Renderer() {
    return std::make_unique<DX12Renderer>();
}

#else  // !_WIN32
bool CreateDX12DeviceAndSwapChain(void*, int, int, ID3D12Device**, ID3D12CommandQueue**,
                                  ID3D12GraphicsCommandList**, IDXGISwapChain3**,
                                  ID3D12DescriptorHeap**, ID3D12DescriptorHeap**) {
    std::fprintf(stderr, "[unigui] DX12 is Windows-only\n");
    return false;
}
std::unique_ptr<RendererBackend> CreateDX12Renderer() {
    return nullptr;
}
#endif // _WIN32

} // namespace unigui
