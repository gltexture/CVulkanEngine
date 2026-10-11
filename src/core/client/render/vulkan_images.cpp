//
// Created by ded on 24.09.2026.
//

#include "vulkan_images.h"

#include "vulkan_render_core.h"

namespace cvulkan::client::render::vk {
    CVulkanImage2D::CVulkanImage2D(const CVulkanContext& context, const CVulkanImage2DData& imageData) : _context(context) {
        this->_format = imageData._format;
        this->_mipLevels = imageData._mipLevels;

        const VkImageCreateInfo vkImageCreateInfo  {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = imageData._format,
            .extent = imageData._extent,
            .mipLevels = imageData._mipLevels,
            .arrayLayers = imageData._arrayLayers,
            .samples = imageData._sampleCount,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = imageData._usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        utility::vkCheck(vkCreateImage(context.device().vkDevice(), &vkImageCreateInfo, nullptr, &this->_vkImage), "Failed to create image");

        VkMemoryRequirements vkMemoryRequirements{};
        vkGetImageMemoryRequirements(context.device().vkDevice(), this->_vkImage, &vkMemoryRequirements);

        VkMemoryAllocateInfo vkMemoryAllocateInfo  {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = vkMemoryRequirements.size,
            .memoryTypeIndex = utility::getMemoryTypeIndexFromProperties(context.physicalDevice().vkMemoryProperties(), vkMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
        };

        utility::vkCheck(vkAllocateMemory(context.device().vkDevice(), &vkMemoryAllocateInfo, nullptr, &this->_memory),  "Failed to allocate memory");
        utility::vkCheck(vkBindImageMemory(context.device().vkDevice(), this->_vkImage, this->_memory, 0), "Failed to bind image memory");
    }

    CVulkanImage2D::~CVulkanImage2D() {
        if (this->_vkImage != VK_NULL_HANDLE) {
            vkDestroyImage(this->_context.device().vkDevice(), this->_vkImage, nullptr);
            this->_vkImage = VK_NULL_HANDLE;
        }
        if (this->_memory != VK_NULL_HANDLE) {
            vkFreeMemory(this->_context.device().vkDevice(), this->_memory, nullptr);
            this->_memory = VK_NULL_HANDLE;
        }
    }

    CVulkanImage2DView::CVulkanImage2DView(const CVulkanContext& context, const CVulkanImage2DViewData& view_data, const VkImage& vk_image) : _context(context) {
        const VkImageViewCreateInfo image_view_create_info  {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = vk_image,
            .viewType = view_data._viewType,
            .format = view_data._format,
            .subresourceRange  {
                .aspectMask = view_data._aspectMask,
                .baseMipLevel = 0,
                .levelCount = view_data._mipLevels,
                .baseArrayLayer = view_data._baseArrayLayer,
                .layerCount = view_data._layerCount
            }
        };
        utility::vkCheck(vkCreateImageView(this->_context.device().vkDevice(), &image_view_create_info, nullptr, &this->_vkImageView), "Failed to createImageView image view");
        this->_vkImage = vk_image;
    }

    CVulkanImage2DView::~CVulkanImage2DView() {
        if (this->_vkImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(this->_context.device().vkDevice(), this->_vkImageView, nullptr);
            this->_vkImageView = VK_NULL_HANDLE;
        }
    }

    CVulkanAttachment2D::CVulkanAttachment2D(const CVulkanContext& context, CVulkanImage2D&& image, CVulkanImage2DView&& view, const bool depthAttachment)
    : _context(context), _image(std::move(image)), _view(std::move(view)), depthAttachment(depthAttachment) {
    }

    CVulkanAttachment2D::~CVulkanAttachment2D() {
    }

    CVulkanAttachment2D CVulkanAttachment2D::createSimpleAttachment(const CVulkanContext& context, const VkExtent3D& extent, const VkFormat format, const VkImageUsageFlags usage) {
        const CVulkanImage2DData imageData{
            ._format = format,
            ._usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT,
            ._extent = extent
        };

        CVulkanImage2D image{context, imageData};

        VkImageAspectFlags aspectMask{};
        bool depthAttachment{};

        if ((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0) {
            aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            depthAttachment = false;
        }

        if ((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            depthAttachment = true;
        }

        const CVulkanImage2DViewData imageViewData{
            ._aspectMask = aspectMask,
            ._format = image.format()
        };

        CVulkanImage2DView view{context, imageViewData, image.vkImage()};

        return CVulkanAttachment2D{context, std::move(image), std::move(view), depthAttachment};
    }
}
