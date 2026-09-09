//
// Created by gltex on 07.09.2026.
//

#include "vulkan_synchronization.h"

#include "vulkan_render_core.h"
#include "vulkan_utility.h"

namespace cvulkan::client::renderSync {
    void CVulkanSemaphore::createSemaphore() {
        const VkSemaphoreCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        utility::vkCheck(vkCreateSemaphore(this->_context.deviceData().vkDevice, &createInfo, nullptr, &this->_vkSemaphore),"Failed to create semaphore");
    }

    void CVulkanSemaphore::destroySemaphore() {
        if (this->_vkSemaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(this->_context.deviceData().vkDevice, this->_vkSemaphore, nullptr);
            this->_vkSemaphore = VK_NULL_HANDLE;
        }
    }

    void CVulkanFence::createFence(const bool signaled) {
        const uint32_t sign = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        const VkFenceCreateInfo vk_fence_create_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = sign,
        };
        utility::vkCheck(vkCreateFence(this->_context.deviceData().vkDevice, &vk_fence_create_info, nullptr, &this->_vkFence), "Failed to create fence");
    }

    void CVulkanFence::destroyFence() {
        if (this->_vkFence != VK_NULL_HANDLE) {
            vkDestroyFence(this->_context.deviceData().vkDevice, this->_vkFence, nullptr);
            this->_vkFence = VK_NULL_HANDLE;
        }
    }

    void CVulkanFence::wait() const {
        vkWaitForFences(this->_context.deviceData().vkDevice, 1, &this->_vkFence, true, UINT64_MAX);
    }

    void CVulkanFence::reset() const {
        vkResetFences(this->_context.deviceData().vkDevice, 1, &this->_vkFence);
    }
}
