//
// Created by ded on 11.09.2026.
//

#pragma once

#include <map>
#include <vector>

#include "vulkan_render_cache.h"
#include "client/render/vulkan_pipeline.h"
#include "client/render/vulkan_transform.h"


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
    glm::mat4 model;
    uint32_t materialId;
};

namespace cvulkan::client::render::scene {
    class CVulkanSceneEntitiesCollection {
    public:
        CVulkanSceneEntitiesCollection();
        ~CVulkanSceneEntitiesCollection() = default;

        CLASS_NO_COPY_NO_MOVE(CVulkanSceneEntitiesCollection);

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

        CLASS_NO_COPY(CVulkanSceneRenderer);

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

        [[nodiscard]] vk::CVulkanDescriptorAllocator& descriptorAllocator() {
            return _descriptorAllocator;
        }

        [[nodiscard]] const vk::CVulkanDescriptorAllocator& descriptorAllocator() const {
            return _descriptorAllocator;
        }

    protected:
        void loadMaterials();
        void createRenderData();
        void initCacheData(const std::vector<asset::modelGen::CVulkanMaterialData>& materials, const std::vector<asset::modelGen::CVulkanModelData>& models);
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

        vk::CVulkanDescriptorAllocator _descriptorAllocator;

        std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descVertexUniformLayout {};
        std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descFragmentStorageLayout {};
        std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descTextureLayout {};

        std::unique_ptr<vk::CVulkanSampler> _defaultTextureSampler {};

        std::unique_ptr<vk::CVulkanBuffer> _bufferProjMatrix {};
    };
}
