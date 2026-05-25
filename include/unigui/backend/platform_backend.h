#pragma once

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
    /// Set the window title.
    virtual void SetTitle(const char*) {}
    /// Set the window client area size.
    virtual void SetSize(int, int) {}
    /// Swap front/back buffers (OpenGL/Vulkan present).
    virtual void SwapBuffers() {}
};

} // namespace unigui
