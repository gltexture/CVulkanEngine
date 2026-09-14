//
// Created by ded on 11.09.2026.
//

#pragma once
#include <vulkan/vulkan.h>

#include "client/render/vulkan_render_core.h"
#include "client/render/vulkan_render_loop.h"

namespace cvulkan::client::renderScene {
    class CVulkanSceneRenderer {
    public:
        CVulkanSceneRenderer(const renderCore::CVulkanContext& context, const renderLoop::CVulkanRenderLoop& renderLoop)
            : _context(context), _renderLoop(renderLoop) {}
        ~CVulkanSceneRenderer() = default;

        CVulkanSceneRenderer(const CVulkanSceneRenderer&) = delete;
        CVulkanSceneRenderer& operator=(const CVulkanSceneRenderer&) = delete;

        void initScene();
        void renderScene(const renderCore::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const;
        void destroyScene();

    protected:

    private:
        const renderCore::CVulkanContext& _context;
        const renderLoop::CVulkanRenderLoop& _renderLoop;
        static constexpr VkClearValue VK_CLEAR_VALUE = {0.5f, 0.7f, 0.9f, 1.0f};
        std::vector<VkRenderingAttachmentInfo> _vkColorAttachments {};
        std::vector<VkRenderingInfo> _vkRendering {};
    };
}
