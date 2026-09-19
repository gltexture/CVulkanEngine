//
// Created by ded on 11.09.2026.
//

#pragma once

#include <vector>
#include "client/render/vulkan_pipeline.h"
#include "client/render/vulkan_utility.h"

namespace cvulkan::client::render::loop {
    class CVulkanRenderLoop;
}

namespace cvulkan::client::render::core {
    class CVulkanCommandBuffer;
    class CVulkanContext;
}

namespace cvulkan::client::render::scene {
    class CVulkanSceneRenderer {
    public:
        CVulkanSceneRenderer(const core::CVulkanContext& context, const loop::CVulkanRenderLoop& renderLoop)
            : _context(context), _renderLoop(renderLoop), _defaultRenderPipeline{context} {
        }

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
        std::vector<VkRenderingAttachmentInfo> _vkColorAttachments{};
        std::vector<VkRenderingInfo> _vkRendering{};
        core::CVulkanPipeline _defaultRenderPipeline;
    };
}
