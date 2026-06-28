#include <unigui/backend/backend_factory.h>
#include <unigui/core/log.h>

// clang-format off
#include <glad/glad.h> // glad must precede GLFW / any GL header
#include <GLFW/glfw3.h>
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

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            UNIGUI_LOG_ERROR("gladLoadGLLoader() failed");
            return false;
        }

        // The platform requests a 3.3 CORE context on every OS. GLSL 1.30 is
        // compatibility-profile and macOS' strict core driver rejects it; 1.50 is
        // valid on every GL >= 3.2 core context (Linux/Mesa accepted 130 leniently,
        // which is why this slipped through).
        const char* glsl = "#version 150";
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
