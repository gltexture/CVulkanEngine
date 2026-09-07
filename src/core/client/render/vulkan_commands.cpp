//
// Created by gltex on 07.09.2026.
//

#include "vulkan_commands.h"

namespace cvulkan::client::renderer {
    void CVulkanCommandPool::initCommandPool() {
        logging::info("Creating command pool");
        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = this->queueFamilyIndex,
        };
        if (this->supportReset) {
            commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        }
        utility::vkCheck(vkCreateCommandPool(this->context->vk_device_data().vkDevice, &commandPoolCreateInfo, nullptr, &this->vkCommandPool), "Failed to create command pool");
    }

    void CVulkanCommandPool::destroy() {
        logging::info("Destroying command pool");
        vkDestroyCommandPool(this->context->vk_device_data().vkDevice, this->vkCommandPool, nullptr);
    }

    void CVulkanCommandPool::reset() {
        logging::info("Resetting command pool");
        vkResetCommandPool(this->context->vk_device_data().vkDevice, this->vkCommandPool, 0);
    }


    void CVulkanCommandBuffer::beginRecording(const CVulkanInheritanceInfo *inheritance_info) const {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };
        if (this->oneTimeSubmit) {
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }
        if (!this->primary) {
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
        utility::vkCheck(vkBeginCommandBuffer(this->vkCommandBuffer, &beginInfo), "Failed to begin command buffer");
    }

    void CVulkanCommandBuffer::endRecording() const {
        utility::vkCheck(vkEndCommandBuffer(this->vkCommandBuffer), "Failed to end command buffer");
    }

    void CVulkanCommandBuffer::initCommandBuffer() {
        logging::info("Creating command buffer");

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = this->vkCommandPool->vk_command_pool(),
            .level = this->primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
            .commandBufferCount = 1,
        };

        utility::vkCheck(vkAllocateCommandBuffers(this->context->vk_device_data().vkDevice, &commandBufferAllocateInfo, &this->vkCommandBuffer), "Failed to create command buffer");
    }

    void CVulkanCommandBuffer::destroy() {
        logging::info("Destroying command buffer");
        vkFreeCommandBuffers(this->context->vk_device_data().vkDevice, this->vkCommandPool->vk_command_pool(), 1, &this->vkCommandBuffer);
    }

    void CVulkanCommandBuffer::reset() {
        vkResetCommandBuffer(this->vkCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }


    void init(const CVulkanContext* c_context) {

    }
}
