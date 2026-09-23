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

    struct CVulkanContextBuildData {
        bool _debugMode{false};
    };

    struct CVulkanInstanceBuildData {
        std::initializer_list<std::string> _requiredLayers;
        std::initializer_list<std::string> _requiredExtensions;
    };

    struct CVulkanPhysicalDeviceBuildData {
        std::initializer_list<std::string> _requiredLayers;
        std::initializer_list<std::string> _requiredExtensions;
    };

    struct CVulkanLogicalDeviceBuildData {
        std::vector<CVulkanQueueFamilyCreationRequest> _queueFamilyCreationRequests;
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
        CVulkanInstance(
            const CVulkanContextBuildData& contextBuildData,
            const CVulkanInstanceBuildData& instanceBuildData);
        ~CVulkanInstance();

        [[nodiscard]] VkInstance vkInstance() const {
            return _vkInstance;
        }

        [[nodiscard]] VkDebugUtilsMessengerEXT vkDebugMessenger() const {
            return _vkDebugMessenger;
        }

        [[nodiscard]] const CVulkanLayersAndExtensionsData& vkInstanceLayersExtensionsData() const {
            return _vkInstanceLrExtData;
        }

    protected:
        void tryIncludeInstanceLayer(const std::unordered_set<std::string>& available, const std::string& layer);
        void tryIncludeInstanceExtension(const std::unordered_set<std::string>& available, const std::string& ext);

    private:
        VkInstance _vkInstance{};
        VkDebugUtilsMessengerEXT _vkDebugMessenger{};
        CVulkanLayersAndExtensionsData _vkInstanceLrExtData{};
    };

    class CVulkanPhysicalDevice {
    public:
        ~CVulkanPhysicalDevice();

        CVulkanPhysicalDevice(
            const CVulkanContextBuildData& contextBuildData,
            const CVulkanInstance& instance,
            const CVulkanPhysicalDeviceBuildData& physicalDeviceBuildData);

        [[nodiscard]] VkPhysicalDevice vkPhysicalDevice() const {
            return _vkPhysicalDevice;
        }

        [[nodiscard]] const std::vector<VkExtensionProperties>& vkDeviceExtensions() const {
            return _vkDeviceExtensions;
        }

        [[nodiscard]] const VkPhysicalDeviceMemoryProperties& vkMemoryProperties() const {
            return _vkMemoryProperties;
        }

        [[nodiscard]] const VkPhysicalDeviceFeatures& vkPhysicalDeviceFeatures() const {
            return _vkPhysicalDeviceFeatures;
        }

        [[nodiscard]] const VkPhysicalDeviceProperties& vkPhysicalDeviceProperties() const {
            return _vkPhysicalDeviceProperties;
        }

        [[nodiscard]] const VkPhysicalDeviceProperties2& vkPhysicalDeviceProperties2() const {
            return _vkPhysicalDeviceProperties2;
        }

        [[nodiscard]] const std::vector<VkQueueFamilyProperties>& vkQueueFamilyProps() const {
            return _vkQueueFamilyProps;
        }

        [[nodiscard]] const CVulkanLayersAndExtensionsData& vkDeviceLayersExtensionsData() const {
            return _vkDeviceLrExtData;
        }

    protected:
        void tryIncludeDeviceLayer(const std::unordered_set<std::string>& available, const std::string& layer);
        void tryIncludeDeviceExtension(const std::unordered_set<std::string>& available, const std::string& ext);

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
        CVulkanDevice(
            const CVulkanContextBuildData& contextBuildData,
            const CVulkanPhysicalDevice& physicalDevice,
            VkSurfaceKHR surface,
            const CVulkanLogicalDeviceBuildData& logicalDeviceBuildData);
        ~CVulkanDevice();

        [[nodiscard]] VkDevice vkDevice() const {
            return _vkDevice;
        }

        [[nodiscard]] const CVulkanQueueFamiliesRegistry& queueFamiliesRegistry() const {
            return _queueFamiliesRegistry;
        }

        void deviceWaitIdle() const {
            vkDeviceWaitIdle(this->_vkDevice);
        }

    private:
        VkDevice _vkDevice{};
        CVulkanQueueFamiliesRegistry _queueFamiliesRegistry{};
    };

    class CVulkanContext {
    public:
        CVulkanContext(
            const window::CVulkanWindow& window,
            const CVulkanContextBuildData& contextBuildData,
            const CVulkanInstanceBuildData& instanceBuildData,
            const CVulkanPhysicalDeviceBuildData& physicalDeviceBuildData,
            const CVulkanLogicalDeviceBuildData& logicalDeviceBuildData);
        ~CVulkanContext() = default;

        CVULKAN_NO_COPY(CVulkanContext)

        [[nodiscard]] const window::CVulkanWindow& glfwWindow() const {
            return _glfwWindow;
        }

        [[nodiscard]] const CVulkanInstance& instance() const {
            return _instance;
        }

        [[nodiscard]] const CVulkanPhysicalDevice& physicalDevice() const {
            return _physicalDevice;
        }

        [[nodiscard]] const CVulkanSurface& surface() const {
            return _surface;
        }

        [[nodiscard]] const CVulkanSwapChain& swapChain() const {
            return _swapChain;
        }

        [[nodiscard]] const CVulkanDevice& device() const {
            return _device;
        }

        [[nodiscard]] const CVulkanPipelineCache& pipelineCache() const {
            return _pipelineCache;
        }

    private:
        const window::CVulkanWindow& _glfwWindow;
        CVulkanInstance _instance;
        CVulkanPhysicalDevice _physicalDevice;
        CVulkanSurface _surface;
        CVulkanDevice _device;
        CVulkanSwapChain _swapChain;
        CVulkanPipelineCache _pipelineCache;
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

    std::unordered_set<std::string> availableInstanceExtensions();
    std::unordered_set<std::string> availableInstanceLayers();
    std::unordered_set<std::string> availableDeviceExtensions(const VkPhysicalDevice& device);
    std::unordered_set<std::string> availableDeviceLayers(const VkPhysicalDevice& device);
    std::unordered_set<std::string> getGLFWExtensions();
    uint32_t findVulkanQueueFamily(QueueFamilyBitMask bitmask, const CVulkanPhysicalDevice& device, VkSurfaceKHR surface);

    void createRenderCore(const window::CVulkanWindow& window);
    void runRender();
    void cleanRenderCore();
}
