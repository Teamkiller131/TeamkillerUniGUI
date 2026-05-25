#include <unigui/backend/vulkan_types.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <stdexcept>

namespace unigui {

VulkanPipeline CreatePipeline(VulkanDevice& vd, VulkanSwapchain& sc) {
    VulkanPipeline pl;
    VkDescriptorPoolSize ps{}; ps.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; ps.descriptorCount = 1;
    VkDescriptorPoolCreateInfo pi{}; pi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO; pi.maxSets = 1; pi.poolSizeCount = 1; pi.pPoolSizes = &ps;
    if (vkCreateDescriptorPool(vd.device, &pi, nullptr, &pl.descriptorPool) != VK_SUCCESS) throw std::runtime_error("Descriptor pool failed");

    ImGui_ImplVulkan_InitInfo init{};
    init.Instance = vd.instance; init.PhysicalDevice = vd.physicalDevice;
    init.Device = vd.device; init.QueueFamily = vd.queueFamilies.graphics.value(); init.Queue = vd.graphicsQueue;
    init.DescriptorPool = pl.descriptorPool;
    init.MinImageCount = (uint32_t)sc.images.size(); init.ImageCount = (uint32_t)sc.images.size();
    init.PipelineInfoMain.RenderPass = sc.renderPass;
    init.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init.PipelineInfoMain.Subpass = 0;
    if (!ImGui_ImplVulkan_Init(&init)) throw std::runtime_error("ImGui Vulkan init failed");

    return pl;
}

void DestroyPipeline(VulkanDevice& vd, VulkanPipeline& pl) {
    ImGui_ImplVulkan_Shutdown();
    if (pl.descriptorPool) vkDestroyDescriptorPool(vd.device, pl.descriptorPool, nullptr);
    pl.descriptorPool = VK_NULL_HANDLE;
}

} // namespace unigui