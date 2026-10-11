//
// Created by ded on 11.09.2026.
//

#pragma once

#include <map>
#include <vector>

#include "vulkan_render_cache.h"
#include "client/render/vulkan_pipeline.h"
#include "client/render/vulkan_transform.h"
#include "client/render/vulkan_utility.h"

namespace cvulkan::client::render::entity {
    class CVulkanSceneEntity;
}

namespace cvulkan::client::render::loop {
    class CVulkanRenderLoop;
}

namespace cvulkan::client::render::vk {
    class CVulkanAttachment2D;
    class CVulkanCommandBuffer;
    class CVulkanContext;
}

struct CVulkanPushConstants {
    glm::mat4 projection;
    glm::mat4 model;
};

namespace cvulkan::client::render::scene {
    class CVulkanSceneEntitiesCollection {
    public:
        CVulkanSceneEntitiesCollection();
        ~CVulkanSceneEntitiesCollection() = default;

        CVULKAN_NO_COPY_NO_MOVE(CVulkanSceneEntitiesCollection);

        [[nodiscard]] const std::map<std::string, entity::CVulkanSceneEntity>& entities() const {
            return _entities;
        }

        void addEntity(entity::CVulkanSceneEntity&& entity);
        bool hasEntity(const std::string& id) const;

    private:
        std::map<std::string, entity::CVulkanSceneEntity> _entities;
    };

    class CVulkanSceneRenderer {
    public:
        CVulkanSceneRenderer(const vk::CVulkanContext& context, const loop::CVulkanRenderLoop& renderLoop);
        ~CVulkanSceneRenderer();

        CVULKAN_NO_COPY(CVulkanSceneRenderer);

        void renderScene(const vk::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const;

        void resize();

        [[nodiscard]] CVulkanSceneEntitiesCollection& entities() {
            return _entitiesCollection;
        }

        [[nodiscard]] const cache::CVulkanCacheCollection& caches() const {
            return _caches;
        }

        [[nodiscard]] transform::CVulkanSceneProjection& projection() {
            return _projection;
        }

    protected:
        void createRenderData();
        void loadResources();

    private:
        const vk::CVulkanContext& _context;
        CVulkanSceneEntitiesCollection _entitiesCollection {};
        cache::CVulkanCacheCollection _caches;
        const loop::CVulkanRenderLoop& _renderLoop;
        transform::CVulkanSceneProjection _projection;
        std::unique_ptr<vk::CVulkanPipeline> _defaultRenderPipeline {};
        std::vector<VkRenderingAttachmentInfo> _vkColorAttachmentsInfo{};
        std::vector<VkRenderingAttachmentInfo> _vkDepthAttachmentsInfo{};
        std::vector<vk::CVulkanAttachment2D> _depthAttachments{};
        std::vector<VkRenderingInfo> _vkRenderingInfo{};
    };
}
