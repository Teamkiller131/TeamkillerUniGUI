#pragma once

#include <unigui/backend/renderer_backend.h>

#include <memory>

namespace unigui {

/// WebGPU renderer backend. On Emscripten it renders to the HTML5 `#canvas` through the
/// browser's WebGPU implementation and drives `imgui_impl_wgpu`. All WebGPU (`WGPU*`)
/// handles are confined to the implementation behind a PIMPL, so this header is plain
/// C++ and safe to include from `app.cc`.
///
/// WebGPU device acquisition is asynchronous (requestAdapter → requestDevice), so
/// `BringUp()` only *starts* the request and returns immediately; the renderer is not
/// `Ready()` until the device callback fires (typically within the first frame or two).
/// `NewFrameWGPU()` / `RenderDrawData()` no-op until then, which fits the browser's
/// `emscripten_set_main_loop` model (early frames simply draw nothing).
class WebGPURenderer : public RendererBackend {
public:
    WebGPURenderer();
    ~WebGPURenderer() override;

    bool Init(ImGuiContext* context) override;
    void Shutdown() override;
    void RenderDrawData(ImDrawData* draw_data) override;
    void SetClearColor(float r, float g, float b, float a) override;

    /// Create the WebGPU instance + surface (from the `#canvas` element) and kick off the
    /// asynchronous adapter/device request. Returns false only if the instance/surface
    /// can't be created at all (so the app can fall back to another backend); a pending
    /// device request still returns true.
    bool BringUp(int width, int height);

    /// True once the async device is ready, the swap chain is configured, and
    /// `ImGui_ImplWGPU_Init` has run. Until then the per-frame calls are inert.
    bool Ready() const;

    /// Per-frame: (re)configure the swap chain on resize, acquire the current texture
    /// view, and call `ImGui_ImplWGPU_NewFrame`. Must run before `ImGui::NewFrame()`;
    /// `RenderDrawData()` then encodes the render pass + submits.
    void NewFrameWGPU(int width, int height);

private:
    struct Impl;
    std::unique_ptr<Impl> p_;
};

} // namespace unigui
