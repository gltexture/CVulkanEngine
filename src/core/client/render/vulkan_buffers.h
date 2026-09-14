//
// Created by ded on 13.09.2026.
//

#pragma once
#include "vulkan_render_core.h"

namespace cvulkan::client::renderCore {
    class CVulkanBuffer {
    public:
        explicit CVulkanBuffer(const CVulkanContext& context)
            : _context(context) {}
        ~CVulkanBuffer() = default;

        CVulkanBuffer(const CVulkanBuffer&) = delete;
        CVulkanBuffer& operator=(const CVulkanBuffer&) = delete;

        CVulkanBuffer(CVulkanBuffer&& other) noexcept
        : _context{other._context}, _vkBuffer{other._vkBuffer}, _vkAllocationSize{other._vkAllocationSize}, _vkRequestedSize{other._vkRequestedSize}, _vkDeviceMemory{other._vkDeviceMemory}, _mappedMemory{other._mappedMemory} {
            other._vkBuffer = VK_NULL_HANDLE;
            other._vkDeviceMemory = VK_NULL_HANDLE;
            other._mappedMemory = nullptr;
        }
        CVulkanBuffer& operator=(CVulkanBuffer&&) = delete;

        void initBuffer(const VkBufferUsageFlags& usage, const uint32_t& reqMask, const VkDeviceSize& size);
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
}
