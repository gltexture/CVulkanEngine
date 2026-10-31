//
// Created by ded on 11.09.2026.
//

#pragma once

#include <map>
#include <vector>

#include "vulkan_render_cache.h"
#include "camera/view_camera.h"
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
    constexpr std::string_view DESC_ID_VIEW = "SCN_DESC_ID_VIEW";
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
        using ModelId = std::string;

        struct CVulkanLoadedResourcesData {
            std::vector<asset::modelLoad::CVulkanLoadedModelData> _loadedModels {};
        };

        class CVulkanSceneResourcesInitializer {
        public:
            CVulkanSceneResourcesInitializer() = default;
            ~CVulkanSceneResourcesInitializer() = default;

            ModelId loadModelWithMaterials(const io_utils::CVulkanResourcePath& resourcePath);

            [[nodiscard]] const CVulkanLoadedResourcesData& loadedResourcesData() const {
                return _loadedResourcesData;
            }

        private:
            CVulkanLoadedResourcesData _loadedResourcesData;
        };

        class CVulkanSceneContext {
        public:
            explicit CVulkanSceneContext(const vk::CVulkanContext& context);
            ~CVulkanSceneContext() = default;

            CLASS_NO_COPY_NO_MOVE(CVulkanSceneContext)

            void loadResources(vk::CVulkanDescriptorAllocator& descriptorAllocator, CVulkanSceneResourcesInitializer& sceneResourcesInitializer);
            void initializeObjects(CVulkanSceneEntitiesCollection& entitiesCollection);

            void renderContext(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanDescriptorAllocator& descriptorAllocator, const CVulkanSceneEntitiesCollection& entitiesCollection, const cache::CVulkanCacheCollection& caches, uint32_t currentFrame) const;

            void renderEntities(const vk::CVulkanCommandBuffer& commandBuffer, const CVulkanSceneEntitiesCollection& entitiesCollection, const cache::CVulkanCacheCollection& caches, bool transparent) const;

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

            [[nodiscard]] camera::CVulkanCamera* currentCamera() const {
                return _currentCamera.get();
            }

        private:
            const vk::CVulkanContext& _context;

            transform::CVulkanSceneProjection _projection;

            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descVertexUniformLayout {};
            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descFragmentStorageLayout {};
            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descTextureLayout {};

            std::unique_ptr<vk::CVulkanSampler> _defaultTextureSampler {};

            std::unique_ptr<vk::CVulkanBuffer> _bufferProjMatrix {};
            std::unique_ptr<std::vector<vk::CVulkanBuffer>> _bufferViewMatrices {};

            std::unique_ptr<vk::CVulkanPipeline> _defaultRenderPipeline {};

            std::unique_ptr<camera::CVulkanCamera> _currentCamera {};
        };
    }

    class CVulkanSceneRenderer {
    public:
        CVulkanSceneRenderer(const vk::CVulkanContext& context, const loop::CVulkanRenderLoop& renderLoop);
        ~CVulkanSceneRenderer();

        CLASS_NO_COPY(CVulkanSceneRenderer);

        void renderScene(const vk::CVulkanCommandBuffer& commandBuffer, uint32_t imageIndex, uint32_t currentFrame) const;

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
        void initCacheData(std::vector<asset::modelGen::CVulkanMaterialData>& materials, const std::vector<asset::modelGen::CVulkanModelData>& models);

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