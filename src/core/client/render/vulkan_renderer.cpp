//
// Created by ded on 09.09.2026.
//

#include "vulkan_renderer.h"

#include "vulkan_commands.h"

namespace cvulkan::client::renderLoop {
    std::unique_ptr<CVulkanRenderPipeline> renderPipeline;

    void CVulkanRenderPipeline::initRenderLoop() {
        uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t presentationQueueFamilyIndex = UINT32_MAX;
        for (const auto& t : this->_context.queueFamiliesRegistry().registeredData()) {
            if (t.bitMask & renderCore::CVulkanQueueFamilyBitMasks::GRAPHICS) {
                graphicsQueueFamilyIndex = t.queueFamilyIndex;
                break;
            }
        }
        for (const auto& t : this->_context.queueFamiliesRegistry().registeredData()) {
            if (t.bitMask & renderCore::CVulkanQueueFamilyBitMasks::PRESENT) {
                presentationQueueFamilyIndex = t.queueFamilyIndex;
                break;
            }
        }
        if (graphicsQueueFamilyIndex == UINT32_MAX) {
            throw std::runtime_error("Couldn't find graphicsQueueFamilyIndex in registry");
        }
        if (presentationQueueFamilyIndex == UINT32_MAX) {
            throw std::runtime_error("Couldn't find presentationQueueFamilyIndex in registry");
        }
        {
            this->_commandBuffers.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);
            this->_fences.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);
            this->_presentationCompleteSemaphores.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);
            this->_renderCompleteSemaphores.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);

            for (uint32_t i = 0; i < renderConfig::MAX_IMAGES_IN_FLIGHT; ++i) {
                this->_commandBuffers.emplace_back();
                this->_fences.emplace_back();
                this->_presentationCompleteSemaphores.emplace_back();
                this->_renderCompleteSemaphores.emplace_back();
            }
        }
    }

    void CVulkanRenderPipeline::renderLoop() {

    }

    void CVulkanRenderPipeline::destroyPipelineResources() {

    }

    void initRendering(const renderCore::CVulkanContext& context) {
        renderPipeline = std::make_unique<CVulkanRenderPipeline>(context);
        renderPipeline->initRenderLoop();
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