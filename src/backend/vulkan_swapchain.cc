#include <unigui/backend/vulkan_types.h>
#include <algorithm>
#include <stdexcept>

namespace unigui {

static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
    for (auto& f : formats) if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return f;
    return formats[0];
}
static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes) {
    for (auto& m : modes) if (m == VK_PRESENT_MODE_MAILBOX_KHR) return m;
    return VK_PRESENT_MODE_FIFO_KHR;
}
static VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& caps, int w, int h) {
    if (caps.currentExtent.width != UINT32_MAX) return caps.currentExtent;
    return { (uint32_t)std::clamp((uint32_t)w, caps.minImageExtent.width, caps.maxImageExtent.width),
             (uint32_t)std::clamp((uint32_t)h, caps.minImageExtent.height, caps.maxImageExtent.height) };
}

VulkanSwapchain CreateSwapchain(VulkanDevice& vd, int width, int height) {
    auto support = QuerySwapchainSupport(vd.physicalDevice, vd.surface);
    auto format = ChooseSurfaceFormat(support.formats);
    auto presentMode = ChoosePresentMode(support.presentModes);
    auto extent = ChooseExtent(support.capabilities, width, height);
    uint32_t imageCount = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount) imageCount = support.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vd.surface; createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format; createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent; createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t indices[] = { vd.queueFamilies.graphics.value(), vd.queueFamilies.present.value() };
    if (vd.queueFamilies.graphics != vd.queueFamilies.present) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; createInfo.queueFamilyIndexCount = 2; createInfo.pQueueFamilyIndices = indices;
    } else { createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; }
    createInfo.preTransform = support.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode; createInfo.clipped = VK_TRUE;

    VulkanSwapchain sc;
    sc.imageFormat = format.format; sc.extent = extent;
    if (vkCreateSwapchainKHR(vd.device, &createInfo, nullptr, &sc.swapchain) != VK_SUCCESS) throw std::runtime_error("Swapchain creation failed");
    vkGetSwapchainImagesKHR(vd.device, sc.swapchain, &imageCount, nullptr);
    sc.images.resize(imageCount); sc.imageViews.resize(imageCount);
    vkGetSwapchainImagesKHR(vd.device, sc.swapchain, &imageCount, sc.images.data());

    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo vi{}; vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vi.image = sc.images[i]; vi.viewType = VK_IMAGE_VIEW_TYPE_2D; vi.format = sc.imageFormat;
        vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; vi.subresourceRange.levelCount = 1; vi.subresourceRange.layerCount = 1;
        if (vkCreateImageView(vd.device, &vi, nullptr, &sc.imageViews[i]) != VK_SUCCESS) throw std::runtime_error("ImageView creation failed");
    }

    VkAttachmentDescription ca{}; ca.format = sc.imageFormat; ca.samples = VK_SAMPLE_COUNT_1_BIT;
    ca.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; ca.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ca.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; ca.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference cr{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sp{}; sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; sp.colorAttachmentCount = 1; sp.pColorAttachments = &cr;
    VkSubpassDependency dep{}; dep.srcSubpass = VK_SUBPASS_EXTERNAL; dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo rp{}; rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp.attachmentCount = 1; rp.pAttachments = &ca; rp.subpassCount = 1; rp.pSubpasses = &sp; rp.dependencyCount = 1; rp.pDependencies = &dep;
    if (vkCreateRenderPass(vd.device, &rp, nullptr, &sc.renderPass) != VK_SUCCESS) throw std::runtime_error("RenderPass creation failed");

    sc.framebuffers.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkFramebufferCreateInfo fb{}; fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb.renderPass = sc.renderPass; fb.attachmentCount = 1; fb.pAttachments = &sc.imageViews[i];
        fb.width = sc.extent.width; fb.height = sc.extent.height; fb.layers = 1;
        if (vkCreateFramebuffer(vd.device, &fb, nullptr, &sc.framebuffers[i]) != VK_SUCCESS) throw std::runtime_error("Framebuffer creation failed");
    }
    return sc;
}

void DestroySwapchain(VulkanDevice& vd, VulkanSwapchain& sc) {
    for (auto fb : sc.framebuffers) vkDestroyFramebuffer(vd.device, fb, nullptr);
    if (sc.renderPass) vkDestroyRenderPass(vd.device, sc.renderPass, nullptr);
    for (auto iv : sc.imageViews) vkDestroyImageView(vd.device, iv, nullptr);
    if (sc.swapchain) vkDestroySwapchainKHR(vd.device, sc.swapchain, nullptr);
}

void RecreateSwapchain(VulkanDevice& vd, VulkanSwapchain& sc, int w, int h) {
    vkDeviceWaitIdle(vd.device); DestroySwapchain(vd, sc); sc = CreateSwapchain(vd, w, h);
}

} // namespace unigui
