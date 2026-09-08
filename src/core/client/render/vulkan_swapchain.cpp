//
// Created by ded on 07.09.2026.
//

#include "vulkan_swapchain.h"
#include "vulkan_render_core.h"

namespace cvulkan::client::renderer {
    void CVulkanImageView::createImageView(const CVulkanImageViewData& view_data, const VkImage& vk_image) {
        const VkImageViewCreateInfo image_view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = vk_image,
            .viewType = view_data.viewType,
            .format = view_data.format,
            .subresourceRange =  {
                .aspectMask = view_data.aspectMask,
                .baseMipLevel = 0,
                .levelCount = view_data.mipLevels,
                .baseArrayLayer = view_data.baseArrayLayer,
                .layerCount = view_data.layerCount
            }
        };
        utility::vkCheck(vkCreateImageView(this->_context.deviceData().vkDevice, &image_view_create_info, nullptr, &this->_vkImageView), "Failed to createImageView image view");
    }

    void CVulkanImageView::destroyImageView() {
        if (this->_vkImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(this->_context.deviceData().vkDevice, this->_vkImageView, nullptr);
            this->_vkImageView = VK_NULL_HANDLE;
        }
    }

    void CVulkanSwapChain::createSwapChain(const VkSurfaceKHR& vkSurface, const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities, const VkFormat& vkFormat, const VkColorSpaceKHR& vkColorSpace) {
        const uint32_t requestedImages = 3;

        logging::info("Setting up vulkan vkSwapChain");
        uint32_t imageCount = -1;
        VkExtent2D extent = {};

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
                extent.width = std::clamp(windowSize.x,vkSurfaceCapabilities.minImageExtent.width,vkSurfaceCapabilities.maxImageExtent.width);
                extent.height = std::clamp(windowSize.y,vkSurfaceCapabilities.minImageExtent.height,vkSurfaceCapabilities.maxImageExtent.height);
            } else {
                extent = vkSurfaceCapabilities.currentExtent;
            }
        }

        VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = vkSurface;
        swapChainCreateInfo.minImageCount = imageCount;
        swapChainCreateInfo.imageFormat = vkFormat;
        swapChainCreateInfo.imageColorSpace = vkColorSpace;
        swapChainCreateInfo.imageExtent = extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapChainCreateInfo.preTransform = vkSurfaceCapabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.clipped = VK_TRUE;

        if (true) {
            swapChainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        } else {
            swapChainCreateInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

        utility::vkCheck(vkCreateSwapchainKHR(this->_context.deviceData().vkDevice, &swapChainCreateInfo, nullptr, &this->_vkSwapChain));

        {
            uint32_t swapChainImagesCount = 0;
            utility::vkCheck(vkGetSwapchainImagesKHR(this->_context.deviceData().vkDevice, this->_vkSwapChain, &swapChainImagesCount, nullptr), "Failed to create swapChain Images");
            std::vector<VkImage> images(swapChainImagesCount);
            utility::vkCheck(vkGetSwapchainImagesKHR(this->_context.deviceData().vkDevice, this->_vkSwapChain, &swapChainImagesCount, images.data()), "Failed to create swapChain Images");

            const CVulkanImageViewData image_view_data = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .format = vkFormat};
            this->_imageViews.reserve(images.size());
            for (const auto image : images) {
                this->_imageViews.emplace_back(this->_context);
                this->_imageViews.back().createImageView(image_view_data, image);
            }
        }

        logging::info("Created vkSwapChain imageViews");
    }

    void CVulkanSwapChain::destroySwapChain() {
        for (auto& t : this->_imageViews) {
            if (t.vkImageView() != VK_NULL_HANDLE) {
                t.destroyImageView();
            }
        }
        if (this->_vkSwapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(this->_context.deviceData().vkDevice, this->_vkSwapChain, nullptr);
            this->_vkSwapChain = VK_NULL_HANDLE;
        }
    }

    void CVulkanQueue::initQueue(const uint32_t queueFamilyIndex, const uint32_t queueIndex) {
        vkGetDeviceQueue(this->_context.deviceData().vkDevice, queueFamilyIndex, queueIndex, &this->_vkQueue);
        this->_queueFamilyIndex = queueFamilyIndex;
    }

    void CVulkanSurface::calcSurfaceFormat(const CVulkanPhysicalDevice &physical_device_data, const VkSurfaceKHR& vkSurface, VkFormat& vkFormat, VkColorSpaceKHR& vkColorSpace) {
        uint32_t surfaceFormatCount = 0;
        utility::vkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_data.vkPhysicalDevice, vkSurface, &surfaceFormatCount, nullptr));
        if (surfaceFormatCount == 0) {
            throw std::runtime_error("Failed to get surface format count");
        }
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        utility::vkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_data.vkPhysicalDevice, vkSurface, &surfaceFormatCount, surfaceFormats.data()));
        vkFormat = VK_FORMAT_B8G8R8A8_SRGB;
        vkColorSpace = surfaceFormats[0].colorSpace;
        for (const auto&[f, c] : surfaceFormats) {
            if (f == VK_FORMAT_B8G8R8A8_SRGB && c == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                vkFormat = f;
                vkColorSpace = c;
                logging::info("Setting up format {}, color space {}", static_cast<int>(f), static_cast<int>(c));
                break;
            }
        }
    }

    void CVulkanSurface::createSurface() {
        logging::info("Setting up GLFW surface");
        utility::vkCheck(glfwCreateWindowSurface(this->_context.instanceData().vkInstance, this->_context.glfwWindow().glfw_window_descriptor(), nullptr, &this->_vkSurface));
        utility::vkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->_context.physicalDeviceData().vkPhysicalDevice, this->_vkSurface, &this->_vkSurfaceCapabilities));
        calcSurfaceFormat(this->_context.physicalDeviceData(), this->_vkSurface, this->_vkFormat, this->_vkColorSpace);
        this->_swapChain.createSwapChain(this->_vkSurface, this->_vkSurfaceCapabilities, this->_vkFormat, this->_vkColorSpace);
    }

    void CVulkanSurface::destroySurface() {
        this->_swapChain.destroySwapChain();
        if (this->_vkSurface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(this->_context.instanceData().vkInstance, this->_vkSurface, nullptr);
            this->_vkSurface = VK_NULL_HANDLE;
        }
    }
}
