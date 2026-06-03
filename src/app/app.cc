#include <unigui/app/app.h>
#include <unigui/backend/backend_factory.h>
#include <unigui/theme/theme.h>
#include <unigui/theme/presets/registry.h>
#include <unigui/core/main_thread.h>
#ifdef UNIGUI_HAS_WIDGETS
#include <unigui/widgets/toast.h>
#endif
#include <unigui/core/log.h>
#include <unigui/core/settings.h>
#ifdef UNIGUI_HAS_EVENTS
#include <unigui/events/eventbus.h>
#endif
#include <glad/glad.h>
#include <imgui.h>
#include <implot.h>
#include <cstdio>
#ifdef UNIGUI_HAS_DX11
#include <unigui/backend/dx11_renderer.h>
#include <imgui_impl_dx11.h>
#endif
#ifdef UNIGUI_HAS_DX12
#include <unigui/backend/dx12_renderer.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

namespace unigui {
static bool g_initialized=false;
static BackendType g_backend=BackendType::GLFW_GL3;
static std::unique_ptr<PlatformBackend> g_platform;
static std::unique_ptr<RendererBackend> g_renderer;

static void CleanupAppResources(bool destroy_imgui_context){
    if(g_renderer){g_renderer->Shutdown();g_renderer.reset();}
    if(g_platform){g_platform->Shutdown();g_platform.reset();}
    if(ImPlot::GetCurrentContext()) ImPlot::DestroyContext();
    if(destroy_imgui_context && ImGui::GetCurrentContext()) ImGui::DestroyContext();
}

bool Init(const AppConfig& config){
    if(g_initialized)return false;
    InitLogging("debug");
    ImPlot::CreateContext();

    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    unigui::theme::RegisterAllThemes();

    // DPI is only reliable after the platform layer creates the window and sets the
    // process DPI awareness (GLFW does this during Init). So defer the SINGLE font
    // atlas load until the window exists — loading the atlas more than once corrupts
    // the heap on the GL/DX backends, ruling out a later rebuild.
    float dpi=config.theme.dpi_scale; // 0 = auto-detect once the window is up

    auto be=CreateBackend(config.backend);
    g_backend=config.backend; g_platform=std::move(be.platform); g_renderer=std::move(be.renderer);

    if(!g_platform||!g_platform->Init(nullptr)){UNIGUI_LOG_ERROR("Platform init failed");CleanupAppResources(true);return false;}
    g_platform->SetTitle(config.title); g_platform->SetSize(config.width,config.height);

    // Now that the window exists and DPI awareness is active, resolve the true
    // per-monitor DPI and load the glyph atlas exactly once at that pixel size for
    // crisp HiDPI text. This runs before the renderer uploads the atlas (below).
    if(dpi<=0){
        dpi=DetectDPIScale(g_platform->GetWindowHandle());
        if(dpi<0.5f)dpi=1.0f;
    }
    UNIGUI_LOG_INFO("DPI scale: {:.2f}",dpi);
    LoadDefaultFont(config.theme.font_size*dpi,config.theme.font_path);

#ifdef UNIGUI_HAS_DX11
    if(config.backend==BackendType::DX11){
        auto hwnd=g_platform->GetWindowHandle();
        RECT rc; GetClientRect((HWND)hwnd,&rc);
        int pw=rc.right-rc.left,ph=rc.bottom-rc.top;
        if(pw<=0){pw=config.width;ph=config.height;}
        ID3D11Device* dev=nullptr;ID3D11DeviceContext* ctx=nullptr;
        IDXGISwapChain* swap=nullptr;ID3D11RenderTargetView* rtv=nullptr;
        if(!CreateDX11DeviceAndSwapChain(hwnd,pw,ph,&dev,&ctx,&swap,&rtv)){CleanupAppResources(true);return false;}
        auto* dxr=static_cast<DX11Renderer*>(g_renderer.get());
        dxr->device_=dev;dxr->ctx_=ctx;dxr->swapchain_=swap;dxr->rtv_=rtv;
    }
#endif

#ifdef UNIGUI_HAS_DX12
    if(config.backend==BackendType::DX12){
        auto hwnd=g_platform->GetWindowHandle();
        int pw=0,ph=0;
        g_platform->GetClientSize(&pw,&ph);
        if(pw<=0){pw=config.width;ph=config.height;}
        ID3D12Device* dev=nullptr;ID3D12CommandQueue* queue=nullptr;
        ID3D12GraphicsCommandList* cmdList=nullptr;IDXGISwapChain3* swap=nullptr;
        ID3D12DescriptorHeap* rtvHeap=nullptr;ID3D12DescriptorHeap* srvHeap=nullptr;
        if(!CreateDX12DeviceAndSwapChain(hwnd,pw,ph,&dev,&queue,&cmdList,&swap,&rtvHeap,&srvHeap)){CleanupAppResources(true);return false;}
        auto* dxr=static_cast<DX12Renderer*>(g_renderer.get());
        dxr->device_=dev;dxr->cmdQueue_=queue;dxr->cmdList_=cmdList;dxr->swapchain_=swap;dxr->rtvHeap_=rtvHeap;dxr->srvHeap_=srvHeap;
    }
#endif

    if(!g_renderer||!g_renderer->Init(ImGui::GetCurrentContext())){CleanupAppResources(true);return false;}

    // Build font atlas AFTER renderer Init (RendererHasTextures flag is set).
    // The atlas was already sized for the true per-monitor DPI above, so the
    // upload here is final — no post-upload rebuild (which is unsafe on these
    // backends) is needed for HiDPI.
    ImGui::GetIO().Fonts->Build();

    auto& io=ImGui::GetIO();
    ThemeConfig tc=config.theme; tc.dpi_scale=dpi;
    ApplyTheme(tc);

    if(g_backend!=BackendType::DX11)io.ConfigFlags|=ImGuiConfigFlags_DockingEnable;
    io.DisplaySize=ImVec2((float)config.width,(float)config.height);

    g_initialized=true;
    UNIGUI_LOG_INFO("Init complete: backend={} {}x{} DPI={:.1f}",(int)g_backend,config.width,config.height,dpi);
#ifdef UNIGUI_HAS_EVENTS
    events::Bus::Instance().Publish("app.init", std::make_pair(config.width, config.height));
#endif
    return true;
}

void Shutdown(){if(!g_initialized)return;
#ifdef UNIGUI_HAS_EVENTS
events::Bus::Instance().Publish("app.shutdown",int{0});
events::Bus::Instance().Shutdown();
#endif
    CleanupAppResources(true);Settings::Shutdown();g_initialized=false;}

bool NewFrame(){
    if(!g_initialized)return false;
    g_platform->PollEvents();
    ApplyPendingFontRebuild();
#ifdef UNIGUI_HAS_DX11
    if(g_backend==BackendType::DX11)ImGui_ImplDX11_NewFrame();
#endif
    g_platform->NewFrame();ImGui::NewFrame();
    ProcessMainThreadTasks();
    // Check for window resize (DX11 needs swapchain resize)
#ifdef UNIGUI_HAS_DX11
    if (g_backend == BackendType::DX11) {
        int cw = 0, ch = 0;
        g_platform->GetClientSize(&cw, &ch);
        static int lastW = 0, lastH = 0;
        if (cw > 0 && ch > 0 && (cw != lastW || ch != lastH)) {
            lastW = cw; lastH = ch;
            auto* dxr = static_cast<DX11Renderer*>(g_renderer.get());
            if (dxr->ResizeSwapChain(cw, ch)) {
                ImGui::GetIO().DisplaySize = ImVec2((float)cw, (float)ch);
            }
        }
    }
#endif
    if(g_backend!=BackendType::DX11)ImGui::DockSpaceOverViewport(0,ImGui::GetMainViewport(),ImGuiDockNodeFlags_PassthruCentralNode);
    return true;
}

void Render(){
    if(!g_initialized)return;
#ifdef UNIGUI_HAS_WIDGETS
    unigui::Toast::Instance().Render();
#endif
    ImGui::Render();ImDrawData* dd=ImGui::GetDrawData();
    // Clear to the theme-derived backdrop so translucent (glass) surfaces read
    // against a tinted background. Applies to every backend; GLFW additionally
    // issues the GL clear here (other backends clear inside RenderDrawData).
    {ImVec4 bg=GetBackdropColor();g_renderer->SetClearColor(bg.x,bg.y,bg.z,bg.w);}
    if(g_backend==BackendType::GLFW_GL3){glClear(GL_COLOR_BUFFER_BIT);}
    g_renderer->RenderDrawData(dd);
    if(g_backend==BackendType::GLFW_GL3)g_platform->SwapBuffers();
}

bool ShouldClose(){return g_platform?g_platform->ShouldClose():true;}
void* GetNativeWindowHandle(){return g_platform?g_platform->GetNativeWindowHandle():nullptr;}
void Run(const std::function<void()>& cb,int maxFrames){
    int frame=0;
    // NewFrame() already polls platform events; do not poll again here.
    while(!ShouldClose()){
        if(!NewFrame())break;
        if(cb)cb();
        Render();
        if(maxFrames>0 && ++frame>=maxFrames)break;
    }
    Shutdown();
}
int RunApp(const AppConfig& config,const std::function<void()>& cb,int maxFrames){
    if(!Init(config)){UNIGUI_LOG_ERROR("App init failed; cannot start main loop");return 1;}
    Run(cb,maxFrames);
    return 0;
}
}
