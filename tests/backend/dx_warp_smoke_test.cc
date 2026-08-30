// WARP (Windows Advanced Rasterization Platform) headless render smoke.
//
// The DX11/DX12 backends previously compiled in CI but were never *run* — the
// assumption was that a D3D device needs a GPU the headless runner lacks. WARP
// disproves that: it is Microsoft's high-fidelity software rasterizer, present on
// every supported Windows, and creates a *real* D3D11/D3D12 device with no GPU. That
// lets us exercise the actual device-create → offscreen render → GPU readback path —
// the same primitives our renderers use (D3D11CreateDevice / ClearRenderTargetView /
// Map for DX11; DXGI factory → EnumWarpAdapter → D3D12CreateDevice → command list →
// ExecuteCommandLists → fence for DX12) — and assert the pixels came out correct.
//
// Each half is gated on its backend macro (UNIGUI_HAS_DX11 / UNIGUI_HAS_DX12, both
// PUBLIC on the unigui target), so this file is empty on non-Windows / disabled
// backends and the CMake target is only registered when at least one is on.

#include <gtest/gtest.h>

#if defined(_WIN32) && (defined(UNIGUI_HAS_DX11) || defined(UNIGUI_HAS_DX12))
#include <windows.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

namespace {
// A clear colour with three *distinct*, cleanly-rounding channels so a passing read
// proves the render actually ran (not all-zero, not all-0xFF, not a channel swizzle).
// R8G8B8A8_UNORM is linear (not _SRGB), so the byte is round(component * 255):
//   0.25 → 64, 0.50 → 128, 0.75 → 191, 1.0 → 255.
constexpr float kClear[4] = {0.25f, 0.5f, 0.75f, 1.0f};
constexpr int kExpR = 64, kExpG = 128, kExpB = 191, kExpA = 255;
constexpr UINT kW = 16, kH = 16;

// Rounding tolerance: WARP's float→UNORM conversion is round-to-nearest, but keep a
// ±2 window so a rasteriser revision that rounds a half-bit differently doesn't flake.
void ExpectClearPixel(const uint8_t* px, const char* which) {
    EXPECT_NEAR(px[0], kExpR, 2) << which << " R";
    EXPECT_NEAR(px[1], kExpG, 2) << which << " G";
    EXPECT_NEAR(px[2], kExpB, 2) << which << " B";
    EXPECT_EQ(px[3], kExpA) << which << " A";
}
} // namespace
#endif

// ── DX11 ──────────────────────────────────────────────────────────────────────
#if defined(_WIN32) && defined(UNIGUI_HAS_DX11)
#include <d3d11.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>

TEST(DXWarpSmoke, DX11_WarpDevice_OffscreenClear_ReadbackMatches) {
    // Software (WARP) device — no swapchain, no window, no GPU.
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    D3D_FEATURE_LEVEL got{};
    const D3D_FEATURE_LEVEL want[] = {D3D_FEATURE_LEVEL_11_0};
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, want, 1,
                                   D3D11_SDK_VERSION, &dev, &got, &ctx);
    ASSERT_TRUE(SUCCEEDED(hr)) << "WARP D3D11CreateDevice failed: 0x" << std::hex << (unsigned) hr;
    ASSERT_TRUE(dev);
    ASSERT_TRUE(ctx);
    EXPECT_EQ(got, D3D_FEATURE_LEVEL_11_0);

    // Offscreen RGBA8 render target.
    D3D11_TEXTURE2D_DESC td{};
    td.Width = kW;
    td.Height = kH;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET;
    ComPtr<ID3D11Texture2D> rt;
    ASSERT_TRUE(SUCCEEDED(dev->CreateTexture2D(&td, nullptr, &rt)));

    ComPtr<ID3D11RenderTargetView> rtv;
    ASSERT_TRUE(SUCCEEDED(dev->CreateRenderTargetView(rt.Get(), nullptr, &rtv)));

    ctx->ClearRenderTargetView(rtv.Get(), kClear);
    ctx->Flush();

    // CPU-readable staging copy.
    D3D11_TEXTURE2D_DESC sd = td;
    sd.Usage = D3D11_USAGE_STAGING;
    sd.BindFlags = 0;
    sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ComPtr<ID3D11Texture2D> staging;
    ASSERT_TRUE(SUCCEEDED(dev->CreateTexture2D(&sd, nullptr, &staging)));
    ctx->CopyResource(staging.Get(), rt.Get());

    D3D11_MAPPED_SUBRESOURCE m{};
    ASSERT_TRUE(SUCCEEDED(ctx->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &m)));
    ExpectClearPixel(static_cast<const uint8_t*>(m.pData), "DX11");
    ctx->Unmap(staging.Get(), 0);
}

// ── Multi-viewport RTV-rebind regression (2026-08 client-suite) ─────────────────
//
// ImGui_ImplDX11_RenderWindow() binds each secondary window's RTV and never restores
// the previous binding. DX11Renderer::RenderDrawData therefore binds the MAIN render
// target every frame — otherwise, from the first frame a popped-out window rendered,
// the main window would clear its own surface and then draw into the secondary one
// (blank except the backdrop; the 2026-08-07 fix). This test replays the exact
// sequence against real ImGui draw data on a WARP device:
//
//   main draw → secondary bind + draw → main draw again
//
// and asserts the main surface still received the pixels. Against the pre-fix code
// the final draw lands on the *secondary* surface and the main readback is pure clear.
TEST(DXWarpSmoke, DX11_MultiViewportRtvRebind_MainSurfaceStillDrawn) {
    // WARP device — no GPU, no window.
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    D3D_FEATURE_LEVEL got{};
    const D3D_FEATURE_LEVEL want[] = {D3D_FEATURE_LEVEL_11_0};
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, want, 1,
                                   D3D11_SDK_VERSION, &dev, &got, &ctx);
    ASSERT_TRUE(SUCCEEDED(hr)) << "WARP D3D11CreateDevice failed: 0x" << std::hex << (unsigned) hr;

    // Two offscreen RGBA8 render targets: the "main" window and a "secondary" one.
    const UINT rtW = 64, rtH = 64;
    auto MakeRt = [&](ComPtr<ID3D11Texture2D>& tex, ComPtr<ID3D11RenderTargetView>& rtv) {
        D3D11_TEXTURE2D_DESC td{};
        td.Width = rtW;
        td.Height = rtH;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET;
        ASSERT_TRUE(SUCCEEDED(dev->CreateTexture2D(&td, nullptr, &tex)));
        ASSERT_TRUE(SUCCEEDED(dev->CreateRenderTargetView(tex.Get(), nullptr, &rtv)));
    };
    ComPtr<ID3D11Texture2D> mainTex, secTex;
    ComPtr<ID3D11RenderTargetView> mainRtv, secRtv;
    MakeRt(mainTex, mainRtv);
    MakeRt(secTex, secRtv);

    // Real ImGui draw data on the WARP device (no platform, no window needed).
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float) rtW, (float) rtH);
    // Init the backend BEFORE building fonts: imgui 1.92's new-backend contract
    // asserts RendererHasTextures is set by the time the atlas is built.
    ASSERT_TRUE(ImGui_ImplDX11_Init(dev.Get(), ctx.Get()));
    io.Fonts->Build();

    auto DrawTextFrame = [&]() -> ImDrawData* {
        ImGui_ImplDX11_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float) rtW, (float) rtH));
        ImGui::Begin("m", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextUnformatted("XXXXXXXX");
        ImGui::End();
        ImGui::Render();
        return ImGui::GetDrawData();
    };
    // A clear colour distinct from kClear so a wrong-surface draw is unmistakable.
    const float secClear[4] = {0.10f, 0.20f, 0.30f, 1.0f};

    // Frame 1 — the main window draws into the main surface.
        ImDrawData* dd = DrawTextFrame();
    ctx->OMSetRenderTargets(1, mainRtv.GetAddressOf(), nullptr);
    ctx->ClearRenderTargetView(mainRtv.Get(), kClear);
    ImGui_ImplDX11_RenderDrawData(dd);

    // Simulate a popped-out window: ImGui_ImplDX11_RenderWindow binds the SECONDARY
    // RTV (and, pre-fix, leaves it bound).
        ctx->OMSetRenderTargets(1, secRtv.GetAddressOf(), nullptr);
    ctx->ClearRenderTargetView(secRtv.Get(), secClear);
    ImGui_ImplDX11_RenderDrawData(dd);

    // Frame 2 — the main window draws again. The per-frame rebind must re-target the
    // main surface; without it the draw still goes to the secondary one.
        dd = DrawTextFrame();
    ctx->OMSetRenderTargets(1, mainRtv.GetAddressOf(), nullptr);
    ctx->ClearRenderTargetView(mainRtv.Get(), kClear);
    ImGui_ImplDX11_RenderDrawData(dd);

    // Read the main surface back: drawn pixels prove the frame-2 draw landed here.
        D3D11_TEXTURE2D_DESC td{};
    mainTex->GetDesc(&td);
    D3D11_TEXTURE2D_DESC sd = td;
    sd.Usage = D3D11_USAGE_STAGING;
    sd.BindFlags = 0;
    sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ComPtr<ID3D11Texture2D> staging;
    ASSERT_TRUE(SUCCEEDED(dev->CreateTexture2D(&sd, nullptr, &staging)));
    ctx->CopyResource(staging.Get(), mainTex.Get());

    D3D11_MAPPED_SUBRESOURCE m{};
    ASSERT_TRUE(SUCCEEDED(ctx->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &m)));
    int nonClear = 0;
    const uint8_t* px = static_cast<const uint8_t*>(m.pData);
    for (UINT y = 0; y < rtH; y += 4) {
        for (UINT x = 0; x < rtW; x += 4) {
            const size_t i = (size_t) y * m.RowPitch + (size_t) x * 4u;
            if (std::abs((int) px[i] - kExpR) > 8 || std::abs((int) px[i + 1] - kExpG) > 8 ||
                std::abs((int) px[i + 2] - kExpB) > 8)
                ++nonClear;
        }
    }
    ctx->Unmap(staging.Get(), 0);
    EXPECT_GT(nonClear, 0)
        << "main surface contains only the clear colour — the frame-2 draw went to the "
           "secondary surface (per-frame RTV rebind regression)";

    ImGui_ImplDX11_Shutdown();
    ImGui::DestroyContext();
}
#endif // UNIGUI_HAS_DX11

// ── DX12 ──────────────────────────────────────────────────────────────────────
#if defined(_WIN32) && defined(UNIGUI_HAS_DX12)
#include <d3d12.h>

#include <dxgi1_6.h>

TEST(DXWarpSmoke, DX12_WarpDevice_OffscreenClear_ReadbackMatches) {
    // No debug layer here on purpose: it depends on the "Graphics Tools" optional
    // feature that may be absent on a runner, and this is a render smoke, not a
    // validation run. dx12_renderer.cc still enables it under _DEBUG for real bring-up.
    ComPtr<IDXGIFactory4> factory;
    ASSERT_TRUE(SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory))));

    ComPtr<IDXGIAdapter> warp;
    ASSERT_TRUE(SUCCEEDED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp))))
        << "no WARP adapter — is this Windows 10+?";

    ComPtr<ID3D12Device> dev;
    HRESULT hr = D3D12CreateDevice(warp.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dev));
    ASSERT_TRUE(SUCCEEDED(hr)) << "WARP D3D12CreateDevice failed: 0x" << std::hex << (unsigned) hr;

    D3D12_COMMAND_QUEUE_DESC qd{};
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ComPtr<ID3D12CommandQueue> queue;
    ASSERT_TRUE(SUCCEEDED(dev->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue))));
    ComPtr<ID3D12CommandAllocator> alloc;
    ASSERT_TRUE(SUCCEEDED(
        dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc))));
    ComPtr<ID3D12GraphicsCommandList> cmd;
    ASSERT_TRUE(SUCCEEDED(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.Get(),
                                                 nullptr, IID_PPV_ARGS(&cmd))));

    // Offscreen render target (DEFAULT heap), created already in RENDER_TARGET state so
    // we can clear it without a barrier-in.
    D3D12_HEAP_PROPERTIES heapDefault{};
    heapDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC rd{};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rd.Width = kW;
    rd.Height = kH;
    rd.DepthOrArraySize = 1;
    rd.MipLevels = 1;
    rd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rd.SampleDesc.Count = 1;
    rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_CLEAR_VALUE cv{};
    cv.Format = rd.Format;
    cv.Color[0] = kClear[0];
    cv.Color[1] = kClear[1];
    cv.Color[2] = kClear[2];
    cv.Color[3] = kClear[3];
    ComPtr<ID3D12Resource> rt;
    ASSERT_TRUE(SUCCEEDED(dev->CreateCommittedResource(&heapDefault, D3D12_HEAP_FLAG_NONE, &rd,
                                                       D3D12_RESOURCE_STATE_RENDER_TARGET, &cv,
                                                       IID_PPV_ARGS(&rt))));

    D3D12_DESCRIPTOR_HEAP_DESC rhd{};
    rhd.NumDescriptors = 1;
    rhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ASSERT_TRUE(SUCCEEDED(dev->CreateDescriptorHeap(&rhd, IID_PPV_ARGS(&rtvHeap))));
    const D3D12_CPU_DESCRIPTOR_HANDLE rtvH = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    dev->CreateRenderTargetView(rt.Get(), nullptr, rtvH);

    // Readback buffer. Texture→buffer copies require rows aligned to 256 bytes.
    const UINT rowPitch = (kW * 4 + (D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)) &
                          ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
    const UINT64 rbSize = static_cast<UINT64>(rowPitch) * kH;
    D3D12_HEAP_PROPERTIES heapReadback{};
    heapReadback.Type = D3D12_HEAP_TYPE_READBACK;
    D3D12_RESOURCE_DESC bd{};
    bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bd.Width = rbSize;
    bd.Height = 1;
    bd.DepthOrArraySize = 1;
    bd.MipLevels = 1;
    bd.Format = DXGI_FORMAT_UNKNOWN;
    bd.SampleDesc.Count = 1;
    bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ComPtr<ID3D12Resource> readback;
    ASSERT_TRUE(SUCCEEDED(dev->CreateCommittedResource(&heapReadback, D3D12_HEAP_FLAG_NONE, &bd,
                                                       D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                       IID_PPV_ARGS(&readback))));

    // Record: clear → transition RT to COPY_SOURCE → copy the texture into the buffer.
    cmd->ClearRenderTargetView(rtvH, kClear, 0, nullptr);

    D3D12_RESOURCE_BARRIER toCopy{};
    toCopy.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    toCopy.Transition.pResource = rt.Get();
    toCopy.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    toCopy.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    toCopy.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
    cmd->ResourceBarrier(1, &toCopy);

    D3D12_TEXTURE_COPY_LOCATION dst{};
    dst.pResource = readback.Get();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dst.PlacedFootprint.Offset = 0;
    dst.PlacedFootprint.Footprint.Format = rd.Format;
    dst.PlacedFootprint.Footprint.Width = kW;
    dst.PlacedFootprint.Footprint.Height = kH;
    dst.PlacedFootprint.Footprint.Depth = 1;
    dst.PlacedFootprint.Footprint.RowPitch = rowPitch;
    D3D12_TEXTURE_COPY_LOCATION src{};
    src.pResource = rt.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex = 0;
    cmd->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

    ASSERT_TRUE(SUCCEEDED(cmd->Close()));
    ID3D12CommandList* lists[] = {cmd.Get()};
    queue->ExecuteCommandLists(1, lists);

    // Block until the GPU (WARP) finishes.
    ComPtr<ID3D12Fence> fence;
    ASSERT_TRUE(SUCCEEDED(dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))));
    HANDLE ev = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    ASSERT_NE(ev, nullptr);
    ASSERT_TRUE(SUCCEEDED(queue->Signal(fence.Get(), 1)));
    if (fence->GetCompletedValue() < 1) {
        ASSERT_TRUE(SUCCEEDED(fence->SetEventOnCompletion(1, ev)));
        WaitForSingleObject(ev, INFINITE);
    }
    CloseHandle(ev);

    void* mapped = nullptr;
    const D3D12_RANGE readRange{0, static_cast<SIZE_T>(rbSize)};
    ASSERT_TRUE(SUCCEEDED(readback->Map(0, &readRange, &mapped)));
    ExpectClearPixel(static_cast<const uint8_t*>(mapped), "DX12");
    const D3D12_RANGE noWrite{0, 0};
    readback->Unmap(0, &noWrite);
}
#endif // UNIGUI_HAS_DX12

