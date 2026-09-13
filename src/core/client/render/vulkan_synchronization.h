//
// Created by gltex on 07.09.2026.
//

#pragma once
#include <vulkan/vulkan_core.h>


namespace cvulkan::client::renderCore {
    class CVulkanContext;
}

namespace cvulkan::client::renderSync {
    class CVulkanSemaphore {
    public:
        explicit CVulkanSemaphore(const renderCore::CVulkanContext& context)
            : _context{context} {
        }
        ~CVulkanSemaphore() = default;

        CVulkanSemaphore(const CVulkanSemaphore&) = delete;
        CVulkanSemaphore& operator=(const CVulkanSemaphore&) = delete;
        CVulkanSemaphore(CVulkanSemaphore&& other) noexcept
            : _context{other._context}, _vkSemaphore{other._vkSemaphore} {
            other._vkSemaphore = VK_NULL_HANDLE;
        }

        void initSemaphore();
        void destroySemaphore();

        [[nodiscard]] VkSemaphore vkSemaphore() const {
            return _vkSemaphore;
        }

    private:
        const renderCore::CVulkanContext& _context;
        VkSemaphore _vkSemaphore {};
    };

    class CVulkanFence {
    public:
        explicit CVulkanFence(const renderCore::CVulkanContext& context)
            : _context{context} {
        }
        ~CVulkanFence() = default;

        CVulkanFence(const CVulkanFence&) = delete;
        CVulkanFence& operator=(const CVulkanFence&) = delete;
        CVulkanFence(CVulkanFence&& other) noexcept
            : _context{other._context}, _vkFence{other._vkFence} {
            other._vkFence = VK_NULL_HANDLE;
        }

        void initFence(bool signaled);
        void destroyFence();
        void wait() const;
        void reset() const;

        [[nodiscard]] VkFence vkFence() const {
            return _vkFence;
        }

    private:
        const renderCore::CVulkanContext& _context;
        VkFence _vkFence {};
    };
}
