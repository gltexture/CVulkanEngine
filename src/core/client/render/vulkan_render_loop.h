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

namespace cvulkan::client::render::core {
    class CVulkanContext;
}

namespace cvulkan::client::render::loop {
    class CVulkanRenderLoop {
    public:
        explicit CVulkanRenderLoop(const core::CVulkanContext& context)
            : _context{context}, _graphicsQueue(context), _presentQueue(context) {
        }

        ~CVulkanRenderLoop() = default;

        CVULKAN_NO_COPY(CVulkanRenderLoop)

        void createRenderLoop();
        void runRenderLoop();
        void destroyRenderLoopResources();

        [[nodiscard]] const std::vector<core::CVulkanCommandPool>& commandPools() const {
            return _commandPools;
        }

        [[nodiscard]] const std::vector<core::CVulkanCommandBuffer>& commandBuffers() const {
            return _commandBuffers;
        }

        [[nodiscard]] const std::vector<render::sync::CVulkanFence>& fences() const {
            return _fences;
        }

        [[nodiscard]] const std::vector<render::sync::CVulkanSemaphore>& presentationCompleteSemaphores() const {
            return _presentationCompleteSemaphores;
        }

        [[nodiscard]] const std::vector<render::sync::CVulkanSemaphore>& renderCompleteSemaphores() const {
            return _renderCompleteSemaphores;
        }

        [[nodiscard]] const uint32_t& currentFrame() const {
            return _currentFrame;
        }

        [[nodiscard]] const core::CVulkanQueue& graphicsQueue() const {
            return _graphicsQueue;
        }

        [[nodiscard]] const core::CVulkanQueue& presentQueue() const {
            return _presentQueue;
        }

    protected:
        static void recordingStart(const core::CVulkanCommandPool& commandPool, const core::CVulkanCommandBuffer& commandBuffer);
        static void recordingStop(const core::CVulkanCommandBuffer& commandBuffer);
        void waitForFence() const;
        void submit(const core::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const;

    private:
        const core::CVulkanContext& _context;
        std::vector<core::CVulkanCommandPool> _commandPools {};
        std::vector<core::CVulkanCommandBuffer> _commandBuffers {};
        std::vector<render::sync::CVulkanFence> _fences {};
        std::vector<render::sync::CVulkanSemaphore> _presentationCompleteSemaphores {};
        std::vector<render::sync::CVulkanSemaphore> _renderCompleteSemaphores {};
        uint32_t _currentFrame = 0;
        core::CVulkanQueue _graphicsQueue;
        core::CVulkanQueue _presentQueue;
    };

    extern std::unique_ptr<CVulkanRenderLoop> renderLoop;

    void createRendering(const core::CVulkanContext& context);
    void runRendering();
    void destroyRendering();
}
