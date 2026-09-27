//
// Created by ded on 07.09.2026.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include "vulkan_images.h"
#include "vulkan_utility.h"

namespace cvulkan::client::render::sync {
    class CVulkanSemaphore;
    class CVulkanFence;
}

namespace cvulkan::client::render::core {
    class CVulkanQueue;
    class CVulkanContext;
    struct CVulkanPhysicalDevice;
    struct CVulkanDevice;
    struct CVulkanInstance;

    class CVulkanSwapChain {
    public:
        CVulkanSwapChain(const CVulkanContext& context, const VkSurfaceKHR& vkSurface, const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities, const VkFormat& vkFormat, const VkColorSpaceKHR& vkColorSpace);
        ~CVulkanSwapChain();

        CVULKAN_NO_COPY_NO_MOVE(CVulkanSwapChain);

        uint32_t acquireSwapChainNextImage(const sync::CVulkanSemaphore& semaphore) const;

        [[nodiscard]] VkSwapchainKHR vkSwapChain() const {
            return _vkSwapChain;
        }

        [[nodiscard]] const std::vector<CVulkanImage2DView>& imageViews() const {
            return _imageViews;
        }

        [[nodiscard]] uint32_t numImages() const {
            return _numImages;
        }

        [[nodiscard]] const VkExtent2D& extent() const {
            return _swapChainExtent;
        }

        bool presentImage(const CVulkanQueue& queue, const sync::CVulkanSemaphore& renderCompleteSemaphore, const uint32_t& imageIndex) const;

    private:
        const CVulkanContext& _context;
        VkExtent2D _swapChainExtent  {};
        uint32_t _numImages = UINT32_MAX;
        VkSwapchainKHR _vkSwapChain{};
        std::vector<CVulkanImage2DView> _imageViews{};
    };

    class CVulkanSurface {
    public:
        explicit CVulkanSurface(const CVulkanContext& context);
        ~CVulkanSurface();

        CVULKAN_NO_COPY_NO_MOVE(CVulkanSurface);

        [[nodiscard]] VkSurfaceKHR vkSurface() const {
            return _vkSurface;
        }

        [[nodiscard]] const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities() const {
            return _vkSurfaceCapabilities;
        }

        [[nodiscard]] const VkFormat& vkFormat() const {
            return _vkFormat;
        }

        [[nodiscard]] const VkColorSpaceKHR& vkColorSpace() const {
            return _vkColorSpace;
        }

    protected:
        static void calcSurfaceFormat(const CVulkanPhysicalDevice& physical_device_data, const VkSurfaceKHR& vkSurface, VkFormat& vkFormat, VkColorSpaceKHR& vkColorSpace);

    private:
        const CVulkanContext& _context;
        VkSurfaceKHR _vkSurface{};
        VkSurfaceCapabilitiesKHR _vkSurfaceCapabilities{};
        VkFormat _vkFormat{};
        VkColorSpaceKHR _vkColorSpace{};
    };
}
