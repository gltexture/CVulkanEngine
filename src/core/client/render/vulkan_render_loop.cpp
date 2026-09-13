//
// Created by ded on 09.09.2026.
//

#include "vulkan_render_loop.h"

#include "vulkan_commands.h"
#include "vulkan_render_scene.h"

namespace cvulkan::client::renderLoop {
    std::unique_ptr<CVulkanRenderLoop> renderLoop;
    std::unique_ptr<renderScene::CVulkanSceneRenderer> sceneRenderer;

    void CVulkanRenderLoop::initRenderLoop() {
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
            this->_commandPools.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);
            this->_commandBuffers.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);
            this->_fences.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);
            this->_presentationCompleteSemaphores.reserve(renderConfig::MAX_IMAGES_IN_FLIGHT);
            this->_renderCompleteSemaphores.reserve(this->_context.surface().swapChain().numImages());

            for (uint32_t i = 0; i < renderConfig::MAX_IMAGES_IN_FLIGHT; ++i) {
                this->_commandPools.emplace_back(this->_context, graphicsQueueFamilyIndex, false);
                this->_commandBuffers.emplace_back(this->_context, this->_commandPools.back(), true, true);
                this->_fences.emplace_back(this->_context);
                this->_presentationCompleteSemaphores.emplace_back(this->_context);

                {
                    this->_commandPools[i].initCommandPool();
                    this->_commandBuffers[i].initCommandBuffer();
                    this->_fences[i].initFence(true);
                    this->_presentationCompleteSemaphores[i].initSemaphore();
                }
            }

            for (uint32_t i = 0; i < this->_context.surface().swapChain().numImages(); ++i) {
                this->_renderCompleteSemaphores.emplace_back(this->_context);

                {
                    this->_renderCompleteSemaphores[i].initSemaphore();
                }
            }
        }

        {
            this->_graphicsQueue.initQueue(graphicsQueueFamilyIndex, 0);
            this->_presentQueue.initQueue(presentationQueueFamilyIndex, 0);
        }
    }

    void CVulkanRenderLoop::recordingStart(const renderCore::CVulkanCommandPool& commandPool, const renderCore::CVulkanCommandBuffer& commandBuffer) {
        commandPool.reset();
        commandBuffer.beginRecording();
    }

    void CVulkanRenderLoop::recordingStop(const renderCore::CVulkanCommandBuffer& commandBuffer) {
        commandBuffer.endRecording();
    }

    void CVulkanRenderLoop::waitForFence() const {
        this->_fences[this->_currentFrame].wait();
    }

    void CVulkanRenderLoop::submit(const renderCore::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const {
        const renderSync::CVulkanFence& fence = this->_fences[this->_currentFrame];
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
        const std::vector v_commandSubmitInfos {commands};
        const std::vector v_waitSemaphores {waitSemaphores};
        const std::vector v_signalSemaphores {signalSemaphores};
        this->_graphicsQueue.submit(v_commandSubmitInfos, &v_waitSemaphores, &v_signalSemaphores, &fence);
    }

    void CVulkanRenderLoop::runRenderLoop() {
        const renderCore::CVulkanCommandPool& commandPool = this->_commandPools[this->_currentFrame];
        const renderCore::CVulkanCommandBuffer& commandBuffer = this->_commandBuffers[this->_currentFrame];

        this->waitForFence();
        recordingStart(commandPool, commandBuffer);

        const uint32_t imageIndex = this->_context.surface().swapChain().acquireSwapChainNextImage(this->_presentationCompleteSemaphores[this->_currentFrame]);
        if (imageIndex == UINT32_MAX) {
            recordingStop(commandBuffer);
            return;
        }
        if (sceneRenderer != nullptr) {
            sceneRenderer->renderScene(commandBuffer, imageIndex);
        }

        recordingStop(commandBuffer);
        this->submit(commandBuffer, imageIndex);

        this->_context.surface().swapChain().presentImage(this->_presentQueue, this->_renderCompleteSemaphores[imageIndex], imageIndex);
        this->_currentFrame = (this->_currentFrame + 1) % renderConfig::MAX_IMAGES_IN_FLIGHT;
    }

    void CVulkanRenderLoop::destroyRenderLoopResources() {
        this->_context.device().deviceWaitIdle();

        for (auto& t : this->_renderCompleteSemaphores) {
            t.destroySemaphore();
        }
        for (auto& t : this->_presentationCompleteSemaphores) {
            t.destroySemaphore();
        }
        for (auto& t : this->_fences) {
            t.destroyFence();
        }
        for (auto& t : this->_commandBuffers) {
            t.destroyCommandBuffer();
        }
        for (auto& t : this->_commandPools) {
            t.destroyCommandPool();
        }
    }

    void initRendering(const renderCore::CVulkanContext& context) {
        renderLoop = std::make_unique<CVulkanRenderLoop>(context);
        renderLoop->initRenderLoop();

        sceneRenderer = std::make_unique<renderScene::CVulkanSceneRenderer>(context, *renderLoop);
        sceneRenderer->initScene();
    }

    void runRendering() {
        if (renderLoop != nullptr) {
            renderLoop->runRenderLoop();
        }
    }

    void destroyRendering() {
        if (renderLoop != nullptr) {
            renderLoop->destroyRenderLoopResources();
            renderLoop.reset();
        }
        if (sceneRenderer != nullptr) {
            sceneRenderer->destroyScene();
            sceneRenderer.reset();
        }
    }
}
