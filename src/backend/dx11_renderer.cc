#include <unigui/backend/backend_factory.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>
#include <cstdio>

namespace unigui {
namespace {

class DX11Renderer : public RendererBackend {
public:
    bool Init(ImGuiContext* context) override {
        if (!context && !ImGui::GetCurrentContext()) {
            IMGUI_CHECKVERSION(); ImGui::CreateContext();
        }
        // DX11 device requires an HWND — caller provides via factory
        initialized_ = (device_ != nullptr);
        return initialized_;
    }

    void Shutdown() override {
        if (!initialized_) return;
        ImGui_ImplDX11_Shutdown();
        if (rtv_) { rtv_->Release(); rtv_ = nullptr; }
        if (ctx_) { ctx_->Release(); ctx_ = nullptr; }
        if (device_) { device_->Release(); device_ = nullptr; }
        if (swapchain_) { swapchain_->Release(); swapchain_ = nullptr; }
        initialized_ = false;
    }

    void RenderDrawData(ImDrawData* dd) override {
        if (!initialized_ || !dd) return;
        float c[4] = {clearR_, clearG_, clearB_, clearA_};
        ctx_->ClearRenderTargetView(rtv_, c);
        ImGui_ImplDX11_RenderDrawData(dd);
        swapchain_->Present(1, 0);
    }

    void SetClearColor(float r, float g, float b, float a) override { clearR_=r; clearG_=g; clearB_=b; clearA_=a; }

    // Public for factory to set device handles
    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* ctx_ = nullptr;
    IDXGISwapChain* swapchain_ = nullptr;
    ID3D11RenderTargetView* rtv_ = nullptr;

private:
    bool initialized_ = false;
    float clearR_=0.10f, clearG_=0.10f, clearB_=0.12f, clearA_=1.00f;
};

} // anonymous namespace

std::unique_ptr<RendererBackend> CreateDX11Renderer() { return std::make_unique<DX11Renderer>(); }

} // namespace unigui
