#pragma once

#include <vector>

struct ImDrawData;
struct ImGuiContext;

namespace unigui {

/// Abstract platform (window + input) backend.
/// Implementations handle GLFW, SDL, Win32, macOS, etc.
class PlatformBackend {
public:
    virtual ~PlatformBackend() = default;

    /// Initialize the platform backend.
    /// @param native_window_handle Optional native window handle.
    ///        When nullptr, the backend creates its own window.
    /// @return true on success
    virtual bool Init(void* native_window_handle = nullptr) = 0;

    /// Shut down and release platform resources.
    virtual void Shutdown() = 0;

    /// Prepare for a new frame (poll events, update input state).
    virtual void NewFrame() = 0;

    /// Poll platform events (mouse, keyboard, window).
    virtual void PollEvents() = 0;

    /// Returns true if the window should close.
    virtual bool ShouldClose() const = 0;
    /// Returns the native window handle, or nullptr if not available.
    virtual void* GetWindowHandle() const { return nullptr; }
    virtual void* GetNativeWindowHandle() const { return GetWindowHandle(); }
    /// Get the current client area size.
    virtual void GetClientSize(int* w, int* h) {
        if (w)
            *w = 0;
        if (h)
            *h = 0;
    }
    /// The window's HiDPI content-scale factor (1.0 = 96dpi, 2.0 = retina). Used to
    /// rasterize the font atlas at the right size on macOS/Linux, where the Win32
    /// DPI query does not apply. Default 1.0 for platforms that can't report it.
    virtual float GetContentScale() const { return 1.0f; }
    /// Set the window title.
    virtual void SetTitle(const char*) {}
    /// Set the window client area size.
    virtual void SetSize(int, int) {}
    /// Swap front/back buffers (OpenGL/Vulkan present).
    virtual void SwapBuffers() {}

    /// Save / restore the platform's *current rendering context* around Dear ImGui's
    /// multi-viewport pass (`UpdatePlatformWindows` + `RenderPlatformWindowsDefault`).
    ///
    /// Why this exists: `RenderPlatformWindowsDefault()` makes each secondary window's
    /// GL context current and does **not** put the original one back, so the next frame
    /// would draw into the wrong context. Every upstream GLFW+GL example brackets that
    /// call with `glfwGetCurrentContext()` / `glfwMakeContextCurrent()`. That bracket is
    /// GLFW-specific and platform-specific code belongs in `src/backend/`, so the app
    /// loop asks the platform for it instead of calling GLFW itself.
    ///
    /// Default = no-op: correct for renderers with no per-thread current context
    /// (DX11/DX12/Vulkan/Metal). Returns an opaque handle; `nullptr` when not applicable.
    virtual void* SaveRenderContext() { return nullptr; }
    virtual void RestoreRenderContext(void* /*ctx*/) {}

    /// (Vulkan) Append the VkInstance extension names this platform needs to create
    /// a window surface (e.g. VK_KHR_surface + the OS-specific surface extension).
    /// Default: none — the platform has no Vulkan surface support.
    virtual void GetVulkanInstanceExtensions(std::vector<const char*>& /*out*/) const {}

    /// (Vulkan) Create a window surface for this platform's window.
    /// @p instance is a VkInstance and @p out_surface points to a VkSurfaceKHR; both
    /// are passed as void* so the generic interface stays free of Vulkan headers.
    /// This is the single platform-specific seam of the Vulkan renderer — GLFW backs
    /// it with glfwCreateWindowSurface (all OSes), SDL3 with SDL_Vulkan_CreateSurface.
    /// @return false if unsupported or creation failed.
    virtual bool CreateVulkanSurface(void* /*instance*/, void* /*out_surface*/) { return false; }
};

} // namespace unigui
