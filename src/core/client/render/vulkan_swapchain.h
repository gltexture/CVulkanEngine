//
// Created by ded on 07.09.2026.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>

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

    struct CVulkanImageViewData {
        VkImageAspectFlags aspectMask{};
        uint32_t baseArrayLayer = 0;
        VkFormat format{};
        uint32_t layerCount = 1;
        uint32_t mipLevels = 1;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    };

    class CVulkanImageView {
    public:
        explicit CVulkanImageView(const CVulkanContext& context, const CVulkanImageViewData& view_data, const VkImage& vk_image);
        ~CVulkanImageView();

        CVULKAN_NO_COPY(CVulkanImageView);

        CVulkanImageView(CVulkanImageView&& other) noexcept : _context{other._context}, _vkImage{other._vkImage}, _vkImageView{other._vkImageView} {
            other._vkImage = VK_NULL_HANDLE;
            other._vkImageView = VK_NULL_HANDLE;
        }

        CVulkanImageView& operator=(CVulkanImageView&&) = delete;

        [[nodiscard]] VkImage vkImage() const {
            return _vkImage;
        }

        [[nodiscard]] VkImageView vkImageView() const {
            return _vkImageView;
        }

    private:
        const CVulkanContext& _context;
        VkImage _vkImage{};
        VkImageView _vkImageView{};
    };

    class CVulkanSwapChain {
    public:
        explicit CVulkanSwapChain(const CVulkanContext& context, const VkSurfaceKHR& vkSurface, const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities, const VkFormat& vkFormat, const VkColorSpaceKHR& vkColorSpace);
        ~CVulkanSwapChain();

        CVULKAN_NO_COPY(CVulkanSwapChain);

        uint32_t acquireSwapChainNextImage(const sync::CVulkanSemaphore& semaphore) const;

        [[nodiscard]] VkSwapchainKHR vkSwapChain() const {
            return _vkSwapChain;
        }

        [[nodiscard]] const std::vector<CVulkanImageView>& imageViews() const {
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
        VkExtent2D _swapChainExtent = {};
        uint32_t _numImages = UINT32_MAX;
        VkSwapchainKHR _vkSwapChain{};
        std::vector<CVulkanImageView> _imageViews{};
    };

    class CVulkanSurface {
    public:
        explicit CVulkanSurface(const CVulkanContext& context);
        ~CVulkanSurface();

        CVULKAN_NO_COPY(CVulkanSurface);

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
