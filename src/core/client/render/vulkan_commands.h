//
// Created by gltex on 07.09.2026.
//

#pragma once
#include "vulkan_render_core.h"

namespace cvulkan::client::renderer {
    struct CVulkanInheritanceInfo {
        const VkFormat depthFormat;
        const std::vector<VkFormat> colorFormats;
        const VkSampleCountFlagBits rasterizationSamples;
    };

    class CVulkanCommandPool {
    public:
        CVulkanCommandPool(const CVulkanContext& context, const uint32_t queueFamilyIndex, const bool supportReset)
        : _context{context}, queueFamilyIndex{queueFamilyIndex}, supportReset{supportReset} {}
        ~CVulkanCommandPool() {
            this->destroyCommandPool();
        }

        CVulkanCommandPool(const CVulkanCommandPool&) = delete;
        CVulkanCommandPool& operator=(const CVulkanCommandPool&) = delete;

        void initCommandPool();
        void destroyCommandPool() const;
        void reset() const;

        [[nodiscard]] VkCommandPool vk_command_pool() const {
            return vkCommandPool;
        }

    private:
        const CVulkanContext& _context;
        VkCommandPool vkCommandPool = {};
        const uint32_t queueFamilyIndex;
        const bool supportReset;
    };

    class CVulkanCommandBuffer {
    public:
        CVulkanCommandBuffer(const CVulkanCommandPool* vk_command_pool, const CVulkanContext& context, const bool primary, const bool one_time_submit)
            : _context{context}, vkCommandPool{vk_command_pool}, primary{primary}, oneTimeSubmit{one_time_submit} {}
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

        void initCommandBuffer();
        void destroyCommandBuffer() const;
        void reset() const;

        [[nodiscard]] VkCommandBuffer vk_command_buffer() const {
            return vkCommandBuffer;
        }

    private:
        const CVulkanContext& _context;
        VkCommandBuffer vkCommandBuffer = {};
        const CVulkanCommandPool* vkCommandPool = {};
        const bool primary;
        const bool oneTimeSubmit;
    };

    void init(const CVulkanContext* c_context);
    void cleanUp();
}
