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
            this->destroyCommandPool();
        }

        CVulkanCommandPool(const CVulkanCommandPool&) = delete;
        CVulkanCommandPool& operator=(const CVulkanCommandPool&) = delete;

        void initCommandPool();
        void destroyCommandPool() const;
        void reset() const;

        [[nodiscard]] VkCommandPool vk_command_pool() const {
            return _vkCommandPool;
        }

    private:
        const CVulkanContext& _context;
        VkCommandPool _vkCommandPool = {};
        const uint32_t _queueFamilyIndex;
        const bool _supportReset;
    };

    class CVulkanCommandBuffer {
    public:
        CVulkanCommandBuffer(const CVulkanCommandPool& commandPool, const CVulkanContext& context, const bool primary, const bool oneTimeSubmit)
            : _context{context}, _commandPool{commandPool}, _primary{primary}, _oneTimeSubmit{oneTimeSubmit} {}
        ~CVulkanCommandBuffer() {
            this->destroyCommandBuffer();
        }

        CVulkanCommandBuffer(const CVulkanCommandBuffer&) = delete;
        CVulkanCommandBuffer& operator=(const CVulkanCommandBuffer&) = delete;

        void beginRecording() const {
            this->beginRecording(nullptr);
        }

        void beginRecording(const CVulkanInheritanceInfo* inheritance_info) const;
        void endRecording() const;

        void submitAndWait(const CVulkanQueue& queue) const;

        void initCommandBuffer();
        void destroyCommandBuffer() const;
        void reset() const;

        [[nodiscard]] VkCommandBuffer vk_command_buffer() const {
            return _vkCommandBuffer;
        }

        [[nodiscard]] const CVulkanCommandPool& commandPool() const {
            return _commandPool;
        }

    private:
        const CVulkanContext& _context;
        VkCommandBuffer _vkCommandBuffer = {};
        const CVulkanCommandPool& _commandPool;
        const bool _primary;
        const bool _oneTimeSubmit;
    };

    void init(const CVulkanContext* c_context);
    void cleanUp();
}
