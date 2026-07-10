#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace cvulkan::client::renderer {
    class VulkanInstance {
        public:
            VulkanInstance() = default;
            ~VulkanInstance() {
                this->cleanUp();
            }

        VkInstance m_instance { };
        std::vector<VkPhysicalDevice> m_physicalDevices { };

        void cleanUp() const;
        void initValidationLayer();
        [[nodiscard]] VkResult initVulkanInstance();
    };

    extern std::unique_ptr<VulkanInstance> g_vulkanInstance;

    void init();
    void render();
    void cleanUp();
}
