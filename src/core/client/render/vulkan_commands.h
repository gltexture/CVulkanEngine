//
// Created by gltex on 07.09.2026.
//

#pragma once
#include "client_render_core.h"

namespace cvulkan::client::renderer {
    struct CVulkanInheritanceInfo {
        const VkFormat depthFormat;
        const std::vector<VkFormat> colorFormats;
        const VkSampleCountFlagBits rasterizationSamples;
    };

    class CVulkanCommandPool {
    public:
        CVulkanCommandPool(const CVulkanContext* c_context, const uint32_t queueFamilyIndex, const bool supportReset)
        : context(c_context), queueFamilyIndex(queueFamilyIndex), supportReset(supportReset) {};
        ~CVulkanCommandPool() {
            this->destroy();
        }

        void initCommandPool();
        void destroy();
        void reset();

        [[nodiscard]] VkCommandPool vk_command_pool() const {
            return vkCommandPool;
        }

    private:
        VkCommandPool vkCommandPool = {};

        const CVulkanContext* context;
        const uint32_t queueFamilyIndex;
        const bool supportReset;
    };

    class CVulkanCommandBuffer {
    public:
        CVulkanCommandBuffer(const CVulkanCommandPool* vk_command_pool, const CVulkanContext* context, const bool primary, const bool one_time_submit)
            : vkCommandPool(vk_command_pool), context(context), primary(primary), oneTimeSubmit(one_time_submit) {
        }

        ~CVulkanCommandBuffer() {
            this->destroy();
        }

        void beginRecording() const {
            this->beginRecording(nullptr);
        }

        void beginRecording(const CVulkanInheritanceInfo* inheritance_info) const;
        void endRecording() const;

        void initCommandBuffer();
        void destroy();
        void reset();

        [[nodiscard]] VkCommandBuffer vk_command_buffer() const {
            return vkCommandBuffer;
        }

    private:
        VkCommandBuffer vkCommandBuffer = {};

        const CVulkanCommandPool* vkCommandPool;
        const CVulkanContext* context;
        const bool primary;
        const bool oneTimeSubmit;
    };

    void init(const CVulkanContext* c_context);
    void clean_up();
}
