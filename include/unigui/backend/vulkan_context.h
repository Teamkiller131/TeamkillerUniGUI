#pragma once

#include <unigui/backend/vulkan_types.h>

struct SDL_Window;

namespace unigui {

/// Complete Vulkan backend context — owns all Vulkan resources.
/// Created once at init, destroyed at shutdown.
struct VulkanContext {
    VulkanDevice device;
    VulkanSwapchain swapchain;
    VulkanPipeline pipeline;
    std::vector<FrameResources> frames;
};

/// Initialize all Vulkan resources given an SDL_Window.
VulkanContext InitVulkanContext(SDL_Window* window, int width, int height);

/// Destroy all Vulkan resources in correct reverse order.
void DestroyVulkanContext(VulkanContext& ctx);

} // namespace unigui
