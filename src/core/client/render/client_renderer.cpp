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

    void VulkanContext::checkRequiredLayer(const std::unordered_set<std::string> &available, const std::string &layer) {
        if (available.contains(layer)) {
            this->enabledRequiredLayers.insert(layer);
            logging::info("Enabled Layer {}", layer);
        } else {
            logging::error("Layer {} is not available!", layer);
        }
    }

    void VulkanContext::checkRequiredExtension(const std::unordered_set<std::string> &available, const std::string &layer) {
        if (available.contains(layer)) {
            this->enabledRequiredExtensions.insert(layer);
            logging::info("Enabled extension {}", layer);
        } else {
            logging::error("Extension {} is not available!", layer);
        }
    }

    std::unordered_set<std::string> VulkanContext::availableExtensions() {
        std::unordered_set<std::string> set = {};
        uint32_t extensionsCount = 0;
        std::vector<VkExtensionProperties> extensionProperties = {};
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
        if (extensionsCount == 0) {
            logging::error("No vulkan extensions available");
            return set;
        }
        logging::info("Available {} vulkan extensions", extensionsCount);
        extensionProperties.resize(extensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensionProperties.data());
        for (uint32_t i = 0; i < extensionsCount; i++) {
            const VkExtensionProperties prop = extensionProperties[i];
            logging::debug("+ extension {}", prop.extensionName);
            set.emplace(prop.extensionName);
        }
        return set;
    }

    std::unordered_set<std::string> VulkanContext::availableLayers() {
        std::unordered_set<std::string> set = {};
        uint32_t layerCount = 0;
        std::vector<VkLayerProperties> layerProperties = {};
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        if (layerCount == 0) {
            logging::error("No vulkan layers available");
            return set;
        }
        logging::info("Available {} vulkan layers", layerCount);
        layerProperties.resize(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
        for (uint32_t i = 0; i < layerCount; i++) {
            const VkLayerProperties prop = layerProperties[i];
            logging::debug("+ Layer {}", prop.layerName);
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

    VkResult VulkanContext::initVulkan(const bool debugMode) {
        VkResult result = VK_SUCCESS;
        //result = vkCreateInstance(&instanceCreateInfo, nullptr, &this->vkInstance);
        //if (result == VK_SUCCESS) {
        //    uint32_t physicalDeviceCount = 0;
        //    vkEnumeratePhysicalDevices(this->vkInstance, &physicalDeviceCount, nullptr);
//
        //    if (physicalDeviceCount > 0) {
        //        this->m_physicalDevices.resize(physicalDeviceCount);
        //        vkEnumeratePhysicalDevices(this->vkInstance, &physicalDeviceCount, this->m_physicalDevices.data());
//
        //        {
        //            for (const auto& physicalDevice : this->m_physicalDevices) {
        //                VkPhysicalDeviceProperties physicalDeviceProperties = {};
        //                vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        //                logging::info("Physical device: {}, type: {}", physicalDeviceProperties.deviceName, static_cast<int>(physicalDeviceProperties.deviceType));
        //            }
        //        }
        //    } else {
        //        throw std::runtime_error("Physical device not found");
        //    }
        //}

        bool USE_PORTABILITY_MODE = false;
        const std::unordered_set<std::string> setOfExtensions = availableExtensions();
        const std::unordered_set<std::string> setOfLayers = availableLayers();
        {
            if (debugMode) {
                this->checkRequiredLayer(setOfLayers, LR_VK_LAYER_KHRONOS_validation());
                this->checkRequiredExtension(setOfExtensions, EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME());
                //this->checkRequiredExtension(setOfExtensions, EXT_VK_EXT_DEBUG_REPORT_EXTENSION_NAME());
            }

            for (const auto glfwExtensions = getGLFWExtensions(); const auto& extension : glfwExtensions)
            {
                this->checkRequiredExtension(setOfExtensions, extension);
            }

            if constexpr (utility::checkOS(utility::MAC)) {
                this->checkRequiredExtension(setOfExtensions, EXT_VK_KHR_portability_enumeration());
                USE_PORTABILITY_MODE = this->hasRequiredExtension(EXT_VK_KHR_portability_enumeration());
            }
        }

        {
            std::vector<const char*> enabledLayerNames{};
            std::vector<const char*> enabledExtNames{};

            for (const auto& layer : this->enabledRequiredLayers)
            {
                enabledLayerNames.emplace_back(layer.c_str());
            }
            for (const auto& layer : this->enabledRequiredExtensions)
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
            instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledRequiredExtensions.size());
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size());
            instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
            instanceCreateInfo.ppEnabledExtensionNames = enabledExtNames.data();
            VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info_ext = {};
            if (debugMode && this->hasRequiredExtension(EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME())) {
                vk_debug_utils_messenger_create_info_ext = vulkan::createDebugMessengerCreateInfo();
                instanceCreateInfo.pNext = &vk_debug_utils_messenger_create_info_ext;
            }
            utility::vkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &this->vkInstance));
            logging::info("Successfully created vulkan instance!");

            if (debugMode && this->hasRequiredExtension(EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME())) {
                const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->vkInstance,"vkCreateDebugUtilsMessengerEXT"));
                if (func == nullptr) {
                    throw std::runtime_error{"Failed to load vkCreateDebugUtilsMessengerEXT"};
                }
                utility::vkCheck(func(vulkanContext->vkInstance, &vk_debug_utils_messenger_create_info_ext, nullptr, &this->vkDebugMessenger));
                logging::debug("Created debug messenger");
            }
        }
        return result;
    }

    void init() {
        vulkanContext = std::make_unique<VulkanContext>();
        vulkanContext->initVulkan(utility::debug_mode);
    }

    void render() {
    }

    void cleanUp() {
        vulkanContext.reset();
    }
}
