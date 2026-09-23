//
// Created by ded on 09.09.2026.
//

#include "vulkan_render_loop.h"

#include "vulkan_commands.h"
#include "vulkan_config.h"
#include "scene/vulkan_render_scene.h"

namespace cvulkan::client::render::loop {
    std::unique_ptr<scene::CVulkanSceneRenderer> sceneRenderer;
    std::unique_ptr<CVulkanRenderLoop> renderLoop;

    CVulkanRenderLoop::CVulkanRenderLoop(const core::CVulkanContext& context) : _context{context} {
        uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t presentationQueueFamilyIndex = UINT32_MAX;
        for (const auto& t: this->_context.device().queueFamiliesRegistry().registeredData()) {
            if (t._bitMask & core::CVulkanQueueFamilyBitMasks::GRAPHICS) {
                graphicsQueueFamilyIndex = t._queueFamilyIndex;
                break;
            }
        }
        for (const auto& t: this->_context.device().queueFamiliesRegistry().registeredData()) {
            if (t._bitMask & core::CVulkanQueueFamilyBitMasks::PRESENT) {
                presentationQueueFamilyIndex = t._queueFamilyIndex;
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
            this->_commandPools.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);
            this->_commandBuffers.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);
            this->_fences.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);
            this->_presentationCompleteSemaphores.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);
            this->_renderCompleteSemaphores.reserve(this->_context.swapChain().numImages());

            for (uint32_t i = 0; i < renderConfig::MAX_IMAGES_IN_FLIGHT; ++i) {
                this->_commandPools.emplace_back(this->_context, graphicsQueueFamilyIndex, false);
                this->_commandBuffers.emplace_back(this->_context, this->_commandPools.back(), true, true);
                this->_fences.emplace_back(this->_context, true);
                this->_presentationCompleteSemaphores.emplace_back(this->_context);
            }

            for (uint32_t i = 0; i < this->_context.swapChain().numImages(); ++i) {
                this->_renderCompleteSemaphores.emplace_back(this->_context);
            }
        }

        {
            this->_graphicsQueue =std::make_unique<core::CVulkanQueue>(this->_context, graphicsQueueFamilyIndex, 0);
            this->_presentQueue =std::make_unique<core::CVulkanQueue>(this->_context, presentationQueueFamilyIndex, 0);
        }
    }

    CVulkanRenderLoop::~CVulkanRenderLoop() {
    }

    void CVulkanRenderLoop::recordingStart(const core::CVulkanCommandPool& commandPool, const core::CVulkanCommandBuffer& commandBuffer) {
        commandPool.reset();
        commandBuffer.beginRecording();
    }

    void CVulkanRenderLoop::recordingStop(const core::CVulkanCommandBuffer& commandBuffer) {
        commandBuffer.endRecording();
    }

    void CVulkanRenderLoop::waitForFence() const {
        this->_fences[this->_currentFrame].wait();
    }

    void CVulkanRenderLoop::submit(const core::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const {
        const sync::CVulkanFence& fence = this->_fences[this->_currentFrame];
        fence.reset();
        const VkCommandBufferSubmitInfo commands = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = commandBuffer.vkCommandBuffer()
        };
        const VkSemaphoreSubmitInfo waitSemaphores = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = this->_presentationCompleteSemaphores[this->_currentFrame].vkSemaphore(),
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
        };
        const VkSemaphoreSubmitInfo signalSemaphores = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = this->_renderCompleteSemaphores[imageIndex].vkSemaphore(),
            .stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
        };
        const std::vector v_commandSubmitInfos{commands};
        const std::vector v_waitSemaphores{waitSemaphores};
        const std::vector v_signalSemaphores{signalSemaphores};
        this->_graphicsQueue->submitQueue(v_commandSubmitInfos, &v_waitSemaphores, &v_signalSemaphores, &fence);
    }

    void CVulkanRenderLoop::runRenderLoop() {
        const core::CVulkanCommandPool& commandPool = this->_commandPools[this->_currentFrame];
        const core::CVulkanCommandBuffer& commandBuffer = this->_commandBuffers[this->_currentFrame];

        this->waitForFence();
        recordingStart(commandPool, commandBuffer);

        const uint32_t imageIndex = this->_context.swapChain().acquireSwapChainNextImage(this->_presentationCompleteSemaphores[this->_currentFrame]);
        if (imageIndex == UINT32_MAX) {
            recordingStop(commandBuffer);
            return;
        }
        if (sceneRenderer != nullptr) {
            sceneRenderer->renderScene(commandBuffer, imageIndex);
        }

        recordingStop(commandBuffer);
        this->submit(commandBuffer, imageIndex);

        this->_context.swapChain().presentImage(*this->_presentQueue, this->_renderCompleteSemaphores[imageIndex], imageIndex);
        this->_currentFrame = (this->_currentFrame + 1) % renderConfig::MAX_IMAGES_IN_FLIGHT;
    }

    void createRendering(const core::CVulkanContext& context) {
        renderLoop = std::make_unique<CVulkanRenderLoop>(context);
        sceneRenderer = std::make_unique<scene::CVulkanSceneRenderer>(context, *renderLoop);
    }

    void destroyRendering() {
        if (sceneRenderer != nullptr) {
            sceneRenderer.reset();
        }
        if (renderLoop != nullptr) {
            renderLoop.reset();
        }
    }

    void runRendering() {
        if (renderLoop != nullptr) {
            renderLoop->runRenderLoop();
        }
    }
}
