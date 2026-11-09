//
// Created by ded on 09.09.2026.
//

#include "vulkan_render_loop.h"

#include "client/render/vulkan_commands.h"
#include "client/render/vulkan_config.hpp"
#include "vulkan_render_scene__main.h"
#include "test_logic/test_logic.h"
#include "client/render/scene/vulkan_render_scene__post.h"
#include "client/render/scene/vulkan_render_scene__swapchain.h"
#include "client/render/scene/vulkan_render_scene__main.h"
#include "utility/exceptions.h"

namespace cvulkan::client::render::loop {
    CVulkanRenderLoop::CVulkanRenderLoop(vk::CVulkanContext& context) : _context{context}, _caches{context}, _descriptorAllocator{context} {
        const uint32_t graphicsQueueFamilyIndex = vk::util::findQueueFamilyIndex(context, vk::CVulkanQueueFamilyBitMasks::GRAPHICS);
        const uint32_t presentationQueueFamilyIndex = vk::util::findQueueFamilyIndex(context, vk::CVulkanQueueFamilyBitMasks::PRESENT);
        if (graphicsQueueFamilyIndex == UINT32_MAX) {
            throw exc::CVulkanException("Graphics queue family index not found in registry");
        }
        if (presentationQueueFamilyIndex == UINT32_MAX) {
            throw exc::CVulkanException("Presentation queue family index not found in registry");
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
            this->_graphicsQueue = std::make_unique<vk::CVulkanQueue>(this->_context, graphicsQueueFamilyIndex, 0);
            this->_presentationQueue = std::make_unique<vk::CVulkanQueue>(this->_context, presentationQueueFamilyIndex, 0);
        }

        {
            this->_mainSceneRenderer = std::make_unique<scene::main::CVulkanSceneRenderer>(context, this->_descriptorAllocator, this->_caches);
            this->_postSceneRenderer = std::make_unique<scene::post::CVulkanPostFXRenderer>(context, this->_descriptorAllocator, this->_mainSceneRenderer->colorAttachment());
            this->_swapchainSceneRenderer = std::make_unique<scene::swapchain::CVulkanSwapchainRenderer>(context, this->_descriptorAllocator, this->_postSceneRenderer->colorAttachment());
        }
    }

    CVulkanRenderLoop::~CVulkanRenderLoop() {
    }

    void CVulkanRenderLoop::runRenderLoop() {
        static double LastTime = glfwGetTime();
        const double Time = glfwGetTime();
        const double DeltaTime = Time - LastTime;
        LastTime = Time;

        const vk::CVulkanCommandPool& commandPool = this->_commandPools[this->_currentFrame];
        const vk::CVulkanCommandBuffer& commandBuffer = this->_commandBuffers[this->_currentFrame];

        this->waitForFence();
        recordingStart(commandPool, commandBuffer);

        const uint32_t imageIndex = this->_context.swapChain().acquireSwapChainNextImage(this->_presentationCompleteSemaphores[this->_currentFrame]);
        if (this->_shouldBeResized || imageIndex == UINT32_MAX) {
            recordingStop(commandBuffer);
            this->resize();
            return;
        }

        {
            {
                test_logic::updateInput(DeltaTime);
            }
            this->_mainSceneRenderer->renderScene(commandBuffer, this->_currentFrame);
            this->_postSceneRenderer->renderPostFX(commandBuffer, this->_mainSceneRenderer->colorAttachment());
            this->_swapchainSceneRenderer->renderSwapchain(commandBuffer, this->_postSceneRenderer->colorAttachment(), imageIndex);
        }

        recordingStop(commandBuffer);
        this->submit(commandBuffer, imageIndex);

        this->_shouldBeResized = this->_context.swapChain().presentImage(*this->_presentationQueue, this->_renderCompleteSemaphores[imageIndex], imageIndex);
        this->_currentFrame = (this->_currentFrame + 1) % renderConfig::MAX_IMAGES_IN_FLIGHT;
    }

    void CVulkanRenderLoop::resize() {
        if (const glm::vec2 windowSize = this->_context.glfwWindow().size(); windowSize.x * windowSize.y <= 0) {
            return;
        }
        this->_context.device().deviceWaitIdle();
        this->_context.resize();

        this->_renderCompleteSemaphores.clear();
        this->_presentationCompleteSemaphores.clear();

        for (uint32_t i = 0; i < renderConfig::MAX_IMAGES_IN_FLIGHT; ++i) {
            this->_presentationCompleteSemaphores.emplace_back(this->_context);
        }

        for (uint32_t i = 0; i < this->_context.swapChain().numImages(); ++i) {
            this->_renderCompleteSemaphores.emplace_back(this->_context);
        }

        this->_mainSceneRenderer->onResize();
        this->_postSceneRenderer->onResize(this->_mainSceneRenderer->colorAttachment());
        this->_swapchainSceneRenderer->onResize(this->_postSceneRenderer->colorAttachment());
        
        this->_shouldBeResized = false;
    }

    void CVulkanRenderLoop::recordingStart(const vk::CVulkanCommandPool& commandPool, const vk::CVulkanCommandBuffer& commandBuffer) {
        commandPool.reset();
        commandBuffer.beginRecording();
    }

    void CVulkanRenderLoop::recordingStop(const vk::CVulkanCommandBuffer& commandBuffer) {
        commandBuffer.endRecording();
    }

    void CVulkanRenderLoop::waitForFence() const {
        this->_fences[this->_currentFrame].wait();
    }

    void CVulkanRenderLoop::submit(const vk::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const {
        const sync::CVulkanFence& fence = this->_fences[this->_currentFrame];
        fence.reset();
        const VkCommandBufferSubmitInfo commands{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = commandBuffer.vkCommandBuffer()
        };
        const VkSemaphoreSubmitInfo waitSemaphores{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = this->_presentationCompleteSemaphores[this->_currentFrame].vkSemaphore(),
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
        };
        const VkSemaphoreSubmitInfo signalSemaphores{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = this->_renderCompleteSemaphores[imageIndex].vkSemaphore(),
            .stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
        };
        const std::vector v_commandSubmitInfos{commands};
        const std::vector v_waitSemaphores{waitSemaphores};
        const std::vector v_signalSemaphores{signalSemaphores};
        this->_graphicsQueue->submitQueue(v_commandSubmitInfos, &v_waitSemaphores, &v_signalSemaphores, &fence);
    }

    std::unique_ptr<CVulkanRenderLoop> _renderLoop;

    void createRendering(vk::CVulkanContext& context) {
        _renderLoop = std::make_unique<CVulkanRenderLoop> (context);
    }

    void runRendering() {
        if (_renderLoop != nullptr) {
            _renderLoop->runRenderLoop();
        }
    }

    void destroyRendering() {
        if (_renderLoop != nullptr) {
            _renderLoop.reset();
        }
    }
}
