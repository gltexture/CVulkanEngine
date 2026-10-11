//
// Created by ded on 13.09.2026.
//

#include "vulkan_buffers.h"

#include "vulkan_commands.h"
#include "assets/asset_models.h"
#include "scene/vulkan_render_structs.h"

namespace cvulkan::client::render::vk {
    CVulkanBuffer::CVulkanBuffer(const CVulkanContext& context, const VkBufferUsageFlags& usage, const uint32_t& reqMask, const VkDeviceSize& size) : _context(context) {
        this->_vkRequestedSize = size;
        const VkBufferCreateInfo bufferCreateInfo  {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        utility::vkCheck(vkCreateBuffer(this->_context.device().vkDevice(), &bufferCreateInfo, nullptr, &this->_vkBuffer), "Failed to create buffer");

        VkMemoryRequirements memoryRequirements  {};
        vkGetBufferMemoryRequirements(this->_context.device().vkDevice(), this->_vkBuffer, &memoryRequirements);

        const VkMemoryAllocateInfo memoryAllocateInfo  {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = utility::getMemoryTypeIndexFromProperties(context.physicalDevice().vkMemoryProperties(), memoryRequirements.memoryTypeBits, reqMask),
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

    void CVulkanTransferBufferData::recordTransferCommand(const CVulkanCommandBuffer& commandBuffer) const {
        const VkBufferCopy copyRegion  {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = this->_src.vkRequestedSize()
        };
        vkCmdCopyBuffer(commandBuffer.vkCommandBuffer(), this->_src.vkBuffer(), this->_dst.vkBuffer(), 1, &copyRegion);
    }

    CVulkanTransferBufferData createVerticesBuffer(const CVulkanContext& context, const asset::modelGen::CVulkanMeshData& rawMeshData, const std::filesystem::path& vtxPath) {
        const VkDeviceSize bufferSize = rawMeshData._vtxSize;

        CVulkanBuffer src{context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize};
        CVulkanBuffer dst{context, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize};

        std::ifstream binFile {vtxPath, std::ios::binary | std::ios::in};
        if (!binFile) {
            throw std::runtime_error(std::format("Failed to open {}", vtxPath.string()));
        }
        src.mapMem();
        binFile.read(static_cast<char*>(src.mappedMemory()), bufferSize);
        src.unMapMem();

        return {(std::move(src)), (std::move(dst))};
    }

    CVulkanTransferBufferData createIndicesBuffers(const CVulkanContext& context, const asset::modelGen::CVulkanMeshData& rawMeshData, const std::filesystem::path& idxPath) {
        const VkDeviceSize bufferSize = rawMeshData._idxSize;

        CVulkanBuffer src{context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize};
        CVulkanBuffer dst{context, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize};

        std::ifstream binFile {idxPath, std::ios::binary | std::ios::in};
        if (!binFile) {
            throw std::runtime_error(std::format("Failed to open {}", idxPath.string()));
        }
        src.mapMem();
        binFile.read(static_cast<char*>(src.mappedMemory()), bufferSize);
        src.unMapMem();

        return {(std::move(src)), (std::move(dst))};
    }
}
