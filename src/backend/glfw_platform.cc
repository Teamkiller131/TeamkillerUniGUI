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
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA // exposes glfwGetCocoaWindow (NSWindow*) for Metal
#include <GLFW/glfw3native.h>
#endif
#include <functional>
#include <memory>

namespace unigui {
namespace {

class GLFWPlatform : public PlatformBackend {
public:
    explicit GLFWPlatform(bool needGL)
            : needGL_(needGL) {}

    bool Init([[maybe_unused]] void* native_window_handle = nullptr) override {
        // Set the error callback BEFORE glfwInit so failures inside glfwInit itself
        // (no X11 DISPLAY on a headless box, NSGL pixel-format failure on macOS, a
        // Wayland mismatch) report their real cause instead of a bare "failed".
        glfwSetErrorCallback([](int code, const char* desc) {
            UNIGUI_LOG_ERROR("GLFW error {}: {}", code, desc ? desc : "(no description)");
        });
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
#ifdef __APPLE__
            // macOS only grants a 3.2+ CORE profile to a FORWARD-COMPAT context;
            // without this hint glfwCreateWindow returns null on every Mac, which
            // (since Metal is a stub) leaves macOS with no working backend at all.
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
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

        bool imguiOk = false;
        if (needGL_) {
            glfwMakeContextCurrent(window_);
            glfwSwapInterval(1);
            imguiOk = ImGui_ImplGlfw_InitForOpenGL(window_, true);
        } else {
            imguiOk = ImGui_ImplGlfw_InitForOther(window_, true);
        }
        if (!imguiOk) {
            UNIGUI_LOG_ERROR("ImGui_ImplGlfw_Init* failed");
            glfwDestroyWindow(window_);
            window_ = nullptr;
            glfwTerminate();
            return false;
        }
        UNIGUI_LOG_DEBUG("ImGui GLFW backend initialized");
        // Report the framebuffer scale to ImGui: the client area is sized in logical
        // pixels but the back buffer is physical (720×540 on a 150% monitor for a
        // 480×360 client), and the render backends scale their projection by
        // DisplayFramebufferScale. Without this the main viewport renders at the
        // wrong physical size and viewport/window coordinates disagree at any
        // non-1.0 DPI.
        // [WIN-DPI-FIX 2026-08-30] ⚠ This "client = logical DIPs" premise holds on
        // macOS/Wayland only. On Windows a DPI-aware process gets PHYSICAL client
        // pixels from GLFW (no GLFW_SCALE_TO_MONITOR → framebuffer == window), so
        // forcing DisplayFramebufferScale here double-counts the scale: projection =
        // physical DisplaySize × 1.5 squeezes the UI into the top-left 1/1.5 of the
        // window (2026-08-30 regression after the 4.9.1 merge). On Windows the
        // impl_glfw fb/window ratio IS the truth — keep it.
        lastScale_ = ReadContentScale();
#ifndef _WIN32
        ApplyScaleToIO(lastScale_);
#endif
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

    void NewFrame() override {
        ImGui_ImplGlfw_NewFrame();
        // Runtime content-scale changes (the window moved to a differently-scaled
        // monitor, OS zoom): poll every frame (cheap — one GLFW call) and fire the
        // callback once per change, before the frame's layout runs.
        if (window_) {
            const float s = ReadContentScale();
            if (s != lastScale_) {
                lastScale_ = s;
                ApplyScaleToIO(s);
                if (scaleCb_)
                    scaleCb_(s);
            }
            // ImGui_ImplGlfw_NewFrame OVERWRITES io.DisplayFramebufferScale with the
            // GLFW framebuffer/window ratio. For the GL backends that ratio is the
            // truth (GLFW owns the framebuffer). For the external-swapchain backends
            // (DX11/DX12/Vulkan) the framebuffer is OUR swapchain, sized at
            // client×content-scale — GLFW's ratio there is the meaningless
            // window/window = 1.0, and letting it stand makes the projection render
            // at the wrong physical size on a non-1.0 monitor. Re-assert the live
            // monitor scale every frame.
            // [WIN-DPI-FIX 2026-08-30] On Windows the premise is inverted: the client
            // rect is PHYSICAL (DPI-aware process), our swapchain is sized at client
            // size directly (see app.cc), so GLFW's fb/window ratio (1.0) is exactly
            // right and re-asserting the monitor scale double-counts it. Only the
            // logical-coordinate platforms (macOS/Wayland) need the re-assert.
#ifndef _WIN32
            if (!needGL_)
                ApplyScaleToIO(lastScale_);
#endif
        }
    }
    void PollEvents() override { glfwPollEvents(); }
    bool ShouldClose() const override { return window_ ? glfwWindowShouldClose(window_) : false; }

    void SetContentScaleCallback(std::function<void(float)> cb) override {
        scaleCb_ = std::move(cb);
    }

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
#elif defined(__APPLE__)
        return (void*) glfwGetCocoaWindow(window_); // NSWindow* — Metal attaches a CAMetalLayer
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

    float GetContentScale() const override { return ReadContentScale(); }

    std::vector<MonitorInfo> GetMonitors() const override {
        std::vector<MonitorInfo> out;
        if (!window_) // glfwGetMonitors needs an initialized library
            return out;
        int count = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        if (!monitors)
            return out;
        out.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            MonitorInfo m;
            glfwGetMonitorPos(monitors[i], &m.x, &m.y);
            const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
            if (mode) {
                m.width = mode->width;
                m.height = mode->height;
            }
            // Work area (GLFW 3.3+): fall back to the full rect when the query
            // fails or reports a zero area (GLFW issue #1761 on monitor changes).
            int wx = m.x, wy = m.y, ww = m.width, wh = m.height;
            glfwGetMonitorWorkarea(monitors[i], &wx, &wy, &ww, &wh);
            if (ww > 0 && wh > 0) {
                m.workX = wx;
                m.workY = wy;
                m.workWidth = ww;
                m.workHeight = wh;
            } else {
                m.workX = m.x;
                m.workY = m.y;
                m.workWidth = m.width;
                m.workHeight = m.height;
            }
            float sx = 1.0f, sy = 1.0f;
            glfwGetMonitorContentScale(monitors[i], &sx, &sy);
            m.dpiScale = sx > 0.f ? sx : 1.0f;
            out.push_back(m);
        }
        return out;
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

    // Multi-viewport: bracket ImGui's secondary-window pass so the main window's GL
    // context is current again afterwards. Only meaningful for the GL renderers
    // (needGL_); with DX11/Vulkan on GLFW there is no current-context notion and the
    // no-op base behaviour is correct.
    void* SaveRenderContext() override {
        return needGL_ ? static_cast<void*>(glfwGetCurrentContext()) : nullptr;
    }
    void RestoreRenderContext(void* ctx) override {
        if (needGL_ && ctx)
            glfwMakeContextCurrent(static_cast<GLFWwindow*>(ctx));
    }

#ifdef UNIGUI_HAS_VULKAN
    void GetVulkanInstanceExtensions(std::vector<const char*>& out) const override {
        uint32_t count = 0;
        const char** exts = glfwGetRequiredInstanceExtensions(&count);
        if (!exts) // null on error (no Vulkan loader / display); count is then unspecified
            return;
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
    float ReadContentScale() const {
        if (!window_)
            return 1.0f;
#ifdef _WIN32
        // GLFW caches the Win32 content scale and refreshes it only on
        // WM_DPICHANGED, which the OS delivers asynchronously after show — an
        // immediate glfwGetWindowContentScale can be a stale 1.0 on a 150%
        // monitor (observed on a 4×150% machine: the first frames rendered
        // with a 1.0 projection on a 1.5 monitor). Query the LIVE per-monitor
        // DPI of the window instead.
        if (HWND hwnd = glfwGetWin32Window(window_)) {
            using GetDpiForWindow_t = UINT(WINAPI*)(HWND);
            static GetDpiForWindow_t pGetDpiForWindow = [] {
                HMODULE user32 = GetModuleHandleA("user32.dll");
                return (GetDpiForWindow_t) (user32 ? GetProcAddress(user32, "GetDpiForWindow")
                                                   : nullptr);
            }();
            if (pGetDpiForWindow) {
                const UINT dpi = pGetDpiForWindow(hwnd);
                if (dpi > 0)
                    return dpi / 96.0f;
            }
        }
#endif
        float xs = 1.0f, ys = 1.0f;
        glfwGetWindowContentScale(window_, &xs, &ys);
        return xs > 0.f ? xs : 1.0f;
    }
    static void ApplyScaleToIO(float s) {
        // Tell ImGui the back buffer is `s`× the logical client size, so the render
        // backends scale their projection and the main viewport renders at the correct
        // physical resolution at any DPI.
        ImGui::GetIO().DisplayFramebufferScale = ImVec2(s, s);
    }

    GLFWwindow* window_ = nullptr;
    bool initialized_ = false;
    bool needGL_ = true;
    float lastScale_ = 1.0f;
    std::function<void(float)> scaleCb_;
};

} // anonymous namespace

std::unique_ptr<PlatformBackend> CreateGLFWPlatform(BackendType type) {
    return std::make_unique<GLFWPlatform>(type == BackendType::GLFW_GL3);
}
} // namespace unigui
