//
// Created by gltex on 07.09.2026.
//

#include "vulkan_syncronization.h"

namespace cvulkan::client::renderer {
    void CVulkanSemaphore::create() {
        const VkSemaphoreCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        utility::vkCheck(vkCreateSemaphore(this->context->vk_device_data().vkDevice, &createInfo, nullptr, &this->semaphore),"Failed to create semaphore");
    }

    void CVulkanSemaphore::destroy() const {
        vkDestroySemaphore(this->context->vk_device_data().vkDevice, this->semaphore, nullptr);
    }

    void CVulkanFence::create(bool signaled) {
        const uint32_t sign = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        const VkFenceCreateInfo vk_fence_create_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = sign,
        };

        utility::vkCheck(vkCreateFence(this->context->vk_device_data().vkDevice, &vk_fence_create_info, nullptr, &this->fence), "Failed to create fence");
    }

    void CVulkanFence::destroy() const {
        vkDestroyFence(this->context->vk_device_data().vkDevice, this->fence, nullptr);
    }

    void CVulkanFence::wait() {
        vkWaitForFences(this->context->vk_device_data().vkDevice, 1, &this->fence, true, UINT64_MAX);
    }

    void CVulkanFence::reset() {
        vkResetFences(this->context->vk_device_data().vkDevice, 1, &this->fence);
    }
}
