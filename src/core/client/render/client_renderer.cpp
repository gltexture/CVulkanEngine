#include "client_renderer.h"
#include <vulkan/vulkan.h>

#include "client_render_config.h"

namespace cvulkan::client::renderer {
    std::unique_ptr<VulkanInstance> g_vulkanInstance;

    void VulkanInstance::cleanUp() const {
        if (this->m_instance != nullptr) {
            vkDestroyInstance(this->m_instance, nullptr);
        }
    }

    void VulkanInstance::initValidationLayer() {
        uint32_t layerCount = 0;

        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    }

    VkResult VulkanInstance::initVulkanInstance() {
        VkResult result = VK_SUCCESS;
        VkApplicationInfo appInfo = { };
        VkInstanceCreateInfo instanceCreateInfo = { };

        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "CVulkan";
        appInfo.apiVersion = VK_API_VERSION_1_3;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;

        result = vkCreateInstance(&instanceCreateInfo, nullptr, &this->m_instance);
        if (result == VK_SUCCESS) {
            uint32_t physicalDeviceCount = 0;
            vkEnumeratePhysicalDevices(this->m_instance, &physicalDeviceCount, nullptr);

            this->m_physicalDevices.resize(physicalDeviceCount);
            vkEnumeratePhysicalDevices(this->m_instance, &physicalDeviceCount, &this->m_physicalDevices[0]);
        }

        {
            if (config::VK_USE_VALIDATION_LAYER) {
                this->initValidationLayer();
            }
        }
        return result;
    }

    void init() {
        g_vulkanInstance = std::make_unique<VulkanInstance>();
    }

    void render() {
    }

    void cleanUp() {
        if (const auto* vulkanInst = g_vulkanInstance.release(); vulkanInst != nullptr) {
            delete vulkanInst;
        }
    }
}
