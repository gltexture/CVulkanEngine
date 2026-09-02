#pragma once

#include <memory>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.h>
#include "vulkanPrograms/vulkan_utility.h"
#include "util/logger.inl"

namespace cvulkan::client::renderer {
    class VulkanContext {
    public:
        VulkanContext() = default;
        ~VulkanContext() {
            this->cleanUp();
        }

        VkDebugUtilsMessengerEXT vkDebugMessenger = {};
        VkInstance vkInstance = {};
        VkPhysicalDevice vkPhysicalDevice = {};
        std::unordered_set<std::string> enabledVulkanInstanceLayers = {};
        std::unordered_set<std::string> enabledVulkanInstanceExtensions = {};
        std::unordered_set<std::string> enabledVulkanDeviceLayers = {};
        std::unordered_set<std::string> enabledVulkanDeviceExtensions = {};

        bool hasVkInstanceRequiredLayer(const std::string& layerName) const {
            return this->enabledVulkanInstanceLayers.contains(layerName);
        }

        bool hasVkInstanceRequiredExtension(const std::string& extName) const {
            return this->enabledVulkanInstanceExtensions.contains(extName);
        }

        bool hasVkDeviceRequiredLayer(const std::string& layerName) const {
            return this->enabledVulkanDeviceLayers.contains(layerName);
        }

        bool hasVkDeviceRequiredExtension(const std::string& extName) const {
            return this->enabledVulkanDeviceExtensions.contains(extName);
        }

        void cleanUp();
        void tryIncludeInstanceLayer(const std::unordered_set<std::string>& available, const std::string &layer);
        void tryIncludeInstanceExtension(const std::unordered_set<std::string> &available, const std::string &layer);

        void initVulkanInstance(bool debugMode, std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string>requiredExtensions);
        void initVulkanDevice(std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string> requiredExtensions);

        static std::unordered_set<std::string> availableInstanceLayers();
        static std::unordered_set<std::string> availableDeviceExtensions(const VkPhysicalDevice& device);
        static std::unordered_set<std::string> availableDeviceLayers(const VkPhysicalDevice& device);
        static std::unordered_set<std::string> availableInstanceExtensions();
        static std::unordered_set<std::string> getGLFWExtensions();
    };

    inline std::string EXT_VK_KHR_portability_enumeration() {
        return "VK_KHR_portability_enumeration";
    }

    inline std::string EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME() {
        return VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    inline std::string EXT_VK_EXT_DEBUG_REPORT_EXTENSION_NAME() {
        return VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
    }

    inline std::string LR_VK_LAYER_KHRONOS_validation() {
        return "VK_LAYER_KHRONOS_validation";
    }

    extern std::unique_ptr<VulkanContext> vulkanContext;

    void init();
    void render();
    void cleanUp();
}
