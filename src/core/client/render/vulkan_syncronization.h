//
// Created by gltex on 07.09.2026.
//

#pragma once
#include <vulkan/vulkan_core.h>

#include "client_render_core.h"

namespace cvulkan::client::renderer {
    class CVulkanSemaphore {
    public:
        explicit CVulkanSemaphore(const CVulkanContext *context)
            : context(context) {
        }
        ~CVulkanSemaphore() = default;

        void create();
        void destroy() const;

        [[nodiscard]] VkSemaphore vk_semaphore() const {
            return semaphore;
        }

    private:
        const CVulkanContext* context;
        VkSemaphore semaphore = {};
    };

    class CVulkanFence {
    public:
        explicit CVulkanFence(const CVulkanContext *context)
            : context(context) {
        }
        ~CVulkanFence() = default;

        void create(bool signaled);
        void destroy() const;
        void wait();
        void reset();

        [[nodiscard]] VkFence vk_fence() const {
            return fence;
        }

    private:
        const CVulkanContext* context;
        VkFence fence = {};
    };
}
