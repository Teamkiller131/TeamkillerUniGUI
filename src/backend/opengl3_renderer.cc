#include <glad/glad.h>
#include <unigui/backend/backend_factory.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
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

        // Load OpenGL via GLAD
        if (!gladLoadGL()) {
            return false;
        }

        // GLSL 1.30 for maximum compatibility (macOS 4.1, Linux, Windows)
        if (!ImGui_ImplOpenGL3_Init("#version 130")) {
            return false;
        }

        initialized_ = true;
        return true;
    }

    void Shutdown() override {
        if (!initialized_) return;
        ImGui_ImplOpenGL3_Shutdown();
        initialized_ = false;
    }

    void RenderDrawData(ImDrawData* draw_data) override {
        if (draw_data) {
            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
        }
    }

    void SetClearColor(float r, float g, float b, float a) override {
        glClearColor(r, g, b, a);
    }

private:
    bool initialized_ = false;
};

} // anonymous namespace

std::unique_ptr<RendererBackend> CreateOpenGL3Renderer() {
    return std::make_unique<OpenGL3Renderer>();
}

} // namespace unigui
