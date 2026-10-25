#include "vulkan_render_core.h"
#include "../../utility/logger.h"
#include <unordered_set>

#include "GLFW/glfw3.h"

#include "vulkan_ext.h"
#include "vulkan_render_loop.h"

namespace cvulkan::client::render::vk {
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

            if constexpr (sys_utils::checkOS(sys_utils::MAC)) {
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

            VkApplicationInfo appInfo  {};
            VkInstanceCreateInfo instanceCreateInfo  {};
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
            vk::util::vkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &this->_vkInstance));
            logging::info("Created Vulkan instance");

            if (contextBuildData._debugMode && this->_vkInstanceLrExtData.hasVkInstanceRequiredExtension(EXT_VK_EXT_DEBUG_UTILS_EXTENSION_NAME())) {
                const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->_vkInstance, "vkCreateDebugUtilsMessengerEXT"));
                if (func == nullptr) {
                    throw std::runtime_error{"Failed to load vkCreateDebugUtilsMessengerEXT"};
                }
                vk::util::vkCheck(func(this->_vkInstance, &vk_debug_utils_messenger_create_info_ext, nullptr, &this->_vkDebugMessenger));
                logging::debug("Created Vulkan debug messenger");
            }
        }
    }

    CVulkanInstance::~CVulkanInstance() {
        if (this->_vkDebugMessenger != VK_NULL_HANDLE) {
            const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->_vkInstance, "vkDestroyDebugUtilsMessengerEXT"));
            if (func == nullptr) {
                logging::error("Failed to load vkDestroyDebugUtilsMessengerEXT");
            } else {
                func(this->_vkInstance, this->_vkDebugMessenger, nullptr);
            }
            logging::debug("Destroyed Vulkan debug messenger");
            this->_vkDebugMessenger = VK_NULL_HANDLE;
        }
        if (this->_vkInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(this->_vkInstance, nullptr);
            this->_vkInstance = VK_NULL_HANDLE;
            logging::info("Destroyed Vulkan instance");
        }
    }

    void CVulkanInstance::tryIncludeInstanceLayer(const std::unordered_set<std::string>& available, const std::string& layer) {
        if (available.contains(layer)) {
            this->_vkInstanceLrExtData._enabledLayers.insert(layer);
            logging::info("Enabled instance layer '{}'", layer);
        } else {
            logging::error("Instance layer '{}' is not available", layer);
        }
    }

    void CVulkanInstance::tryIncludeInstanceExtension(const std::unordered_set<std::string>& available, const std::string& ext) {
        if (available.contains(ext)) {
            this->_vkInstanceLrExtData._enabledExtensions.insert(ext);
            logging::info("Enabled instance extension '{}'", ext);
        } else {
            logging::error("Instance extension '{}' is not available", ext);
        }
    }

    CVulkanPhysicalDevice::~CVulkanPhysicalDevice() {
    }

    CVulkanPhysicalDevice::CVulkanPhysicalDevice(
        const CVulkanContextBuildData& contextBuildData,
        const CVulkanInstance& instance,
        const CVulkanPhysicalDeviceBuildData& physicalDeviceBuildData) {

        uint32_t physicalDeviceCount = 0;
        vk::util::vkCheck(vkEnumeratePhysicalDevices(instance.vkInstance(), &physicalDeviceCount, nullptr), "Failed to get physical devices");
        if (physicalDeviceCount > 0) {
            std::vector<VkPhysicalDevice> priorityVectorToChooseDevice{};
            std::vector<VkPhysicalDevice> physicalDevices{};
            physicalDevices.resize(physicalDeviceCount);
            vk::util::vkCheck(vkEnumeratePhysicalDevices(instance.vkInstance(), &physicalDeviceCount, physicalDevices.data()), "Failed to get physical devices");
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
                            logging::debug("Skipping device '{}': layer '{}' is not supported", physicalDeviceProperties.deviceName, layer);
                            success = false;
                        }
                    }

                    for (const auto& extension: physicalDeviceBuildData._requiredExtensions) {
                        if (!availableExtensions.contains(extension)) {
                            logging::debug("Skipping device '{}': extension '{}' is not supported", physicalDeviceProperties.deviceName, extension);
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
                            logging::debug("Skipping device '{}': required queue families are not supported", physicalDeviceProperties.deviceName);
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
                        vk::util::vkCheck(vkEnumerateDeviceExtensionProperties(this->_vkPhysicalDevice, nullptr, &count, nullptr), "Failed to get device ext");
                        this->_vkDeviceExtensions.resize(count);
                        vk::util::vkCheck(vkEnumerateDeviceExtensionProperties(this->_vkPhysicalDevice, nullptr, &count, this->_vkDeviceExtensions.data()), "Failed to get device ext");

                        vkGetPhysicalDeviceMemoryProperties(this->_vkPhysicalDevice, &this->_vkMemoryProperties);
                        vkGetPhysicalDeviceFeatures(this->_vkPhysicalDevice, &this->_vkPhysicalDeviceFeatures);

                        vkGetPhysicalDeviceQueueFamilyProperties(this->_vkPhysicalDevice, &count, nullptr);
                        this->_vkDeviceExtensions.resize(count);
                        vkGetPhysicalDeviceQueueFamilyProperties(this->_vkPhysicalDevice, &count, this->_vkQueueFamilyProps.data());

                        logging::info("Selected physical device '{}', type {}", this->_vkPhysicalDeviceProperties.deviceName, static_cast<int>(this->_vkPhysicalDeviceProperties.deviceType));
                    }

                    for (const auto& t: physicalDeviceBuildData._requiredLayers) {
                        this->tryIncludeDeviceLayer(availableLayers, t);
                    }

                    for (const auto& t: physicalDeviceBuildData._requiredExtensions) {
                        this->tryIncludeDeviceExtension(availableExtensions, t);
                    }

                    if constexpr (sys_utils::checkOS(sys_utils::MAC)) {
                        this->tryIncludeDeviceExtension(availableExtensions, EXT_VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME());
                    }
                } else {
                    throw std::runtime_error{"Failed to find a suitable physical device"};
                }
            }
        } else {
            throw std::runtime_error("No suitable Vulkan physical device found");
        }
    }

    void CVulkanPhysicalDevice::tryIncludeDeviceLayer(const std::unordered_set<std::string>& available, const std::string& layer) {
        if (available.contains(layer)) {
            this->_vkDeviceLrExtData._enabledLayers.insert(layer);
            logging::info("Enabled device layer '{}'", layer);
        } else {
            logging::error("Device layer '{}' is not available", layer);
        }
    }

    void CVulkanPhysicalDevice::tryIncludeDeviceExtension(const std::unordered_set<std::string>& available, const std::string& ext) {
        if (available.contains(ext)) {
            this->_vkDeviceLrExtData._enabledExtensions.insert(ext);
            logging::info("Enabled device extension '{}'", ext);
        } else {
            logging::error("Device extension '{}' is not available", ext);
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
                throw std::runtime_error(std::format("Graphics queue family not found (queue flags: {})", t._bitMask));
            }
            queueFamilies.emplace(queueFamilyIndex, t);
            this->_queueFamiliesRegistry.registerQueueFamily(t._bitMask, t._queueCount, queueFamilyIndex);
        }
        queueCreateInfos.reserve(queueFamilies.size());
        for (const auto& [fst, snd]: queueFamilies) {
            const VkDeviceQueueCreateInfo queueCreateInfo  {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = fst,
                .queueCount = snd._queueCount,
                .pQueuePriorities = snd._priorities.data(),
            };
            queueCreateInfos.emplace_back(queueCreateInfo);
        }

        VkDeviceCreateInfo vkDeviceCreateInfo  {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        vkDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        vkDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

        VkPhysicalDeviceVulkan13Features features13  {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE
        };
        VkPhysicalDeviceFeatures2 features2  {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &features13
        };

        if (physicalDevice.vkPhysicalDeviceFeatures().samplerAnisotropy) {
            this->_supportedSamplerAnisotropy = true;
            features2.features.samplerAnisotropy = VK_TRUE;
        }

        vkDeviceCreateInfo.pNext = &features2;
        vkDeviceCreateInfo.pEnabledFeatures = nullptr;
        vkDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(physicalDevice.vkDeviceLayersExtensionsData()._enabledExtensions.size());
        vkDeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(physicalDevice.vkDeviceLayersExtensionsData()._enabledLayers.size());

        std::vector<const char*> enabledLayerNames{};
        std::vector<const char*> enabledExtNames{};

        for (const auto& layer: physicalDevice.vkDeviceLayersExtensionsData()._enabledLayers) {
            enabledLayerNames.emplace_back(layer.c_str());
        }
        for (const auto& layer: physicalDevice.vkDeviceLayersExtensionsData()._enabledExtensions) {
            enabledExtNames.emplace_back(layer.c_str());
        }

        vkDeviceCreateInfo.ppEnabledExtensionNames = enabledExtNames.data();
        vkDeviceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();

        vk::util::vkCheck(vkCreateDevice(physicalDevice.vkPhysicalDevice(), &vkDeviceCreateInfo, nullptr, &this->_vkDevice));

        for (const auto& t: this->_queueFamiliesRegistry.registeredData()) {
            logging::info("Available queue family {}: {} queue(s), flags {}", t._queueFamilyIndex, t._queueCount, t._bitMask);
        }
        logging::info("Created Vulkan device");
    }

    CVulkanDevice::~CVulkanDevice() {
        if (this->_vkDevice != VK_NULL_HANDLE) {
            this->deviceWaitIdle();
            vkDestroyDevice(this->_vkDevice, nullptr);
            this->_vkDevice = VK_NULL_HANDLE;
            logging::info("Destroyed Vulkan device");
        }
    }




    CVulkanContext::CVulkanContext(
        const window::CVulkanWindow& window,
        const CVulkanContextBuildData& contextBuildData,
        const CVulkanInstanceBuildData& instanceBuildData,
        const CVulkanPhysicalDeviceBuildData& physicalDeviceBuildData,
        const CVulkanLogicalDeviceBuildData& logicalDeviceBuildData)
    : _glfwWindow{window},
    _instance{contextBuildData, instanceBuildData},
    _physicalDevice(contextBuildData, this->_instance, physicalDeviceBuildData),
    _surface{std::make_unique<CVulkanSurface>(*this)},
    _device{contextBuildData, this->_physicalDevice, this->_surface->vkSurface(), logicalDeviceBuildData},
    _swapChain{std::make_unique<CVulkanSwapChain>(*this, this->_surface->vkSurface(), this->_surface->vkSurfaceCapabilities(), this->_surface->vkFormat(), this->_surface->vkColorSpace())},
    _pipelineCache{*this} {
    }

    void CVulkanContext::resize() {
        {
            this->_swapChain.reset();
            this->_surface.reset();
        }
        {
            this->_surface = std::make_unique<CVulkanSurface>(*this);
            this->_swapChain = std::make_unique<CVulkanSwapChain>(*this, this->_surface->vkSurface(), this->_surface->vkSurfaceCapabilities(), this->_surface->vkFormat(), this->_surface->vkColorSpace());
        }
    }

    std::unordered_set<std::string> availableInstanceExtensions() {
        std::unordered_set<std::string> set{};
        uint32_t extensionsCount = 0;
        std::vector<VkExtensionProperties> extensionProperties{};
        vk::util::vkCheck(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr), "Failed to get instance ext");
        if (extensionsCount == 0) {
            logging::error("No Vulkan instance extensions available");
            return set;
        }
        logging::debug("Found {} instance extensions", extensionsCount);
        extensionProperties.resize(extensionsCount);
        vk::util::vkCheck(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensionProperties.data()), "Failed to get instance ext");
        for (uint32_t i = 0; i < extensionsCount; i++) {
            const VkExtensionProperties& prop = extensionProperties[i];
            logging::trace("Instance extension '{}'", prop.extensionName);
            set.emplace(prop.extensionName);
        }
        return set;
    }

    std::unordered_set<std::string> availableInstanceLayers() {
        std::unordered_set<std::string> set{};
        uint32_t layerCount = 0;
        std::vector<VkLayerProperties> layerProperties{};
        vk::util::vkCheck(vkEnumerateInstanceLayerProperties(&layerCount, nullptr), "Failed to enumerate Vulkan instance layers");
        if (layerCount == 0) {
            logging::warn("No Vulkan instance layers available");
            return set;
        }
        logging::debug("Found {} Vulkan instance layers", layerCount);
        layerProperties.resize(layerCount);
        vk::util::vkCheck(vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data()), "Failed to enumerate Vulkan instance layers");
        for (uint32_t i = 0; i < layerCount; i++) {
            const VkLayerProperties& prop = layerProperties[i];
            logging::trace("Vulkan instance layer: '{}'", prop.layerName);
            set.emplace(prop.layerName);
        }
        return set;
    }

    std::unordered_set<std::string> availableDeviceExtensions(const VkPhysicalDevice& device) {
        std::unordered_set<std::string> set{};
        uint32_t extensionsCount = 0;
        std::vector<VkExtensionProperties> extensionProperties{};
        VkPhysicalDeviceProperties properties {};
        vkGetPhysicalDeviceProperties(device, &properties);
        vk::util::vkCheck(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr), std::format("Failed to get device extensions for '{}'", properties.deviceName));
        if (extensionsCount == 0) {
            logging::error("No Vulkan device extensions available for '{}'", properties.deviceName);
            return set;
        }
        logging::debug("Found {} device extensions for '{}'", extensionsCount, properties.deviceName);
        extensionProperties.resize(extensionsCount);
        vk::util::vkCheck(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensionProperties.data()), std::format("Failed to get device extensions for '{}'", properties.deviceName));
        for (uint32_t i = 0; i < extensionsCount; i++) {
            const VkExtensionProperties& prop = extensionProperties[i];
            set.emplace(prop.extensionName);
        }
        return set;
    }

    std::unordered_set<std::string> availableDeviceLayers(const VkPhysicalDevice& device) {
        std::unordered_set<std::string> set{};
        uint32_t layerCount = 0;
        std::vector<VkLayerProperties> layerProperties{};
        VkPhysicalDeviceProperties properties {};
        vkGetPhysicalDeviceProperties(device, &properties);
        util::vkCheck(vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr), std::format("Failed to get device layers for '{}'", properties.deviceName));
        if (layerCount == 0) {
            logging::error("No Vulkan device layers available for '{}'", properties.deviceName);
            return set;
        }
        logging::debug("Found {} device layers for '{}'", layerCount, properties.deviceName);
        layerProperties.resize(layerCount);
        util::vkCheck(vkEnumerateDeviceLayerProperties(device, &layerCount, layerProperties.data()), std::format("Failed to get device layers for '{}'", properties.deviceName));
        for (uint32_t i = 0; i < layerCount; i++) {
            const VkLayerProperties& prop = layerProperties[i];
            set.emplace(prop.layerName);
        }
        return set;
    }

    std::unordered_set<std::string> getGLFWExtensions() {
        std::unordered_set<std::string> set{};
        uint32_t countGlfwExtensions = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&countGlfwExtensions);
        if (extensions == nullptr) {
            logging::error("Failed to get required GLFW Vulkan extensions");
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
            sys_utils::debug_mode
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

    uint32_t util::getMemoryTypeIndexFromProperties(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t typeBits, const uint32_t& reqMask) {
        const VkMemoryType* vkMemoryTypes = memoryProperties.memoryTypes;
        uint32_t result = UINT32_MAX;
        for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
            if ((typeBits & 1) && (vkMemoryTypes[i].propertyFlags & reqMask) == reqMask) {
                result = i;
                break;
            }
            typeBits >>= 1;
        }
        if (result == UINT32_MAX) {
            throw std::runtime_error("Failed to find suitable Vulkan memory type");
        }
        return result;
    }
}
