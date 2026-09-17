//
// Created by ded on 11.09.2026.
//

#pragma once
#include <vulkan/vulkan.h>

#include "client/render/vulkan_render_core.h"
#include "client/render/vulkan_render_loop.h"

namespace cvulkan::client::render::scene {
    class CVulkanSceneRenderer {
    public:
        CVulkanSceneRenderer(const core::CVulkanContext& context, const loop::CVulkanRenderLoop& renderLoop)
            : _context(context), _renderLoop(renderLoop), _defaultRenderPipeline{context} {}
        ~CVulkanSceneRenderer() = default;

        CVULKAN_NO_COPY(CVulkanSceneRenderer);

        void createScene();
        void renderScene(const core::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const;
        void destroyScene();

    protected:
        void loadResources();

    private:
        const core::CVulkanContext& _context;
        const loop::CVulkanRenderLoop& _renderLoop;
        static constexpr VkClearValue VK_CLEAR_VALUE = {0.5f, 0.7f, 0.9f, 1.0f};
        std::vector<VkRenderingAttachmentInfo> _vkColorAttachments {};
        std::vector<VkRenderingInfo> _vkRendering {};
        core::CVulkanPipeline _defaultRenderPipeline;
    };
}
