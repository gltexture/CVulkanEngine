//
// Created by gltex on 07.09.2026.
//

#include "vulkan_syncronization.h"

namespace cvulkan::client::renderer {
    void CVulkanSemaphore::createSemaphore() {
        const VkSemaphoreCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        utility::vkCheck(vkCreateSemaphore(this->_context.deviceData().vkDevice, &createInfo, nullptr, &this->_semaphore),"Failed to createSemaphore _semaphore");
    }

    void CVulkanSemaphore::destroySemaphore() const {
        vkDestroySemaphore(this->_context.deviceData().vkDevice, this->_semaphore, nullptr);
    }

    void CVulkanFence::createFence(bool signaled) {
        const uint32_t sign = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        const VkFenceCreateInfo vk_fence_create_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = sign,
        };

        utility::vkCheck(vkCreateFence(this->_context.deviceData().vkDevice, &vk_fence_create_info, nullptr, &this->_fence), "Failed to createSemaphore _fence");
    }

    void CVulkanFence::destroyFence() const {
        vkDestroyFence(this->_context.deviceData().vkDevice, this->_fence, nullptr);
    }

    void CVulkanFence::wait() const {
        vkWaitForFences(this->_context.deviceData().vkDevice, 1, &this->_fence, true, UINT64_MAX);
    }

    void CVulkanFence::reset() const {
        vkResetFences(this->_context.deviceData().vkDevice, 1, &this->_fence);
    }
}
