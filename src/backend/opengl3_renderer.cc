#include <unigui/backend/backend_factory.h>
#include <unigui/core/log.h>

// clang-format off
#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>  // WebGL2 / GLES3 — emscripten provides GL; no loader needed
#include <GLFW/glfw3.h>
#else
#include <glad/glad.h> // glad must precede GLFW / any GL header
#include <GLFW/glfw3.h>
#endif
// clang-format on
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <memory>

namespace unigui {
namespace {

class OpenGL3Renderer : public RendererBackend {
public:
    bool Init(ImGuiContext* context = nullptr) override {
        if (initialized_)
            return true; // idempotent — a redundant Init is a no-op

        // Context ownership stays with the caller (app.cc / the tests create and
        // destroy it) — Shutdown must NOT destroy it, or it double-frees.
        if (!context) {
            if (!ImGui::GetCurrentContext()) {
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
            }
            context = ImGui::GetCurrentContext();
        }

#ifndef __EMSCRIPTEN__
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            UNIGUI_LOG_ERROR("gladLoadGLLoader() failed");
            return false;
        }
        // The platform requests a 3.3 CORE context on every OS. GLSL 1.30 is
        // compatibility-profile and macOS' strict core driver rejects it; 1.50 is
        // valid on every GL >= 3.2 core context (Linux/Mesa accepted 130 leniently,
        // which is why this slipped through).
        const char* glsl = "#version 150";
#else
        // Emscripten/WebGL2: GL is provided by the runtime (no glad loader). ImGui's
        // OpenGL3 backend compiles for GLES3, which requires a GLSL-ES shader header.
        const char* glsl = "#version 300 es";
#endif
        if (!ImGui_ImplOpenGL3_Init(glsl)) {
            UNIGUI_LOG_ERROR("ImGui_ImplOpenGL3_Init({}) failed", glsl);
            return false;
        }
        UNIGUI_LOG_DEBUG("ImGui_ImplOpenGL3_Init OK ({})", glsl);

        initialized_ = true;
        UNIGUI_LOG_INFO("OpenGL3 renderer initialized");
        return true;
    }

    void Shutdown() override {
        if (!initialized_)
            return;
        UNIGUI_LOG_DEBUG("OpenGL3 renderer shutdown");
        ImGui_ImplOpenGL3_Shutdown();
        initialized_ = false;
    }

    void NewFrame() override {
        // Lazily creates the GL device objects (shader program, vertex/index buffers,
        // font texture) on the first call. Skipping it leaves ImGui's draw calls with no
        // shader/buffer bound — the WebGL/GLES path then logs "no valid shader program in
        // use" / "bufferData: no buffer" and renders nothing.
        ImGui_ImplOpenGL3_NewFrame();
    }

    void RenderDrawData(ImDrawData* draw_data) override {
        if (draw_data)
            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
    }

    void SetClearColor(float r, float g, float b, float a) override { glClearColor(r, g, b, a); }

private:
    bool initialized_ = false;
};

} // anonymous namespace

std::unique_ptr<RendererBackend> CreateOpenGL3Renderer() {
    return std::make_unique<OpenGL3Renderer>();
}
} // namespace unigui
