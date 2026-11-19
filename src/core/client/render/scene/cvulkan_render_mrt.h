//
// Created by ded on 16.11.2026.
//

#pragma once
#include "client/render/vulkan_render_core.h"

namespace cvulkan::client::render::vk {
    class CVulkanAttachment2D;
    class CVulkanContext;
}

namespace cvulkan::client::render::scene::mrt {
    constexpr VkFormat ALBEDO_FORMAT {VK_FORMAT_R16G16B16A16_SFLOAT};
    constexpr VkFormat DEPTH_FORMAT {VK_FORMAT_D32_SFLOAT};

    class CVulkanMultipleRenderTargets {
    public:
        explicit CVulkanMultipleRenderTargets(const vk::CVulkanContext& context);
        ~CVulkanMultipleRenderTargets() = default;

        CLASS_NO_COPY_NO_MOVE(CVulkanMultipleRenderTargets);

        void reset();

        [[nodiscard]] const vk::CVulkanAttachment2D& colorAlbedoAttachment() const {
            return *_colorAlbedoAttachment;
        }

        [[nodiscard]] const vk::CVulkanAttachment2D& depthAttachment() const {
            return *_depthAttachment;
        }

        [[nodiscard]] VkExtent3D extent3D() const {
            return {this->_context.swapChain().vkExtent2D().width, this->_context.swapChain().vkExtent2D().height, 1};
        }

        [[nodiscard]] VkExtent2D extent2D() const {
            return this->_context.swapChain().vkExtent2D();
        }

    protected:
        void createPtrs();

    private:
        const vk::CVulkanContext& _context;
        std::unique_ptr<vk::CVulkanAttachment2D> _colorAlbedoAttachment {};
        std::unique_ptr<vk::CVulkanAttachment2D> _depthAttachment {};
    };
}
