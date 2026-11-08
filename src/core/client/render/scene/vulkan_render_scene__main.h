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

namespace cvulkan::client::render::scene::main {
    constexpr inline std::string_view DESC_ID_MAT = "SCN_DESC_ID_MAT";
    constexpr inline std::string_view DESC_ID_PRJ = "SCN_DESC_ID_PRJ";
    constexpr inline std::string_view DESC_ID_VIEW = "SCN_DESC_ID_VIEW";
    constexpr inline std::string_view DESC_ID_TEXT = "SCN_DESC_ID_TEXT";

    constexpr VkFormat COLOR_FORMAT {VK_FORMAT_R16G16B16A16_SFLOAT};
    constexpr VkFormat DEPTH_FORMAT {VK_FORMAT_D16_UNORM};

    constexpr VkClearValue CLEAR_COLOR{
        .color = {.float32 = {0.5f, 0.7f, 0.9f, 1.0f}}
    };
    constexpr VkClearValue CLEAR_DEPTH{
        .depthStencil = {1.0f, 0}
    };

    /*
    class ISceneRenderStage {
    public:
        ISceneRenderStage() = default;
        virtual ~ISceneRenderStage() = 0;

        virtual void render(VkCommandBuffer commandBuffer) = 0;
        virtual void onResize() = 0;
    };
    */

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
            std::vector<asset::modelLoad::CVulkanLoadedModelData> _loadedModels{};
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

            void renderContext(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanDescriptorAllocator& descriptorAllocator, const CVulkanSceneEntitiesCollection& entitiesCollection, const cache::CVulkanCacheCollection& caches,
                               uint32_t currentFrame) const;

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

            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descVertexUniformLayout{};
            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descFragmentStorageLayout{};
            std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descTextureLayout{};

            std::unique_ptr<vk::CVulkanSampler> _defaultTextureSampler{};

            std::unique_ptr<vk::CVulkanBuffer> _bufferProjMatrix{};
            std::unique_ptr<std::vector<vk::CVulkanBuffer>> _bufferViewMatrices{};

            std::unique_ptr<vk::CVulkanPipeline> _defaultRenderPipeline{};

            std::unique_ptr<camera::CVulkanCamera> _currentCamera{};
        };
    }

    class CVulkanSceneRenderer {
    public:
        CVulkanSceneRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, cache::CVulkanCacheCollection& caches);
        ~CVulkanSceneRenderer();

        CLASS_NO_COPY(CVulkanSceneRenderer);

        void renderScene(const vk::CVulkanCommandBuffer& commandBuffer, uint32_t currentFrame) const;

        void onResize();

        static void setDefaultViewportAndScissor(const vk::CVulkanCommandBuffer& commandBuffer, const VkExtent2D extent2D) {
            assert(extent2D.width > 0 && "Viewport width must be greater than zero");
            assert(extent2D.height > 0 && "Viewport height must be greater than zero");

            if (extent2D.width == 0 || extent2D.height == 0) {
                logging::error("Cannot set viewport and scissor: extent width or height is zero");
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

        [[nodiscard]] const context::CVulkanSceneContext& sceneContext() const {
            return _sceneContext;
        }

        [[nodiscard]] context::CVulkanSceneContext& sceneContext() {
            return _sceneContext;
        }


        [[nodiscard]] const CVulkanSceneEntitiesCollection& entitiesCollection() const {
            return _entitiesCollection;
        }

        [[nodiscard]] CVulkanSceneEntitiesCollection& entitiesCollection() {
            return _entitiesCollection;
        }

        [[nodiscard]] const vk::CVulkanAttachment2D& colorAttachment() const {
            return *_colorAttachment;
        }

        [[nodiscard]] const vk::CVulkanAttachment2D& depthAttachment() const {
            return *_depthAttachment;
        }

    protected:
        void createMaterialDescriptors() const;
        void createRenderingData();

        void initCacheData(std::vector<asset::modelGen::CVulkanMaterialData>& materials, const std::vector<asset::modelGen::CVulkanModelData>& models) const;

    private:
        const vk::CVulkanContext& _context;
        vk::CVulkanDescriptorAllocator& _descriptorAllocator;
        cache::CVulkanCacheCollection& _caches;
        context::CVulkanSceneContext _sceneContext;
        CVulkanSceneEntitiesCollection _entitiesCollection{};
        std::unique_ptr<vk::CVulkanAttachment2D> _colorAttachment{};
        std::unique_ptr<vk::CVulkanAttachment2D> _depthAttachment{};
        VkRenderingAttachmentInfo _vkAttachmentColorInfo {};
        VkRenderingAttachmentInfo _vkAttachmentDepthInfo {};
        VkRenderingInfo _vkRenderingInfo {};
        //std::vector<VkRenderingAttachmentInfo> _vkColorAttachmentsInfo{};
        //std::vector<VkRenderingAttachmentInfo> _vkDepthAttachmentsInfo{};
        //std::vector<vk::CVulkanAttachment2D> _depthAttachments{};
        //std::vector<VkRenderingInfo> _vkRenderingInfo{};
    };
}
