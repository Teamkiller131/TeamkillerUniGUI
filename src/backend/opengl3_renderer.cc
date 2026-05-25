#include <glad/glad.h>
#include <unigui/backend/backend_factory.h>
#include <unigui/core/log.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <memory>

namespace unigui {
namespace {

class OpenGL3Renderer : public RendererBackend {
public:
    bool Init(ImGuiContext* context = nullptr) override {
        if (!context) {
            if (!ImGui::GetCurrentContext()) { IMGUI_CHECKVERSION(); ImGui::CreateContext(); }
            context = ImGui::GetCurrentContext();
        }

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            UNIGUI_LOG_ERROR("gladLoadGLLoader() failed");
            return false;
        }

        if (!ImGui_ImplOpenGL3_Init("#version 130")) {
            UNIGUI_LOG_ERROR("ImGui_ImplOpenGL3_Init(#version 130) failed");
            return false;
        }
        UNIGUI_LOG_DEBUG("ImGui_ImplOpenGL3_Init OK");

        initialized_ = true;
        UNIGUI_LOG_INFO("OpenGL3 renderer initialized");
        return true;
    }

    void Shutdown() override {
        if (!initialized_) return;
        UNIGUI_LOG_DEBUG("OpenGL3 renderer shutdown");
        ImGui_ImplOpenGL3_Shutdown();
        initialized_ = false;
    }

    void RenderDrawData(ImDrawData* draw_data) override {
        if (draw_data) ImGui_ImplOpenGL3_RenderDrawData(draw_data);
    }

    void SetClearColor(float r, float g, float b, float a) override { glClearColor(r, g, b, a); }

private:
    bool initialized_ = false;
};

} // anonymous namespace

std::unique_ptr<RendererBackend> CreateOpenGL3Renderer() { return std::make_unique<OpenGL3Renderer>(); }
} // namespace unigui
