#pragma once

#include <memory>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.h>
#include "vulkanPrograms/vulkan_utility.h"
#include "util/logger.inl"

namespace cvulkan::client::renderer {
    struct LayersExtensions_Data {
        std::unordered_set<std::string> enabledLayers = {};
        std::unordered_set<std::string> enabledExtensions = {};

        bool hasVkInstanceRequiredLayer(const std::string& layerName) const {
            return this->enabledLayers.contains(layerName);
        }

        bool hasVkInstanceRequiredExtension(const std::string& extName) const {
            return this->enabledExtensions.contains(extName);
        }
    };

    struct VulkanInstance_Data {
        VkInstance vkInstance = {};
        LayersExtensions_Data vkInstanceLrExtData = {};
    };

    struct VulkanPhysicalDevice_Data {
        VkPhysicalDevice vkPhysicalDevice{};
        std::vector<VkExtensionProperties> vkDeviceExtensions{};
        VkPhysicalDeviceMemoryProperties vkMemoryProperties{};
        VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures{};
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties{};
        VkPhysicalDeviceProperties2 vkPhysicalDeviceProperties2{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        std::vector<VkQueueFamilyProperties> vkQueueFamilyProps;
        LayersExtensions_Data vkDeviceLrExtData = {};
    };

    struct VulkanLogicalDevice_Data {
        VkDevice vkDevice = {};

        void deviceWaitIdle() const {
            vkDeviceWaitIdle(this->vkDevice);
        }
    };

    class VulkanContext {
    public:
        VulkanContext() = default;
        ~VulkanContext() {
            this->cleanUp();
            this->vkInstanceData.vkInstanceLrExtData.enabledExtensions.clear();
            this->vkInstanceData.vkInstanceLrExtData.enabledLayers.clear();
            this->vkPhysicalDeviceData.vkDeviceLrExtData.enabledExtensions.clear();
            this->vkPhysicalDeviceData.vkDeviceLrExtData.enabledLayers.clear();
        }

        VkDebugUtilsMessengerEXT vkDebugMessenger = {};

        VulkanInstance_Data vkInstanceData = {};
        VulkanPhysicalDevice_Data vkPhysicalDeviceData = {};
        VulkanLogicalDevice_Data vkDeviceData = {};

        void initVulkanInstance(bool debugMode, std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string>requiredExtensions);
        void initVulkanPhysicalDevice(std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string> requiredExtensions);
        void initVulkanLogicalDevice(const VulkanPhysicalDevice_Data &data);
    protected:
        void cleanUp();

        void tryIncludeInstanceLayer(const std::unordered_set<std::string> &available, const std::string &layer) {
            if (available.contains(layer)) {
                this->vkInstanceData.vkInstanceLrExtData.enabledLayers.insert(layer);
                logging::info("Enabled instance Layer {}", layer);
            } else {
                logging::error("Instance Layer {} is not available!", layer);
            }
        }

        void tryIncludeInstanceExtension(const std::unordered_set<std::string> &available, const std::string &layer) {
            if (available.contains(layer)) {
                this->vkInstanceData.vkInstanceLrExtData.enabledExtensions.insert(layer);
                logging::info("Enabled instance extension {}", layer);
            } else {
                logging::error("Instance extension {} is not available!", layer);
            }
        }

        void tryIncludeDeviceLayer(const std::unordered_set<std::string> &available, const std::string &layer) {
            if (available.contains(layer)) {
                this->vkPhysicalDeviceData.vkDeviceLrExtData.enabledLayers.insert(layer);
                logging::info("Enabled device Layer {}", layer);
            } else {
                logging::error("Device Layer {} is not available!", layer);
            }
        }

        void tryIncludeDeviceExtension(const std::unordered_set<std::string> &available, const std::string &layer) {
            if (available.contains(layer)) {
                this->vkPhysicalDeviceData.vkDeviceLrExtData.enabledExtensions.insert(layer);
                logging::info("Enabled device extension {}", layer);
            } else {
                logging::error("Device extension {} is not available!", layer);
            }
        }

        static std::unordered_set<std::string> availableInstanceLayers(const VkInstance& instance);
        static std::unordered_set<std::string> availableDeviceExtensions(const VkPhysicalDevice& device);
        static std::unordered_set<std::string> availableDeviceLayers(const VkPhysicalDevice& device);
        static std::unordered_set<std::string> availableInstanceExtensions(const VkInstance& instance);
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

    inline std::string EXT_VK_KHR_SWAPCHAIN_EXTENSION_NAME() {
        return VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    }

    inline std::string LR_VK_LAYER_KHRONOS_validation() {
        return "VK_LAYER_KHRONOS_validation";
    }

    inline std::string EXT_VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME() {
        return "VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME";
    }

    extern std::unique_ptr<VulkanContext> vulkanContext;

    void init();
    void render();
    void cleanUp();
}
