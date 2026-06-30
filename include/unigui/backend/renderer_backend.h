#pragma once

#include <functional>

struct ImDrawData;
struct ImGuiContext;

namespace unigui {

/// Abstract renderer backend.
/// Implementations handle OpenGL, Vulkan, DirectX, Metal, etc.
class RendererBackend {
public:
    virtual ~RendererBackend() = default;

    /// Initialize the renderer backend.
    /// @param context The ImGui context to use.
    /// @return true on success
    virtual bool Init(ImGuiContext* context) = 0;

    /// Per-frame renderer setup, called once before `ImGui::NewFrame()`. The default is
    /// a no-op; backends whose ImGui impl needs a per-frame call that lazily creates/
    /// validates its GPU device objects (notably `ImGui_ImplOpenGL3_NewFrame`, which
    /// builds the shader program + vertex/index buffers) override this. Without it the
    /// GL backend renders with "no valid shader program in use" / "bufferData: no
    /// buffer" and draws nothing. (DX/Vulkan/Metal drive their per-frame setup through
    /// their own typed calls in the app loop.)
    virtual void NewFrame() {}

    /// Run one whole frame (the app's NewFrame → user callback → Render) inside any
    /// per-frame scope the backend requires. The default just invokes `body()`; the Metal
    /// backend overrides it to bracket the frame in an `@autoreleasepool` so the
    /// autoreleased `CAMetalDrawable`/command-buffer/encoder are drained every frame. On a
    /// manual (non-NSApp-driven) loop nothing else drains them, so the layer's small
    /// drawable pool is exhausted within a few frames and `nextDrawable` starts blocking /
    /// returning nil — the app stalls. Keeping this on the renderer (rather than putting
    /// `@autoreleasepool` in app.cc) keeps app.cc plain C++.
    virtual void RunFrameInScope(const std::function<void()>& body) { body(); }

    /// Shut down and release renderer resources.
    virtual void Shutdown() = 0;

    /// Render the draw data produced by ImGui.
    /// @param draw_data The draw data from ImGui::GetDrawData().
    ///        May be nullptr (empty frame).
    virtual void RenderDrawData(ImDrawData* draw_data) = 0;

    /// Set the clear color for the frame buffer.
    virtual void SetClearColor(float r, float g, float b, float a) = 0;
};

} // namespace unigui
