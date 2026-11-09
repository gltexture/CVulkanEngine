//
// Created by gltex on 08.11.2026.
//

#pragma once
#include <imgui.h>

#include "utility/sys_utils.h"
#include "client/render/vulkan_buffers.h"
#include "client/render/vulkan_config.hpp"

namespace cvulkan::client::render::cache {
    class CVulkanCacheCollection;
}

namespace cvulkan::client::render::asset {
    class CVulkanTexture2D;
}

namespace cvulkan::client::render::vk {
    class CVulkanDescriptorAllocator;
    class CVulkanContext;
}

namespace cvulkan::client::render::scene::imgui {
    constexpr std::string_view DESC_ID_TEXTURE = "GUI_DESC_ID_TEXTURE";
    constexpr VkFormat COLOR_FORMAT {VK_FORMAT_R16G16B16A16_SFLOAT};

    struct CVulkanImGuiTextureData {
        uint64_t _id;
        std::string _texturePath;
    };

    class CVulkanImGuiRenderer {
    public:
        CVulkanImGuiRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const vk::CVulkanAttachment2D& dstAttachment, const cache::CVulkanCacheCollection& caches);
        ~CVulkanImGuiRenderer();

        CLASS_NO_COPY_NO_MOVE(CVulkanImGuiRenderer);

        void renderImGui(const vk::CVulkanCommandBuffer& commandBuffer) const;

        void onResize();

        void loadTextures(const std::span<CVulkanImGuiTextureData> textures);

    protected:
        void createRenderingData(const vk::CVulkanAttachment2D& dstAttachment);
        void initImGui();

    private:
        const vk::CVulkanContext& _context;
        vk::CVulkanDescriptorAllocator& _descriptorAllocator;
        const cache::CVulkanCacheCollection& _caches;
        VkRenderingAttachmentInfo _colorAttachmentInfo {};
        VkRenderingInfo _renderingInfo {};
        std::array<std::unique_ptr<vk::CVulkanBuffer>, renderConfig::SWAP_CHAIN_IMAGES> _vtxBuffers{};
        std::array<std::unique_ptr<vk::CVulkanBuffer>, renderConfig::SWAP_CHAIN_IMAGES> _idxBuffers{};
        std::unique_ptr<asset::CVulkanTexture2D> _fontTexture {};
        std::unique_ptr<vk::CVulkanPipeline> _imguiPipeline {};
        std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descriptorSetLayout_texture {};
        std::unordered_map<VkImageView, ImTextureID> _guiTextures;
    };
}
