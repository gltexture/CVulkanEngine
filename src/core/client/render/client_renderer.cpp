#include "client_renderer.h"
#include <vulkan/vulkan.h>
#include "client_render_config.h"
#include "util/logger.h"
#include <unordered_set>

#include "GLFW/glfw3.h"
#include "vulkanPrograms/vulkan_utility.h"
#include "vulkanPrograms/vulkan_ext.h"

namespace cvulkan::client::renderer {
    std::unique_ptr<VulkanContext> vulkanContext;

    void VulkanContext::cleanUp() {
        if (this->vkDebugMessenger != VK_NULL_HANDLE) {
            const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->vkInstance,"vkDestroyDebugUtilsMessengerEXT"));
            if (func == nullptr) {
                logging::error("Failed to get vkDestroyDebugUtilsMessengerEXT");
            } else {
                func(this->vkInstance, this->vkDebugMessenger, nullptr);
            }
            logging::debug("Vulkan debug messenger destroyed");
        }
        if (this->vkInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(this->vkInstance, nullptr);
            this->vkInstance = VK_NULL_HANDLE;
            logging::info("Vulkan instance destroyed");
        }
    }

    void VulkanContext::tryIncludeInstanceLayer(const std::unordered_set<std::string> &available, const std::string &layer) {
        if (available.contains(layer)) {
            this->enabledVulkanInstanceLayers.insert(layer);
            logging::info("Enabled instance Layer {}", layer);
        } else {
            logging::error("Instance Layer {} is not available!", layer);
        }
    }

    void VulkanContext::tryIncludeInstanceExtension(const std::unordered_set<std::string> &available, const std::string &layer) {
        if (available.contains(layer)) {
            this->enabledVulkanInstanceExtensions.insert(layer);
            logging::info("Enabled instance extension {}", layer);
        } else {
            logging::error("Instance extension {} is not available!", layer);
        }
    }

    std::unordered_set<std::string> VulkanContext::availableInstanceExtensions() {
        std::unordered_set<std::string> set = {};
        uint32_t extensionsCount = 0;
        std::vector<VkExtensionProperties> extensionProperties = {};
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
        if (extensionsCount == 0) {
            logging::error("No vulkan instance extensions available");
            return set;
        }
        logging::info("Available {} instance vulkan extensions", extensionsCount);
        extensionProperties.resize(extensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensionProperties.data());
        for (uint32_t i = 0; i < extensionsCount; i++) {
            const VkExtensionProperties prop = extensionProperties[i];
            logging::debug("+ instance extension {}", prop.extensionName);
            set.emplace(prop.extensionName);
        }
        return set;
    }

    std::unordered_set<std::string> VulkanContext::availableInstanceLayers() {
        std::unordered_set<std::string> set = {};
        uint32_t layerCount = 0;
        std::vector<VkLayerProperties> layerProperties = {};
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        if (layerCount == 0) {
            logging::error("No vulkan instance layers available");
            return set;
        }
        logging::info("Available {} instance vulkan layers", layerCount);
        layerProperties.resize(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
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
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);
        if (extensionsCount == 0) {
            logging::error("No vulkan device extensions available");
            return set;
        }
        logging::info("Available {} device vulkan extensions", extensionsCount);
        extensionProperties.resize(extensionsCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensionProperties.data());
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
        vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr);
        if (layerCount == 0) {
            logging::error("No vulkan device layers available");
            return set;
        }
        logging::info("Available {} device vulkan layers", layerCount);
        layerProperties.resize(layerCount);
        vkEnumerateDeviceLayerProperties(device, &layerCount, layerProperties.data());
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
        const std::unordered_set<std::string> setOfExtensions = availableInstanceExtensions();
        const std::unordered_set<std::string> setOfLayers = availableInstanceLayers();
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
                USE_PORTABILITY_MODE = this->hasVkInstanceRequiredExtension(EXT_VK_KHR_portability_enumeration());
            }
        }

        {
            std::vector<const char*> enabledLayerNames{};
            std::vector<const char*> enabledExtNames{};

            for (const auto& layer : this->enabledVulkanInstanceLayers)
            {
                enabledLayerNames.emplace_back(layer.c_str());
            }
            for (const auto& layer : this->enabledVulkanInstanceExtensions)
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
            instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledVulkanInstanceExtensions.size());
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size());
            instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
            instanceCreateInfo.ppEnabledExtensionNames = enabledExtNames.data();
            VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info_ext = {};
            if (debugMode && this->hasVkInstanceRequiredExtension(EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME())) {
                vk_debug_utils_messenger_create_info_ext = vulkan::createDebugMessengerCreateInfo();
                instanceCreateInfo.pNext = &vk_debug_utils_messenger_create_info_ext;
            }
            utility::vkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &this->vkInstance));
            logging::info("Successfully created vulkan instance!");

            if (debugMode && this->hasVkInstanceRequiredExtension(EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME())) {
                const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->vkInstance,"vkCreateDebugUtilsMessengerEXT"));
                if (func == nullptr) {
                    throw std::runtime_error{"Failed to load vkCreateDebugUtilsMessengerEXT"};
                }
                utility::vkCheck(func(vulkanContext->vkInstance, &vk_debug_utils_messenger_create_info_ext, nullptr, &this->vkDebugMessenger));
                logging::debug("Created debug messenger");
            }
        }
    }

    void VulkanContext::initVulkanDevice(
        const std::initializer_list<std::string> requiredLayers,
        const std::initializer_list<std::string> requiredExtensions) {

        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(this->vkInstance, &physicalDeviceCount, nullptr);
        if (physicalDeviceCount > 0) {
            std::vector<VkPhysicalDevice> priorityVectorToChooseDevice = {};
            std::vector<VkPhysicalDevice> physicalDevices = {};
            physicalDevices.resize(physicalDeviceCount);
            vkEnumeratePhysicalDevices(this->vkInstance, &physicalDeviceCount, physicalDevices.data());
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

                    if (!success) {
                        continue;
                    }

                    uint32_t queueFamilyCount = 0;
                    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

                    if (queueFamilyCount == 0) {
                        logging::warn("Device {} doesn't support queue families, skipping", physicalDeviceProperties.deviceName);
                        success = false;
                    }

                    if (!success) {
                        continue;
                    }

                    if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                        priorityVectorToChooseDevice.insert(priorityVectorToChooseDevice.begin(), physicalDevice);
                    }
                }

                if (!priorityVectorToChooseDevice.empty()) {
                    this->vkPhysicalDevice = priorityVectorToChooseDevice[0];
                    VkPhysicalDeviceProperties physicalDeviceProperties = {};
                    vkGetPhysicalDeviceProperties(this->vkPhysicalDevice, &physicalDeviceProperties);
                    logging::info("Chosen physical device: {}, type: {}", physicalDeviceProperties.deviceName, static_cast<int>(physicalDeviceProperties.deviceType));
                } else {
                    throw std::runtime_error{"Failed to find physical device"};
                }
            }
        } else {
            throw std::runtime_error("Physical device not found");
        }
    }

    void init() {
        vulkanContext = std::make_unique<VulkanContext>();
        vulkanContext->initVulkanInstance(utility::debug_mode,{},{});
        vulkanContext->initVulkanDevice({}, {});
    }

    void render() {
    }

    void cleanUp() {
        vulkanContext.reset();
    }
}
