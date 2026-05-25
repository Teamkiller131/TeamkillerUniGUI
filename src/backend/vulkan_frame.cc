#include <unigui/backend/vulkan_types.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <stdexcept>

namespace unigui {

std::vector<FrameResources> CreateFrameResources(VulkanDevice& vd, VulkanSwapchain&, int maxFramesInFlight) {
    std::vector<FrameResources> frames(maxFramesInFlight);
    for (auto& fr : frames) {
        VkCommandPoolCreateInfo pi{}; pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; pi.queueFamilyIndex = vd.queueFamilies.graphics.value();
        if (vkCreateCommandPool(vd.device, &pi, nullptr, &fr.commandPool) != VK_SUCCESS) throw std::runtime_error("CmdPool failed");

        VkCommandBufferAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; ai.commandPool = fr.commandPool; ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandBufferCount = 1;
        if (vkAllocateCommandBuffers(vd.device, &ai, &fr.commandBuffer) != VK_SUCCESS) throw std::runtime_error("CmdBuf alloc failed");

        VkSemaphoreCreateInfo si{}; si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fi{}; fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO; fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateSemaphore(vd.device, &si, nullptr, &fr.imageAvailable);
        vkCreateSemaphore(vd.device, &si, nullptr, &fr.renderFinished);
        vkCreateFence(vd.device, &fi, nullptr, &fr.inFlight);
    }
    return frames;
}

void DestroyFrameResources(VulkanDevice& vd, std::vector<FrameResources>& frames, size_t) {
    for (auto& fr : frames) {
        vkDestroySemaphore(vd.device, fr.imageAvailable, nullptr);
        vkDestroySemaphore(vd.device, fr.renderFinished, nullptr);
        vkDestroyFence(vd.device, fr.inFlight, nullptr);
        vkDestroyCommandPool(vd.device, fr.commandPool, nullptr);
    }
}

void RecordCommandBuffer(VkCommandBuffer cmd, VkRenderPass, VkExtent2D, VkFramebuffer, VkPipeline, VkPipelineLayout, VkDescriptorSet, ImDrawData* drawData) {
    if (drawData) ImGui_ImplVulkan_RenderDrawData(drawData, cmd);
}

} // namespace unigui
