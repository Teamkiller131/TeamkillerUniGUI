#include <unigui/backend/backend_factory.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#ifdef UNIGUI_HAS_VULKAN
#include <vulkan/vulkan.h>
#endif
#include <memory>

namespace unigui {
namespace {

class SDL3Platform : public PlatformBackend {
public:
    bool Init(void* native_window_handle = nullptr) override {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            return false;
        }
        if (!ImGui::GetCurrentContext()) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
        }
        if (native_window_handle) {
            window_ = static_cast<SDL_Window*>(native_window_handle);
        } else {
            window_ =
                SDL_CreateWindow("UniGUI", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
            if (!window_) {
                SDL_Quit();
                return false;
            }
        }
        ImGui_ImplSDL3_InitForVulkan(window_);
        initialized_ = true;
        return true;
    }

    void Shutdown() override {
        if (!initialized_)
            return;
        ImGui_ImplSDL3_Shutdown();
        if (window_) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        SDL_Quit();
        initialized_ = false;
    }

    void NewFrame() override { ImGui_ImplSDL3_NewFrame(); }

    void PollEvents() override {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                should_close_ = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                event.window.windowID == SDL_GetWindowID(window_))
                should_close_ = true;
        }
    }

    bool ShouldClose() const override { return should_close_; }
    void* GetWindowHandle() const override { return window_; }
    SDL_Window* GetWindow() const { return window_; }

    void GetClientSize(int* w, int* h) override {
        int ww = 0, hh = 0;
        if (window_)
            SDL_GetWindowSizeInPixels(window_, &ww, &hh);
        if (w)
            *w = ww;
        if (h)
            *h = hh;
    }
    void SetTitle(const char* title) override {
        if (window_)
            SDL_SetWindowTitle(window_, title);
    }
    void SetSize(int w, int h) override {
        if (window_)
            SDL_SetWindowSize(window_, w, h);
    }

#ifdef UNIGUI_HAS_VULKAN
    void GetVulkanInstanceExtensions(std::vector<const char*>& out) const override {
        Uint32 count = 0;
        const char* const* exts = SDL_Vulkan_GetInstanceExtensions(&count);
        for (Uint32 i = 0; i < count; ++i)
            out.push_back(exts[i]);
    }

    bool CreateVulkanSurface(void* instance, void* out_surface) override {
        if (!window_)
            return false;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        if (!SDL_Vulkan_CreateSurface(window_, (VkInstance) instance, nullptr, &surface))
            return false;
        *reinterpret_cast<VkSurfaceKHR*>(out_surface) = surface;
        return true;
    }
#endif

private:
    SDL_Window* window_ = nullptr;
    bool initialized_ = false;
    bool should_close_ = false;
};

} // anonymous namespace

std::unique_ptr<PlatformBackend> CreateSDL3Platform() {
    return std::make_unique<SDL3Platform>();
}

} // namespace unigui
