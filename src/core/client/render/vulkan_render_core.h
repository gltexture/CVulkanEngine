#pragma once

#include <memory>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "glfw_window.h"
#include "vulkan_pipeline.h"
#include "vulkan_swapchain.h"
#include "util/logger.inl"

namespace cvulkan::client::render::core {
    using QueueFamilyBitMask = uint32_t;

    struct CVulkanQueueFamilyBitMasks {
        static constexpr QueueFamilyBitMask GRAPHICS = 1u << 0;
        static constexpr QueueFamilyBitMask COMPUTE = 1u << 1;
        static constexpr QueueFamilyBitMask TRANSFER = 1u << 2;
        static constexpr QueueFamilyBitMask PRESENT = 1u << 3;
    };

    struct CVulkanQueueFamilyCreationRequest {
        QueueFamilyBitMask _bitMask;
        uint32_t _queueCount;
        std::vector<float> _priorities;
    };

    struct CVulkanQueueFamilyRegisteredData {
        QueueFamilyBitMask _bitMask;
        uint32_t _queueCount;
        uint32_t _queueFamilyIndex;
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
        std::vector<CVulkanQueueFamilyRegisteredData> _registeredData{};
    };


    struct CVulkanLayersAndExtensionsData {
        std::unordered_set<std::string> _enabledLayers{};
        std::unordered_set<std::string> _enabledExtensions{};

        bool hasVkInstanceRequiredLayer(const std::string& layerName) const {
            return this->_enabledLayers.contains(layerName);
        }

        bool hasVkInstanceRequiredExtension(const std::string& extName) const {
            return this->_enabledExtensions.contains(extName);
        }
    };

    class CVulkanInstance {
    public:
        CVulkanInstance();
        ~CVulkanInstance();

        [[nodiscard]] VkInstance vkInstance() const {
            return _vkInstance;
        }

        [[nodiscard]] CVulkanLayersAndExtensionsData vkInstanceLayersExtensionsData() const {
            return _vkInstanceLrExtData;
        }

    private:
        VkInstance _vkInstance{};
        CVulkanLayersAndExtensionsData _vkInstanceLrExtData{};
    };

    class CVulkanPhysicalDevice {
    public:
        CVulkanPhysicalDevice();
        ~CVulkanPhysicalDevice();

        [[nodiscard]] VkPhysicalDevice vkPhysicalDevice() const {
            return _vkPhysicalDevice;
        }

        [[nodiscard]] std::vector<VkExtensionProperties> vkDeviceExtensions() const {
            return _vkDeviceExtensions;
        }

        [[nodiscard]] VkPhysicalDeviceMemoryProperties vkMemoryProperties() const {
            return _vkMemoryProperties;
        }

        [[nodiscard]] VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures() const {
            return _vkPhysicalDeviceFeatures;
        }

        [[nodiscard]] VkPhysicalDeviceProperties vkPhysicalDeviceProperties() const {
            return _vkPhysicalDeviceProperties;
        }

        [[nodiscard]] VkPhysicalDeviceProperties2 vkPhysicalDeviceProperties2() const {
            return _vkPhysicalDeviceProperties2;
        }

        [[nodiscard]] std::vector<VkQueueFamilyProperties> vkQueueFamilyProps() const {
            return _vkQueueFamilyProps;
        }

        [[nodiscard]] CVulkanLayersAndExtensionsData vkDeviceLayersExtensionsData() const {
            return _vkDeviceLrExtData;
        }

    private:
        VkPhysicalDevice _vkPhysicalDevice{};
        std::vector<VkExtensionProperties> _vkDeviceExtensions{};
        VkPhysicalDeviceMemoryProperties _vkMemoryProperties{};
        VkPhysicalDeviceFeatures _vkPhysicalDeviceFeatures{};
        VkPhysicalDeviceProperties _vkPhysicalDeviceProperties{};
        VkPhysicalDeviceProperties2 _vkPhysicalDeviceProperties2 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
        };
        std::vector<VkQueueFamilyProperties> _vkQueueFamilyProps{};
        CVulkanLayersAndExtensionsData _vkDeviceLrExtData{};
    };

    class CVulkanDevice {
    public:
        CVulkanDevice();
        ~CVulkanDevice();

        [[nodiscard]] VkDevice vkDevice() const {
            return _vkDevice;
        }

    private:
        VkDevice _vkDevice{};

        void deviceWaitIdle() const {
            vkDeviceWaitIdle(this->_vkDevice);
        }
    };

    class CVulkanContext {
    public:
        explicit CVulkanContext(const window::CVulkanWindow& window) : _glfwWindow{window}, _surface{*this}, _pipelineCache(*this) {
        }

        ~CVulkanContext() = default;

        CVULKAN_NO_COPY(CVulkanContext)

        void createVulkanInstance(bool debugMode, std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string> requiredExtensions);
        void createVulkanPhysicalDevice(std::initializer_list<std::string> requiredLayers, std::initializer_list<std::string> requiredExtensions);
        void createVulkanLogicalDevice(std::vector<CVulkanQueueFamilyCreationRequest>&& requiredQueueFamilies);

        [[nodiscard]] VkDebugUtilsMessengerEXT vkDebugMessenger() const {
            return _vkDebugMessenger;
        }

        [[nodiscard]] const CVulkanInstance& instance() const {
            return _instance;
        }

        [[nodiscard]] const CVulkanPipelineCache& pipelineCache() const {
            return _pipelineCache;
        }

        [[nodiscard]] CVulkanPipelineCache& pipelineCache() {
            return _pipelineCache;
        }

        [[nodiscard]] const CVulkanPhysicalDevice& physicalDevice() const {
            return _physicalDevice;
        }

        [[nodiscard]] const CVulkanDevice& device() const {
            return _device;
        }

        [[nodiscard]] const window::CVulkanWindow& glfwWindow() const {
            return _glfwWindow;
        }

        [[nodiscard]] CVulkanSurface& surface() {
            return _surface;
        }

        [[nodiscard]] const CVulkanSurface& surface() const {
            return _surface;
        }

        [[nodiscard]] const CVulkanQueueFamiliesRegistry& queueFamiliesRegistry() const {
            return _queueFamiliesRegistry;
        }

    private:
        const window::CVulkanWindow& _glfwWindow;
        CVulkanInstance _instance{};
        VkDebugUtilsMessengerEXT _vkDebugMessenger{};
        CVulkanPhysicalDevice _physicalDevice{};
        CVulkanSurface _surface;
        CVulkanDevice _device{};
        CVulkanQueueFamiliesRegistry _queueFamiliesRegistry{};
        CVulkanPipelineCache _pipelineCache;

    protected:
        [[nodiscard]] uint32_t findVulkanQueueFamily(QueueFamilyBitMask bitmask) const;

        void tryIncludeInstanceLayer(const std::unordered_set<std::string>& available, const std::string& layer) {
            if (available.contains(layer)) {
                this->_instance.vkInstanceLayersExtensionsData()._enabledLayers.insert(layer);
                logging::info("Enabled instance Layer {}", layer);
            } else {
                logging::error("Instance Layer {} is not available!", layer);
            }
        }

        void tryIncludeInstanceExtension(const std::unordered_set<std::string>& available, const std::string& layer) {
            if (available.contains(layer)) {
                this->_instance.vkInstanceLayersExtensionsData()._enabledExtensions.insert(layer);
                logging::info("Enabled instance extension {}", layer);
            } else {
                logging::error("Instance extension {} is not available!", layer);
            }
        }

        void tryIncludeDeviceLayer(const std::unordered_set<std::string>& available, const std::string& layer) {
            if (available.contains(layer)) {
                this->_physicalDevice.vkDeviceLayersExtensionsData()._enabledLayers.insert(layer);
                logging::info("Enabled device Layer {}", layer);
            } else {
                logging::error("Device Layer {} is not available!", layer);
            }
        }

        void tryIncludeDeviceExtension(const std::unordered_set<std::string>& available, const std::string& layer) {
            if (available.contains(layer)) {
                this->_physicalDevice.vkDeviceLayersExtensionsData()._enabledExtensions.insert(layer);
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

    void createRenderCore(const window::CVulkanWindow& window);

    void runRender();

    void cleanRenderCore();
}
