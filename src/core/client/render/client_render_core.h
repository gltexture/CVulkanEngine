#pragma once

#include <memory>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.h>

#include "client_window.h"
#include "vulkanPrograms/vulkan_utility.h"
#include "util/logger.inl"

namespace cvulkan::client::renderer {
    struct CVVulkanLayersAndExtensionsData {
        std::unordered_set<std::string> enabledLayers = {};
        std::unordered_set<std::string> enabledExtensions = {};

        bool hasVkInstanceRequiredLayer(const std::string& layerName) const {
            return this->enabledLayers.contains(layerName);
        }

        bool hasVkInstanceRequiredExtension(const std::string& extName) const {
            return this->enabledExtensions.contains(extName);
        }
    };

    struct CVVulkanSurfaceData {
        VkSurfaceKHR vkSurface = {};
        VkSurfaceCapabilitiesKHR vkSurfaceCapabilities = {};

        VkFormat format = {};
        VkColorSpaceKHR colorSpace = {};
    };

    struct CVVulkanInstanceData {
        VkInstance vkInstance = {};
        CVVulkanLayersAndExtensionsData vkInstanceLrExtData = {};
    };

    struct CVVulkanPhysicalDeviceData {
        VkPhysicalDevice vkPhysicalDevice{};
        std::vector<VkExtensionProperties> vkDeviceExtensions{};
        VkPhysicalDeviceMemoryProperties vkMemoryProperties{};
        VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures{};
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties{};
        VkPhysicalDeviceProperties2 vkPhysicalDeviceProperties2{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        std::vector<VkQueueFamilyProperties> vkQueueFamilyProps;
        CVVulkanLayersAndExtensionsData vkDeviceLrExtData = {};
    };

    struct CVVulkanQueue {
        VkQueue vkQueue = {};
        uint32_t queueFamilyIndex = {};

        void queue_wait_dle() const {
            vkQueueWaitIdle(this->vkQueue);
        }
    };

    struct CVVulkanLogicalDeviceData {
        VkDevice vkDevice = {};

        void device_wait_dle() const {
            vkDeviceWaitIdle(this->vkDevice);
        }
    };

    class CVVulkanContext {
    public:
        explicit CVVulkanContext(const window::CVWindow& window) : glfwWindow{window} {}
        ~CVVulkanContext() {
            this->cleanUp();
            this->vkInstanceData.vkInstanceLrExtData.enabledExtensions.clear();
            this->vkInstanceData.vkInstanceLrExtData.enabledLayers.clear();
            this->vkPhysicalDeviceData.vkDeviceLrExtData.enabledExtensions.clear();
            this->vkPhysicalDeviceData.vkDeviceLrExtData.enabledLayers.clear();
        }

        void setup_GLFWSurface(const CVVulkanInstanceData &instanceData, const CVVulkanPhysicalDeviceData &physical_device_data);
        void init_vulkan_instance(bool debugMode, std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string>requiredExtensions);
        void init_vulkan_physicalDevice(std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string> requiredExtensions);
        void init_vulkan_logicalDevice(const CVVulkanPhysicalDeviceData &data);

        [[nodiscard]] CVVulkanSurfaceData vk_surface_data() const {
            return vkSurfaceData;
        }

        [[nodiscard]] VkDebugUtilsMessengerEXT vk_debug_messenger() const {
            return vkDebugMessenger;
        }

        [[nodiscard]] CVVulkanInstanceData vk_instance_data() const {
            return vkInstanceData;
        }

        [[nodiscard]] CVVulkanPhysicalDeviceData vk_physical_device_data() const {
            return vkPhysicalDeviceData;
        }

        [[nodiscard]] CVVulkanLogicalDeviceData vk_device_data() const {
            return vkDeviceData;
        }

        [[nodiscard]] window::CVWindow glfw_window() const {
            return glfwWindow;
        }

        [[nodiscard]] CVVulkanQueue graphics_queue() const {
            return graphicsQueue;
        }

    private:
        CVVulkanSurfaceData vkSurfaceData = {};
        VkDebugUtilsMessengerEXT vkDebugMessenger = {};
        CVVulkanInstanceData vkInstanceData = {};
        CVVulkanPhysicalDeviceData vkPhysicalDeviceData = {};
        CVVulkanLogicalDeviceData vkDeviceData = {};
        CVVulkanQueue graphicsQueue = {};
        const window::CVWindow& glfwWindow;

    protected:
        void calc_surface_format(const CVVulkanPhysicalDeviceData &physical_device_data);
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

    extern std::unique_ptr<CVVulkanContext> vulkanContext;

    void init(const window::CVWindow& window);
    void render();
    void clean_up();
}
