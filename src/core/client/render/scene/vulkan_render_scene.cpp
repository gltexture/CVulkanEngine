//
// Created by ded on 11.09.2026.
//

#include "vulkan_render_scene.h"

#include <ranges>

#include "vulkan_render_cache.h"
#include "vulkan_scene_objects.h"
#include "client/render/vulkan_commands.h"
#include "client/render/vulkan_images.h"
#include "client/render/vulkan_shaders.h"
#include "client/render/assets/asset_shaders.h"
#include "client/render/assets/asset_models.h"

namespace cvulkan::client::render::scene {
    static constexpr VkClearValue CLEAR_COLOR  {0.5f, 0.7f, 0.9f, 1.0f};
    static constexpr VkClearValue CLEAR_DEPTH  {1.0f, 0.0f};

    /*
    *             VkUtils.imageBarrier(stack, cmdHandle, swapChainImage,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT);
     */

    void CVulkanSceneRenderer::renderScene(const vk::CVulkanCommandBuffer& commandBuffer, const uint32_t imageIndex, const uint32_t currentFrame) const {
        vk::img_util::imageBarrier(
            commandBuffer.vkCommandBuffer(),
            this->_context.swapChain().imageViews()[imageIndex].vkImage(),
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_NONE,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT
        );
        vk::img_util::imageBarrier(
            commandBuffer.vkCommandBuffer(),
            this->_depthAttachments[imageIndex].image().vkImage(),
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT
        );

        {
            vkCmdBeginRendering(commandBuffer.vkCommandBuffer(), &this->_vkRenderingInfo[imageIndex]);

            const VkExtent2D extent = this->_context.swapChain().extent();
            const VkViewport viewport {
                .x = 0.0F,
                .y = static_cast<float>(extent.height),
                .width = static_cast<float>(extent.width),
                .height = -static_cast<float>(extent.height),
                .minDepth = 0.0F,
                .maxDepth = 1.0F
            };
            vkCmdSetViewport(commandBuffer.vkCommandBuffer(), 0, 1, &viewport);

            const VkRect2D scissor{
                .offset  {
                    .x = 0,
                    .y = 0
                },
                .extent = extent
            };
            vkCmdSetScissor(commandBuffer.vkCommandBuffer(), 0, 1, &scissor);

            {
                this->_sceneContext.renderContext(commandBuffer, this->_descriptorAllocator, this->_entitiesCollection, this->_caches, currentFrame);
            }

            vkCmdEndRendering(commandBuffer.vkCommandBuffer());
        }

        vk::img_util::imageBarrier(
            commandBuffer.vkCommandBuffer(),
            this->_context.swapChain().imageViews()[imageIndex].vkImage(),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_NONE,
            VK_IMAGE_ASPECT_COLOR_BIT
        );
    }

    void CVulkanSceneRenderer::onResize() {
        this->_vkColorAttachmentsInfo.clear();
        this->_vkDepthAttachmentsInfo.clear();
        this->_depthAttachments.clear();
        this->_vkRenderingInfo.clear();
        this->createRenderAttachmentsData();
        {
            this->_sceneContext.onResize();
        }
    }

    void CVulkanSceneRenderer::setDefaults() {
    }

    void CVulkanSceneRenderer::createMaterialDescriptors() {
        const vk::CVulkanDescriptorSet& mat_descriptorSet = this->_descriptorAllocator.allocateDescriptorSets(std::string {DESC_ID_MAT}, 1, *this->_sceneContext.descFragmentStorageLayout()).front();
        const vk::CVulkanBuffer& materials = *this->_caches.materialsCache().materialsBuffer();
        mat_descriptorSet.setBuffer(materials, materials.vkRequestedSize(), this->_sceneContext.descFragmentStorageLayout()->layoutInfoArray().front()._binding, this->_sceneContext.descFragmentStorageLayout()->layoutInfoArray().front()._descType);

        const auto& texturesMap = this->_caches.texturesCache().indexedTexturesMap();
        std::vector<const vk::CVulkanImage2DView*> imageViews;
        imageViews.reserve(texturesMap.keys().size());
        for (const auto& key : texturesMap.keys()) {
            imageViews.push_back(texturesMap.getValue(key).imageView());
        }

        const vk::CVulkanDescriptorSet& txt_descriptorSet = this->_descriptorAllocator.allocateDescriptorSets(std::string {DESC_ID_TEXT}, 1, *this->_sceneContext.descTextureLayout()).front();
        txt_descriptorSet.setImagesArray(imageViews, *this->_sceneContext.defaultTextureSampler(), 0);
    }

    void CVulkanSceneRenderer::createRenderAttachmentsData() {
        {
            this->_vkColorAttachmentsInfo.resize(this->_context.swapChain().numImages());
            for (uint32_t i = 0; i < this->_context.swapChain().numImages(); ++i) {
                this->_vkColorAttachmentsInfo[i] = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = this->_context.swapChain().imageViews()[i].vkImageView(),
                    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = CLEAR_COLOR
                };
            }

            this->_depthAttachments.reserve(this->_context.swapChain().numImages());
            for (uint32_t i = 0; i < this->_context.swapChain().numImages(); ++i) {
                this->_depthAttachments.emplace_back(vk::CVulkanAttachment2D::createSimpleAttachment(this->_context, {this->_context.swapChain().extent().width, this->_context.swapChain().extent().height, 1}, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));
            }

            this->_vkDepthAttachmentsInfo.resize(this->_context.swapChain().numImages());
            for (uint32_t i = 0; i < this->_context.swapChain().numImages(); ++i) {
                this->_vkDepthAttachmentsInfo[i] = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = this->_depthAttachments[i].view().vkImageView(),
                    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = CLEAR_DEPTH
                };
            }

            this->_vkRenderingInfo.resize(this->_context.swapChain().numImages());
            for (uint32_t i = 0; i < this->_context.swapChain().numImages(); ++i) {
                this->_vkRenderingInfo[i] = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                    .renderArea  {
                            {}, this->_context.swapChain().extent(),
                        },
                        .layerCount = 1,
                        .colorAttachmentCount = 1,
                        .pColorAttachments = &this->_vkColorAttachmentsInfo[i],
                        .pDepthAttachment = &this->_vkDepthAttachmentsInfo[i],
                    };
            }
        }
    }

    void CVulkanSceneRenderer::initCacheData(std::vector<asset::modelGen::CVulkanMaterialData>& materials, const std::vector<asset::modelGen::CVulkanModelData>& models) {
        const uint32_t transferQueueFamilyIndex = vk::util::findQueueFamilyIndex(this->_context, vk::CVulkanQueueFamilyBitMasks::TRANSFER);
        const vk::CVulkanCommandPool tempPool {this->_context, transferQueueFamilyIndex, true};
        const vk::CVulkanQueue tempQueue {this->_context, transferQueueFamilyIndex, 0};
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

    CVulkanSceneRenderer::CVulkanSceneRenderer(const vk::CVulkanContext& context, const loop::CVulkanRenderLoop& renderLoop) :
        _context{context},
        _sceneContext{context},
        _caches{context},
        _renderLoop{renderLoop},
        _descriptorAllocator{context} {
        this->createRenderAttachmentsData();

        context::CVulkanSceneResourcesInitializer resourcesInitializer {};
        this->_sceneContext.loadResources(this->_descriptorAllocator, resourcesInitializer);

        std::vector<asset::modelGen::CVulkanMaterialData> materials {};
        std::vector<asset::modelGen::CVulkanModelData> models {};
        for (const auto& [_modelData, _materialsData] : resourcesInitializer.loadedResourcesData()._loadedModels) {
            materials.insert(materials.end(), _materialsData.begin(), _materialsData.end());
            models.push_back(_modelData);
        }
        this->initCacheData(materials, models);

        this->_sceneContext.initializeObjects(this->_entitiesCollection);
    }

    CVulkanSceneRenderer::~CVulkanSceneRenderer() {
        logging::info("Destroying scene renderer");
    }

    CVulkanSceneEntitiesCollection::CVulkanSceneEntitiesCollection() {
        logging::info("Destroying scene entities");
    }

    void CVulkanSceneEntitiesCollection::addEntity(entity::CVulkanSceneEntity&& entity) {
        this->_entities.emplace(entity.id(), std::move(entity));
    }

    bool CVulkanSceneEntitiesCollection::hasEntity(const std::string& id) const {
        return this->_entities.contains(id);
    }
}