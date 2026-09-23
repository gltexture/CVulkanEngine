#include "vulkan_render_core.h"
#include "util/logger.h"
#include <unordered_set>

#include "GLFW/glfw3.h"
#include "vulkan_utility.h"
#include "vulkan_ext.h"
#include "vulkan_render_loop.h"

namespace cvulkan::client::render::core {
    std::unique_ptr<CVulkanContext> vulkanContext;

    CVulkanInstance::CVulkanInstance(const CVulkanContextBuildData& contextBuildData, const CVulkanInstanceBuildData& instanceBuildData) {
        bool USE_PORTABILITY_MODE = false;
        const std::unordered_set<std::string> setOfExtensions = availableInstanceExtensions();
        const std::unordered_set<std::string> setOfLayers = availableInstanceLayers();
        {
            for (const auto& t: instanceBuildData._requiredLayers) {
                this->tryIncludeInstanceLayer(setOfLayers, t);
            }
            for (const auto& t: instanceBuildData._requiredExtensions) {
                this->tryIncludeInstanceExtension(setOfExtensions, t);
            }

            if (contextBuildData._debugMode) {
                this->tryIncludeInstanceLayer(setOfLayers, LR_VK_LAYER_KHRONOS_validation());
                this->tryIncludeInstanceExtension(setOfExtensions, EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME());
            }

            for (const auto glfwExtensions = getGLFWExtensions(); const auto& extension: glfwExtensions) {
                this->tryIncludeInstanceExtension(setOfExtensions, extension);
            }

            if constexpr (utility::checkOS(utility::MAC)) {
                this->tryIncludeInstanceExtension(setOfExtensions, EXT_VK_KHR_portability_enumeration());
                USE_PORTABILITY_MODE = this->_vkInstanceLrExtData.hasVkInstanceRequiredExtension(EXT_VK_KHR_portability_enumeration());
            }
        }

        {
            std::vector<const char *> enabledLayerNames{};
            std::vector<const char *> enabledExtNames{};

            for (const auto& layer: this->_vkInstanceLrExtData._enabledLayers) {
                enabledLayerNames.emplace_back(layer.c_str());
            }
            for (const auto& layer: this->_vkInstanceLrExtData._enabledExtensions) {
                enabledExtNames.emplace_back(layer.c_str());
            }

            VkApplicationInfo appInfo = {};
            VkInstanceCreateInfo instanceCreateInfo = {};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "CVulkan";
            appInfo.apiVersion = VK_API_VERSION_1_3;
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instanceCreateInfo.pApplicationInfo = &appInfo;
            if (USE_PORTABILITY_MODE) {
                instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            }
            instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(this->_vkInstanceLrExtData._enabledExtensions.size());
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size());
            instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
            instanceCreateInfo.ppEnabledExtensionNames = enabledExtNames.data();
            VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info_ext{};
            if (contextBuildData._debugMode && this->_vkInstanceLrExtData.hasVkInstanceRequiredExtension(EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME())) {
                vk_debug_utils_messenger_create_info_ext = ext::createDebugMessengerCreateInfo();
                instanceCreateInfo.pNext = &vk_debug_utils_messenger_create_info_ext;
            }
            utility::vkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &this->_vkInstance));
            logging::info("Successfully created vulkan instance!");

            if (contextBuildData._debugMode && this->_vkInstanceLrExtData.hasVkInstanceRequiredExtension(EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME())) {
                const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->_vkInstance, "vkCreateDebugUtilsMessengerEXT"));
                if (func == nullptr) {
                    throw std::runtime_error{"Failed to load vkCreateDebugUtilsMessengerEXT"};
                }
                utility::vkCheck(func(this->_vkInstance, &vk_debug_utils_messenger_create_info_ext, nullptr, &this->_vkDebugMessenger));
                logging::debug("Created debug messenger");
            }
        }
    }

    CVulkanInstance::~CVulkanInstance() {
        if (this->_vkDebugMessenger != VK_NULL_HANDLE) {
            const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->_vkInstance, "vkDestroyDebugUtilsMessengerEXT"));
            if (func == nullptr) {
                logging::error("Failed to get vkDestroyDebugUtilsMessengerEXT");
            } else {
                func(this->_vkInstance, this->_vkDebugMessenger, nullptr);
            }
            logging::debug("Vulkan debug messenger destroyed");
            this->_vkDebugMessenger = VK_NULL_HANDLE;
        }
        if (this->_vkInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(this->_vkInstance, nullptr);
            this->_vkInstance = VK_NULL_HANDLE;
            logging::info("Destroyed vkInstance");
        }
    }

    void CVulkanInstance::tryIncludeInstanceLayer(const std::unordered_set<std::string>& available, const std::string& layer) {
        if (available.contains(layer)) {
            this->_vkInstanceLrExtData._enabledLayers.insert(layer);
            logging::info("Enabled instance Layer {}", layer);
        } else {
            logging::error("Instance Layer {} is not available!", layer);
        }
    }

    void CVulkanInstance::tryIncludeInstanceExtension(const std::unordered_set<std::string>& available, const std::string& ext) {
        if (available.contains(ext)) {
            this->_vkInstanceLrExtData._enabledExtensions.insert(ext);
            logging::info("Enabled instance extension {}", ext);
        } else {
            logging::error("Instance extension {} is not available!", ext);
        }
    }

    CVulkanPhysicalDevice::~CVulkanPhysicalDevice() {
    }

    CVulkanPhysicalDevice::CVulkanPhysicalDevice(
        const CVulkanContextBuildData& contextBuildData,
        const CVulkanInstance& instance,
        const CVulkanPhysicalDeviceBuildData& physicalDeviceBuildData) {

        uint32_t physicalDeviceCount = 0;
        utility::vkCheck(vkEnumeratePhysicalDevices(instance.vkInstance(), &physicalDeviceCount, nullptr), "Failed to get physical devices");
        if (physicalDeviceCount > 0) {
            std::vector<VkPhysicalDevice> priorityVectorToChooseDevice{};
            std::vector<VkPhysicalDevice> physicalDevices{};
            physicalDevices.resize(physicalDeviceCount);
            utility::vkCheck(vkEnumeratePhysicalDevices(instance.vkInstance(), &physicalDeviceCount, physicalDevices.data()), "Failed to get physical devices");
            {
                std::unordered_set<std::string> availableLayers{};
                std::unordered_set<std::string> availableExtensions{};

                for (const auto& physicalDevice: physicalDevices) {
                    bool success = true;
                    VkPhysicalDeviceProperties physicalDeviceProperties{};
                    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

                    availableLayers = availableDeviceLayers(physicalDevice);
                    availableExtensions = availableDeviceExtensions(physicalDevice);

                    for (const auto& layer: physicalDeviceBuildData._requiredLayers) {
                        if (!availableLayers.contains(layer)) {
                            logging::warn("Device {} doesn't support layer {}, skipping", physicalDeviceProperties.deviceName, layer);
                            success = false;
                        }
                    }

                    for (const auto& extension: physicalDeviceBuildData._requiredExtensions) {
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
                        for (const auto& t: family_properties) {
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
                    this->_vkPhysicalDevice = priorityVectorToChooseDevice[0];
                    {
                        uint32_t queueFamilyCount = 0;
                        vkGetPhysicalDeviceQueueFamilyProperties(this->_vkPhysicalDevice, &queueFamilyCount, nullptr);
                        this->_vkQueueFamilyProps.resize(queueFamilyCount);
                        vkGetPhysicalDeviceQueueFamilyProperties(this->_vkPhysicalDevice, &queueFamilyCount, this->_vkQueueFamilyProps.data());

                        vkGetPhysicalDeviceProperties(this->_vkPhysicalDevice, &this->_vkPhysicalDeviceProperties);
                        vkGetPhysicalDeviceProperties2(this->_vkPhysicalDevice, &this->_vkPhysicalDeviceProperties2);

                        uint32_t count = 0;
                        utility::vkCheck(vkEnumerateDeviceExtensionProperties(this->_vkPhysicalDevice, nullptr, &count, nullptr), "Failed to get device ext");
                        this->_vkDeviceExtensions.resize(count);
                        utility::vkCheck(vkEnumerateDeviceExtensionProperties(this->_vkPhysicalDevice, nullptr, &count, this->_vkDeviceExtensions.data()), "Failed to get device ext");

                        vkGetPhysicalDeviceMemoryProperties(this->_vkPhysicalDevice, &this->_vkMemoryProperties);
                        vkGetPhysicalDeviceFeatures(this->_vkPhysicalDevice, &this->_vkPhysicalDeviceFeatures);

                        vkGetPhysicalDeviceQueueFamilyProperties(this->_vkPhysicalDevice, &count, nullptr);
                        this->_vkDeviceExtensions.resize(count);
                        vkGetPhysicalDeviceQueueFamilyProperties(this->_vkPhysicalDevice, &count, this->_vkQueueFamilyProps.data());

                        logging::info("Chosen physical device: {}, type: {}", this->_vkPhysicalDeviceProperties.deviceName, static_cast<int>(this->_vkPhysicalDeviceProperties.deviceType));
                    }

                    for (const auto& t: physicalDeviceBuildData._requiredLayers) {
                        this->tryIncludeDeviceLayer(availableLayers, t);
                    }

                    for (const auto& t: physicalDeviceBuildData._requiredExtensions) {
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

    void CVulkanPhysicalDevice::tryIncludeDeviceLayer(const std::unordered_set<std::string>& available, const std::string& layer) {
        if (available.contains(layer)) {
            this->_vkDeviceLrExtData._enabledLayers.insert(layer);
            logging::info("Enabled device Layer {}", layer);
        } else {
            logging::error("Device Layer {} is not available!", layer);
        }
    }

    void CVulkanPhysicalDevice::tryIncludeDeviceExtension(const std::unordered_set<std::string>& available, const std::string& ext) {
        if (available.contains(ext)) {
            this->_vkDeviceLrExtData._enabledExtensions.insert(ext);
            logging::info("Enabled device extension {}", ext);
        } else {
            logging::error("Device extension {} is not available!", ext);
        }
    }

    CVulkanDevice::CVulkanDevice(
        const CVulkanContextBuildData& contextBuildData,
        const CVulkanPhysicalDevice& physicalDevice,
        VkSurfaceKHR surface,
        const CVulkanLogicalDeviceBuildData& logicalDeviceBuildData) {
        std::unordered_map<uint32_t, CVulkanQueueFamilyCreationRequest> queueFamilies{};
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
        for (const auto& t: logicalDeviceBuildData._queueFamilyCreationRequests) {
            const uint32_t queueFamilyIndex = findVulkanQueueFamily(t._bitMask, physicalDevice, surface);
            if (queueFamilyIndex == UINT32_MAX) {
                throw std::runtime_error(std::format("Graphics queue family not found: {}", t._bitMask));
            }
            queueFamilies.emplace(queueFamilyIndex, t);
            this->_queueFamiliesRegistry.registerQueueFamily(t._bitMask, t._queueCount, queueFamilyIndex);
        }
        queueCreateInfos.reserve(queueFamilies.size());
        for (const auto& [fst, snd]: queueFamilies) {
            const VkDeviceQueueCreateInfo queueCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = fst,
                .queueCount = snd._queueCount,
                .pQueuePriorities = snd._priorities.data(),
            };
            queueCreateInfos.emplace_back(queueCreateInfo);
        }

        VkDeviceCreateInfo vkDeviceCreateInfo = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        vkDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        vkDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

        VkPhysicalDeviceVulkan13Features features13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .synchronization2 = true,
            .dynamicRendering = true
        };
        VkPhysicalDeviceFeatures2 features2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &features13
        };

        vkDeviceCreateInfo.pNext = &features2;
        vkDeviceCreateInfo.pEnabledFeatures = nullptr;
        vkDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(physicalDevice.vkDeviceLayersExtensionsData()._enabledExtensions.size());
        vkDeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(physicalDevice.vkDeviceLayersExtensionsData()._enabledLayers.size());

        std::vector<const char *> enabledLayerNames{};
        std::vector<const char *> enabledExtNames{};

        for (const auto& layer: physicalDevice.vkDeviceLayersExtensionsData()._enabledLayers) {
            enabledLayerNames.emplace_back(layer.c_str());
        }
        for (const auto& layer: physicalDevice.vkDeviceLayersExtensionsData()._enabledExtensions) {
            enabledExtNames.emplace_back(layer.c_str());
        }

        vkDeviceCreateInfo.ppEnabledExtensionNames = enabledExtNames.data();
        vkDeviceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();

        utility::vkCheck(vkCreateDevice(physicalDevice.vkPhysicalDevice(), &vkDeviceCreateInfo, nullptr, &this->_vkDevice));

        logging::info("vkDevice:");
        for (const auto& t: this->_queueFamiliesRegistry.registeredData()) {
            logging::info("Queue family index: {}, queues: {}, bits: {}", t._queueCount, t._queueFamilyIndex, t._bitMask);
        }
        logging::info("Created vkDevice");
    }

    CVulkanDevice::~CVulkanDevice() {
        if (this->_vkDevice != VK_NULL_HANDLE) {
            this->deviceWaitIdle();
            vkDestroyDevice(this->_vkDevice, nullptr);
            this->_vkDevice = VK_NULL_HANDLE;
            logging::info("Destroyed vkDevice");
        }
    }




    CVulkanContext::CVulkanContext(
        const window::CVulkanWindow& window,
        const CVulkanContextBuildData& contextBuildData,
        const CVulkanInstanceBuildData& instanceBuildData,
        const CVulkanPhysicalDeviceBuildData& physicalDeviceBuildData,
        const CVulkanLogicalDeviceBuildData& logicalDeviceBuildData)
    : _glfwWindow{window},
    _instance(contextBuildData, instanceBuildData),
    _physicalDevice(contextBuildData, this->_instance, physicalDeviceBuildData),
    _surface{*this},
    _device(contextBuildData, this->_physicalDevice, this->_surface.vkSurface(), logicalDeviceBuildData),
    _swapChain{*this, this->_surface.vkSurface(), this->_surface.vkSurfaceCapabilities(), this->_surface.vkFormat(), this->_surface.vkColorSpace()},
    _pipelineCache(*this) {
    }

    std::unordered_set<std::string> availableInstanceExtensions() {
        std::unordered_set<std::string> set{};
        uint32_t extensionsCount = 0;
        std::vector<VkExtensionProperties> extensionProperties{};
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

    std::unordered_set<std::string> availableInstanceLayers() {
        std::unordered_set<std::string> set{};
        uint32_t layerCount = 0;
        std::vector<VkLayerProperties> layerProperties{};
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

    std::unordered_set<std::string> availableDeviceExtensions(const VkPhysicalDevice& device) {
        std::unordered_set<std::string> set{};
        uint32_t extensionsCount = 0;
        std::vector<VkExtensionProperties> extensionProperties{};
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

    std::unordered_set<std::string> availableDeviceLayers(const VkPhysicalDevice& device) {
        std::unordered_set<std::string> set{};
        uint32_t layerCount = 0;
        std::vector<VkLayerProperties> layerProperties{};
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

    std::unordered_set<std::string> getGLFWExtensions() {
        std::unordered_set<std::string> set{};
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

    uint32_t findVulkanQueueFamily(QueueFamilyBitMask bitmask, const CVulkanPhysicalDevice& device, const VkSurfaceKHR surface) {
        const bool checkPresentation = bitmask & CVulkanQueueFamilyBitMasks::PRESENT;
        bitmask &= ~CVulkanQueueFamilyBitMasks::PRESENT;
        for (uint32_t i = 0; i < device.vkQueueFamilyProps().size(); ++i) {
            const auto& queueFamily = device.vkQueueFamilyProps()[i];
            if (checkPresentation) {
                VkBool32 flag{};
                vkGetPhysicalDeviceSurfaceSupportKHR(device.vkPhysicalDevice(), i, surface, &flag);
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

    void runRender() {
        loop::runRendering();
    }

    void createRenderCore(const window::CVulkanWindow& window) {
        const CVulkanContextBuildData contextBuildData {
            utility::debug_mode
        };

        const CVulkanInstanceBuildData instanceBuildData {
            {},
            {}
        };

        const CVulkanPhysicalDeviceBuildData physicalDeviceBuildData {
            {},
            {EXT_VK_KHR_SWAPCHAIN_EXTENSION_NAME()}
        };

        const CVulkanLogicalDeviceBuildData logicalDeviceBuildData {
            std::vector<CVulkanQueueFamilyCreationRequest>{
                    {
                        CVulkanQueueFamilyBitMasks::GRAPHICS,
                        1,
                        {1.0f}
                    },
                    {
                        CVulkanQueueFamilyBitMasks::PRESENT,
                        1,
                        {1.0f}
                    }
            }
        };

        vulkanContext = std::make_unique<CVulkanContext>(
            window,
            contextBuildData,
            instanceBuildData,
            physicalDeviceBuildData,
            logicalDeviceBuildData
        );
        loop::createRendering(*vulkanContext);
    }

    void cleanRenderCore() {
        vulkanContext->device().deviceWaitIdle();
        {
            loop::destroyRendering();
        }
        vulkanContext.reset();
    }
}