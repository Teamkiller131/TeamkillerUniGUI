#include <unigui/backend/backend_factory.h>
#include <unigui/backend/platform_backend.h>
#include <unigui/backend/vulkan_renderer.h>

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

#include <cstdio>
#include <cstring>
#include <vector>

namespace unigui {

namespace {
void CheckVk(VkResult err) {
    if (err == VK_SUCCESS)
        return;
    std::fprintf(stderr, "[unigui] Vulkan error: VkResult=%d\n", (int) err);
}
} // namespace

// ── Implementation state ──────────────────────────────────────────────────────
struct VulkanRenderer::Impl {
    PlatformBackend* platform = nullptr; // surface + instance-extension provider
    VkAllocationCallbacks* allocator = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    uint32_t queueFamily = (uint32_t) -1;
    VkQueue queue = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    ImGui_ImplVulkanH_Window window{};
    uint32_t minImageCount = 2;
    bool swapChainRebuild = false;
    int wantW = 0, wantH = 0;

    bool imguiInited = false;
    bool ready = false;

    float clearR = 0.10f, clearG = 0.10f, clearB = 0.12f, clearA = 1.00f;

    bool CreateInstance();
    bool CreateDeviceObjects();
    bool CreateDescriptorPool();
    void FrameRender(ImDrawData* dd);
    void FramePresent();
};

bool VulkanRenderer::Impl::CreateInstance() {
    VkApplicationInfo app{};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName = "UniGUI";
    app.pEngineName = "UniGUI";
    app.apiVersion = VK_API_VERSION_1_3;

    // The platform (GLFW / SDL3) reports exactly the surface extensions it needs
    // for the current OS (VK_KHR_surface + e.g. win32/xlib/wayland/metal surface).
    std::vector<const char*> exts;
    if (platform)
        platform->GetVulkanInstanceExtensions(exts);
    bool hasSurface = false;
    for (const char* e : exts)
        if (std::strcmp(e, VK_KHR_SURFACE_EXTENSION_NAME) == 0) {
            hasSurface = true;
            break;
        }
    if (!hasSurface)
        exts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &app;
    ci.enabledExtensionCount = (uint32_t) exts.size();
    ci.ppEnabledExtensionNames = exts.empty() ? nullptr : exts.data();

    VkResult err = vkCreateInstance(&ci, allocator, &instance);
    if (err != VK_SUCCESS) {
        std::fprintf(stderr, "[unigui] Vulkan: vkCreateInstance failed (%d)\n", (int) err);
        return false;
    }
    return true;
}

bool VulkanRenderer::Impl::CreateDeviceObjects() {
    physicalDevice = ImGui_ImplVulkanH_SelectPhysicalDevice(instance);
    if (physicalDevice == VK_NULL_HANDLE) {
        std::fprintf(stderr, "[unigui] Vulkan: no suitable physical device\n");
        return false;
    }
    queueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(physicalDevice);
    if (queueFamily == (uint32_t) -1) {
        std::fprintf(stderr, "[unigui] Vulkan: no graphics queue family\n");
        return false;
    }

    const char* deviceExts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const float prio = 1.0f;
    VkDeviceQueueCreateInfo q{};
    q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    q.queueFamilyIndex = queueFamily;
    q.queueCount = 1;
    q.pQueuePriorities = &prio;

    VkDeviceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount = 1;
    ci.pQueueCreateInfos = &q;
    ci.enabledExtensionCount = 1;
    ci.ppEnabledExtensionNames = deviceExts;

    VkResult err = vkCreateDevice(physicalDevice, &ci, allocator, &device);
    if (err != VK_SUCCESS) {
        std::fprintf(stderr, "[unigui] Vulkan: vkCreateDevice failed (%d)\n", (int) err);
        return false;
    }
    vkGetDeviceQueue(device, queueFamily, 0, &queue);
    return true;
}

bool VulkanRenderer::Impl::CreateDescriptorPool() {
    VkDescriptorPoolSize sizes[] = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 256},
    };
    VkDescriptorPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ci.maxSets = 256;
    ci.poolSizeCount = (uint32_t) (sizeof(sizes) / sizeof(sizes[0]));
    ci.pPoolSizes = sizes;
    VkResult err = vkCreateDescriptorPool(device, &ci, allocator, &descriptorPool);
    if (err != VK_SUCCESS) {
        std::fprintf(stderr, "[unigui] Vulkan: CreateDescriptorPool failed (%d)\n", (int) err);
        return false;
    }
    return true;
}

void VulkanRenderer::Impl::FrameRender(ImDrawData* dd) {
    ImGui_ImplVulkanH_Window* wd = &window;
    VkSemaphore imageAcquired = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore renderComplete = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;

    VkResult err = vkAcquireNextImageKHR(device, wd->Swapchain, UINT64_MAX, imageAcquired,
                                         VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        swapChainRebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    if (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR) {
        CheckVk(err);
        return;
    }

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    vkWaitForFences(device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &fd->Fence);
    vkResetCommandPool(device, fd->CommandPool, 0);

    VkCommandBufferBeginInfo bi{};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(fd->CommandBuffer, &bi);

    wd->ClearValue.color.float32[0] = clearR;
    wd->ClearValue.color.float32[1] = clearG;
    wd->ClearValue.color.float32[2] = clearB;
    wd->ClearValue.color.float32[3] = clearA;

    VkRenderPassBeginInfo rp{};
    rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp.renderPass = wd->RenderPass;
    rp.framebuffer = fd->Framebuffer;
    rp.renderArea.extent.width = (uint32_t) wd->Width;
    rp.renderArea.extent.height = (uint32_t) wd->Height;
    rp.clearValueCount = 1;
    rp.pClearValues = &wd->ClearValue;
    vkCmdBeginRenderPass(fd->CommandBuffer, &rp, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplVulkan_RenderDrawData(dd, fd->CommandBuffer);

    vkCmdEndRenderPass(fd->CommandBuffer);
    vkEndCommandBuffer(fd->CommandBuffer);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &imageAcquired;
    si.pWaitDstStageMask = &waitStage;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &fd->CommandBuffer;
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &renderComplete;
    err = vkQueueSubmit(queue, 1, &si, fd->Fence);
    CheckVk(err);
}

void VulkanRenderer::Impl::FramePresent() {
    if (swapChainRebuild)
        return;
    ImGui_ImplVulkanH_Window* wd = &window;
    VkSemaphore renderComplete = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR pi{};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &renderComplete;
    pi.swapchainCount = 1;
    pi.pSwapchains = &wd->Swapchain;
    pi.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(queue, &pi);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        swapChainRebuild = true;
        return;
    }
    if (err != VK_SUCCESS)
        CheckVk(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount;
}

// ── Public class ──────────────────────────────────────────────────────────────
VulkanRenderer::VulkanRenderer()
        : p_(std::make_unique<Impl>()) {}
VulkanRenderer::~VulkanRenderer() = default;

bool VulkanRenderer::BringUp(PlatformBackend* platform, int w, int h) {
    if (w <= 0)
        w = 1280;
    if (h <= 0)
        h = 720;

    p_->platform = platform;
    if (!platform) {
        std::fprintf(stderr, "[unigui] Vulkan: no platform backend\n");
        return false;
    }

    if (!p_->CreateInstance())
        return false;
    if (!p_->CreateDeviceObjects())
        return false;
    if (!p_->CreateDescriptorPool())
        return false;

    // Delegate surface creation to the platform (glfwCreateWindowSurface /
    // SDL_Vulkan_CreateSurface) — the only OS-specific step.
    if (!platform->CreateVulkanSurface(p_->instance, &p_->surface) ||
        p_->surface == VK_NULL_HANDLE) {
        std::fprintf(stderr, "[unigui] Vulkan: platform surface creation failed\n");
        return false;
    }

    // Check window-system-integration support for the chosen queue family.
    VkBool32 wsiSupport = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(p_->physicalDevice, p_->queueFamily, p_->surface,
                                         &wsiSupport);
    if (wsiSupport != VK_TRUE) {
        std::fprintf(stderr, "[unigui] Vulkan: queue family lacks present (WSI) support\n");
        return false;
    }

    ImGui_ImplVulkanH_Window* wd = &p_->window;
    wd->Surface = p_->surface;

    const VkFormat reqFormats[] = {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8_UNORM,
        VK_FORMAT_R8G8B8_UNORM,
    };
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
        p_->physicalDevice, wd->Surface, reqFormats,
        (int) (sizeof(reqFormats) / sizeof(reqFormats[0])), VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);

    const VkPresentModeKHR reqModes[] = {VK_PRESENT_MODE_FIFO_KHR};
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
        p_->physicalDevice, wd->Surface, reqModes, (int) (sizeof(reqModes) / sizeof(reqModes[0])));

    ImGui_ImplVulkanH_CreateOrResizeWindow(p_->instance, p_->physicalDevice, p_->device, wd,
                                           p_->queueFamily, p_->allocator, w, h, p_->minImageCount,
                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    ImGui_ImplVulkan_InitInfo info{};
    info.ApiVersion = VK_API_VERSION_1_3;
    info.Instance = p_->instance;
    info.PhysicalDevice = p_->physicalDevice;
    info.Device = p_->device;
    info.QueueFamily = p_->queueFamily;
    info.Queue = p_->queue;
    info.PipelineCache = VK_NULL_HANDLE;
    info.DescriptorPool = p_->descriptorPool;
    info.MinImageCount = p_->minImageCount;
    info.ImageCount = wd->ImageCount;
    info.PipelineInfoMain.RenderPass = wd->RenderPass;
    info.PipelineInfoMain.Subpass = 0;
    info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.Allocator = p_->allocator;
    info.CheckVkResultFn = CheckVk;

    if (!ImGui_ImplVulkan_Init(&info)) {
        std::fprintf(stderr, "[unigui] Vulkan: ImGui_ImplVulkan_Init failed\n");
        return false;
    }
    p_->imguiInited = true;
    p_->ready = true;
    std::fprintf(stderr,
                 "[unigui] Vulkan: device + swapchain ready (%dx%d, images=%u), "
                 "renderer=imgui_impl_vulkan\n",
                 w, h, wd->ImageCount);
    return true;
}

bool VulkanRenderer::Init(ImGuiContext*) {
    return p_->ready;
}

void VulkanRenderer::RequestResize(int w, int h) {
    if (w <= 0 || h <= 0)
        return;
    if (w == p_->window.Width && h == p_->window.Height)
        return;
    p_->wantW = w;
    p_->wantH = h;
    p_->swapChainRebuild = true;
}

void VulkanRenderer::NewFrameVk() {
    if (!p_->ready)
        return;
    if (p_->swapChainRebuild && p_->wantW > 0 && p_->wantH > 0) {
        vkDeviceWaitIdle(p_->device);
        ImGui_ImplVulkan_SetMinImageCount(p_->minImageCount);
        ImGui_ImplVulkanH_CreateOrResizeWindow(p_->instance, p_->physicalDevice, p_->device,
                                               &p_->window, p_->queueFamily, p_->allocator,
                                               p_->wantW, p_->wantH, p_->minImageCount,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        p_->window.FrameIndex = 0;
        p_->swapChainRebuild = false;
    }
    ImGui_ImplVulkan_NewFrame();
}

void VulkanRenderer::RenderDrawData(ImDrawData* dd) {
    if (!p_->ready || !dd)
        return;
    // Skip rendering to a zero-sized (minimised) swap-chain.
    if (dd->DisplaySize.x <= 0.0f || dd->DisplaySize.y <= 0.0f)
        return;
    if (p_->swapChainRebuild)
        return;
    p_->FrameRender(dd);
    p_->FramePresent();
}

void VulkanRenderer::SetClearColor(float r, float g, float b, float a) {
    p_->clearR = r;
    p_->clearG = g;
    p_->clearB = b;
    p_->clearA = a;
}

void VulkanRenderer::Shutdown() {
    if (!p_->ready)
        return;
    if (p_->device)
        vkDeviceWaitIdle(p_->device);
    if (p_->imguiInited) {
        ImGui_ImplVulkan_Shutdown();
        p_->imguiInited = false;
    }
    if (p_->device)
        ImGui_ImplVulkanH_DestroyWindow(p_->instance, p_->device, &p_->window, p_->allocator);
    if (p_->descriptorPool) {
        vkDestroyDescriptorPool(p_->device, p_->descriptorPool, p_->allocator);
        p_->descriptorPool = VK_NULL_HANDLE;
    }
    // ImGui_ImplVulkanH_DestroyWindow does not destroy the surface (caller owns it).
    if (p_->surface) {
        vkDestroySurfaceKHR(p_->instance, p_->surface, p_->allocator);
        p_->surface = VK_NULL_HANDLE;
    }
    if (p_->device) {
        vkDestroyDevice(p_->device, p_->allocator);
        p_->device = VK_NULL_HANDLE;
    }
    if (p_->instance) {
        vkDestroyInstance(p_->instance, p_->allocator);
        p_->instance = VK_NULL_HANDLE;
    }
    p_->ready = false;
}

std::unique_ptr<RendererBackend> CreateVulkanRenderer() {
    return std::make_unique<VulkanRenderer>();
}

} // namespace unigui
