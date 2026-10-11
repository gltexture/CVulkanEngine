//
// Created by ded on 24.09.2026.
//

#pragma once
#include "vulkan_utility.h"

namespace cvulkan::client::render::vk {
    class CVulkanContext;

    struct CVulkanImage2DData {
        uint32_t _arrayLayers = 1;
        VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB;
        uint32_t _mipLevels = 1;
        VkSampleCountFlagBits _sampleCount = VK_SAMPLE_COUNT_1_BIT;
        VkImageUsageFlags _usage;
        VkExtent3D _extent;
    };

    struct CVulkanImage2DViewData {
        VkImageAspectFlags _aspectMask{};
        uint32_t _baseArrayLayer = 0;
        VkFormat _format{};
        uint32_t _layerCount = 1;
        uint32_t _mipLevels = 1;
        VkImageViewType _viewType = VK_IMAGE_VIEW_TYPE_2D;
    };

    class CVulkanImage2D {
    public:
        CVulkanImage2D(const CVulkanContext& context, const CVulkanImage2DData& imageData);
        ~CVulkanImage2D();

        CVULKAN_NO_COPY(CVulkanImage2D)
        CVulkanImage2D(CVulkanImage2D&& other) noexcept
            : _context(other._context), _vkImage(other._vkImage), _format(other._format), _mipLevels(other._mipLevels), _memory(other._memory) {
            other._vkImage = VK_NULL_HANDLE;
            other._memory = VK_NULL_HANDLE;
        }

        [[nodiscard]] VkImage vkImage() const {
            return _vkImage;
        }

        [[nodiscard]] const VkFormat& format() const {
            return _format;
        }

        [[nodiscard]] uint32_t mipLevels() const {
            return _mipLevels;
        }

        [[nodiscard]] VkDeviceMemory memory() const {
            return _memory;
        }

    private:
        const CVulkanContext& _context;
        VkImage _vkImage{};
        VkFormat _format{};
        uint32_t _mipLevels = -1;
        VkDeviceMemory _memory{};
    };

    class CVulkanImage2DView {
    public:
        explicit CVulkanImage2DView(const CVulkanContext& context, const CVulkanImage2DViewData& view_data, const VkImage& vk_image);
        ~CVulkanImage2DView();

        CVULKAN_NO_COPY_NO_ASSIGN_MOVE(CVulkanImage2DView);

        CVulkanImage2DView(CVulkanImage2DView&& other) noexcept : _context{other._context}, _vkImage{other._vkImage}, _vkImageView{other._vkImageView} {
            other._vkImage = VK_NULL_HANDLE;
            other._vkImageView = VK_NULL_HANDLE;
        }

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

    class CVulkanAttachment2D {
    public:
        CVulkanAttachment2D(const CVulkanContext& context, CVulkanImage2D&& image, CVulkanImage2DView&& view, bool depthAttachment);
        ~CVulkanAttachment2D();

        CVULKAN_NO_COPY(CVulkanAttachment2D)
        CVulkanAttachment2D(CVulkanAttachment2D&& other) noexcept = default;

        [[nodiscard]] const CVulkanImage2D& image() const {
            return _image;
        }

        [[nodiscard]] const CVulkanImage2DView& view() const {
            return _view;
        }

        [[nodiscard]] bool depthAttachment1() const {
            return depthAttachment;
        }

        static CVulkanAttachment2D createSimpleAttachment(const CVulkanContext& context, const VkExtent3D& extent, VkFormat format, VkImageUsageFlags usage);

    private:
        const CVulkanContext& _context;
        CVulkanImage2D _image;
        CVulkanImage2DView _view;
        bool depthAttachment;
    };
}