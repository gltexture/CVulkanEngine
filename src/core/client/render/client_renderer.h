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
        VulkanContext() {
        };
        ~VulkanContext() {
            this->cleanUp();
        }

        VkDebugUtilsMessengerEXT vkDebugMessenger = {};
        VkInstance vkInstance = {};
        std::vector<VkPhysicalDevice> m_physicalDevices = {};
        std::unordered_set<std::string> enabledRequiredLayers = {};
        std::unordered_set<std::string> enabledRequiredExtensions = {};

        bool isValid() const {
            return this->vkInstance != VK_NULL_HANDLE;
        }

        bool hasRequiredLayer(const std::string& layerName) const {
            return this->enabledRequiredLayers.contains(layerName);
        }

        bool hasRequiredExtension(const std::string& extName) const {
            return this->enabledRequiredExtensions.contains(extName);
        }

        void cleanUp();
        void checkRequiredLayer(const std::unordered_set<std::string>& available, const std::string &layer);
        void checkRequiredExtension(const std::unordered_set<std::string> &available, const std::string &layer);

        [[nodiscard]] VkResult initVulkan(bool debugMode);

        static std::unordered_set<std::string> availableLayers();
        static std::unordered_set<std::string> availableExtensions();
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
