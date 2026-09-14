//
// Created by ded on 13.09.2026.
//

#include "vulkan_buffers.h"

namespace cvulkan::client::renderCore {
    void CVulkanBuffer::initBuffer(const VkBufferUsageFlags& usage, const uint32_t& reqMask, const VkDeviceSize& size) {
        this->_vkRequestedSize = size;
        const VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        utility::vkCheck(vkCreateBuffer(this->_context.device().vkDevice, &bufferCreateInfo, nullptr, &this->_vkBuffer), "Failed to create buffer");

        VkMemoryRequirements memoryRequirements = {};
        vkGetBufferMemoryRequirements(this->_context.device().vkDevice, this->_vkBuffer, &memoryRequirements);

        const VkMemoryAllocateInfo memoryAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = this->getMemoryTypeIndexFromProperties(memoryRequirements.memoryTypeBits, reqMask),
        };
        this->_vkAllocationSize = memoryAllocateInfo.allocationSize;

        utility::vkCheck(vkAllocateMemory(this->_context.device().vkDevice, &memoryAllocateInfo, nullptr, &this->_vkDeviceMemory), "Failed to allocate memory");
        utility::vkCheck(vkBindBufferMemory(this->_context.device().vkDevice, this->_vkBuffer, this->_vkDeviceMemory, 0), "Failed to bind buffer memory");
    }

    void CVulkanBuffer::destroyBuffer() {
        if (this->_vkBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(this->_context.device().vkDevice, this->_vkBuffer, nullptr);
            this->_vkBuffer = VK_NULL_HANDLE;
        }
        if (this->_vkDeviceMemory != VK_NULL_HANDLE) {
            vkFreeMemory(this->_context.device().vkDevice, this->_vkDeviceMemory, nullptr);
            this->_vkDeviceMemory = VK_NULL_HANDLE;
        }
    }

    void CVulkanBuffer::mapMem() {
        if (this->_mappedMemory == nullptr) {
            utility::vkCheck(vkMapMemory(this->_context.device().vkDevice, this->_vkDeviceMemory, 0, this->_vkAllocationSize, 0, &this->_mappedMemory), "Failed to bind buffer memory");
        }
    }

    void CVulkanBuffer::unMapMem() {
        if (this->_mappedMemory != nullptr) {
            vkUnmapMemory(this->_context.device().vkDevice,this->_vkDeviceMemory);
            this->_mappedMemory = nullptr;
        }
    }

    uint32_t CVulkanBuffer::getMemoryTypeIndexFromProperties(uint32_t typeBits, const uint32_t& reqMask) const {
        const VkMemoryType* vkMemoryTypes = this->_context.physicalDevice().vkMemoryProperties.memoryTypes;
        uint32_t result = UINT32_MAX;
        for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
            if ((typeBits & 1) && (vkMemoryTypes[i].propertyFlags & reqMask) == reqMask) {
                result = i;
                break;
            }
            typeBits >>= 1;
        }
        if (result == UINT32_MAX) {
            throw std::runtime_error("Failed to find memoryType");
        }
        return result;
    }
}
