//
// Created by gltex on 07.09.2026.
//

#pragma once
#include <vulkan/vulkan_core.h>

#include "vulkan_utility.h"


namespace cvulkan::client::render::core {
    class CVulkanContext;
}

namespace cvulkan::client::render::sync {
    class CVulkanSemaphore {
    public:
        explicit CVulkanSemaphore(const core::CVulkanContext& context)
            : _context{context} {
        }
        ~CVulkanSemaphore() = default;

        CVULKAN_NO_COPY(CVulkanSemaphore);

        CVulkanSemaphore(CVulkanSemaphore&& other) noexcept
            : _context{other._context}, _vkSemaphore{other._vkSemaphore} {
            other._vkSemaphore = VK_NULL_HANDLE;
        }

        void createSemaphore();
        void destroySemaphore();

        [[nodiscard]] VkSemaphore vkSemaphore() const {
            return _vkSemaphore;
        }

    private:
        const core::CVulkanContext& _context;
        VkSemaphore _vkSemaphore {};
    };

    class CVulkanFence {
    public:
        explicit CVulkanFence(const core::CVulkanContext& context)
            : _context{context} {
        }
        ~CVulkanFence() = default;

        CVULKAN_NO_COPY(CVulkanFence);

        CVulkanFence(CVulkanFence&& other) noexcept
            : _context{other._context}, _vkFence{other._vkFence} {
            other._vkFence = VK_NULL_HANDLE;
        }

        void createFence(bool signaled);
        void destroyFence();
        void wait() const;
        void reset() const;

        [[nodiscard]] VkFence vkFence() const {
            return _vkFence;
        }

    private:
        const core::CVulkanContext& _context;
        VkFence _vkFence {};
    };
}
