//
// Created by gltex on 07.09.2026.
//

#pragma once
#include <vulkan/vulkan_core.h>

#include "vulkan_render_core.h"

namespace cvulkan::client::renderer {
    class CVulkanSemaphore {
    public:
        explicit CVulkanSemaphore(const CVulkanContext& context)
            : _context{context} {
        }
        ~CVulkanSemaphore() = default;

        CVulkanSemaphore(const CVulkanSemaphore&) = delete;
        CVulkanSemaphore& operator=(const CVulkanSemaphore&) = delete;

        void createSemaphore();
        void destroySemaphore() const;

        [[nodiscard]] VkSemaphore vkSemaphore() const {
            return _semaphore;
        }

    private:
        const CVulkanContext& _context;
        VkSemaphore _semaphore = {};
    };

    class CVulkanFence {
    public:
        explicit CVulkanFence(const CVulkanContext& context)
            : _context{context} {
        }
        ~CVulkanFence() = default;

        CVulkanFence(const CVulkanFence&) = delete;
        CVulkanFence& operator=(const CVulkanFence&) = delete;

        void createFence(bool signaled);
        void destroyFence() const;
        void wait() const;
        void reset() const;

        [[nodiscard]] VkFence vkFence() const {
            return _fence;
        }

    private:
        const CVulkanContext& _context;
        VkFence _fence = {};
    };
}
