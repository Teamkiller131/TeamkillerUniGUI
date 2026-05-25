#include <unigui/backend/vulkan_context.h>
#include <SDL3/SDL.h>

namespace unigui {

VulkanContext InitVulkanContext(SDL_Window* window, int width, int height) {
    VulkanContext ctx;
    ctx.device = CreateVulkanDevice(window);
    ctx.swapchain = CreateSwapchain(ctx.device, width, height);
    ctx.pipeline = CreatePipeline(ctx.device, ctx.swapchain);
    ctx.frames = CreateFrameResources(ctx.device, ctx.swapchain);
    return ctx;
}

void DestroyVulkanContext(VulkanContext& ctx) {
    vkDeviceWaitIdle(ctx.device.device);
    DestroyFrameResources(ctx.device, ctx.frames, ctx.frames.size());
    DestroyPipeline(ctx.device, ctx.pipeline);
    DestroySwapchain(ctx.device, ctx.swapchain);
    DestroyVulkanDevice(ctx.device);
}

} // namespace unigui
