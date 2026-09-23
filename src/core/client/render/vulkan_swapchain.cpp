//
// Created by ded on 07.09.2026.
//

#include "vulkan_swapchain.h"

#include "vulkan_commands.h"
#include "vulkan_config.h"
#include "vulkan_render_core.h"
#include "vulkan_synchronization.h"

namespace cvulkan::client::render::core {
    CVulkanImageView::CVulkanImageView(const CVulkanContext& context, const CVulkanImageViewData& view_data, const VkImage& vk_image) : _context(context) {
        const VkImageViewCreateInfo image_view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = vk_image,
            .viewType = view_data.viewType,
            .format = view_data.format,
            .subresourceRange = {
                .aspectMask = view_data.aspectMask,
                .baseMipLevel = 0,
                .levelCount = view_data.mipLevels,
                .baseArrayLayer = view_data.baseArrayLayer,
                .layerCount = view_data.layerCount
            }
        };
        utility::vkCheck(vkCreateImageView(this->_context.device().vkDevice(), &image_view_create_info, nullptr, &this->_vkImageView), "Failed to createImageView image view");
        this->_vkImage = vk_image;
    }

    CVulkanImageView::~CVulkanImageView() {
        if (this->_vkImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(this->_context.device().vkDevice(), this->_vkImageView, nullptr);
            this->_vkImageView = VK_NULL_HANDLE;
        }
    }

    CVulkanSwapChain::CVulkanSwapChain(const CVulkanContext& context, const VkSurfaceKHR& vkSurface, const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities, const VkFormat& vkFormat, const VkColorSpaceKHR& vkColorSpace) : _context(context) {
        constexpr uint32_t requestedImages = renderConfig::SWAP_CHAIN_IMAGES;

        uint32_t imageCount = -1;

        {
            const uint32_t minImages = vkSurfaceCapabilities.minImageCount;
            const uint32_t maxImages = vkSurfaceCapabilities.maxImageCount;
            imageCount = minImages;
            if (maxImages != 0) {
                imageCount = std::min(requestedImages, maxImages);
            }
            imageCount = std::max(imageCount, minImages);
            logging::info("Requested {} images", imageCount);
        }

        {
            if (vkSurfaceCapabilities.currentExtent.width == UINT32_MAX) {
                const auto windowSize = this->_context.glfwWindow().size();
                this->_swapChainExtent.width = std::clamp(windowSize.x, vkSurfaceCapabilities.minImageExtent.width, vkSurfaceCapabilities.maxImageExtent.width);
                this->_swapChainExtent.height = std::clamp(windowSize.y, vkSurfaceCapabilities.minImageExtent.height, vkSurfaceCapabilities.maxImageExtent.height);
            } else {
                this->_swapChainExtent = vkSurfaceCapabilities.currentExtent;
            }
        }

        VkSwapchainCreateInfoKHR swapChainCreateInfo{};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = vkSurface;
        swapChainCreateInfo.minImageCount = imageCount;
        swapChainCreateInfo.imageFormat = vkFormat;
        swapChainCreateInfo.imageColorSpace = vkColorSpace;
        swapChainCreateInfo.imageExtent = this->_swapChainExtent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapChainCreateInfo.preTransform = vkSurfaceCapabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.clipped = VK_TRUE;

        if (renderConfig::VSYNC) {
            swapChainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        } else {
            swapChainCreateInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

        utility::vkCheck(vkCreateSwapchainKHR(this->_context.device().vkDevice(), &swapChainCreateInfo, nullptr, &this->_vkSwapChain));

        {
            utility::vkCheck(vkGetSwapchainImagesKHR(this->_context.device().vkDevice(), this->_vkSwapChain, &this->_numImages, nullptr), "Failed to create swapChain Images");
            std::vector<VkImage> images(this->numImages());
            utility::vkCheck(vkGetSwapchainImagesKHR(this->_context.device().vkDevice(), this->_vkSwapChain, &this->_numImages, images.data()), "Failed to create swapChain Images");

            const CVulkanImageViewData image_view_data = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .format = vkFormat};
            this->_imageViews.reserve(images.size());
            for (const auto image: images) {
                this->_imageViews.emplace_back(this->_context, image_view_data, image);
            }
        }

        logging::info("Created vkSwapChain");
    }

    CVulkanSwapChain::~CVulkanSwapChain() {
        if (this->_vkSwapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(this->_context.device().vkDevice(), this->_vkSwapChain, nullptr);
            this->_vkSwapChain = VK_NULL_HANDLE;
            logging::info("Destroyed vkSwapChain");
        }
    }

    CVulkanSurface::CVulkanSurface(const CVulkanContext& context) : _context(context) {
        utility::vkCheck(glfwCreateWindowSurface(context.instance().vkInstance(), this->_context.glfwWindow().glfwWindowDescriptor(), nullptr, &this->_vkSurface));
        utility::vkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physicalDevice().vkPhysicalDevice(), this->_vkSurface, &this->_vkSurfaceCapabilities));
        calcSurfaceFormat(this->_context.physicalDevice(), this->_vkSurface, this->_vkFormat, this->_vkColorSpace);
        logging::info("Created vkSurface");
    }

    CVulkanSurface::~CVulkanSurface() {
        if (this->_vkSurface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(this->_context.instance().vkInstance(), this->_vkSurface, nullptr);
            this->_vkSurface = VK_NULL_HANDLE;
            logging::info("Destroyed vkSurface");
        }
    }

    void CVulkanSurface::calcSurfaceFormat(const CVulkanPhysicalDevice& physical_device_data, const VkSurfaceKHR& vkSurface, VkFormat& vkFormat, VkColorSpaceKHR& vkColorSpace) {
        uint32_t surfaceFormatCount = 0;
        utility::vkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_data.vkPhysicalDevice(), vkSurface, &surfaceFormatCount, nullptr));
        if (surfaceFormatCount == 0) {
            throw std::runtime_error("Failed to get surface format count");
        }
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        utility::vkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_data.vkPhysicalDevice(), vkSurface, &surfaceFormatCount, surfaceFormats.data()));
        vkFormat = VK_FORMAT_B8G8R8A8_SRGB;
        vkColorSpace = surfaceFormats[0].colorSpace;
        for (const auto& [f, c]: surfaceFormats) {
            if (f == VK_FORMAT_B8G8R8A8_SRGB && c == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                vkFormat = f;
                vkColorSpace = c;
                logging::info("Setting up format {}, color space {}", static_cast<int>(f), static_cast<int>(c));
                break;
            }
        }
    }

    uint32_t CVulkanSwapChain::acquireSwapChainNextImage(const sync::CVulkanSemaphore& semaphore) const {
        uint32_t imageIndex = 0;
        switch (VkResult result = vkAcquireNextImageKHR(this->_context.device().vkDevice(), this->_vkSwapChain, UINT64_MAX, semaphore.vkSemaphore(), nullptr, &imageIndex)) {
            case VK_ERROR_OUT_OF_DATE_KHR:
                return UINT32_MAX;
            case VK_SUBOPTIMAL_KHR:
                break;
            case VK_SUCCESS:
                break;
            default:
                throw std::runtime_error("Failed to acquire next swap chain image");
        }
        return imageIndex;
    }

    bool CVulkanSwapChain::presentImage(const CVulkanQueue& queue, const sync::CVulkanSemaphore& renderCompleteSemaphore, const uint32_t& imageIndex) const {
        const VkSemaphore vkSemaphore = renderCompleteSemaphore.vkSemaphore();
        const VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &vkSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &this->_vkSwapChain,
            .pImageIndices = &imageIndex,
        };
        switch (VkResult result = vkQueuePresentKHR(queue.vkQueue(), &presentInfo)) {
            case VK_ERROR_OUT_OF_DATE_KHR:
                return true;
            case VK_SUBOPTIMAL_KHR:
            case VK_SUCCESS:
                break;
            default:
                throw std::runtime_error("Failed to present KHRe");
        }
        return false;
    }
}
