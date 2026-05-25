#include <unigui/backend/backend_factory.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <memory>

namespace unigui {
namespace {

class GLFWPlatform : public PlatformBackend {
public:
    bool Init(void* native_window_handle = nullptr) override {
        if (!glfwInit()) {
            return false;
        }

        // Ensure ImGui context exists
        if (!ImGui::GetCurrentContext()) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
        }

        glfwWindowHint(GLFW_VISIBLE, native_window_handle ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        if (native_window_handle) {
            window_ = static_cast<GLFWwindow*>(native_window_handle);
        } else {
            window_ = glfwCreateWindow(800, 600, "UniGUI", nullptr, nullptr);
            if (!window_) {
                glfwTerminate();
                return false;
            }
        }

        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1); // Enable vsync

        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        initialized_ = true;
        return true;
    }

    void Shutdown() override {
        if (!initialized_) return;
        ImGui_ImplGlfw_Shutdown();
        if (window_) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        glfwTerminate();
        initialized_ = false;
    }

    void NewFrame() override {
        ImGui_ImplGlfw_NewFrame();
    }

    void PollEvents() override {
        glfwPollEvents();
    }

    bool ShouldClose() const override {
        return window_ ? glfwWindowShouldClose(window_) : false;
    }

    GLFWwindow* GetWindow() const { return window_; }

private:
    GLFWwindow* window_ = nullptr;
    bool initialized_ = false;
};

} // anonymous namespace

std::unique_ptr<PlatformBackend> CreateGLFWPlatform() {
    return std::make_unique<GLFWPlatform>();
}

} // namespace unigui
