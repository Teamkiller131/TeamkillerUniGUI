#include <unigui/backend/backend_factory.h>
#include <unigui/core/log.h>

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
        // Use the refcount-aware subsystem API (not SDL_Init/SDL_Quit) so an embedding
        // host that already uses SDL is not torn down; only quit video if WE started it.
        const bool videoAlready = SDL_WasInit(SDL_INIT_VIDEO) != 0;
        if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
            UNIGUI_LOG_ERROR("SDL_InitSubSystem(VIDEO) failed: {}", SDL_GetError());
            return false;
        }
        owns_sdl_video_ = !videoAlready;
        if (!ImGui::GetCurrentContext()) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
        }
        if (native_window_handle) {
            window_ = static_cast<SDL_Window*>(native_window_handle);
            owns_window_ = false; // host owns it — never destroy it
        } else {
            window_ =
                SDL_CreateWindow("UniGUI", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
            if (!window_) {
                UNIGUI_LOG_ERROR("SDL_CreateWindow failed: {}", SDL_GetError());
                if (owns_sdl_video_)
                    SDL_QuitSubSystem(SDL_INIT_VIDEO);
                owns_sdl_video_ = false;
                return false;
            }
            owns_window_ = true;
        }
        if (!ImGui_ImplSDL3_InitForVulkan(window_)) {
            UNIGUI_LOG_ERROR("ImGui_ImplSDL3_InitForVulkan failed");
            if (owns_window_)
                SDL_DestroyWindow(window_);
            window_ = nullptr;
            if (owns_sdl_video_)
                SDL_QuitSubSystem(SDL_INIT_VIDEO);
            owns_window_ = false;
            owns_sdl_video_ = false;
            return false; // lets app.cc fall back to GLFW/OpenGL3
        }
        UNIGUI_LOG_INFO("SDL3 platform initialized (window {})",
                        owns_window_ ? "created" : "host-supplied");
        initialized_ = true;
        return true;
    }

    void Shutdown() override {
        if (!initialized_)
            return;
        ImGui_ImplSDL3_Shutdown();
        if (window_ && owns_window_)
            SDL_DestroyWindow(window_); // never destroy a host-supplied window
        window_ = nullptr;
        if (owns_sdl_video_) {
            SDL_QuitSubSystem(SDL_INIT_VIDEO); // refcount-safe, NOT global SDL_Quit()
            owns_sdl_video_ = false;
        }
        owns_window_ = false;
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
    float GetContentScale() const override {
        if (!window_)
            return 1.0f;
        const float s = SDL_GetWindowDisplayScale(window_); // HiDPI content scale
        return s > 0.f ? s : 1.0f;
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
        if (!exts) // NULL on failure (no loader/display); `count` is only set on success
            return;
        out.reserve(out.size() + count);
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
    bool owns_window_ = false;    // true only when we SDL_CreateWindow'd it ourselves
    bool owns_sdl_video_ = false; // true only when we started the SDL video subsystem
};

} // anonymous namespace

std::unique_ptr<PlatformBackend> CreateSDL3Platform() {
    return std::make_unique<SDL3Platform>();
}

} // namespace unigui
