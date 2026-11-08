//
// Created by ded on 06.11.2026.
//

#pragma once
#include "client/render/vulkan_config.hpp"
#include "utility/sys_utils.h"

namespace cvulkan::client::render::scene::context {
    class CVulkanSceneContext;
}

namespace cvulkan::client::render::vk {
    class CVulkanDescriptorSetLayout;
    class CVulkanPipeline;
    class CVulkanSampler;
    class CVulkanCommandBuffer;
    class CVulkanAttachment2D;
    class CVulkanDescriptorAllocator;
    class CVulkanContext;
}

namespace cvulkan::client::render::scene::swapchain {
    constexpr std::string_view DESC_ID_ATT = "FWD_DESC_ID_ATT";

    constexpr VkClearValue CLEAR_COLOR {
        .color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}
    };

    class CVulkanSwapchainRenderer {
    public:
        CVulkanSwapchainRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const vk::CVulkanAttachment2D& srcAttachment);
        ~CVulkanSwapchainRenderer();

        CLASS_NO_COPY_NO_MOVE(CVulkanSwapchainRenderer)

        void renderSwapchain(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanAttachment2D& srcAttachment, uint32_t imageIndex) const;

        void onResize(const vk::CVulkanAttachment2D& srcAttachment);

    protected:
        void createRenderingData();

    private:
        const vk::CVulkanContext& _context;
        vk::CVulkanDescriptorAllocator& _descriptorAllocator;
        std::array<VkRenderingAttachmentInfo, renderConfig::SWAP_CHAIN_IMAGES> _colorAttachmentsInfo {};
        std::array<VkRenderingInfo, renderConfig::SWAP_CHAIN_IMAGES> _renderingInfo {};
        std::unique_ptr<vk::CVulkanSampler> _textureSampler {};
        std::unique_ptr<vk::CVulkanPipeline> _swapchainPipeline {};
        std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descriptorSetLayout_texture {};
    };
}
