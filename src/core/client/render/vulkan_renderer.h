//
// Created by ded on 09.09.2026.
//

#pragma once
#include <array>
#include <memory>
#include <optional>

#include "vulkan_config.h"
#include "vulkan_commands.h"
#include "vulkan_synchronization.h"

namespace cvulkan::client::renderCore {
    class CVulkanContext;
}

namespace cvulkan::client::renderLoop {
    class CVulkanRenderPipeline {
    public:
        explicit CVulkanRenderPipeline(const renderCore::CVulkanContext& context)
            : _context{context}, graphicsQueue(context), presentQueue(context) {
        }

        ~CVulkanRenderPipeline() = default;

        CVulkanRenderPipeline(const CVulkanRenderPipeline&) = delete;
        CVulkanRenderPipeline& operator=(const CVulkanRenderPipeline&) = delete;

        void initRenderLoop();
        void renderLoop();
        void destroyPipelineResources();

    protected:

    private:
        const renderCore::CVulkanContext& _context;
        std::vector<renderCore::CVulkanCommandPool> _commandPools = {};
        std::vector<renderCore::CVulkanCommandBuffer> _commandBuffers = {};
        std::vector<renderSync::CVulkanFence> _fences = {};
        std::vector<renderSync::CVulkanSemaphore> _presentationCompleteSemaphores = {};
        std::vector<renderSync::CVulkanSemaphore> _renderCompleteSemaphores = {};
        uint32_t currentFrame = 0;
        renderCore::CVulkanQueue graphicsQueue;
        renderCore::CVulkanQueue presentQueue;
    };

    extern std::unique_ptr<CVulkanRenderPipeline> renderPipeline;

    void initRendering(const renderCore::CVulkanContext& context);
    void runRendering();
    void destroyRendering();
}
