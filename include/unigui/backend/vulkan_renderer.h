#pragma once
#include <unigui/backend/renderer_backend.h>
#include <memory>

namespace unigui {

class PlatformBackend;

// Platform-agnostic Vulkan renderer backend.
//
// Self-contained: owns the VkInstance / VkDevice / surface / descriptor pool and
// drives ImGui's own ImGui_ImplVulkanH_Window helper for swap-chain, render pass,
// framebuffers, per-frame command buffers and synchronisation (same design as the
// upstream example_glfw_vulkan). The single platform-specific seam — the window
// surface and its required instance extensions — is delegated to the active
// PlatformBackend (GLFW on any OS, or SDL3), so this renderer is cross-platform
// and carries no Win32 / SDL3 dependency of its own.
class VulkanRenderer : public RendererBackend {
public:
    VulkanRenderer();
    ~VulkanRenderer() override;

    bool Init(ImGuiContext*) override;
    void Shutdown() override;
    void RenderDrawData(ImDrawData* dd) override;
    void SetClearColor(float r, float g, float b, float a) override;

    // Bring up instance/device/surface/swapchain and ImGui_ImplVulkan_Init.
    // Called by app.cc after the platform window exists. The surface and the
    // instance extensions it needs are obtained from @p platform.
    bool BringUp(PlatformBackend* platform, int w, int h);

    // Per-frame: rebuild the swap-chain if a resize was requested, then
    // ImGui_ImplVulkan_NewFrame(). Call once per frame before ImGui::NewFrame().
    void NewFrameVk();

    // Record a desired client size; the swap-chain is recreated lazily in NewFrameVk().
    void RequestResize(int w, int h);

private:
    struct Impl;
    std::unique_ptr<Impl> p_;
};

std::unique_ptr<RendererBackend> CreateVulkanRenderer();

} // namespace unigui
