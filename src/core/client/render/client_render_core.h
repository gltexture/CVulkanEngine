#pragma once

#include <memory>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.h>

#include "client_window.h"
#include "vulkanPrograms/vulkan_utility.h"
#include "util/logger.inl"

namespace cvulkan::client::renderer {
    struct CVulkanLayersAndExtensionsData {
        std::unordered_set<std::string> enabledLayers = {};
        std::unordered_set<std::string> enabledExtensions = {};

        bool hasVkInstanceRequiredLayer(const std::string& layerName) const {
            return this->enabledLayers.contains(layerName);
        }

        bool hasVkInstanceRequiredExtension(const std::string& extName) const {
            return this->enabledExtensions.contains(extName);
        }
    };

    struct CVulkanSurfaceData {
        VkSurfaceKHR vkSurface = {};
        VkSurfaceCapabilitiesKHR vkSurfaceCapabilities = {};

        VkFormat format = {};
        VkColorSpaceKHR colorSpace = {};
    };

    struct CVulkanInstanceData {
        VkInstance vkInstance = {};
        CVulkanLayersAndExtensionsData vkInstanceLrExtData = {};
    };

    struct CVulkanPhysicalDeviceData {
        VkPhysicalDevice vkPhysicalDevice{};
        std::vector<VkExtensionProperties> vkDeviceExtensions{};
        VkPhysicalDeviceMemoryProperties vkMemoryProperties{};
        VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures{};
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties{};
        VkPhysicalDeviceProperties2 vkPhysicalDeviceProperties2{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        std::vector<VkQueueFamilyProperties> vkQueueFamilyProps;
        CVulkanLayersAndExtensionsData vkDeviceLrExtData = {};
    };

    struct CVulkanQueue {
        VkQueue vkQueue = {};
        uint32_t queueFamilyIndex = {};

        void queue_wait_dle() const {
            vkQueueWaitIdle(this->vkQueue);
        }
    };

    struct CVulkanLogicalDeviceData {
        VkDevice vkDevice = {};

        void device_wait_dle() const {
            vkDeviceWaitIdle(this->vkDevice);
        }
    };

    struct CVulkanImageViewData {
        VkImageAspectFlagBits aspectMask = {};
        uint32_t baseArrayLayer = 0;
        VkFormat format = {};
        uint32_t layerCount = 1;
        uint32_t mipLevels = 1;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    };

    struct CVulkanImageView {
        VkImage vk_image = {};
        VkImageView vk_image_view = {};

        static CVulkanImageView create(const CVulkanLogicalDeviceData& logical_device_data, VkImage vk_image, const CVulkanImageViewData &image_view_data);
    };

    struct CVVulkanSwapChain {
        VkSwapchainKHR swapChain = {};
        std::vector<CVulkanImageView> image_views = {};
    };

    class CVulkanContext {
    public:
        explicit CVulkanContext(const window::CVWindow& window) : glfwWindow{window} {}
        ~CVulkanContext() {
            this->destroy();
            this->vkInstanceData.vkInstanceLrExtData.enabledExtensions.clear();
            this->vkInstanceData.vkInstanceLrExtData.enabledLayers.clear();
            this->vkPhysicalDeviceData.vkDeviceLrExtData.enabledExtensions.clear();
            this->vkPhysicalDeviceData.vkDeviceLrExtData.enabledLayers.clear();
        }

        void setup_GLFWSurface(const CVulkanInstanceData &instanceData, const CVulkanPhysicalDeviceData &physical_device_data);
        void init_vulkan_instance(bool debugMode, std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string>requiredExtensions);
        void init_vulkan_physicalDevice(std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string> requiredExtensions);
        void init_vulkan_logicalDevice(const CVulkanPhysicalDeviceData &data);
        void init_vulkan_swapChain(const CVulkanSurfaceData &surface_data, const CVulkanLogicalDeviceData &device_data);

        [[nodiscard]] CVulkanSurfaceData vk_surface_data() const {
            return vkSurfaceData;
        }

        [[nodiscard]] VkDebugUtilsMessengerEXT vk_debug_messenger() const {
            return vkDebugMessenger;
        }

        [[nodiscard]] CVulkanInstanceData vk_instance_data() const {
            return vkInstanceData;
        }

        [[nodiscard]] CVulkanPhysicalDeviceData vk_physical_device_data() const {
            return vkPhysicalDeviceData;
        }

        [[nodiscard]] CVulkanLogicalDeviceData vk_device_data() const {
            return vkDeviceData;
        }

        [[nodiscard]] window::CVWindow glfw_window() const {
            return glfwWindow;
        }

        [[nodiscard]] CVulkanQueue graphics_queue() const {
            return graphicsQueue;
        }

    private:
        CVVulkanSwapChain vkSwapChain = {};
        CVulkanSurfaceData vkSurfaceData = {};
        VkDebugUtilsMessengerEXT vkDebugMessenger = {};
        CVulkanInstanceData vkInstanceData = {};
        CVulkanPhysicalDeviceData vkPhysicalDeviceData = {};
        CVulkanLogicalDeviceData vkDeviceData = {};
        CVulkanQueue graphicsQueue = {};
        const window::CVWindow& glfwWindow;

    protected:
        void calc_surface_format(const CVulkanPhysicalDeviceData &physical_device_data);
        void destroy();

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

    extern std::unique_ptr<CVulkanContext> vulkanContext;

    void init(const window::CVWindow& window);
    void render();
    void clean_up();
}
