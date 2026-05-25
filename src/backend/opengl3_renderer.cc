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
        // If no context provided, ensure one exists
        if (!context) {
            if (!ImGui::GetCurrentContext()) {
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
            }
            context = ImGui::GetCurrentContext();
        }

        // Load OpenGL via GLAD using GLFW's loader (required for AMD Core Profile)
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            UNIGUI_LOG_ERROR("gladLoadGLLoader() failed — no OpenGL context?");
            return false;
        }
        UNIGUI_LOG_INFO("OpenGL: {} {} — GLSL {}", (const char*)glGetString(GL_VENDOR),
            (const char*)glGetString(GL_RENDERER), (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

        UNIGUI_LOG_DEBUG("GLAD: glGenVertexArrays={} glBindVertexArray={}",
            (void*)(uintptr_t)glGenVertexArrays, (void*)(uintptr_t)glBindVertexArray);

        // Core Profile requires a VAO before any draw call.
        // AMD drivers are strict about this.
        glGenVertexArrays(1, &vao_);
        glBindVertexArray(vao_);

        if (!ImGui_ImplOpenGL3_Init("#version 330 core")) {
            UNIGUI_LOG_ERROR("ImGui_ImplOpenGL3_Init(#version 330 core) failed");
            return false;
        }
        UNIGUI_LOG_DEBUG("ImGui_ImplOpenGL3_Init OK");

        initialized_ = true;
        UNIGUI_LOG_INFO("OpenGL3 renderer initialized");
        return true;
    }

    void Shutdown() override {
        if (!initialized_) return;
        UNIGUI_LOG_DEBUG("OpenGL3 renderer shutdown: ImGui_ImplOpenGL3_Shutdown");
        ImGui_ImplOpenGL3_Shutdown();
        if (vao_) { glDeleteVertexArrays(1, &vao_); vao_ = 0; }
        initialized_ = false;
    }

    void RenderDrawData(ImDrawData* draw_data) override {
        if (draw_data) {
            // Re-bind VAO — AMD Core Profile requires it before each draw
            glBindVertexArray(vao_);
            UNIGUI_LOG_TRACE("RenderDrawData: {} lists, {} vtx, {} idx",
                draw_data->CmdListsCount, draw_data->TotalVtxCount, draw_data->TotalIdxCount);
            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
        }
    }

    void SetClearColor(float r, float g, float b, float a) override {
        glClearColor(r, g, b, a);
    }

private:
    bool initialized_ = false;
    GLuint vao_ = 0;
};

} // anonymous namespace

std::unique_ptr<RendererBackend> CreateOpenGL3Renderer() {
    return std::make_unique<OpenGL3Renderer>();
}

} // namespace unigui
