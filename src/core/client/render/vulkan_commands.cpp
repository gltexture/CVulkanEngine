//
// Created by gltex on 07.09.2026.
//

#include "vulkan_commands.h"

#include "vulkan_synchronization.h"

namespace cvulkan::client::renderCore {
    void CVulkanCommandPool::initCommandPool() {
        logging::info("Creating command pool");
        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = this->_queueFamilyIndex,
        };
        if (this->_supportReset) {
            commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        }
        utility::vkCheck(vkCreateCommandPool(this->_context.deviceData().vkDevice, &commandPoolCreateInfo, nullptr, &this->_vkCommandPool), "Failed to create command pool");
    }

    void CVulkanCommandPool::destroyCommandPool() const {
        logging::info("Destroying command pool");
        vkDestroyCommandPool(this->_context.deviceData().vkDevice, this->_vkCommandPool, nullptr);
    }

    void CVulkanCommandPool::reset() const {
        logging::info("Resetting command pool");
        vkResetCommandPool(this->_context.deviceData().vkDevice, this->_vkCommandPool, 0);
    }


    void CVulkanCommandBuffer::beginRecording(const CVulkanInheritanceInfo *inheritance_info) const {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };
        if (this->_oneTimeSubmit) {
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }
        if (!this->_primary) {
            if (inheritance_info == nullptr) {
                throw std::invalid_argument("inheritance_info is null");
            }

            VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO,
                .pColorAttachmentFormats = inheritance_info->colorFormats.data(),
                .depthAttachmentFormat = inheritance_info->depthFormat,
                .rasterizationSamples = inheritance_info->rasterizationSamples,
            };
            const VkCommandBufferInheritanceInfo inheritanceInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                .pNext = &inheritanceRenderingInfo,
            };
            beginInfo.pInheritanceInfo = &inheritanceInfo;
        }
        utility::vkCheck(vkBeginCommandBuffer(this->_vkCommandBuffer, &beginInfo), "Failed to begin command buffer");
    }

    void CVulkanCommandBuffer::endRecording() const {
        utility::vkCheck(vkEndCommandBuffer(this->_vkCommandBuffer), "Failed to end command buffer");
    }

    void CVulkanCommandBuffer::submitAndWait(const CVulkanQueue& queue) const {
        CVulkanFence fence{this->_context};
        fence.createFence(false);
        const VkCommandBufferSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = this->_vkCommandBuffer,
        };
        queue.submit(std::vector {submitInfo}, nullptr, nullptr, &fence);
        fence.wait();
        fence.destroyFence();
    }

    void CVulkanCommandBuffer::initCommandBuffer() {
        logging::info("Creating command buffer");

        const VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = this->_commandPool->vk_command_pool(),
            .level = this->_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
            .commandBufferCount = 1,
        };

        utility::vkCheck(vkAllocateCommandBuffers(this->_context.deviceData().vkDevice, &commandBufferAllocateInfo, &this->_vkCommandBuffer), "Failed to create command buffer");
    }

    void CVulkanCommandBuffer::destroyCommandBuffer() const {
        logging::info("Destroying command buffer");
        vkFreeCommandBuffers(this->_context.deviceData().vkDevice, this->_commandPool->vk_command_pool(), 1, &this->_vkCommandBuffer);
    }

    void CVulkanCommandBuffer::reset() const {
        vkResetCommandBuffer(this->_vkCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }


    void init(const CVulkanContext* c_context) {

    }
}
