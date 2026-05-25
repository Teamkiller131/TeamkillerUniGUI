#include <unigui/backend/backend_factory.h>
#include <unigui/backend/backend_types.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif
#include <memory>

namespace unigui {
namespace {

class GLFWPlatform : public PlatformBackend {
public:
    bool Init(void* native_window_handle = nullptr) override {
        if (!glfwInit()) return false;

        // Ensure ImGui context exists
        if (!ImGui::GetCurrentContext()) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
        }

        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window_ = glfwCreateWindow(1280, 720, "UniGUI", nullptr, nullptr);
        if (!window_) { glfwTerminate(); return false; }
        glfwShowWindow(window_);

        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1);
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        initialized_ = true;
        return true;
    }

    void SetTitle(const char* title) override {
        if (window_) glfwSetWindowTitle(window_, title);
    }
    void SetSize(int w, int h) override {
        if (window_) glfwSetWindowSize(window_, w, h);
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

    void* GetWindowHandle() const override {
#ifdef _WIN32
        return glfwGetWin32Window(window_);
#else
        return window_;
#endif
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
