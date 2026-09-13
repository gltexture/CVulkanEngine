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
    class CVulkanRenderLoop {
    public:
        explicit CVulkanRenderLoop(const renderCore::CVulkanContext& context)
            : _context{context}, _graphicsQueue(context), _presentQueue(context) {
        }

        ~CVulkanRenderLoop() = default;

        CVulkanRenderLoop(const CVulkanRenderLoop&) = delete;
        CVulkanRenderLoop& operator=(const CVulkanRenderLoop&) = delete;

        void initRenderLoop();
        void runRenderLoop();
        void destroyRenderLoopResources();

        [[nodiscard]] const std::vector<renderCore::CVulkanCommandPool>& commandPools() const {
            return _commandPools;
        }

        [[nodiscard]] const std::vector<renderCore::CVulkanCommandBuffer>& commandBuffers() const {
            return _commandBuffers;
        }

        [[nodiscard]] const std::vector<renderSync::CVulkanFence>& fences() const {
            return _fences;
        }

        [[nodiscard]] const std::vector<renderSync::CVulkanSemaphore>& presentationCompleteSemaphores() const {
            return _presentationCompleteSemaphores;
        }

        [[nodiscard]] const std::vector<renderSync::CVulkanSemaphore>& renderCompleteSemaphores() const {
            return _renderCompleteSemaphores;
        }

        [[nodiscard]] const uint32_t& currentFrame() const {
            return _currentFrame;
        }

        [[nodiscard]] const renderCore::CVulkanQueue& graphicsQueue() const {
            return _graphicsQueue;
        }

        [[nodiscard]] const renderCore::CVulkanQueue& presentQueue() const {
            return _presentQueue;
        }

    protected:
        static void recordingStart(const renderCore::CVulkanCommandPool& commandPool, const renderCore::CVulkanCommandBuffer& commandBuffer);
        static void recordingStop(const renderCore::CVulkanCommandBuffer& commandBuffer);
        void waitForFence() const;
        void submit(const renderCore::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const;

    private:
        const renderCore::CVulkanContext& _context;
        std::vector<renderCore::CVulkanCommandPool> _commandPools {};
        std::vector<renderCore::CVulkanCommandBuffer> _commandBuffers {};
        std::vector<renderSync::CVulkanFence> _fences {};
        std::vector<renderSync::CVulkanSemaphore> _presentationCompleteSemaphores {};
        std::vector<renderSync::CVulkanSemaphore> _renderCompleteSemaphores {};
        uint32_t _currentFrame = 0;
        renderCore::CVulkanQueue _graphicsQueue;
        renderCore::CVulkanQueue _presentQueue;
    };

    extern std::unique_ptr<CVulkanRenderLoop> renderLoop;

    void initRendering(const renderCore::CVulkanContext& context);
    void runRendering();
    void destroyRendering();
}
