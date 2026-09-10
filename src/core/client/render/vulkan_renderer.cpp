//
// Created by ded on 09.09.2026.
//

#include "vulkan_renderer.h"

#include "vulkan_commands.h"

namespace cvulkan::client::renderLoop {
    std::unique_ptr<CVulkanRenderPipeline> renderPipeline;

    void CVulkanRenderPipeline::initRenderLoop() {

    }

    void CVulkanRenderPipeline::renderLoop() {
    }

    void CVulkanRenderPipeline::destroyPipelineResources() {
    }

    void initRendering(const renderCore::CVulkanContext& context) {
        renderPipeline = std::make_unique<CVulkanRenderPipeline>(context);
    }

    void runRendering() {
        if (renderPipeline != nullptr) {
            renderPipeline->renderLoop();
        }
    }

    void destroyRendering() {
        if (renderPipeline != nullptr) {
            renderPipeline->destroyPipelineResources();
            renderPipeline.reset();
        }
    }
}
