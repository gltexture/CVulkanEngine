//
// Created by ded on 07.09.2026.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace cvulkan::client::renderSync {
    class CVulkanFence;
}

namespace cvulkan::client::renderCore {
    class CVulkanContext;
    struct CVulkanPhysicalDevice;
    struct CVulkanDevice;
    struct CVulkanInstance;

    struct CVulkanImageViewData {
        VkImageAspectFlags aspectMask = {};
        uint32_t baseArrayLayer = 0;
        VkFormat format = {};
        uint32_t layerCount = 1;
        uint32_t mipLevels = 1;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    };

    class CVulkanImageView {
    public:
        explicit CVulkanImageView(const CVulkanContext& context)
            : _context{context} {}
        ~CVulkanImageView() = default;

        CVulkanImageView(const CVulkanImageView&) = delete;
        CVulkanImageView& operator=(const CVulkanImageView&) = delete;

        CVulkanImageView(CVulkanImageView&& other) noexcept: _context{other._context}, _vkImage{other._vkImage}, _vkImageView{other._vkImageView} {
            other._vkImage = VK_NULL_HANDLE;
            other._vkImageView = VK_NULL_HANDLE;
        }

        CVulkanImageView& operator=(CVulkanImageView&&) = delete;

        void createImageView(const CVulkanImageViewData& view_data, const VkImage& vk_image);
        void destroyImageView();

        [[nodiscard]] VkImage vkImage() const {
            return _vkImage;
        }

        [[nodiscard]] VkImageView vkImageView() const {
            return _vkImageView;
        }

    private:
        const CVulkanContext& _context;
        VkImage _vkImage = {};
        VkImageView _vkImageView = {};
    };

    class CVulkanSwapChain {
    public:
        explicit CVulkanSwapChain(const CVulkanContext& context)
            : _context{context} {}
        ~CVulkanSwapChain() = default;

        CVulkanSwapChain(const CVulkanSwapChain&) = delete;
        CVulkanSwapChain& operator=(const CVulkanSwapChain&) = delete;

        void createSwapChain(const VkSurfaceKHR& vkSurface, const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities, const VkFormat& vkFormat, const VkColorSpaceKHR& vkColorSpace);
        void destroySwapChain();

        [[nodiscard]] VkSwapchainKHR vkSwapChain() const {
            return _vkSwapChain;
        }

        [[nodiscard]] const std::vector<CVulkanImageView>& imageViews() const {
            return _imageViews;
        }

    private:
        const CVulkanContext& _context;
        VkSwapchainKHR _vkSwapChain = {};
        std::vector<CVulkanImageView> _imageViews = {};
    };

    class CVulkanQueue {
    public:
        explicit CVulkanQueue(const CVulkanContext& context): _context{context} {}
        ~CVulkanQueue() = default;

        CVulkanQueue(const CVulkanQueue&) = delete;
        CVulkanQueue& operator=(const CVulkanQueue&) = delete;

        void initQueue(uint32_t queueFamilyIndex, uint32_t queueIndex);
        void submit(const std::vector<VkCommandBufferSubmitInfo>& commandSubmitInfos, const std::vector<VkSemaphoreSubmitInfo>* waitSemaphores, const std::vector<VkSemaphoreSubmitInfo>* signalSemaphores, const renderSync::CVulkanFence* fence) const;

        [[nodiscard]] VkQueue vkQueue() const {
            return _vkQueue;
        }

        [[nodiscard]] uint32_t queueFamilyIndex() const {
            return _queueFamilyIndex;
        }

    private:
        const CVulkanContext& _context;
        VkQueue _vkQueue = {};
        uint32_t _queueFamilyIndex = {};
    };

    class CVulkanSurface {
    public:
        explicit CVulkanSurface(const CVulkanContext& context)
            : _swapChain{context}, _context{context} {}
        ~CVulkanSurface() = default;

        CVulkanSurface(const CVulkanSurface&) = delete;
        CVulkanSurface& operator=(const CVulkanSurface&) = delete;

        static void calcSurfaceFormat(const CVulkanPhysicalDevice &physical_device_data, const VkSurfaceKHR& vkSurface, VkFormat& vkFormat, VkColorSpaceKHR& vkColorSpace);
        void createSurface();
        void destroySurface();

        [[nodiscard]] const CVulkanSwapChain& swapChain() const {
            return _swapChain;
        }

        [[nodiscard]] VkSurfaceKHR vkSurface() const {
            return _vkSurface;
        }

        [[nodiscard]] VkSurfaceCapabilitiesKHR vkSurfaceCapabilities() const {
            return _vkSurfaceCapabilities;
        }

        [[nodiscard]] VkFormat vkFormat() const {
            return _vkFormat;
        }

        [[nodiscard]] VkColorSpaceKHR vkColorSpace() const {
            return _vkColorSpace;
        }

    private:
        VkSurfaceKHR _vkSurface = {};
        VkSurfaceCapabilitiesKHR _vkSurfaceCapabilities = {};
        VkFormat _vkFormat = {};
        VkColorSpaceKHR _vkColorSpace = {};
        CVulkanSwapChain _swapChain;
        const CVulkanContext& _context;
    };
}
