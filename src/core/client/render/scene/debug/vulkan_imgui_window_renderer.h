//
// Created by ded on 19.11.2026.
//

#pragma once
#include "client/render/vulkan_descriptors.h"

namespace cvulkan::client::render::scene::mrt {
    class CVulkanMultipleRenderTargets;
}

namespace cvulkan::client::render::vk {
    class CVulkanDescriptorAllocator;
    class CVulkanContext;
}

namespace cvulkan::client::render::debug::imgui {
    constexpr inline std::string_view DESC_ID_ALBEDO_TEXTURE_IMGUI = "DESC_ID_ALBEDO_TEXTURES";

    class CVulkanImguiDebugWindowRenderer {
    public:
        CVulkanImguiDebugWindowRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const scene::mrt::CVulkanMultipleRenderTargets& multipleRenderTargets);
        ~CVulkanImguiDebugWindowRenderer();

        void renderDebugWindow();
        void onResize();

    private:
        const vk::CVulkanContext& _context;
        vk::CVulkanDescriptorAllocator& _descriptorAllocator;
        const scene::mrt::CVulkanMultipleRenderTargets& _multipleRenderTargets;

        std::unique_ptr<vk::CVulkanSampler> _textureSampler{};
        std::unique_ptr<vk::CVulkanDescriptorSetLayout> _textureLayout{};
    };

    inline std::unique_ptr<CVulkanImguiDebugWindowRenderer> ImguiDebugWindowRenderer {};
    void createDebugImguiRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const scene::mrt::CVulkanMultipleRenderTargets& multipleRenderTargets);
    void destroyDebugImguiRenderer();
}
