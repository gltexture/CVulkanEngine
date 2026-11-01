//
// Created by ded on 13.09.2026.
//

#pragma once
#include <filesystem>
#include <vk_mem_alloc.h>

#include "vulkan_render_core.h"
#include "utility/sys_utils.h"

namespace cvulkan::client::render::asset::modelGen {
    struct CVulkanMeshData;
}

namespace cvulkan::client::render::vk {
    class CVulkanCommandBuffer;

    class CVulkanBuffer {
    public:
        CVulkanBuffer(const CVulkanContext& context, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage vmaMemoryUsage, VmaAllocationCreateFlags vmaFlags, VkMemoryPropertyFlags reqFlags);

        ~CVulkanBuffer();

        CLASS_NO_COPY_NO_ASSIGN_MOVE(CVulkanBuffer);

        CVulkanBuffer(CVulkanBuffer&& other) noexcept
                : _context{other._context}, _vkBuffer{other._vkBuffer}, _vkRequestedSize{other._vkRequestedSize}, _allocation{other._allocation}, _mappedMemory{other._mappedMemory} {
            other._vkBuffer = VK_NULL_HANDLE;
            other._allocation = VK_NULL_HANDLE;
            other._mappedMemory = nullptr;
        }

        void mapMem();

        void unMapMem();

        void flush() const;

        [[nodiscard]] VkBuffer vkBuffer() const {
            return _vkBuffer;
        }

        [[nodiscard]] const VkDeviceSize& vkRequestedSize() const {
            return _vkRequestedSize;
        }

        [[nodiscard]] const void* mappedMemory() const {
            return _mappedMemory;
        }

        [[nodiscard]] void* mappedMemory() {
            return _mappedMemory;
        }

    private:
        const CVulkanContext& _context;
        VkBuffer _vkBuffer{};
        VkDeviceSize _vkRequestedSize{};
        VmaAllocation _allocation{};
        void* _mappedMemory{};
    };


    struct CVulkanTransferBufferData {
        CVulkanBuffer _src;
        CVulkanBuffer _dst;

        void recordTransferCommand(const CVulkanCommandBuffer& commandBuffer) const;
    };

    CVulkanTransferBufferData createVerticesBuffer(const CVulkanContext& context, const asset::modelGen::CVulkanMeshData& rawMeshData, const std::filesystem::path& vtxPath);

    CVulkanTransferBufferData createIndicesBuffers(const CVulkanContext& context, const asset::modelGen::CVulkanMeshData& rawMeshData, const std::filesystem::path& idxPath);

    namespace buffutil {
        CVulkanBuffer allocateHostVisibleBuffer(const CVulkanContext& context, CVulkanDescriptorAllocator& allocator, VkDeviceSize size, VkBufferUsageFlags usage, std::string_view id, const CVulkanDescriptorSetLayout& layout);

        std::vector<CVulkanBuffer> allocateHostVisibleBuffers(const CVulkanContext& context, CVulkanDescriptorAllocator& allocator, VkDeviceSize size, VkBufferUsageFlags usage, uint32_t buffersCount, std::string_view id,
                                                              const CVulkanDescriptorSetLayout& layout);

        void copyMatrixToBuffer(CVulkanBuffer& buffer, const glm::mat4& matrix, uint32_t offset);
    }
}
