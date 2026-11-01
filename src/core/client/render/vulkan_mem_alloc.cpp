//
// Created by ded on 01.11.2026.
//

#include "vulkan_mem_alloc.h"
#include "vulkan_render_core.h"

#define VMA_STATIC_VULKAN_FUNCTIONS 1
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

namespace cvulkan::client::render::vk::memalloc {
    CVulkan_VkMemoryAllocator::CVulkan_VkMemoryAllocator(const VkInstance& instance, const VkDevice& device, const VkPhysicalDevice& physicalDevice) {
        constexpr VmaVulkanFunctions vulkanFunctions {
            .vkGetInstanceProcAddr = &vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = &vkGetDeviceProcAddr
        };
        const VmaAllocatorCreateInfo allocatorCreateInfo {
            .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
            .physicalDevice = physicalDevice,
            .device = device,
            .pVulkanFunctions = &vulkanFunctions,
            .instance = instance,
            .vulkanApiVersion = CVULKAN_VK_API_VER
        };

        util::vkCheck(vmaCreateAllocator(&allocatorCreateInfo, &this->_vmaAllocator), "Failed to create VMA allocator");
        logging::info("Created VMA");
    }

    CVulkan_VkMemoryAllocator::~CVulkan_VkMemoryAllocator() {
        if (this->_vmaAllocator != VK_NULL_HANDLE) {
            vmaDestroyAllocator(this->_vmaAllocator);
            this->_vmaAllocator = VK_NULL_HANDLE;
            logging::info("Destroyed VMA");
        }
    }
}
