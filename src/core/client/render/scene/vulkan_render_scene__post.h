//
// Created by ded on 04.11.2026.
//
#pragma once
#include "client/render/vulkan_buffers.h"

namespace cvulkan::client::render::scene::context {
    class CVulkanSceneContext;
}

namespace cvulkan::client::render::vk {
    class CVulkanSampler;
    class CVulkanPipeline;
    class CVulkanDescriptorSetLayout;
    class CVulkanAttachment2D;
    class CVulkanContext;
}

namespace cvulkan::client::render::scene::post {
    constexpr VkFormat COLOR_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
    constexpr std::string_view DESC_ID_ATT = "POST_DESC_ID_ATT";
    constexpr std::string_view DESC_ID_SCREEN_SIZE = "POST_DESC_ID_SCREEN_SIZE";

    constexpr VkClearValue CLEAR_COLOR {
        .color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}
    };

    class CVulkanPostFXRenderer {
    public:
        CVulkanPostFXRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const vk::CVulkanAttachment2D& srcAttachment);
        ~CVulkanPostFXRenderer();

        CLASS_NO_COPY_NO_MOVE(CVulkanPostFXRenderer)

        void renderPostFX(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanAttachment2D& srcAttachment) const;

        void onResize(const vk::CVulkanAttachment2D& srcAttachment);

        [[nodiscard]] const vk::CVulkanAttachment2D& colorAttachment() const {
            return *_colorAttachment;
        }

    protected:
        void setSrcSizeBuffer() const;
        void createRenderingData();

    private:
        const vk::CVulkanContext& _context;
        vk::CVulkanDescriptorAllocator& _descriptorAllocator;
        std::unique_ptr<vk::CVulkanAttachment2D> _colorAttachment {};
        VkRenderingAttachmentInfo _colorAttachmentInfo {};
        VkRenderingInfo _renderingInfo {};
        std::unique_ptr<vk::CVulkanSampler> _textureSampler {};
        std::unique_ptr<vk::CVulkanBuffer> _srcSizeBuffer {};
        std::unique_ptr<vk::CVulkanPipeline> _postPipeline {};
        std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descriptorSetLayout_texture {};
    };
}
