//
// Created by gltex on 07.09.2026.
//

#include "vulkan_commands.h"
#include "vulkan_synchronization.h"

namespace cvulkan::client::render::core {
    CVulkanCommandPool::CVulkanCommandPool(const CVulkanContext& context, const uint32_t queueFamilyIndex, const bool supportReset)
    : _context{context}, _queueFamilyIndex{queueFamilyIndex}, _supportReset{supportReset} {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = this->_queueFamilyIndex,
        };
        if (this->_supportReset) {
            commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        }
        utility::vkCheck(vkCreateCommandPool(this->_context.device().vkDevice(), &commandPoolCreateInfo, nullptr, &this->_vkCommandPool), "Failed to create command pool");
        logging::info("Created vkCommandPool");
    }

    CVulkanCommandPool::~CVulkanCommandPool() {
        if (this->_vkCommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(this->_context.device().vkDevice(), this->_vkCommandPool, nullptr);
            this->_vkCommandPool = VK_NULL_HANDLE;
            logging::info("Destroyed vkCommandPool");
        }
    }

    void CVulkanCommandPool::reset() const {
        //logging::info("Resetting command pool");
        vkResetCommandPool(this->_context.device().vkDevice(), this->_vkCommandPool, 0);
    }

    CVulkanCommandBuffer::CVulkanCommandBuffer(const CVulkanContext& context, const CVulkanCommandPool& commandPool, const bool primary, const bool oneTimeSubmit)
    : _context{context}, _commandPool{commandPool}, _primary{primary}, _oneTimeSubmit{oneTimeSubmit} {
        const VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = this->_commandPool.vkCommandPool(),
            .level = this->_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
            .commandBufferCount = 1,
        };
        utility::vkCheck(vkAllocateCommandBuffers(this->_context.device().vkDevice(), &commandBufferAllocateInfo, &this->_vkCommandBuffer), "Failed to create command buffer");
        logging::info("Created vkCommandBuffer");
    }

    CVulkanCommandBuffer::~CVulkanCommandBuffer() {
        vkFreeCommandBuffers(this->_context.device().vkDevice(), this->_commandPool.vkCommandPool(), 1, &this->_vkCommandBuffer);
        logging::info("Destroyed vkCommandBuffer");
    }

    void CVulkanCommandBuffer::reset() const {
        vkResetCommandBuffer(this->_vkCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }


    void CVulkanCommandBuffer::beginRecording(const CVulkanInheritanceInfo* inheritance_info) const {
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
        const sync::CVulkanFence fence{this->_context, false};
        const VkCommandBufferSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = this->_vkCommandBuffer,
        };
        queue.submitQueue(std::vector{submitInfo}, nullptr, nullptr, &fence);
        fence.wait();
    }

    CVulkanQueue::CVulkanQueue(const CVulkanContext& context, uint32_t const queueFamilyIndex, const uint32_t queueIndex) : _context(context) {
        vkGetDeviceQueue(this->_context.device().vkDevice(), queueFamilyIndex, queueIndex, &this->_vkQueue);
        this->_queueFamilyIndex = queueFamilyIndex;
    }

    void CVulkanQueue::submitQueue(const std::vector<VkCommandBufferSubmitInfo>& commandSubmitInfos, const std::vector<VkSemaphoreSubmitInfo>* waitSemaphores, const std::vector<VkSemaphoreSubmitInfo>* signalSemaphores,
    const sync::CVulkanFence* fence) const {
        VkSubmitInfo2 vkSubmitInfo2 = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .commandBufferInfoCount = static_cast<uint32_t>(commandSubmitInfos.size()),
            .pCommandBufferInfos = commandSubmitInfos.data(),
        };
        if (waitSemaphores != nullptr) {
            vkSubmitInfo2.waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphores->size());
            vkSubmitInfo2.pWaitSemaphoreInfos = waitSemaphores->data();
        }
        if (signalSemaphores != nullptr) {
            vkSubmitInfo2.signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphores->size());
            vkSubmitInfo2.pSignalSemaphoreInfos = signalSemaphores->data();
        }
        utility::vkCheck(vkQueueSubmit2(this->_vkQueue, 1, &vkSubmitInfo2, fence != nullptr ? fence->vkFence() : VK_NULL_HANDLE), "Failed to submit command to queue");
    }
}
