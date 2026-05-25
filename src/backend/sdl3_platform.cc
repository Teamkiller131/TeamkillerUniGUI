#include <unigui/backend/backend_factory.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include <memory>

namespace unigui {
namespace {

class SDL3Platform : public PlatformBackend {
public:
    bool Init(void* native_window_handle = nullptr) override {
        if (!SDL_Init(SDL_INIT_VIDEO)) { return false; }
        if (!ImGui::GetCurrentContext()) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
        }
        if (native_window_handle) {
            window_ = static_cast<SDL_Window*>(native_window_handle);
        } else {
            window_ = SDL_CreateWindow("UniGUI", 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);
            if (!window_) { SDL_Quit(); return false; }
        }
        ImGui_ImplSDL3_InitForVulkan(window_);
        initialized_ = true;
        return true;
    }

    void Shutdown() override {
        if (!initialized_) return;
        ImGui_ImplSDL3_Shutdown();
        if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; }
        SDL_Quit();
        initialized_ = false;
    }

    void NewFrame() override { ImGui_ImplSDL3_NewFrame(); }

    void PollEvents() override {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) should_close_ = true;
        }
    }

    bool ShouldClose() const override { return should_close_; }
    void* GetWindowHandle() const override { return window_; }
    SDL_Window* GetWindow() const { return window_; }

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
