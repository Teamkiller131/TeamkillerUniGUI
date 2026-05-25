#pragma once

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
