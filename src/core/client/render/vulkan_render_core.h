#pragma once

#include <memory>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.h>

#include "glfw_window.h"
#include "vulkan_swapchain.h"
#include "vulkan_utility.h"
#include "util/logger.inl"

namespace cvulkan::client::renderCore {
    using QueueFamilyBitMask = uint32_t;

    struct CVulkanQueueFamilyBitMasks {
        static constexpr QueueFamilyBitMask GRAPHICS = 1u << 0;
        static constexpr QueueFamilyBitMask COMPUTE = 1u << 1;
        static constexpr QueueFamilyBitMask TRANSFER = 1u << 2;
        static constexpr QueueFamilyBitMask PRESENT = 1u << 3;
    };

    struct CVulkanQueueFamilyCreationRequest {
        QueueFamilyBitMask bitMask;
        uint32_t queueCount;
        std::vector<float> priorities;
    };

    struct CVulkanQueueFamilyRegisteredData {
        QueueFamilyBitMask bitMask;
        uint32_t queueCount;
        uint32_t queueFamilyIndex;
    };

    class CVulkanQueueFamiliesRegistry {
    public:
        CVulkanQueueFamiliesRegistry() = default;
        ~CVulkanQueueFamiliesRegistry() = default;

        void registerQueueFamily(QueueFamilyBitMask bitMask, uint32_t queueCount, uint32_t queueFamilyIndex);

        [[nodiscard]] const std::vector<CVulkanQueueFamilyRegisteredData>& registeredData() const {
            return _registeredData;
        }

    private:
        std::vector<CVulkanQueueFamilyRegisteredData> _registeredData = {};
    };



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

    struct CVulkanInstance {
        VkInstance vkInstance = {};
        CVulkanLayersAndExtensionsData vkInstanceLrExtData = {};
    };

    struct CVulkanPhysicalDevice {
        VkPhysicalDevice vkPhysicalDevice{};
        std::vector<VkExtensionProperties> vkDeviceExtensions{};
        VkPhysicalDeviceMemoryProperties vkMemoryProperties{};
        VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures{};
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties{};
        VkPhysicalDeviceProperties2 vkPhysicalDeviceProperties2{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        std::vector<VkQueueFamilyProperties> vkQueueFamilyProps;
        CVulkanLayersAndExtensionsData vkDeviceLrExtData = {};
    };

    struct CVulkanDevice {
        VkDevice vkDevice = {};

        void deviceWaitIdle() const {
            vkDeviceWaitIdle(this->vkDevice);
        }
    };

    class CVulkanContext {
    public:
        explicit CVulkanContext(const window::CVWindow& window) : _glfwWindow{window}, _surface{*this} {}
        ~CVulkanContext() {
            this->destroyRenderCore();
        }

        CVulkanContext(const CVulkanContext&) = delete;
        CVulkanContext& operator=(const CVulkanContext&) = delete;

        void initVulkanInstance(bool debugMode, std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string>requiredExtensions);
        void initVulkanPhysicalDevice(std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string> requiredExtensions);
        void initVulkanLogicalDevice(std::vector<CVulkanQueueFamilyCreationRequest>&& requiredQueueFamilies);

        [[nodiscard]] VkDebugUtilsMessengerEXT vkDebugMessenger() const {
            return _vkDebugMessenger;
        }

        [[nodiscard]] const CVulkanInstance& instanceData() const {
            return _instance;
        }

        [[nodiscard]] const CVulkanPhysicalDevice& physicalDeviceData() const {
            return _physicalDevice;
        }

        [[nodiscard]] const CVulkanDevice& deviceData() const {
            return _device;
        }

        [[nodiscard]] const window::CVWindow& glfwWindow() const {
            return _glfwWindow;
        }

        [[nodiscard]] CVulkanSurface& surface() {
            return _surface;
        }

        [[nodiscard]] CVulkanQueueFamiliesRegistry queueFamiliesRegistry() const {
            return _queueFamiliesRegistry;
        }

    private:
        const window::CVWindow& _glfwWindow;
        VkDebugUtilsMessengerEXT _vkDebugMessenger = {};
        CVulkanInstance _instance = {};
        CVulkanPhysicalDevice _physicalDevice = {};
        CVulkanDevice _device = {};
        CVulkanSurface _surface;
        CVulkanQueueFamiliesRegistry _queueFamiliesRegistry = {};

    protected:
        void destroyRenderCore();

        [[nodiscard]] uint32_t findVulkanQueueFamily(QueueFamilyBitMask bitmask) const;

        void tryIncludeInstanceLayer(const std::unordered_set<std::string> &available, const std::string &layer) {
            if (available.contains(layer)) {
                this->_instance.vkInstanceLrExtData.enabledLayers.insert(layer);
                logging::info("Enabled instance Layer {}", layer);
            } else {
                logging::error("Instance Layer {} is not available!", layer);
            }
        }

        void tryIncludeInstanceExtension(const std::unordered_set<std::string> &available, const std::string &layer) {
            if (available.contains(layer)) {
                this->_instance.vkInstanceLrExtData.enabledExtensions.insert(layer);
                logging::info("Enabled instance extension {}", layer);
            } else {
                logging::error("Instance extension {} is not available!", layer);
            }
        }

        void tryIncludeDeviceLayer(const std::unordered_set<std::string> &available, const std::string &layer) {
            if (available.contains(layer)) {
                this->_physicalDevice.vkDeviceLrExtData.enabledLayers.insert(layer);
                logging::info("Enabled device Layer {}", layer);
            } else {
                logging::error("Device Layer {} is not available!", layer);
            }
        }

        void tryIncludeDeviceExtension(const std::unordered_set<std::string> &available, const std::string &layer) {
            if (available.contains(layer)) {
                this->_physicalDevice.vkDeviceLrExtData.enabledExtensions.insert(layer);
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
    void cleanUp();
}
