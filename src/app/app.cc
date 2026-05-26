#include <unigui/app/app.h>
#include <unigui/backend/backend_factory.h>
#include <unigui/theme/theme.h>
#include <unigui/theme/presets/registry.h>
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
#ifdef _WIN32
#include <windows.h>
#endif

namespace unigui {
static bool g_initialized=false;
static BackendType g_backend=BackendType::GLFW_GL3;
static std::unique_ptr<PlatformBackend> g_platform;
static std::unique_ptr<RendererBackend> g_renderer;

bool Init(const AppConfig& config){
    if(g_initialized)return false;
    InitLogging("debug");
    ImPlot::CreateContext();

    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    unigui::theme::RegisterAllThemes();

    // Load font BEFORE any backend init (DX11 locks atlas)
    float dpi=config.theme.dpi_scale;
    if(dpi<=0){
        dpi=DetectDPIScale(nullptr);
        if(dpi<0.5f)dpi=1.0f;
    }
    UNIGUI_LOG_INFO("DPI scale: {:.2f}",dpi);
    float fontSize=config.theme.font_size*dpi;
    LoadDefaultFont(fontSize,config.theme.font_path);

    auto be=CreateBackend(config.backend);
    g_backend=config.backend; g_platform=std::move(be.platform); g_renderer=std::move(be.renderer);

    if(!g_platform||!g_platform->Init(nullptr)){UNIGUI_LOG_ERROR("Platform init failed");return false;}
    g_platform->SetTitle(config.title); g_platform->SetSize(config.width,config.height);

#ifdef UNIGUI_HAS_DX11
    if(config.backend==BackendType::DX11){
        auto hwnd=g_platform->GetWindowHandle();
        RECT rc; GetClientRect((HWND)hwnd,&rc);
        int pw=rc.right-rc.left,ph=rc.bottom-rc.top;
        if(pw<=0){pw=config.width;ph=config.height;}
        ID3D11Device* dev=nullptr;ID3D11DeviceContext* ctx=nullptr;
        IDXGISwapChain* swap=nullptr;ID3D11RenderTargetView* rtv=nullptr;
        if(!CreateDX11DeviceAndSwapChain(hwnd,pw,ph,&dev,&ctx,&swap,&rtv)){g_platform->Shutdown();return false;}
        auto* dxr=static_cast<DX11Renderer*>(g_renderer.get());
        dxr->device_=dev;dxr->ctx_=ctx;dxr->swapchain_=swap;dxr->rtv_=rtv;
    }
#endif

    if(!g_renderer||!g_renderer->Init(ImGui::GetCurrentContext())){g_platform->Shutdown();return false;}

    // Build font atlas AFTER renderer Init (RendererHasTextures flag is set)
    ImGui::GetIO().Fonts->Build();

    // Re-detect DPI with actual window handle for per-monitor accuracy
    float actualDpi=DetectDPIScale(g_platform->GetWindowHandle());
    if(actualDpi>0.5f && actualDpi>dpi*1.1f){
        UNIGUI_LOG_INFO("DPI updated: {:.2f} -> {:.2f}", dpi, actualDpi);
        dpi=actualDpi;
        // For embedded font: scale via FontGlobalScale instead of reloading
        ImGui::GetIO().FontGlobalScale = dpi;
        // Re-apply theme sizing for new DPI
        ThemeConfig tc2=config.theme; tc2.dpi_scale=dpi;
        ApplyTheme(tc2);
    }

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
    if(g_renderer){g_renderer->Shutdown();g_renderer.reset();}if(g_platform){g_platform->Shutdown();g_platform.reset();}ImPlot::DestroyContext();Settings::Shutdown();g_initialized=false;}

bool NewFrame(){
    if(!g_initialized)return false;
    g_platform->PollEvents();
#ifdef UNIGUI_HAS_DX11
    if(g_backend==BackendType::DX11)ImGui_ImplDX11_NewFrame();
#endif
    g_platform->NewFrame();ImGui::NewFrame();
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
    if(g_backend==BackendType::GLFW_GL3){g_renderer->SetClearColor(0.10f,0.10f,0.12f,1.0f);glClear(GL_COLOR_BUFFER_BIT);}
    g_renderer->RenderDrawData(dd);
    if(g_backend==BackendType::GLFW_GL3)g_platform->SwapBuffers();
}

bool ShouldClose(){return g_platform?g_platform->ShouldClose():true;}
void* GetNativeWindowHandle(){return g_platform?g_platform->GetNativeWindowHandle():nullptr;}
void Run(const std::function<void()>& cb){while(!ShouldClose()){g_platform->PollEvents();NewFrame();cb();Render();}Shutdown();}
}
