#include <unigui/backend/vulkan_types.h>
#include <SDL3/SDL_vulkan.h>
#include <cstring>
#include <set>
#include <stdexcept>
#include <cstdio>

namespace unigui {

static const std::vector<const char*> kValidationLayers = { "VK_LAYER_KHRONOS_validation" };
static const std::vector<const char*> kDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
static constexpr bool kEnableValidation =
#ifdef _DEBUG
    true;
#else
    false;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pData, void*) {
    std::fprintf(stderr, "[vulkan] %s\n", pData->pMessage);
    return VK_FALSE;
}

static bool CheckValidationLayerSupport() {
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> available(count);
    vkEnumerateInstanceLayerProperties(&count, available.data());
    for (const char* name : kValidationLayers) {
        bool found = false;
        for (auto& layer : available) { if (std::strcmp(name, layer.layerName) == 0) { found = true; break; } }
        if (!found) return false;
    }
    return true;
}

static VkInstance CreateInstance() {
    if (kEnableValidation && !CheckValidationLayerSupport()) throw std::runtime_error("Validation layers not available");
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "UniGUI"; appInfo.applicationVersion = VK_MAKE_VERSION(0, 2, 0);
    appInfo.pEngineName = "UniGUI"; appInfo.engineVersion = VK_MAKE_VERSION(0, 2, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    uint32_t sdlExtCount = 0;
    const char* const* sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);
    std::vector<const char*> extensions(sdlExts, sdlExts + sdlExtCount);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = (uint32_t)extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    if (kEnableValidation) {
        createInfo.enabledLayerCount = (uint32_t)kValidationLayers.size();
        createInfo.ppEnabledLayerNames = kValidationLayers.data();
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugInfo.pfnUserCallback = DebugCallback;
        createInfo.pNext = &debugInfo;
    }

    VkInstance instance;
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan instance");
    return instance;
}

static VkDebugUtilsMessengerEXT CreateDebugMessenger(VkInstance instance) {
    if (!kEnableValidation) return VK_NULL_HANDLE;
    VkDebugUtilsMessengerCreateInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    info.pfnUserCallback = DebugCallback;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    VkDebugUtilsMessengerEXT messenger;
    if (func && func(instance, &info, nullptr, &messenger) == VK_SUCCESS) return messenger;
    return VK_NULL_HANDLE;
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());
    for (uint32_t i = 0; i < count; i++) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphics = i;
        VkBool32 present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present);
        if (present) indices.present = i;
        if (indices.IsComplete()) break;
    }
    return indices;
}

SwapchainSupport QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapchainSupport support;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &support.capabilities);
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
    if (count) { support.formats.resize(count); vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, support.formats.data()); }
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
    if (count) { support.presentModes.resize(count); vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, support.presentModes.data()); }
    return support;
}

static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    auto indices = FindQueueFamilies(device, surface);
    if (!indices.IsComplete()) return false;
    uint32_t extCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> available(extCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, available.data());
    std::set<std::string> required(kDeviceExtensions.begin(), kDeviceExtensions.end());
    for (auto& ext : available) required.erase(ext.extensionName);
    if (!required.empty()) return false;
    auto support = QuerySwapchainSupport(device, surface);
    return !support.formats.empty() && !support.presentModes.empty();
}

static VkPhysicalDevice PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (!count) throw std::runtime_error("No Vulkan-capable GPU found");
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());
    for (auto& dev : devices) if (IsDeviceSuitable(dev, surface)) return dev;
    throw std::runtime_error("No suitable GPU found");
}

static VkDevice CreateLogicalDevice(VkPhysicalDevice phys, VkSurfaceKHR surface, VkQueue& gfxQ, VkQueue& presQ) {
    auto indices = FindQueueFamilies(phys, surface);
    std::set<uint32_t> unique = { indices.graphics.value(), indices.present.value() };
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    float priority = 1.0f;
    for (auto idx : unique) {
        VkDeviceQueueCreateInfo qInfo{};
        qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qInfo.queueFamilyIndex = idx; qInfo.queueCount = 1; qInfo.pQueuePriorities = &priority;
        queueInfos.push_back(qInfo);
    }
    VkPhysicalDeviceFeatures features{};
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = (uint32_t)queueInfos.size();
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledExtensionCount = (uint32_t)kDeviceExtensions.size();
    createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();
    createInfo.pEnabledFeatures = &features;
    VkDevice device;
    if (vkCreateDevice(phys, &createInfo, nullptr, &device) != VK_SUCCESS) throw std::runtime_error("Failed to create logical device");
    vkGetDeviceQueue(device, indices.graphics.value(), 0, &gfxQ);
    vkGetDeviceQueue(device, indices.present.value(), 0, &presQ);
    return device;
}

VulkanDevice CreateVulkanDevice(void* windowHandle) {
    VulkanDevice vd;
    vd.instance = CreateInstance();
    vd.debugMessenger = CreateDebugMessenger(vd.instance);
    if (!SDL_Vulkan_CreateSurface((SDL_Window*)windowHandle, vd.instance, nullptr, &vd.surface))
        throw std::runtime_error("Failed to create Vulkan surface");
    vd.physicalDevice = PickPhysicalDevice(vd.instance, vd.surface);
    vd.queueFamilies = FindQueueFamilies(vd.physicalDevice, vd.surface);
    vd.device = CreateLogicalDevice(vd.physicalDevice, vd.surface, vd.graphicsQueue, vd.presentQueue);
    return vd;
}

void DestroyVulkanDevice(VulkanDevice& vd) {
    if (vd.device) vkDestroyDevice(vd.device, nullptr);
    if (vd.surface) vkDestroySurfaceKHR(vd.instance, vd.surface, nullptr);
    if (vd.debugMessenger) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vd.instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func) func(vd.instance, vd.debugMessenger, nullptr);
    }
    if (vd.instance) vkDestroyInstance(vd.instance, nullptr);
}

} // namespace unigui
