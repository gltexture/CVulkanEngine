#include "vulkan_render_core.h"
#include "util/logger.h"
#include <unordered_set>

#include "GLFW/glfw3.h"
#include "vulkan_utility.h"
#include "vulkan_ext.h"
#include "vulkan_renderer.h"

namespace cvulkan::client::renderCore {
    std::unique_ptr<CVulkanContext> vulkanContext;

    void CVulkanContext::destroyRenderCore() {
        this->_surface.destroySurface();
        if (this->_vkDebugMessenger != VK_NULL_HANDLE) {
            const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->_instance.vkInstance,"vkDestroyDebugUtilsMessengerEXT"));
            if (func == nullptr) {
                logging::error("Failed to get vkDestroyDebugUtilsMessengerEXT");
            } else {
                func(this->_instance.vkInstance, this->_vkDebugMessenger, nullptr);
            }
            logging::debug("Vulkan debug messenger destroyed");
            this->_vkDebugMessenger = VK_NULL_HANDLE;
        }
        if (this->_device.vkDevice != VK_NULL_HANDLE) {
            this->_device.deviceWaitIdle();
            vkDestroyDevice(this->_device.vkDevice, nullptr);
            this->_device.vkDevice = VK_NULL_HANDLE;
            logging::info("Vulkan device destroyed");
        }
        if (this->_instance.vkInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(this->_instance.vkInstance, nullptr);
            this->_instance.vkInstance = VK_NULL_HANDLE;
            logging::info("Vulkan instance destroyed");
        }
    }

    std::unordered_set<std::string> CVulkanContext::availableInstanceExtensions(const VkInstance &instance) {
        std::unordered_set<std::string> set = {};
        uint32_t extensionsCount = 0;
        std::vector<VkExtensionProperties> extensionProperties = {};
        utility::vkCheck(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr), "Failed to get instance ext");
        if (extensionsCount == 0) {
            logging::error("No vulkan instance extensions available");
            return set;
        }
        logging::info("Available {} instance vulkan extensions", extensionsCount);
        extensionProperties.resize(extensionsCount);
         utility::vkCheck(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensionProperties.data()), "Failed to get instance ext");
        for (uint32_t i = 0; i < extensionsCount; i++) {
            const VkExtensionProperties& prop = extensionProperties[i];
            logging::debug("+ instance extension {}", prop.extensionName);
            set.emplace(prop.extensionName);
        }
        return set;
    }

    std::unordered_set<std::string> CVulkanContext::availableInstanceLayers(const VkInstance &instance) {
        std::unordered_set<std::string> set = {};
        uint32_t layerCount = 0;
        std::vector<VkLayerProperties> layerProperties = {};
        utility::vkCheck(vkEnumerateInstanceLayerProperties(&layerCount, nullptr), "Failed to get instance layers");
        if (layerCount == 0) {
            logging::error("No vulkan instance layers available");
            return set;
        }
        logging::info("Available {} instance vulkan layers", layerCount);
        layerProperties.resize(layerCount);
        utility::vkCheck(vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data()), "Failed to get instance layers");
        for (uint32_t i = 0; i < layerCount; i++) {
            const VkLayerProperties& prop = layerProperties[i];
            logging::debug("+ Instance Layer {}", prop.layerName);
            set.emplace(prop.layerName);
        }
        return set;
    }

    std::unordered_set<std::string> CVulkanContext::availableDeviceExtensions(const VkPhysicalDevice& device) {
        std::unordered_set<std::string> set = {};
        uint32_t extensionsCount = 0;
        std::vector<VkExtensionProperties> extensionProperties = {};
        utility::vkCheck(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr), "Failed to get device ext");
        if (extensionsCount == 0) {
            logging::error("No vulkan device extensions available");
            return set;
        }
        logging::info("Available {} device vulkan extensions", extensionsCount);
        extensionProperties.resize(extensionsCount);
        utility::vkCheck(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensionProperties.data()), "Failed to get device ext");
        for (uint32_t i = 0; i < extensionsCount; i++) {
            const VkExtensionProperties& prop = extensionProperties[i];
            logging::debug("+ device extension {}", prop.extensionName);
            set.emplace(prop.extensionName);
        }
        return set;
    }

    std::unordered_set<std::string> CVulkanContext::availableDeviceLayers(const VkPhysicalDevice& device) {
        std::unordered_set<std::string> set = {};
        uint32_t layerCount = 0;
        std::vector<VkLayerProperties> layerProperties = {};
        utility::vkCheck(vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr), "Failed to get device layers");
        if (layerCount == 0) {
            logging::error("No vulkan device layers available");
            return set;
        }
        logging::info("Available {} device vulkan layers", layerCount);
        layerProperties.resize(layerCount);
        utility::vkCheck(vkEnumerateDeviceLayerProperties(device, &layerCount, layerProperties.data()), "Failed to get device layers");
        for (uint32_t i = 0; i < layerCount; i++) {
            const VkLayerProperties& prop = layerProperties[i];
            logging::debug("+ Device Layer {}", prop.layerName);
            set.emplace(prop.layerName);
        }
        return set;
    }

    std::unordered_set<std::string> CVulkanContext::getGLFWExtensions() {
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

    void CVulkanQueueFamiliesRegistry::registerQueueFamily(const QueueFamilyBitMask bitMask, const uint32_t queueCount, const uint32_t queueFamilyIndex) {
        this->_registeredData.emplace_back(bitMask, queueCount, queueFamilyIndex);
    }

    void CVulkanContext::initVulkanInstance(const bool debugMode,
                                            const std::initializer_list<std::string> requiredLayers,
                                            const std::initializer_list<std::string> requiredExtensions) {

        bool USE_PORTABILITY_MODE = false;
        const std::unordered_set<std::string> setOfExtensions = availableInstanceExtensions(this->_instance.vkInstance);
        const std::unordered_set<std::string> setOfLayers = availableInstanceLayers(this->_instance.vkInstance);
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
                USE_PORTABILITY_MODE = this->_instance.vkInstanceLrExtData.hasVkInstanceRequiredExtension(EXT_VK_KHR_portability_enumeration());
            }
        }

        {
            std::vector<const char*> enabledLayerNames{};
            std::vector<const char*> enabledExtNames{};

            for (const auto& layer : this->_instance.vkInstanceLrExtData.enabledLayers)
            {
                enabledLayerNames.emplace_back(layer.c_str());
            }
            for (const auto& layer : this->_instance.vkInstanceLrExtData.enabledExtensions)
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
            instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(this->_instance.vkInstanceLrExtData.enabledExtensions.size());
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size());
            instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
            instanceCreateInfo.ppEnabledExtensionNames = enabledExtNames.data();
            VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info_ext = {};
            if (debugMode && this->_instance.vkInstanceLrExtData.hasVkInstanceRequiredExtension(EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME())) {
                vk_debug_utils_messenger_create_info_ext = ext::createDebugMessengerCreateInfo();
                instanceCreateInfo.pNext = &vk_debug_utils_messenger_create_info_ext;
            }
            utility::vkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &this->_instance.vkInstance));
            logging::info("Successfully created vulkan instance!");

            if (debugMode && this->_instance.vkInstanceLrExtData.hasVkInstanceRequiredExtension(EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME())) {
                const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->_instance.vkInstance,"vkCreateDebugUtilsMessengerEXT"));
                if (func == nullptr) {
                    throw std::runtime_error{"Failed to load vkCreateDebugUtilsMessengerEXT"};
                }
                utility::vkCheck(func(vulkanContext->_instance.vkInstance, &vk_debug_utils_messenger_create_info_ext, nullptr, &this->_vkDebugMessenger));
                logging::debug("Created debug messenger");
            }
        }
    }

    void CVulkanContext::initVulkanPhysicalDevice(
        const std::initializer_list<std::string> requiredLayers,
        const std::initializer_list<std::string> requiredExtensions) {

        uint32_t physicalDeviceCount = 0;
        utility::vkCheck(vkEnumeratePhysicalDevices(this->_instance.vkInstance, &physicalDeviceCount, nullptr), "Failed to get physical devices");
        if (physicalDeviceCount > 0) {
            std::vector<VkPhysicalDevice> priorityVectorToChooseDevice = {};
            std::vector<VkPhysicalDevice> physicalDevices = {};
            physicalDevices.resize(physicalDeviceCount);
            utility::vkCheck(vkEnumeratePhysicalDevices(this->_instance.vkInstance, &physicalDeviceCount, physicalDevices.data()), "Failed to get physical devices");
            {
                std::unordered_set<std::string> availableLayers = {};
                std::unordered_set<std::string> availableExtensions = {};

                for (const auto& physicalDevice : physicalDevices) {
                    bool success = true;
                    VkPhysicalDeviceProperties physicalDeviceProperties = {};
                    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

                    availableLayers = this->availableDeviceLayers(physicalDevice);
                    availableExtensions = this->availableDeviceExtensions(physicalDevice);

                    for (const auto& layer : requiredLayers) {
                        if (!availableLayers.contains(layer)) {
                            logging::warn("Device {} doesn't support layer {}, skipping", physicalDeviceProperties.deviceName, layer);
                            success = false;
                        }
                    }

                    for (const auto& extension : requiredExtensions) {
                        if (!availableExtensions.contains(extension)) {
                            logging::warn("Device {} doesn't support extension {}, skipping", physicalDeviceProperties.deviceName, extension);
                            success = false;
                        }
                    }

                    if (!success) {
                        continue;
                    }

                    {
                        uint32_t queueFamilyCount = 0;
                        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

                        if (queueFamilyCount == 0) {
                            logging::warn("Device {} doesn't support queue families, skipping", physicalDeviceProperties.deviceName);
                            continue;
                        }

                        success = false;
                        std::vector<VkQueueFamilyProperties> family_properties(queueFamilyCount);
                        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, family_properties.data());
                        for (const auto& t : family_properties) {
                            if ((t.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                                success = true;
                            }
                        }
                    }

                    if (!success) {
                        continue;
                    }

                    if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                        priorityVectorToChooseDevice.insert(priorityVectorToChooseDevice.begin(), physicalDevice);
                    }
                }

                if (!priorityVectorToChooseDevice.empty()) {
                    this->_physicalDevice.vkPhysicalDevice = priorityVectorToChooseDevice[0];
                    {
                        uint32_t queueFamilyCount = 0;
                        vkGetPhysicalDeviceQueueFamilyProperties(this->_physicalDevice.vkPhysicalDevice, &queueFamilyCount, nullptr);
                        this->_physicalDevice.vkQueueFamilyProps.resize(queueFamilyCount);
                        vkGetPhysicalDeviceQueueFamilyProperties(this->_physicalDevice.vkPhysicalDevice, &queueFamilyCount, this->_physicalDevice.vkQueueFamilyProps.data());

                        vkGetPhysicalDeviceProperties(this->_physicalDevice.vkPhysicalDevice, &this->_physicalDevice.vkPhysicalDeviceProperties);
                        vkGetPhysicalDeviceProperties2(this->_physicalDevice.vkPhysicalDevice, &this->_physicalDevice.vkPhysicalDeviceProperties2);

                        uint32_t count = 0;
                        utility::vkCheck(vkEnumerateDeviceExtensionProperties(this->_physicalDevice.vkPhysicalDevice, nullptr, &count, nullptr), "Failed to get device ext");
                        this->_physicalDevice.vkDeviceExtensions.resize(count);
                        utility::vkCheck(vkEnumerateDeviceExtensionProperties(this->_physicalDevice.vkPhysicalDevice, nullptr, &count, this->_physicalDevice.vkDeviceExtensions.data()), "Failed to get device ext");

                        vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice.vkPhysicalDevice, &this->_physicalDevice.vkMemoryProperties);
                        vkGetPhysicalDeviceFeatures(this->_physicalDevice.vkPhysicalDevice, &this->_physicalDevice.vkPhysicalDeviceFeatures);

                        vkGetPhysicalDeviceQueueFamilyProperties(this->_physicalDevice.vkPhysicalDevice, &count, nullptr);
                        this->_physicalDevice.vkDeviceExtensions.resize(count);
                        vkGetPhysicalDeviceQueueFamilyProperties(this->_physicalDevice.vkPhysicalDevice, &count, this->_physicalDevice.vkQueueFamilyProps.data());

                        logging::info("Chosen physical device: {}, type: {}", this->_physicalDevice.vkPhysicalDeviceProperties.deviceName, static_cast<int>(this->_physicalDevice.vkPhysicalDeviceProperties.deviceType));
                    }

                    for (const auto& t : requiredLayers) {
                        this->tryIncludeDeviceLayer(availableLayers, t);
                    }

                    for (const auto& t : requiredExtensions) {
                        this->tryIncludeDeviceExtension(availableExtensions, t);
                    }

                    if constexpr (utility::checkOS(utility::MAC)) {
                        this->tryIncludeDeviceExtension(availableExtensions, EXT_VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME());
                    }
                } else {
                    throw std::runtime_error{"Failed to find physical device"};
                }
            }
        } else {
            throw std::runtime_error("Physical device not found");
        }
    }

    void CVulkanContext::initVulkanLogicalDevice(std::vector<CVulkanQueueFamilyCreationRequest>&& requiredQueueFamilies) {
        std::unordered_map<uint32_t, CVulkanQueueFamilyCreationRequest> queueFamilies = {};
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};
        for (const auto& t : requiredQueueFamilies) {
            const uint32_t queueFamilyIndex = this->findVulkanQueueFamily(t.bitMask);
            if (queueFamilyIndex == UINT32_MAX) {
                throw std::runtime_error(std::format("Graphics queue family not found: {}", t.bitMask));
            }
            queueFamilies.emplace(queueFamilyIndex, t);
            this->_queueFamiliesRegistry.registerQueueFamily(t.bitMask, t.queueCount, queueFamilyIndex);
        }
        queueCreateInfos.reserve(queueFamilies.size());
        for (const auto& [fst, snd] : queueFamilies) {
            const VkDeviceQueueCreateInfo queueCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = fst,
                .queueCount = snd.queueCount,
                .pQueuePriorities = snd.priorities.data(),
            };
            queueCreateInfos.emplace_back(queueCreateInfo);
        }

        VkDeviceCreateInfo vkDeviceCreateInfo = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        vkDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        vkDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

        vkDeviceCreateInfo.pEnabledFeatures = nullptr;
        vkDeviceCreateInfo.enabledExtensionCount = this->_physicalDevice.vkDeviceLrExtData.enabledExtensions.size();
        vkDeviceCreateInfo.enabledLayerCount = this->_physicalDevice.vkDeviceLrExtData.enabledLayers.size();

        std::vector<const char*> enabledLayerNames{};
        std::vector<const char*> enabledExtNames{};

        for (const auto& layer : this->_physicalDevice.vkDeviceLrExtData.enabledLayers)
        {
            enabledLayerNames.emplace_back(layer.c_str());
        }
        for (const auto& layer : this->_physicalDevice.vkDeviceLrExtData.enabledExtensions)
        {
            enabledExtNames.emplace_back(layer.c_str());
        }

        vkDeviceCreateInfo.ppEnabledExtensionNames = enabledExtNames.data();
        vkDeviceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();

        utility::vkCheck(vkCreateDevice(this->_physicalDevice.vkPhysicalDevice, &vkDeviceCreateInfo, nullptr, &this->_device.vkDevice));
        logging::info("Created logical device");
        for (const auto& t : this->_queueFamiliesRegistry.registeredData()) {
            logging::info("Queue family index: {}, queues: {}, bits: {}", t.queueCount, t.queueFamilyIndex, t.bitMask);
        }
    }

    uint32_t CVulkanContext::findVulkanQueueFamily(QueueFamilyBitMask bitmask) const {
        const bool checkPresentation = bitmask & CVulkanQueueFamilyBitMasks::PRESENT;
        bitmask &= ~CVulkanQueueFamilyBitMasks::PRESENT;
        for (uint32_t i = 0; i < this->_physicalDevice.vkQueueFamilyProps.size(); ++i) {
            const auto& queueFamily = this->_physicalDevice.vkQueueFamilyProps[i];
            if (checkPresentation) {
                VkBool32 flag = {};
                vkGetPhysicalDeviceSurfaceSupportKHR(this->_physicalDevice.vkPhysicalDevice, i, this->_surface.vkSurface(), &flag);
                if (!flag) {
                    continue;
                }
            }
            VkQueueFlags requiredQueueFlags = 0;
            if (bitmask & CVulkanQueueFamilyBitMasks::GRAPHICS) {
                requiredQueueFlags |= VK_QUEUE_GRAPHICS_BIT;
            }
            if (bitmask & CVulkanQueueFamilyBitMasks::COMPUTE) {
                requiredQueueFlags |= VK_QUEUE_COMPUTE_BIT;
            }
            if (bitmask & CVulkanQueueFamilyBitMasks::TRANSFER) {
                requiredQueueFlags |= VK_QUEUE_TRANSFER_BIT;
            }
            if ((queueFamily.queueFlags & requiredQueueFlags) == requiredQueueFlags) {
                return i;
            }
        }
        return UINT32_MAX;
    }

    void init(const window::CVWindow& window) {
        vulkanContext = std::make_unique<CVulkanContext>(window);
        vulkanContext->initVulkanInstance(utility::debug_mode,{},{});
        vulkanContext->initVulkanPhysicalDevice({}, {EXT_VK_KHR_SWAPCHAIN_EXTENSION_NAME()});
        vulkanContext->initVulkanLogicalDevice(
            std::vector<CVulkanQueueFamilyCreationRequest> {
                {
                    CVulkanQueueFamilyBitMasks::GRAPHICS,
                    1,
                    { 1.0f }
                },
                {
                    CVulkanQueueFamilyBitMasks::PRESENT,
                    1,
                    { 1.0f }
                }
            }
        );
        vulkanContext->surface().createSurface();

        {
            renderLoop::initRendering(*vulkanContext);
        }
    }

    void render() {
    }

    void cleanUp() {
        {
            renderLoop::destroyRendering();
        }
        vulkanContext.reset();
    }
}
