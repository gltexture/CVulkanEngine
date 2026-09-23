//
// Created by ded on 11.09.2026.
//

#pragma once

#include <vector>

#include "vulkan_render_cache.h"
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
        CVulkanSceneRenderer(const core::CVulkanContext& context, const loop::CVulkanRenderLoop& renderLoop);
        ~CVulkanSceneRenderer();

        CVULKAN_NO_COPY(CVulkanSceneRenderer);

        void renderScene(const core::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const;

        [[nodiscard]] const cache::CVulkanCacheCollection& caches() const {
            return _caches;
        }

    protected:
        void loadResources();

    private:
        const core::CVulkanContext& _context;
        cache::CVulkanCacheCollection _caches;
        const loop::CVulkanRenderLoop& _renderLoop;
        std::unique_ptr<core::CVulkanPipeline> _defaultRenderPipeline {};
        std::vector<VkRenderingAttachmentInfo> _vkColorAttachments{};
        std::vector<VkRenderingInfo> _vkRendering{};
    };
}
