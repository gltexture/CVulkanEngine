//
// Created by ded on 11.09.2026.
//

#pragma once

#include <map>
#include <source_location>
#include <vector>

#include "cvulkan_render_mrt.h"
#include "vulkan_render_cache.h"
#include "vulkan_render_loop.h"
#include "vulkan_scene_objects.h"
#include "camera/view_camera.h"
#include "client/render/vulkan_pipeline.h"
#include "client/render/vulkan_transform.h"
#include "client/render/assets/asset_models.h"
#include "core/utility/debug.h"

namespace cvulkan::client::render::entity {
    class CVulkanSceneEntitiesCollection;
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

namespace cvulkan::client::render::scene::deferred {
    namespace context {
        using ModelId = std::string;

        struct CVulkanLoadedResourcesData {
            std::vector<asset::modelLoad::CVulkanLoadedModelData> _loadedModels{};
        };

        class CVulkanSceneResourcesInitializer {
        public:
            CVulkanSceneResourcesInitializer() = default;

            ~CVulkanSceneResourcesInitializer() = default;

            ModelId loadModelWithMaterials(const io::CVulkanResourcePath& resourcePath);

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

            void initializeObjects(entity::CVulkanSceneEntitiesCollection& entitiesCollection);

            void renderContext(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanDescriptorAllocator& descriptorAllocator, const entity::CVulkanSceneEntitiesCollection& entitiesCollection, const cache::CVulkanCacheCollection& caches,
                               uint32_t currentFrame) const;

            void renderEntities(const vk::CVulkanCommandBuffer& commandBuffer, const entity::CVulkanSceneEntitiesCollection& entitiesCollection, const cache::CVulkanCacheCollection& caches, bool transparent) const;

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

            [[nodiscard]] vk::CVulkanSampler* textureSampler() const {
                return _textureSampler.get();
            }

            [[nodiscard]] vk::CVulkanPipeline* renderPipeline() const {
                return _renderPipeline.get();
            }

            [[nodiscard]] camera::CVulkanCamera* currentCamera() const {
                return _currentCamera.get();
            }

        private:
            const vk::CVulkanContext& _context;

            transform::CVulkanSceneProjection _projection;

            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descVertexUniformLayout{};
            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descFragmentStorageLayout{};
            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descTextureLayout{};

            std::unique_ptr<vk::CVulkanSampler> _textureSampler{};

            std::unique_ptr<vk::CVulkanBuffer> _bufferProjMatrix{};
            std::unique_ptr<std::vector<vk::CVulkanBuffer>> _bufferViewMatrices{};

            std::unique_ptr<vk::CVulkanPipeline> _renderPipeline{};

            std::unique_ptr<camera::CVulkanCamera> _currentCamera{};
        };
    }

    namespace lpass {
        constexpr inline std::string_view DESC_ID_ATT = "LIGHT_DESC_ID_ATT";
        constexpr VkFormat COLOR_FORMAT {VK_FORMAT_R32G32B32A32_SFLOAT};
        constexpr VkClearValue CLEAR_COLOR {
            .color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}
        };

        class CVulkanSceneRendererLPass {
        public:
            CVulkanSceneRendererLPass(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const mrt::CVulkanMultipleRenderTargets& multipleRenderTargets);
            ~CVulkanSceneRendererLPass();

            CLASS_NO_COPY(CVulkanSceneRendererLPass);

            void renderScene(const vk::CVulkanCommandBuffer& commandBuffer, uint32_t currentFrame) const;

            void onResize();

            [[nodiscard]] const vk::CVulkanAttachment2D& colorAttachment() const {
                return *_colorAttachment;
            }

        protected:
            void createRenderingData();

        private:
            const vk::CVulkanContext& _context;
            vk::CVulkanDescriptorAllocator& _descriptorAllocator;
            const mrt::CVulkanMultipleRenderTargets& _multipleRenderTargets;
            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descTextureLayout{};
            std::unique_ptr<vk::CVulkanSampler> _textureSampler{};
            std::unique_ptr<vk::CVulkanAttachment2D> _colorAttachment {};
            VkRenderingAttachmentInfo _vkAttachmentColorInfo {};
            VkRenderingInfo _vkRenderingInfo {};
            std::unique_ptr<vk::CVulkanPipeline> _renderPipeline{};
        };
    }

    namespace gpass {
        constexpr inline std::string_view DESC_ID_MAT = "SCN_DESC_ID_MAT";
        constexpr inline std::string_view DESC_ID_PRJ = "SCN_DESC_ID_PRJ";
        constexpr inline std::string_view DESC_ID_VIEW = "SCN_DESC_ID_VIEW";
        constexpr inline std::string_view DESC_ID_TEXT = "SCN_DESC_ID_TEXT";

        constexpr VkClearValue CLEAR_COLOR {
            .color = {.float32 = {0.5f, 0.7f, 0.9f, 1.0f}}
        };
        constexpr VkClearValue CLEAR_DEPTH {
            .depthStencil = {1.0f, 0}
        };

        class CVulkanSceneRendererGPass {
        public:
            CVulkanSceneRendererGPass(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, cache::CVulkanCacheCollection& caches);
            ~CVulkanSceneRendererGPass();

            CLASS_NO_COPY(CVulkanSceneRendererGPass);

            void renderScene(const vk::CVulkanCommandBuffer& commandBuffer, uint32_t currentFrame) const;

            void onResize();

            [[nodiscard]] const context::CVulkanSceneContext& sceneContext() const {
                return _sceneContext;
            }

            [[nodiscard]] context::CVulkanSceneContext& sceneContext() {
                return _sceneContext;
            }

            [[nodiscard]] const entity::CVulkanSceneEntitiesCollection& entitiesCollection() const {
                return _entitiesCollection;
            }

            [[nodiscard]] entity::CVulkanSceneEntitiesCollection& entitiesCollection() {
                return _entitiesCollection;
            }

            [[nodiscard]] const mrt::CVulkanMultipleRenderTargets& multipleRenderTargets() const {
                return _multipleRenderTargets;
            }

        protected:
            void createMaterialDescriptors() const;
            void createRenderingData();
            void initCacheData(std::vector<asset::modelGen::CVulkanMaterialData>& materials, const std::vector<asset::modelGen::CVulkanModelData>& models) const;

        private:
            const vk::CVulkanContext& _context;
            vk::CVulkanDescriptorAllocator& _descriptorAllocator;
            cache::CVulkanCacheCollection& _caches;
            mrt::CVulkanMultipleRenderTargets _multipleRenderTargets;
            context::CVulkanSceneContext _sceneContext;
            entity::CVulkanSceneEntitiesCollection _entitiesCollection{};
            std::vector<VkRenderingAttachmentInfo> _vkAttachmentColorInfos {};
            VkRenderingAttachmentInfo _vkAttachmentDepthInfo {};
            VkRenderingInfo _vkRenderingInfo {};
        };
    }

    class CVulkanSceneRenderer {
    public:
        CVulkanSceneRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, cache::CVulkanCacheCollection& caches);
        ~CVulkanSceneRenderer() = default;

        void renderScene(const vk::CVulkanCommandBuffer& commandBuffer, uint32_t currentFrame) const;
        void onResize();

        [[nodiscard]] const gpass::CVulkanSceneRendererGPass& geometryPassRendering() const {
            return _geometryPassRendering;
        }

        [[nodiscard]] const lpass::CVulkanSceneRendererLPass& lightPassRendering() const {
            return _lightPassRendering;
        }

        static void setViewportAndScissor(const vk::CVulkanCommandBuffer& commandBuffer, const VkExtent2D extent2D) {
            if (!DEBUG::CHECK(std::source_location::current(), extent2D.width > 0 && extent2D.height > 0, "Cannot set viewport and scissor: extent width or height is zero")) {
                return;
            }

            const VkViewport viewport {
                .x = 0.0F,
                .y = static_cast<float>(extent2D.height),
                .width = static_cast<float>(extent2D.width),
                .height = -static_cast<float>(extent2D.height),
                .minDepth = 0.0F,
                .maxDepth = 1.0F
            };
            vkCmdSetViewport(commandBuffer.vkCommandBuffer(), 0, 1, &viewport);

            const VkRect2D scissor {
                .offset {
                    .x = 0,
                    .y = 0
                },
                .extent = extent2D
            };
            vkCmdSetScissor(commandBuffer.vkCommandBuffer(), 0, 1, &scissor);
        }

        static void setViewport(const vk::CVulkanCommandBuffer& commandBuffer, const VkExtent2D extent2D) {
            if (!DEBUG::CHECK(std::source_location::current(), extent2D.width > 0 && extent2D.height > 0, "Cannot set viewport: extent width or height is zero")) {
                return;
            }

            const VkViewport viewport {
                .x = 0.0F,
                .y = static_cast<float>(extent2D.height),
                .width = static_cast<float>(extent2D.width),
                .height = -static_cast<float>(extent2D.height),
                .minDepth = 0.0F,
                .maxDepth = 1.0F
            };
            vkCmdSetViewport(commandBuffer.vkCommandBuffer(), 0, 1, &viewport);
        }

        static void setScissor(const vk::CVulkanCommandBuffer& commandBuffer, const VkExtent2D extent2D) {
            if (!DEBUG::CHECK(std::source_location::current(), extent2D.width > 0 && extent2D.height > 0, "Cannot set viewport: extent width or height is zero")) {
                return;
            }

            const VkRect2D scissor {
                .offset {
                    .x = 0,
                    .y = 0
                },
                .extent = extent2D
            };
            vkCmdSetScissor(commandBuffer.vkCommandBuffer(), 0, 1, &scissor);
        }

    private:
        gpass::CVulkanSceneRendererGPass _geometryPassRendering;
        lpass::CVulkanSceneRendererLPass _lightPassRendering;
    };

    inline CVulkanSceneRenderer::CVulkanSceneRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, cache::CVulkanCacheCollection& caches)
        : _geometryPassRendering(context, descriptorAllocator, caches), _lightPassRendering(context, descriptorAllocator, _geometryPassRendering.multipleRenderTargets()){
    }

    inline void CVulkanSceneRenderer::renderScene(const vk::CVulkanCommandBuffer& commandBuffer, const uint32_t currentFrame) const {
        this->_geometryPassRendering.renderScene(commandBuffer, currentFrame);
        this->_lightPassRendering.renderScene(commandBuffer, currentFrame);
    }

    inline void CVulkanSceneRenderer::onResize() {
        this->_geometryPassRendering.onResize();
        this->_lightPassRendering.onResize();
    }
}
