//
// Created by ded on 01.11.2026.
//

#pragma once
#include <vk_mem_alloc.h>

#include "utility/sys_utils.h"

namespace cvulkan::client::render::vk {
    class CVulkanInstance;
    class CVulkanPhysicalDevice;
    class CVulkanDevice;
}

namespace cvulkan::client::render::vk::memalloc {
    class CVulkan_VkMemoryAllocator {
    public:
        CVulkan_VkMemoryAllocator(const VkInstance& instance, const VkDevice& device, const VkPhysicalDevice& physicalDevice);
        ~CVulkan_VkMemoryAllocator();

        CLASS_NO_COPY_NO_MOVE(CVulkan_VkMemoryAllocator)

        [[nodiscard]] VmaAllocator vmaAllocator() const {
            return _vmaAllocator;
        }

    private:
        VmaAllocator _vmaAllocator {};
    };
}
