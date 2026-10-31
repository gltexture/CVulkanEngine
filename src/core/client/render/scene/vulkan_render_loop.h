//
// Created by ded on 09.09.2026.
//

#pragma once
#include <memory>

#include "../vulkan_commands.h"
#include "../vulkan_synchronization.h"

namespace cvulkan::client::render::scene {
    class CVulkanSceneRenderer;
}

namespace cvulkan::client::render::vk {
    class CVulkanContext;
}

namespace cvulkan::client::render::loop {
    class CVulkanRenderLoop {
    public:
        explicit CVulkanRenderLoop(vk::CVulkanContext& context);
        ~CVulkanRenderLoop();

        CLASS_NO_COPY(CVulkanRenderLoop)

        void runRenderLoop();

        [[nodiscard]] const std::vector<vk::CVulkanCommandPool>& commandPools() const {
            return _commandPools;
        }

        [[nodiscard]] const std::vector<vk::CVulkanCommandBuffer>& commandBuffers() const {
            return _commandBuffers;
        }

        [[nodiscard]] const std::vector<sync::CVulkanFence>& fences() const {
            return _fences;
        }

        [[nodiscard]] const std::vector<sync::CVulkanSemaphore>& presentationCompleteSemaphores() const {
            return _presentationCompleteSemaphores;
        }

        [[nodiscard]] const std::vector<sync::CVulkanSemaphore>& renderCompleteSemaphores() const {
            return _renderCompleteSemaphores;
        }

        [[nodiscard]] const uint32_t& currentFrame() const {
            return _currentFrame;
        }

        [[nodiscard]] const vk::CVulkanQueue& graphicsQueue() const {
            return *_graphicsQueue;
        }

        [[nodiscard]] const vk::CVulkanQueue& presentQueue() const {
            return *_presentQueue;
        }

        [[nodiscard]] bool shouldBeResized() const {
            return _shouldBeResized;
        }

        void resize();

    protected:
        static void recordingStart(const vk::CVulkanCommandPool& commandPool, const vk::CVulkanCommandBuffer& commandBuffer);

        static void recordingStop(const vk::CVulkanCommandBuffer& commandBuffer);

        void waitForFence() const;

        void submit(const vk::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const;
    private:
        vk::CVulkanContext& _context;
        std::vector<vk::CVulkanCommandPool> _commandPools{};
        std::vector<vk::CVulkanCommandBuffer> _commandBuffers{};
        std::vector<sync::CVulkanFence> _fences{};
        std::vector<sync::CVulkanSemaphore> _presentationCompleteSemaphores{};
        std::vector<sync::CVulkanSemaphore> _renderCompleteSemaphores{};
        uint32_t _currentFrame {0};
        std::unique_ptr<vk::CVulkanQueue> _graphicsQueue;
        std::unique_ptr<vk::CVulkanQueue> _presentQueue;
        bool _shouldBeResized {false};
    };

    extern std::unique_ptr<scene::CVulkanSceneRenderer> _sceneRenderer;
    extern std::unique_ptr<CVulkanRenderLoop> _renderLoop;

    void createRendering(vk::CVulkanContext& context);

    void runRendering();

    void destroyRendering();
}
