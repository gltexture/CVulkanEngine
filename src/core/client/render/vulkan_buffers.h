//
// Created by ded on 13.09.2026.
//

#pragma once
#include "vulkan_render_core.h"

namespace cvulkan::client::render::structs {
    struct CVulkanRawMeshData;
}

namespace cvulkan::client::render::core {
    class CVulkanCommandBuffer;

    class CVulkanBuffer {
    public:
        explicit CVulkanBuffer(const CVulkanContext& context)
            : _context(context) {}
        ~CVulkanBuffer() = default;

        CVULKAN_NO_COPY(CVulkanBuffer);

        CVulkanBuffer(CVulkanBuffer&& other) noexcept
        : _context{other._context}, _vkBuffer{other._vkBuffer}, _vkAllocationSize{other._vkAllocationSize}, _vkRequestedSize{other._vkRequestedSize}, _vkDeviceMemory{other._vkDeviceMemory}, _mappedMemory{other._mappedMemory} {
            other._vkBuffer = VK_NULL_HANDLE;
            other._vkDeviceMemory = VK_NULL_HANDLE;
            other._mappedMemory = nullptr;
        }
        CVulkanBuffer& operator=(CVulkanBuffer&&) = delete;

        void createBuffer(const VkBufferUsageFlags& usage, const uint32_t& reqMask, const VkDeviceSize& size);
        void destroyBuffer();

        void mapMem();
        void unMapMem();

        [[nodiscard]] VkBuffer vkBuffer() const {
            return _vkBuffer;
        }

        [[nodiscard]] VkDeviceSize vkAllocationSize() const {
            return _vkAllocationSize;
        }

        [[nodiscard]] VkDeviceSize vkRequestedSize() const {
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

    protected:
        uint32_t getMemoryTypeIndexFromProperties(uint32_t typeBits, const uint32_t& reqMask) const;

    private:
        const CVulkanContext& _context;
        VkBuffer _vkBuffer {};
        VkDeviceSize _vkAllocationSize {};
        VkDeviceSize _vkRequestedSize {};
        VkDeviceMemory _vkDeviceMemory {};
        void* _mappedMemory {};
    };



    struct CVulkanTransferBufferData {
        CVulkanBuffer src;
        CVulkanBuffer dst;

        void recordTransferCommand(const CVulkanCommandBuffer& commandBuffer) const;
    };

    CVulkanTransferBufferData createVerticesBuffer(const CVulkanContext& context, const structs::CVulkanRawMeshData& rawMeshData);

    CVulkanTransferBufferData createIndicesBuffers(const CVulkanContext& context, const structs::CVulkanRawMeshData& rawMeshData);
}
