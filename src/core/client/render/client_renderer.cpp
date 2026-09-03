#include "client_renderer.h"
#include "util/logger.h"
#include <unordered_set>

#include "GLFW/glfw3.h"
#include "vulkanPrograms/vulkan_utility.h"
#include "vulkanPrograms/vulkan_ext.h"

namespace cvulkan::client::renderer {
    std::unique_ptr<VulkanContext> vulkanContext;

    void VulkanContext::cleanUp() {
        if (this->vkDebugMessenger != VK_NULL_HANDLE) {
            const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->vkInstanceData.vkInstance,"vkDestroyDebugUtilsMessengerEXT"));
            if (func == nullptr) {
                logging::error("Failed to get vkDestroyDebugUtilsMessengerEXT");
            } else {
                func(this->vkInstanceData.vkInstance, this->vkDebugMessenger, nullptr);
            }
            logging::debug("Vulkan debug messenger destroyed");
        }
        if (this->vkDeviceData.vkDevice != VK_NULL_HANDLE) {
            this->vkDeviceData.deviceWaitIdle();
            vkDestroyDevice(this->vkDeviceData.vkDevice, nullptr);
            this->vkDeviceData.vkDevice = VK_NULL_HANDLE;
            logging::info("Vulkan device destroyed");
        }
        if (this->vkInstanceData.vkInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(this->vkInstanceData.vkInstance, nullptr);
            this->vkInstanceData.vkInstance = VK_NULL_HANDLE;
            logging::info("Vulkan instance destroyed");
        }
    }

    std::unordered_set<std::string> VulkanContext::availableInstanceExtensions(const VkInstance &instance) {
        std::unordered_set<std::string> set = {};
        uint32_t extensionsCount = 0;
        std::vector<VkExtensionProperties> extensionProperties = {};
        utility::vkCheck(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr), "Failed to get instance ext");
        if (extensionsCount == 0) {
            logging::error("No vulkan instance extensions available");
            return set;
        }
        logging::info("Available {} instance vulkan extensions", extensionsCount);
        extensionProperties.resize(extensionsCount);
         utility::vkCheck(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensionProperties.data()), "Failed to get instance ext");
        for (uint32_t i = 0; i < extensionsCount; i++) {
            const VkExtensionProperties prop = extensionProperties[i];
            logging::debug("+ instance extension {}", prop.extensionName);
            set.emplace(prop.extensionName);
        }
        return set;
    }

    std::unordered_set<std::string> VulkanContext::availableInstanceLayers(const VkInstance &instance) {
        std::unordered_set<std::string> set = {};
        uint32_t layerCount = 0;
        std::vector<VkLayerProperties> layerProperties = {};
        utility::vkCheck(vkEnumerateInstanceLayerProperties(&layerCount, nullptr), "Failed to get instance layers");
        if (layerCount == 0) {
            logging::error("No vulkan instance layers available");
            return set;
        }
        logging::info("Available {} instance vulkan layers", layerCount);
        layerProperties.resize(layerCount);
        utility::vkCheck(vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data()), "Failed to get instance layers");
        for (uint32_t i = 0; i < layerCount; i++) {
            const VkLayerProperties prop = layerProperties[i];
            logging::debug("+ Instance Layer {}", prop.layerName);
            set.emplace(prop.layerName);
        }
        return set;
    }

    std::unordered_set<std::string> VulkanContext::availableDeviceExtensions(const VkPhysicalDevice& device) {
        std::unordered_set<std::string> set = {};
        uint32_t extensionsCount = 0;
        std::vector<VkExtensionProperties> extensionProperties = {};
        utility::vkCheck(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr), "Failed to get device ext");
        if (extensionsCount == 0) {
            logging::error("No vulkan device extensions available");
            return set;
        }
        logging::info("Available {} device vulkan extensions", extensionsCount);
        extensionProperties.resize(extensionsCount);
        utility::vkCheck(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensionProperties.data()), "Failed to get device ext");
        for (uint32_t i = 0; i < extensionsCount; i++) {
            const VkExtensionProperties prop = extensionProperties[i];
            logging::debug("+ device extension {}", prop.extensionName);
            set.emplace(prop.extensionName);
        }
        return set;
    }

    std::unordered_set<std::string> VulkanContext::availableDeviceLayers(const VkPhysicalDevice& device) {
        std::unordered_set<std::string> set = {};
        uint32_t layerCount = 0;
        std::vector<VkLayerProperties> layerProperties = {};
        utility::vkCheck(vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr), "Failed to get device layers");
        if (layerCount == 0) {
            logging::error("No vulkan device layers available");
            return set;
        }
        logging::info("Available {} device vulkan layers", layerCount);
        layerProperties.resize(layerCount);
        utility::vkCheck(vkEnumerateDeviceLayerProperties(device, &layerCount, layerProperties.data()), "Failed to get device layers");
        for (uint32_t i = 0; i < layerCount; i++) {
            const VkLayerProperties prop = layerProperties[i];
            logging::debug("+ Device Layer {}", prop.layerName);
            set.emplace(prop.layerName);
        }
        return set;
    }

    std::unordered_set<std::string> VulkanContext::getGLFWExtensions() {
        std::unordered_set<std::string> set = {};
        uint32_t countGlfwExtensions = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&countGlfwExtensions);
        if (extensions == nullptr) {
            logging::error("Failed to get GLFW Vulkan extensions");
            return {};
        }
        for (uint32_t i = 0; i < countGlfwExtensions; i++) {
            set.emplace(extensions[i]);
        }
        return set;
    }

    void VulkanContext::initVulkanInstance(const bool debugMode,
        const std::initializer_list<std::string> requiredLayers,
        const std::initializer_list<std::string> requiredExtensions) {

        bool USE_PORTABILITY_MODE = false;
        const std::unordered_set<std::string> setOfExtensions = availableInstanceExtensions(this->vkInstanceData.vkInstance);
        const std::unordered_set<std::string> setOfLayers = availableInstanceLayers(this->vkInstanceData.vkInstance);
        {
            for (const auto& t : requiredLayers) {
                this->tryIncludeInstanceLayer(setOfLayers, t);
            }
            for (const auto& t : requiredExtensions) {
                this->tryIncludeInstanceExtension(setOfExtensions, t);
            }

            if (debugMode) {
                this->tryIncludeInstanceLayer(setOfLayers, LR_VK_LAYER_KHRONOS_validation());
                this->tryIncludeInstanceExtension(setOfExtensions, EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME());
            }

            for (const auto glfwExtensions = getGLFWExtensions(); const auto& extension : glfwExtensions)
            {
                this->tryIncludeInstanceExtension(setOfExtensions, extension);
            }

            if constexpr (utility::checkOS(utility::MAC)) {
                this->tryIncludeInstanceExtension(setOfExtensions, EXT_VK_KHR_portability_enumeration());
                USE_PORTABILITY_MODE = this->vkInstanceData.vkInstanceLrExtData.hasVkInstanceRequiredExtension(EXT_VK_KHR_portability_enumeration());
            }
        }

        {
            std::vector<const char*> enabledLayerNames{};
            std::vector<const char*> enabledExtNames{};

            for (const auto& layer : this->vkInstanceData.vkInstanceLrExtData.enabledLayers)
            {
                enabledLayerNames.emplace_back(layer.c_str());
            }
            for (const auto& layer : this->vkInstanceData.vkInstanceLrExtData.enabledExtensions)
            {
                enabledExtNames.emplace_back(layer.c_str());
            }

            VkApplicationInfo appInfo = { };
            VkInstanceCreateInfo instanceCreateInfo = { };
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "CVulkan";
            appInfo.apiVersion = VK_API_VERSION_1_3;
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instanceCreateInfo.pApplicationInfo = &appInfo;
            if (USE_PORTABILITY_MODE) {
                instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            }
            instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(this->vkInstanceData.vkInstanceLrExtData.enabledExtensions.size());
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size());
            instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
            instanceCreateInfo.ppEnabledExtensionNames = enabledExtNames.data();
            VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info_ext = {};
            if (debugMode && this->vkInstanceData.vkInstanceLrExtData.hasVkInstanceRequiredExtension(EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME())) {
                vk_debug_utils_messenger_create_info_ext = vulkan::createDebugMessengerCreateInfo();
                instanceCreateInfo.pNext = &vk_debug_utils_messenger_create_info_ext;
            }
            utility::vkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &this->vkInstanceData.vkInstance));
            logging::info("Successfully created vulkan instance!");

            if (debugMode && this->vkInstanceData.vkInstanceLrExtData.hasVkInstanceRequiredExtension(EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME())) {
                const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->vkInstanceData.vkInstance,"vkCreateDebugUtilsMessengerEXT"));
                if (func == nullptr) {
                    throw std::runtime_error{"Failed to load vkCreateDebugUtilsMessengerEXT"};
                }
                utility::vkCheck(func(vulkanContext->vkInstanceData.vkInstance, &vk_debug_utils_messenger_create_info_ext, nullptr, &this->vkDebugMessenger));
                logging::debug("Created debug messenger");
            }
        }
    }

    void VulkanContext::initVulkanPhysicalDevice(
        const std::initializer_list<std::string> requiredLayers,
        const std::initializer_list<std::string> requiredExtensions) {

        uint32_t physicalDeviceCount = 0;
        utility::vkCheck(vkEnumeratePhysicalDevices(this->vkInstanceData.vkInstance, &physicalDeviceCount, nullptr), "Failed to get physical devices");
        if (physicalDeviceCount > 0) {
            std::vector<VkPhysicalDevice> priorityVectorToChooseDevice = {};
            std::vector<VkPhysicalDevice> physicalDevices = {};
            physicalDevices.resize(physicalDeviceCount);
            utility::vkCheck(vkEnumeratePhysicalDevices(this->vkInstanceData.vkInstance, &physicalDeviceCount, physicalDevices.data()), "Failed to get physical devices");
            {
                std::unordered_set<std::string> availableLayers = {};
                std::unordered_set<std::string> availableExtensions = {};

                for (const auto& physicalDevice : physicalDevices) {
                    bool success = true;
                    VkPhysicalDeviceProperties physicalDeviceProperties = {};
                    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

                    availableLayers = availableDeviceLayers(physicalDevice);
                    availableExtensions = availableDeviceExtensions(physicalDevice);

                    for (const auto& layer : requiredLayers) {
                        if (!availableLayers.contains(layer)) {
                            logging::warn("Device {} doesn't support layer {}, skipping", physicalDeviceProperties.deviceName, layer);
                            success = false;
                        }
                    }

                    for (const auto& extension : requiredExtensions) {
                        if (!availableExtensions.contains(extension)) {
                            logging::warn("Device {} doesn't support extension {}, skipping", physicalDeviceProperties.deviceName, extension);
                            success = false;
                        }
                    }

                    if (!success) {
                        continue;
                    }

                    {
                        uint32_t queueFamilyCount = 0;
                        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

                        if (queueFamilyCount == 0) {
                            logging::warn("Device {} doesn't support queue families, skipping", physicalDeviceProperties.deviceName);
                            continue;
                        }

                        success = false;
                        std::vector<VkQueueFamilyProperties> family_properties(queueFamilyCount);
                        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, family_properties.data());
                        for (const auto& t : family_properties) {
                            if ((t.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                                success = true;
                            }
                        }
                    }

                    if (!success) {
                        continue;
                    }

                    if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                        priorityVectorToChooseDevice.insert(priorityVectorToChooseDevice.begin(), physicalDevice);
                    }
                }

                if (!priorityVectorToChooseDevice.empty()) {
                    this->vkPhysicalDeviceData.vkPhysicalDevice = priorityVectorToChooseDevice[0];
                    {
                        uint32_t queueFamilyCount = 0;
                        vkGetPhysicalDeviceQueueFamilyProperties(this->vkPhysicalDeviceData.vkPhysicalDevice, &queueFamilyCount, nullptr);
                        this->vkPhysicalDeviceData.vkQueueFamilyProps.resize(queueFamilyCount);
                        vkGetPhysicalDeviceQueueFamilyProperties(this->vkPhysicalDeviceData.vkPhysicalDevice, &queueFamilyCount, this->vkPhysicalDeviceData.vkQueueFamilyProps.data());

                        vkGetPhysicalDeviceProperties(this->vkPhysicalDeviceData.vkPhysicalDevice, &this->vkPhysicalDeviceData.vkPhysicalDeviceProperties);
                        vkGetPhysicalDeviceProperties2(this->vkPhysicalDeviceData.vkPhysicalDevice, &this->vkPhysicalDeviceData.vkPhysicalDeviceProperties2);

                        uint32_t count = 0;
                        utility::vkCheck(vkEnumerateDeviceExtensionProperties(this->vkPhysicalDeviceData.vkPhysicalDevice, nullptr, &count, nullptr), "Failed to get device ext");
                        this->vkPhysicalDeviceData.vkDeviceExtensions.resize(count);
                        utility::vkCheck(vkEnumerateDeviceExtensionProperties(this->vkPhysicalDeviceData.vkPhysicalDevice, nullptr, &count, this->vkPhysicalDeviceData.vkDeviceExtensions.data()), "Failed to get device ext");

                        vkGetPhysicalDeviceMemoryProperties(this->vkPhysicalDeviceData.vkPhysicalDevice, &this->vkPhysicalDeviceData.vkMemoryProperties);
                        vkGetPhysicalDeviceFeatures(this->vkPhysicalDeviceData.vkPhysicalDevice, &this->vkPhysicalDeviceData.vkPhysicalDeviceFeatures);

                        vkGetPhysicalDeviceQueueFamilyProperties(this->vkPhysicalDeviceData.vkPhysicalDevice, &count, nullptr);
                        this->vkPhysicalDeviceData.vkDeviceExtensions.resize(count);
                        vkGetPhysicalDeviceQueueFamilyProperties(this->vkPhysicalDeviceData.vkPhysicalDevice, &count, this->vkPhysicalDeviceData.vkQueueFamilyProps.data());

                        logging::info("Chosen physical device: {}, type: {}", this->vkPhysicalDeviceData.vkPhysicalDeviceProperties.deviceName, static_cast<int>(this->vkPhysicalDeviceData.vkPhysicalDeviceProperties.deviceType));
                    }

                    for (const auto& t : requiredLayers) {
                        this->tryIncludeDeviceLayer(availableLayers, t);
                    }

                    for (const auto& t : requiredExtensions) {
                        this->tryIncludeDeviceExtension(availableExtensions, t);
                    }

                    if constexpr (utility::checkOS(utility::MAC)) {
                        this->tryIncludeDeviceExtension(availableExtensions, EXT_VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME());
                    }
                } else {
                    throw std::runtime_error{"Failed to find physical device"};
                }
            }
        } else {
            throw std::runtime_error("Physical device not found");
        }
    }

    void VulkanContext::initVulkanLogicalDevice(const VulkanPhysicalDevice_Data& data) {
        VkDeviceQueueCreateInfo queueCreateInfo = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};

        float priority = 1.0f;
        VkQueueFamilyProperties queueFamilyProperties = {};
        uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
        for (uint32_t i = 0; i < data.vkQueueFamilyProps.size(); ++i) {
            const auto& queueFamily = data.vkQueueFamilyProps[i];
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsQueueFamilyIndex = i;
                break;
            }
        }

        if (graphicsQueueFamilyIndex == UINT32_MAX) {
            throw std::runtime_error("Graphics queue family not found");
        }

        queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &priority;

        VkDeviceCreateInfo vkDeviceCreateInfo = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        vkDeviceCreateInfo.queueCreateInfoCount = 1;
        vkDeviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

        vkDeviceCreateInfo.pEnabledFeatures = nullptr;
        vkDeviceCreateInfo.enabledExtensionCount = data.vkDeviceLrExtData.enabledExtensions.size();
        vkDeviceCreateInfo.enabledLayerCount = data.vkDeviceLrExtData.enabledLayers.size();

        std::vector<const char*> enabledLayerNames{};
        std::vector<const char*> enabledExtNames{};

        for (const auto& layer : data.vkDeviceLrExtData.enabledLayers)
        {
            enabledLayerNames.emplace_back(layer.c_str());
        }
        for (const auto& layer : data.vkDeviceLrExtData.enabledExtensions)
        {
            enabledExtNames.emplace_back(layer.c_str());
        }

        vkDeviceCreateInfo.ppEnabledExtensionNames = enabledExtNames.data();
        vkDeviceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();

        utility::vkCheck(vkCreateDevice(data.vkPhysicalDevice, &vkDeviceCreateInfo, nullptr, &this->vkDeviceData.vkDevice));
        logging::info("Created logical device: {} queue families", queueCreateInfo.queueCount);
    }

    void init() {
        vulkanContext = std::make_unique<VulkanContext>();
        vulkanContext->initVulkanInstance(utility::debug_mode,{},{});
        vulkanContext->initVulkanPhysicalDevice({}, {EXT_VK_KHR_SWAPCHAIN_EXTENSION_NAME()});
        vulkanContext->initVulkanLogicalDevice(vulkanContext->vkPhysicalDeviceData);
    }

    void render() {
    }

    void cleanUp() {
        vulkanContext.reset();
    }
}
