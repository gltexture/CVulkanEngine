//
// Created by gltex on 07.09.2026.
//

#pragma once
#include "vulkan_render_core.h"

namespace cvulkan::client::renderCore {
    struct CVulkanInheritanceInfo {
        const VkFormat depthFormat;
        const std::vector<VkFormat> colorFormats;
        const VkSampleCountFlagBits rasterizationSamples;
    };

    class CVulkanCommandPool {
    public:
        CVulkanCommandPool(const CVulkanContext& context, const uint32_t queueFamilyIndex, const bool supportReset)
        : _context{context}, _queueFamilyIndex{queueFamilyIndex}, _supportReset{supportReset} {}
        ~CVulkanCommandPool() {
        }

        CVulkanCommandPool(const CVulkanCommandPool&) = delete;
        CVulkanCommandPool& operator=(const CVulkanCommandPool&) = delete;
        CVulkanCommandPool(CVulkanCommandPool&& other) noexcept
            : _context{other._context}, _vkCommandPool{other._vkCommandPool}, _queueFamilyIndex{other._queueFamilyIndex}, _supportReset{other._supportReset} {
            other._vkCommandPool = VK_NULL_HANDLE;
        }

        void initCommandPool();
        void destroyCommandPool() const;
        void reset() const;

        [[nodiscard]] VkCommandPool vkCommandPool() const {
            return _vkCommandPool;
        }

    private:
        const CVulkanContext& _context;
        VkCommandPool _vkCommandPool {};
        const uint32_t _queueFamilyIndex;
        const bool _supportReset;
    };

    class CVulkanCommandBuffer {
    public:
        CVulkanCommandBuffer(const CVulkanContext& context, const CVulkanCommandPool& commandPool, const bool primary, const bool oneTimeSubmit)
            : _context{context}, _commandPool{commandPool}, _primary{primary}, _oneTimeSubmit{oneTimeSubmit} {}
        ~CVulkanCommandBuffer() {
        }

        CVulkanCommandBuffer(const CVulkanCommandBuffer&) = delete;
        CVulkanCommandBuffer& operator=(const CVulkanCommandBuffer&) = delete;
        CVulkanCommandBuffer(CVulkanCommandBuffer&& other) noexcept
            : _context{other._context}, _vkCommandBuffer{other._vkCommandBuffer}, _commandPool{other._commandPool}, _primary(other._primary), _oneTimeSubmit(other._oneTimeSubmit) {
            other._vkCommandBuffer = VK_NULL_HANDLE;
        }

        void beginRecording() const {
            this->beginRecording(nullptr);
        }

        void beginRecording(const CVulkanInheritanceInfo* inheritance_info) const;
        void endRecording() const;

        void submitAndWait(const CVulkanQueue& queue) const;

        void initCommandBuffer();
        void destroyCommandBuffer() const;
        void reset() const;

        [[nodiscard]] VkCommandBuffer vkCommandBuffer() const {
            return _vkCommandBuffer;
        }

        [[nodiscard]] const CVulkanCommandPool& commandPool() const {
            return _commandPool;
        }

    private:
        const CVulkanContext& _context;
        VkCommandBuffer _vkCommandBuffer {};
        const CVulkanCommandPool& _commandPool;
        const bool _primary;
        const bool _oneTimeSubmit;
    };

    void init(const CVulkanContext* c_context);
    void cleanUp();
}
