//
// Created by gltex on 07.09.2026.
//

#pragma once
#include "vulkan_render_core.h"

namespace cvulkan::client::render::vk {
    struct CVulkanInheritanceInfo {
        const VkFormat depthFormat;
        const std::vector<VkFormat> colorFormats;
        const VkSampleCountFlagBits rasterizationSamples;
    };

    class CVulkanCommandPool {
    public:
        CVulkanCommandPool(const CVulkanContext& context, uint32_t queueFamilyIndex, bool supportReset);
        ~CVulkanCommandPool();

        CLASS_NO_COPY_NO_ASSIGN_MOVE(CVulkanCommandPool)

        CVulkanCommandPool(CVulkanCommandPool&& other) noexcept
            : _context{other._context}, _vkCommandPool{other._vkCommandPool}, _queueFamilyIndex{other._queueFamilyIndex}, _supportReset{other._supportReset} {
            other._vkCommandPool = VK_NULL_HANDLE;
        }

        void reset() const;

        [[nodiscard]] VkCommandPool vkCommandPool() const {
            return _vkCommandPool;
        }

    private:
        const CVulkanContext& _context;
        VkCommandPool _vkCommandPool{};
        const uint32_t _queueFamilyIndex;
        const bool _supportReset;
    };

    class CVulkanCommandBuffer {
    public:
        CVulkanCommandBuffer(const CVulkanContext& context, const CVulkanCommandPool& commandPool, const bool primary, const bool oneTimeSubmit);
        ~CVulkanCommandBuffer();

        CLASS_NO_COPY_NO_ASSIGN_MOVE(CVulkanCommandBuffer)

        CVulkanCommandBuffer(CVulkanCommandBuffer&& other) noexcept
            : _context{other._context}, _vkCommandBuffer{other._vkCommandBuffer}, _commandPool{other._commandPool}, _primary(other._primary), _oneTimeSubmit(other._oneTimeSubmit) {
            other._vkCommandBuffer = VK_NULL_HANDLE;
        }

        void beginRecording() const {
            this->beginRecording(nullptr);
        }

        void beginRecording(const CVulkanInheritanceInfo* inheritance_info) const;

        void endRecording() const;

        void submitAndWait(const CVulkanQueue& queue) const;

        void reset() const;

        [[nodiscard]] VkCommandBuffer vkCommandBuffer() const {
            return _vkCommandBuffer;
        }

        [[nodiscard]] const CVulkanCommandPool& commandPool() const {
            return _commandPool;
        }

    private:
        const CVulkanContext& _context;
        VkCommandBuffer _vkCommandBuffer{};
        const CVulkanCommandPool& _commandPool;
        const bool _primary;
        const bool _oneTimeSubmit;
    };

    class CVulkanQueue {
    public:
        explicit CVulkanQueue(const CVulkanContext& context, uint32_t queueFamilyIndex, uint32_t queueIndex);
        ~CVulkanQueue() = default;

        CLASS_NO_COPY(CVulkanQueue);

        void submitQueue(const std::vector<VkCommandBufferSubmitInfo>& commandSubmitInfos, const std::vector<VkSemaphoreSubmitInfo>* waitSemaphores, const std::vector<VkSemaphoreSubmitInfo>* signalSemaphores,
                    const sync::CVulkanFence* fence) const;

        [[nodiscard]] VkQueue vkQueue() const {
            return _vkQueue;
        }

        [[nodiscard]] uint32_t queueFamilyIndex() const {
            return _queueFamilyIndex;
        }

    private:
        const CVulkanContext& _context;
        VkQueue _vkQueue{};
        uint32_t _queueFamilyIndex{};
    };
}
