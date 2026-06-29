#pragma once

#include <unigui/backend/renderer_backend.h>

#include <memory>

namespace unigui {

/// Metal renderer backend (macOS). Owns a `CAMetalLayer` attached to the GLFW window's
/// `NSView` and drives `imgui_impl_metal`. All Objective-C / Metal types are confined to
/// the Objective-C++ implementation behind a PIMPL, so this header is plain C++ and safe
/// to include from non-objc translation units (e.g. `app.cc`).
class MetalRenderer : public RendererBackend {
public:
    MetalRenderer();
    ~MetalRenderer() override;

    bool Init(ImGuiContext* context) override;
    void Shutdown() override;
    void RenderDrawData(ImDrawData* draw_data) override;
    void SetClearColor(float r, float g, float b, float a) override;

    /// Create the Metal device + command queue and attach a `CAMetalLayer` to
    /// `nsWindow` (an `NSWindow*`, type-erased to `void*` — from
    /// `PlatformBackend::GetNativeWindowHandle()`). Returns false if Metal is
    /// unavailable, so the app can fall back to GLFW+OpenGL3.
    bool BringUp(void* nsWindow, int width, int height);

    /// Per-frame: resize the drawable, acquire the next `CAMetalDrawable`, build the
    /// render pass + command buffer/encoder, and call `ImGui_ImplMetal_NewFrame`. Must
    /// run before `ImGui::NewFrame()`; `RenderDrawData()` then encodes + presents.
    void NewFrameMetal(int width, int height);

private:
    struct Impl;
    std::unique_ptr<Impl> p_;
};

} // namespace unigui
