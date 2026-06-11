// Emscripten platform backend — Web/HTML5 canvas
// Requires: emscripten >= 3.x, imgui_impl_glfw.h + imgui_impl_opengl3.h
// Build: emcmake cmake .. -DUNIGUI_BACKEND=EMSCRIPTEN
//
// The Emscripten port of GLFW provides glfw3.h APIs that map to WebGL/HTML5.
// ImGui's glfw3+opengl3 backends work transparently on Emscripten.

#include <unigui/backend/backend_factory.h>
#include <unigui/core/log.h>

#include <cstdio>
#include <memory>

#ifdef __EMSCRIPTEN__
#include <GLFW/glfw3.h>

#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace unigui {

#ifdef __EMSCRIPTEN__

namespace {
class EmscriptenPlatform : public PlatformBackend {
public:
    bool Init(void* handle) override {
        // GLFW window is already created by GLFW3 backend init path.
        // This platform wrapper provides Emscripten-specific canvas sizing and input loop.
        if (handle)
            window_ = static_cast<GLFWwindow*>(handle);
        initialized_ = true;
        UNIGUI_LOG_INFO("Emscripten: platform initialized");
        return true;
    }

    void Shutdown() override {
        initialized_ = false;
        UNIGUI_LOG_DEBUG("Emscripten: platform shutdown");
    }

    void NewFrame() override {
        if (!initialized_)
            return;
        // Canvas auto-sizing to match CSS viewport
        int w, h, fbW, fbH;
        emscripten_get_canvas_element_size("#canvas", &w, &h);
        emscripten_get_element_css_size("#canvas", &fbW, &fbH);
        if (w != fbW || h != fbH) {
            emscripten_set_canvas_element_size("#canvas", fbW, fbH);
        }
    }

    void PollEvents() override {
        // Emscripten uses its own event loop — pollEvents is a no-op
    }

    bool ShouldClose() const override {
        return false; // Web app never "closes" — runs until tab closed
    }

    void SetTitle(const char* title) override { emscripten_set_window_title(title); }

    void SetSize(int w, int h) override { emscripten_set_canvas_element_size("#canvas", w, h); }

    void SwapBuffers() override {
        // Emscripten automatically swaps via RAF callback
    }

    void* GetWindowHandle() const override { return window_; }
    void* GetNativeWindowHandle() const override { return window_; }

    void GetClientSize(int* w, int* h) override {
        emscripten_get_element_css_size("#canvas", w, h);
    }

private:
    GLFWwindow* window_ = nullptr;
    bool initialized_ = false;
};
} // anonymous namespace

std::unique_ptr<PlatformBackend> CreateEmscriptenPlatform() {
    return std::make_unique<EmscriptenPlatform>();
}

// ── Emscripten main loop helper ──────────────────────────────────────────────

void EmscriptenSetMainLoop(std::function<void()> callback, int fps = 0,
                           bool simulateInfiniteLoop = true) {
    emscripten_set_main_loop_arg(
        [](void* arg) {
            auto* cb = static_cast<std::function<void()>*>(arg);
            (*cb)();
        },
        &callback, fps, simulateInfiniteLoop);
}

#else // !__EMSCRIPTEN__

std::unique_ptr<PlatformBackend> CreateEmscriptenPlatform() {
    std::fprintf(stderr, "[unigui] Emscripten platform requires __EMSCRIPTEN__ compiler\n");
    return nullptr;
}

#endif // __EMSCRIPTEN__

} // namespace unigui
