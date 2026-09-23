//
// Created by ded on 13.09.2026.
//

#include "vulkan_buffers.h"

#include "vulkan_commands.h"
#include "scene/vulkan_render_structs.h"

namespace cvulkan::client::render::core {
    CVulkanBuffer::CVulkanBuffer(const CVulkanContext& context, const VkBufferUsageFlags& usage, const uint32_t& reqMask, const VkDeviceSize& size) : _context(context) {
        this->_vkRequestedSize = size;
        const VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        utility::vkCheck(vkCreateBuffer(this->_context.device().vkDevice(), &bufferCreateInfo, nullptr, &this->_vkBuffer), "Failed to create buffer");

        VkMemoryRequirements memoryRequirements = {};
        vkGetBufferMemoryRequirements(this->_context.device().vkDevice(), this->_vkBuffer, &memoryRequirements);

        const VkMemoryAllocateInfo memoryAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = this->getMemoryTypeIndexFromProperties(memoryRequirements.memoryTypeBits, reqMask),
        };
        this->_vkAllocationSize = memoryAllocateInfo.allocationSize;

        utility::vkCheck(vkAllocateMemory(this->_context.device().vkDevice(), &memoryAllocateInfo, nullptr, &this->_vkDeviceMemory), "Failed to allocate memory");
        utility::vkCheck(vkBindBufferMemory(this->_context.device().vkDevice(), this->_vkBuffer, this->_vkDeviceMemory, 0), "Failed to bind buffer memory");
    }

    CVulkanBuffer::~CVulkanBuffer() {
        if (this->_vkBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(this->_context.device().vkDevice(), this->_vkBuffer, nullptr);
            this->_vkBuffer = VK_NULL_HANDLE;
        }
        if (this->_vkDeviceMemory != VK_NULL_HANDLE) {
            vkFreeMemory(this->_context.device().vkDevice(), this->_vkDeviceMemory, nullptr);
            this->_vkDeviceMemory = VK_NULL_HANDLE;
        }
    }
    void CVulkanBuffer::mapMem() {
        if (this->_mappedMemory == nullptr) {
            utility::vkCheck(vkMapMemory(this->_context.device().vkDevice(), this->_vkDeviceMemory, 0, this->_vkAllocationSize, 0, &this->_mappedMemory), "Failed to bind buffer memory");
        }
    }

    void CVulkanBuffer::unMapMem() {
        if (this->_mappedMemory != nullptr) {
            vkUnmapMemory(this->_context.device().vkDevice(), this->_vkDeviceMemory);
            this->_mappedMemory = nullptr;
        }
    }

    uint32_t CVulkanBuffer::getMemoryTypeIndexFromProperties(uint32_t typeBits, const uint32_t& reqMask) const {
        const VkMemoryType* vkMemoryTypes = this->_context.physicalDevice().vkMemoryProperties().memoryTypes;
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


    void CVulkanTransferBufferData::recordTransferCommand(const CVulkanCommandBuffer& commandBuffer) const {
        const VkBufferCopy copyRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = this->src.vkRequestedSize()
        };
        vkCmdCopyBuffer(commandBuffer.vkCommandBuffer(), this->src.vkBuffer(), this->dst.vkBuffer(), 1, &copyRegion);
    }

    CVulkanTransferBufferData createVerticesBuffer(const CVulkanContext& context, const structs::CVulkanRawMeshData& rawMeshData) {
        const VkDeviceSize bufferSize = rawMeshData.positions.size() * sizeof(float);

        CVulkanBuffer src{context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize};
        CVulkanBuffer dst{context, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize};

        src.mapMem();
        std::memcpy(src.mappedMemory(), rawMeshData.positions.data(), bufferSize);
        src.unMapMem();

        return {(std::move(src)), (std::move(dst))};
    }

    CVulkanTransferBufferData createIndicesBuffers(const CVulkanContext& context, const structs::CVulkanRawMeshData& rawMeshData) {
        const VkDeviceSize bufferSize = rawMeshData.indices.size() * sizeof(uint32_t);

        CVulkanBuffer src{context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize};
        CVulkanBuffer dst{context, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize};

        src.mapMem();
        std::memcpy(src.mappedMemory(), rawMeshData.indices.data(), bufferSize);
        src.unMapMem();

        return {(std::move(src)), (std::move(dst))};
    }
}
