// Metal renderer backend — macOS.
//
// STATUS: not yet functional. A correct implementation must:
//   1. Obtain the window's CAMetalLayer from the active platform backend
//      (e.g. glfwGetCocoaWindow() -> NSView, attach a CAMetalLayer), then
//   2. Acquire the next id<CAMetalDrawable> each frame, build a
//      MTLRenderPassDescriptor with that drawable's texture as the color
//      attachment, create a render command encoder, and pass it to
//      ImGui_ImplMetal_RenderDrawData(drawData, cmdBuf, encoder), and finally
//   3. presentDrawable + commit.
//
// The previous version hard-coded `id<CAMetalDrawable> drawable = nil;` and
// always skipped present, so it silently rendered nothing while reporting a
// successful Init(). Until the CAMetalLayer plumbing exists, Init() now fails
// honestly so the application falls back to the GLFW+OpenGL3 or Vulkan backend
// on macOS instead of showing a blank window.
#ifdef __APPLE__
#include <unigui/backend/renderer_backend.h>
#include <unigui/core/log.h>

#include <imgui.h>

#include <memory>

namespace unigui {

namespace {
class MetalRenderer : public RendererBackend {
public:
    bool Init(ImGuiContext* /*context*/) override {
        // A renderer Init() that returns false must have NO side effects. Context
        // lifetime belongs to the app/Init layer, not to a stub declaring failure —
        // so do not CreateContext() here.
        UNIGUI_LOG_ERROR("Metal backend is not yet implemented (no CAMetalLayer/drawable "
                         "integration). Use the GLFW+OpenGL3 or Vulkan backend on macOS.");
        return false;
    }
    void Shutdown() override {}
    void RenderDrawData(ImDrawData*) override {}
    void SetClearColor(float, float, float, float) override {}
};
} // anonymous namespace

std::unique_ptr<RendererBackend> CreateMetalRenderer() {
    return std::make_unique<MetalRenderer>();
}

} // namespace unigui

#else // !__APPLE__
// Non-macOS stub — compilation guard
#include <unigui/backend/backend_factory.h>

#include <cstdio>
namespace unigui {
std::unique_ptr<RendererBackend> CreateMetalRenderer() {
    std::fprintf(stderr, "[unigui] Metal backend is only available on macOS\n");
    return nullptr;
}
} // namespace unigui
#endif // __APPLE__
