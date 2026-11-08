//
// Created by ded on 11.09.2026.
//

#include "vulkan_render_scene__main.h"

#include <ranges>

#include "vulkan_render_cache.h"
#include "vulkan_scene_objects.h"
#include "client/render/vulkan_commands.h"
#include "client/render/vulkan_images.h"
#include "client/render/assets/asset_shaders.h"
#include "client/render/assets/asset_models.h"

namespace cvulkan::client::render::scene {
    main::CVulkanSceneEntitiesCollection::CVulkanSceneEntitiesCollection() {
        logging::info("Destroying scene entities");
    }

    void main::CVulkanSceneEntitiesCollection::addEntity(entity::CVulkanSceneEntity&& entity) {
        this->_entities.emplace(entity.id(), std::move(entity));
    }

    bool main::CVulkanSceneEntitiesCollection::hasEntity(const std::string& id) const {
        return this->_entities.contains(id);
    }

    main::CVulkanSceneRenderer::CVulkanSceneRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, cache::CVulkanCacheCollection& caches)
        : _context{context}, _descriptorAllocator{descriptorAllocator}, _caches {caches}, _sceneContext{context} {
        this->createRenderingData();

        context::CVulkanSceneResourcesInitializer resourcesInitializer{};
        this->_sceneContext.loadResources(this->_descriptorAllocator, resourcesInitializer);

        std::vector<asset::modelGen::CVulkanMaterialData> materials{};
        std::vector<asset::modelGen::CVulkanModelData> models{};
        for (const auto& [_modelData, _materialsData]: resourcesInitializer.loadedResourcesData()._loadedModels) {
            materials.insert(materials.end(), _materialsData.begin(), _materialsData.end());
            models.push_back(_modelData);
        }
        this->initCacheData(materials, models);

        this->_sceneContext.initializeObjects(this->_entitiesCollection);
    }

    main::CVulkanSceneRenderer::~CVulkanSceneRenderer() {
        logging::info("Destroying main scene renderer");
    }

    void main::CVulkanSceneRenderer::renderScene(const vk::CVulkanCommandBuffer& commandBuffer, const uint32_t currentFrame) const {
        vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), this->_colorAttachment->image().vkImage(),
                                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                   VK_IMAGE_ASPECT_COLOR_BIT
        );
        vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), this->_depthAttachment->image().vkImage(),
                                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                   VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                                   VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                   VK_IMAGE_ASPECT_DEPTH_BIT
        );

        vkCmdBeginRendering(commandBuffer.vkCommandBuffer(), &this->_vkRenderingInfo);
        setDefaultViewportAndScissor(commandBuffer, this->_colorAttachment->image().extent2D());
        this->_sceneContext.renderContext(commandBuffer, this->_descriptorAllocator, this->_entitiesCollection, this->_caches, currentFrame);
        vkCmdEndRendering(commandBuffer.vkCommandBuffer());
    }

    void main::CVulkanSceneRenderer::onResize() {
        this->sceneContext().projection().resize({this->_context.glfwWindow().size().x, this->_context.glfwWindow().size().y});
        this->_depthAttachment.reset();
        this->_colorAttachment.reset();
        this->createRenderingData();
        {
            this->_sceneContext.onResize();
        }
    }

    void main::CVulkanSceneRenderer::createMaterialDescriptors() const {
        const vk::CVulkanDescriptorSet& mat_descriptorSet = this->_descriptorAllocator.allocateDescriptorSets(DESC_ID_MAT, 1, *this->_sceneContext.descFragmentStorageLayout()).front();
        const vk::CVulkanBuffer& materials = *this->_caches.materialsCache().materialsBuffer();
        mat_descriptorSet.setBuffer(materials, materials.vkRequestedSize(), this->_sceneContext.descFragmentStorageLayout()->layoutInfoArray().front()._binding,
                                    this->_sceneContext.descFragmentStorageLayout()->layoutInfoArray().front()._descType);

        const auto& texturesMap = this->_caches.texturesCache().indexedTexturesMap();
        std::vector<vk::CVulkanImage2DView_CRef> imageViews;
        imageViews.reserve(texturesMap.keys().size());
        for (const auto& key: texturesMap.keys()) {
            imageViews.push_back(std::cref(texturesMap.getValue(key).imageView()));
        }

        const vk::CVulkanDescriptorSet& txt_descriptorSet = this->_descriptorAllocator.allocateDescriptorSets(DESC_ID_TEXT, 1, *this->_sceneContext.descTextureLayout()).front();
        txt_descriptorSet.setImagesArray(imageViews, *this->_sceneContext.defaultTextureSampler(), 0);
    }

    void main::CVulkanSceneRenderer::createRenderingData() {
        {
            this->_colorAttachment = std::make_unique<vk::CVulkanAttachment2D>(vk::CVulkanAttachment2D::createSimpleAttachment(this->_context, this->_context.swapChain().extent3D(), COLOR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
            this->_depthAttachment = std::make_unique<vk::CVulkanAttachment2D>(vk::CVulkanAttachment2D::createSimpleAttachment(this->_context, this->_context.swapChain().extent3D(), DEPTH_FORMAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));
        }

        {
            this->_vkAttachmentColorInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = this->_colorAttachment->view().vkImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = CLEAR_COLOR
            };

            this->_vkAttachmentDepthInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = this->_depthAttachment->view().vkImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = CLEAR_DEPTH
            };
        }

        {
            this->_vkRenderingInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .renderArea = {
                    {}, {this->_colorAttachment->image().extent().width, this->_colorAttachment->image().extent().height}
                },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &this->_vkAttachmentColorInfo,
                .pDepthAttachment = &this->_vkAttachmentDepthInfo,
            };
        }
    }

    void main::CVulkanSceneRenderer::initCacheData(std::vector<asset::modelGen::CVulkanMaterialData>& materials, const std::vector<asset::modelGen::CVulkanModelData>& models) const {
        const uint32_t transferQueueFamilyIndex = vk::util::findQueueFamilyIndex(this->_context, vk::CVulkanQueueFamilyBitMasks::TRANSFER);
        const vk::CVulkanCommandPool tempPool{this->_context, transferQueueFamilyIndex, true};
        const vk::CVulkanQueue tempQueue{this->_context, transferQueueFamilyIndex, 0};
        logging::info("Loading {} material(s) into cache", materials.size());
        this->_caches.materialsCache().loadMaterials(materials, this->_caches.texturesCache(), tempPool, tempQueue);
        logging::info("Loaded {} material(s) into cache", materials.size());

        logging::info("Transitioning textures into cache");
        this->_caches.texturesCache().transitTextures(tempPool, tempQueue);
        logging::info("Textures transitioned into cache");

        logging::info("Loading {} model(s) into cache", models.size());
        this->_caches.modelCache().loadModels(models, tempPool, tempQueue);
        logging::info("Loaded {} model(s) into cache", models.size());

        this->createMaterialDescriptors();
    }
}
