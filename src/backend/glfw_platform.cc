#include <unigui/backend/backend_factory.h>
#include <unigui/backend/backend_types.h>
#include <unigui/core/log.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#ifdef UNIGUI_HAS_VULKAN
#include <vulkan/vulkan.h> // must precede glfw3.h so its Vulkan surface helpers are declared
#endif
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
    explicit GLFWPlatform(bool needGL)
            : needGL_(needGL) {}

    bool Init([[maybe_unused]] void* native_window_handle = nullptr) override {
        if (!glfwInit()) {
            UNIGUI_LOG_ERROR("glfwInit() failed");
            return false;
        }
        UNIGUI_LOG_DEBUG("glfwInit OK");

        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        if (needGL_) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        } else {
            // DX11/DX12/Metal/WebGPU own the swapchain on the native HWND — do NOT
            // let GLFW create an OpenGL context (it would be unused and can confuse
            // some drivers when a D3D swapchain renders to the same window).
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        window_ = glfwCreateWindow(1280, 720, "UniGUI", nullptr, nullptr);
        if (!window_) {
            UNIGUI_LOG_ERROR("glfwCreateWindow(1280x720) failed");
            glfwTerminate();
            return false;
        }
        UNIGUI_LOG_INFO("GLFW window created: 1280x720 'UniGUI' ({})",
                        needGL_ ? "OpenGL context" : "GLFW_NO_API");
        glfwShowWindow(window_);

        if (needGL_) {
            glfwMakeContextCurrent(window_);
            glfwSwapInterval(1);
            ImGui_ImplGlfw_InitForOpenGL(window_, true);
        } else {
            ImGui_ImplGlfw_InitForOther(window_, true);
        }
        UNIGUI_LOG_DEBUG("ImGui GLFW backend initialized");
        initialized_ = true;
        return true;
    }

    void Shutdown() override {
        if (!initialized_)
            return;
        UNIGUI_LOG_DEBUG("GLFWPlatform shutdown: destroying window");
        ImGui_ImplGlfw_Shutdown();
        if (window_) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        glfwTerminate();
        initialized_ = false;
    }

    void NewFrame() override { ImGui_ImplGlfw_NewFrame(); }
    void PollEvents() override { glfwPollEvents(); }
    bool ShouldClose() const override { return window_ ? glfwWindowShouldClose(window_) : false; }

    void* GetWindowHandle() const override {
#ifdef _WIN32
        return glfwGetWin32Window(window_);
#else
        return window_;
#endif
    }

    void* GetNativeWindowHandle() const override {
#ifdef _WIN32
        return glfwGetWin32Window(window_);
#else
        return window_;
#endif
    }

    void GetClientSize(int* w, int* h) override {
        if (window_)
            glfwGetWindowSize(window_, w, h);
        else {
            if (w)
                *w = 0;
            if (h)
                *h = 0;
        }
    }

    void SetTitle(const char* title) override {
        if (window_)
            glfwSetWindowTitle(window_, title);
    }
    void SetSize(int w, int h) override {
        if (window_)
            glfwSetWindowSize(window_, w, h);
    }
    void SwapBuffers() override {
        if (needGL_ && window_)
            glfwSwapBuffers(window_);
    }

#ifdef UNIGUI_HAS_VULKAN
    void GetVulkanInstanceExtensions(std::vector<const char*>& out) const override {
        uint32_t count = 0;
        const char** exts = glfwGetRequiredInstanceExtensions(&count);
        for (uint32_t i = 0; i < count; ++i)
            out.push_back(exts[i]);
    }

    bool CreateVulkanSurface(void* instance, void* out_surface) override {
        if (!window_)
            return false;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        if (glfwCreateWindowSurface((VkInstance) instance, window_, nullptr, &surface) !=
            VK_SUCCESS)
            return false;
        *reinterpret_cast<VkSurfaceKHR*>(out_surface) = surface;
        return true;
    }
#endif

    GLFWwindow* GetWindow() const { return window_; }

private:
    GLFWwindow* window_ = nullptr;
    bool initialized_ = false;
    bool needGL_ = true;
};

} // anonymous namespace

std::unique_ptr<PlatformBackend> CreateGLFWPlatform(BackendType type) {
    return std::make_unique<GLFWPlatform>(type == BackendType::GLFW_GL3);
}
} // namespace unigui
