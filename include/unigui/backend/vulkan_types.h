#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

struct ImDrawData;

namespace unigui {

// ── Queue family indices ──────────────────────────────────────────────────
struct QueueFamilyIndices {
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;
    bool IsComplete() const { return graphics.has_value() && present.has_value(); }
};

// ── Swapchain support ─────────────────────────────────────────────────────
struct SwapchainSupport {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// ── Vulkan device resources ───────────────────────────────────────────────
struct VulkanDevice {
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    QueueFamilyIndices queueFamilies;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

// ── Swapchain resources ───────────────────────────────────────────────────
struct VulkanSwapchain {
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    VkExtent2D extent{};
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;
};

// ── Per-frame resources ───────────────────────────────────────────────────
struct FrameResources {
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    VkSemaphore renderFinished = VK_NULL_HANDLE;
    VkFence inFlight = VK_NULL_HANDLE;
};

// ── Pipeline resources ────────────────────────────────────────────────────
struct VulkanPipeline {
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
};

// ── Device creation ───────────────────────────────────────────────────────
VulkanDevice CreateVulkanDevice(void* windowHandle);
void DestroyVulkanDevice(VulkanDevice& vd);
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
SwapchainSupport QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

// ── Swapchain creation ────────────────────────────────────────────────────
VulkanSwapchain CreateSwapchain(VulkanDevice& vd, int width, int height);
void DestroySwapchain(VulkanDevice& vd, VulkanSwapchain& sc);
void RecreateSwapchain(VulkanDevice& vd, VulkanSwapchain& sc, int w, int h);

// ── Pipeline creation ─────────────────────────────────────────────────────
VulkanPipeline CreatePipeline(VulkanDevice& vd, VulkanSwapchain& sc);
void DestroyPipeline(VulkanDevice& vd, VulkanPipeline& pl);

// ── Frame resources ───────────────────────────────────────────────────────
std::vector<FrameResources> CreateFrameResources(VulkanDevice& vd, VulkanSwapchain& sc, int maxFramesInFlight = 2);
void DestroyFrameResources(VulkanDevice& vd, std::vector<FrameResources>& frames, size_t count);
void BeginFrame(VulkanDevice& vd, VulkanSwapchain& sc, VulkanPipeline& pl, FrameResources& fr, uint32_t& imageIndex, VkDescriptorSet fontDescriptor);
void EndFrame(VulkanDevice& vd, VulkanSwapchain& sc, FrameResources& fr, uint32_t imageIndex);
void RecordCommandBuffer(VkCommandBuffer cmd, VkRenderPass rp, VkExtent2D ext, VkFramebuffer fb, VkPipeline pipeline, VkPipelineLayout layout, VkDescriptorSet fontDesc, ImDrawData* drawData);

} // namespace unigui
