//
// Created by ded on 13.09.2026.
//

#pragma once
#include "vulkan_render_core.h"

namespace cvulkan::client::render::asset::modelGen {
    struct CVulkanMeshData;
}

namespace cvulkan::client::render::vk {
    class CVulkanCommandBuffer;

    class CVulkanBuffer {
    public:
        explicit CVulkanBuffer(const CVulkanContext& context, const VkBufferUsageFlags& usage, const uint32_t& reqMask, const VkDeviceSize& size);
        ~CVulkanBuffer();

        CVULKAN_NO_COPY_NO_ASSIGN_MOVE(CVulkanBuffer);

        CVulkanBuffer(CVulkanBuffer&& other) noexcept
            : _context{other._context}, _vkBuffer{other._vkBuffer}, _vkAllocationSize{other._vkAllocationSize}, _vkRequestedSize{other._vkRequestedSize}, _vkDeviceMemory{other._vkDeviceMemory}, _mappedMemory{other._mappedMemory} {
            other._vkBuffer = VK_NULL_HANDLE;
            other._vkDeviceMemory = VK_NULL_HANDLE;
            other._mappedMemory = nullptr;
        }

        void mapMem();
        void unMapMem();

        [[nodiscard]] VkBuffer vkBuffer() const {
            return _vkBuffer;
        }

        [[nodiscard]] const VkDeviceSize& vkAllocationSize() const {
            return _vkAllocationSize;
        }

        [[nodiscard]] const VkDeviceSize& vkRequestedSize() const {
            return _vkRequestedSize;
        }

        [[nodiscard]] VkDeviceMemory vkDeviceMemory() const {
            return _vkDeviceMemory;
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
        VkDeviceSize _vkAllocationSize{};
        VkDeviceSize _vkRequestedSize{};
        VkDeviceMemory _vkDeviceMemory{};
        void* _mappedMemory{};
    };


    struct CVulkanTransferBufferData {
        CVulkanBuffer _src;
        CVulkanBuffer _dst;

        void recordTransferCommand(const CVulkanCommandBuffer& commandBuffer) const;
    };

    CVulkanTransferBufferData createVerticesBuffer(const CVulkanContext& context, const asset::modelGen::CVulkanMeshData& rawMeshData, const std::filesystem::path& vtxPath);
    CVulkanTransferBufferData createIndicesBuffers(const CVulkanContext& context, const asset::modelGen::CVulkanMeshData& rawMeshData, const std::filesystem::path& idxPath);
}
