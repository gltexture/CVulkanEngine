//
// Created by ded on 11.09.2026.
//

#pragma once

#include <map>
#include <vector>

#include "vulkan_render_cache.h"
#include "client/render/vulkan_pipeline.h"
#include "client/render/vulkan_transform.h"
#include "client/render/assets/asset_models.h"

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
    constexpr std::string_view DESC_ID_MAT = "SCN_DESC_ID_MAT";
    constexpr std::string_view DESC_ID_PRJ = "SCN_DESC_ID_PRJ";
    constexpr std::string_view DESC_ID_TEXT = "SCN_DESC_ID_TEXT";

    namespace context {
        class CVulkanSceneContext;
    }

    class CVulkanSceneEntitiesCollection {
    public:
        CVulkanSceneEntitiesCollection();
        ~CVulkanSceneEntitiesCollection() = default;

        CLASS_NO_COPY_NO_MOVE(CVulkanSceneEntitiesCollection);

        [[nodiscard]] const std::map<std::string, entity::CVulkanSceneEntity>& entities() const {
            return _entities;
        }

        void addEntity(entity::CVulkanSceneEntity&& entity);
        [[nodiscard]] bool hasEntity(const std::string& id) const;

    private:
        std::map<std::string, entity::CVulkanSceneEntity> _entities;
    };

    namespace context {
        struct CVulkanLoadedResourcesData {
            std::vector<asset::modelLoad::CVulkanLoadedModelData> _loadedModels {};
        };

        class CVulkanSceneContext {
        public:
            explicit CVulkanSceneContext(const vk::CVulkanContext& context);
            ~CVulkanSceneContext() = default;

            CLASS_NO_COPY_NO_MOVE(CVulkanSceneContext)

            CVulkanLoadedResourcesData loadResources(vk::CVulkanDescriptorAllocator& descriptorAllocator, CVulkanSceneEntitiesCollection& entitiesCollection);
            void renderContext(const vk::CVulkanCommandBuffer &commandBuffer, const vk::CVulkanDescriptorAllocator &descriptorAllocator, const
                               CVulkanSceneEntitiesCollection
                               &entitiesCollection, const cache::CVulkanCacheCollection &caches) const;
            void onResize();

            [[nodiscard]] transform::CVulkanSceneProjection& projection() {
                return _projection;
            }

            [[nodiscard]] vk::CVulkanDescriptorSetLayout* descVertexUniformLayout() const {
                return _descVertexUniformLayout.get();
            }

            [[nodiscard]] vk::CVulkanDescriptorSetLayout* descFragmentStorageLayout() const {
                return _descFragmentStorageLayout.get();
            }

            [[nodiscard]] vk::CVulkanDescriptorSetLayout* descTextureLayout() const {
                return _descTextureLayout.get();
            }

            [[nodiscard]] vk::CVulkanBuffer* bufferProjMatrix() const {
                return _bufferProjMatrix.get();
            }

            [[nodiscard]] vk::CVulkanSampler* defaultTextureSampler() const {
                return _defaultTextureSampler.get();
            }

            [[nodiscard]] vk::CVulkanPipeline* defaultRenderPipeline() const {
                return _defaultRenderPipeline.get();
            }

        private:
            const vk::CVulkanContext& _context;

            transform::CVulkanSceneProjection _projection;

            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descVertexUniformLayout {};
            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descFragmentStorageLayout {};
            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descTextureLayout {};

            std::unique_ptr<vk::CVulkanSampler> _defaultTextureSampler {};

            std::unique_ptr<vk::CVulkanBuffer> _bufferProjMatrix {};
            std::unique_ptr<vk::CVulkanPipeline> _defaultRenderPipeline {};
        };
    }

    class CVulkanSceneRenderer {
    public:
        CVulkanSceneRenderer(const vk::CVulkanContext& context, const loop::CVulkanRenderLoop& renderLoop);
        ~CVulkanSceneRenderer();

        CLASS_NO_COPY(CVulkanSceneRenderer);

        void renderScene(const vk::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const;

        void onResize();

        [[nodiscard]] CVulkanSceneEntitiesCollection& entities() {
            return _entitiesCollection;
        }

        [[nodiscard]] const cache::CVulkanCacheCollection& caches() const {
            return _caches;
        }

        [[nodiscard]] vk::CVulkanDescriptorAllocator& descriptorAllocator() {
            return _descriptorAllocator;
        }

        [[nodiscard]] const vk::CVulkanDescriptorAllocator& descriptorAllocator() const {
            return _descriptorAllocator;
        }

        [[nodiscard]] const context::CVulkanSceneContext& sceneContext() const {
            return _sceneContext;
        }

        [[nodiscard]] context::CVulkanSceneContext& sceneContext() {
            return _sceneContext;
        }

    protected:
        void setDefaults();
        void createMaterialDescriptors();
        void createRenderAttachmentsData();
        void initCacheData(const std::vector<asset::modelGen::CVulkanMaterialData>& materials, const std::vector<asset::modelGen::CVulkanModelData>& models);

    private:
        const vk::CVulkanContext& _context;
        context::CVulkanSceneContext _sceneContext;
        CVulkanSceneEntitiesCollection _entitiesCollection {};
        cache::CVulkanCacheCollection _caches;
        const loop::CVulkanRenderLoop& _renderLoop;
        std::vector<VkRenderingAttachmentInfo> _vkColorAttachmentsInfo{};
        std::vector<VkRenderingAttachmentInfo> _vkDepthAttachmentsInfo{};
        std::vector<vk::CVulkanAttachment2D> _depthAttachments{};
        std::vector<VkRenderingInfo> _vkRenderingInfo{};

        vk::CVulkanDescriptorAllocator _descriptorAllocator;
    };
}