//
// Created by ded on 11.09.2026.
//

#include "vulkan_render_scene__deferred.h"

#include <ranges>

#include "cvulkan_render_mrt.h"
#include "vulkan_render_cache.h"
#include "vulkan_scene_objects.h"
#include "client/render/vulkan_commands.h"
#include "client/render/vulkan_images.h"
#include "client/render/assets/asset_models.h"

namespace cvulkan::client::render::scene::deferred::gpass {
    CVulkanSceneRendererGPass::CVulkanSceneRendererGPass(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, cache::CVulkanCacheCollection& caches)
        : _context{context}, _descriptorAllocator{descriptorAllocator}, _caches {caches}, _multipleRenderTargets{context}, _sceneContext{context} {
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

    CVulkanSceneRendererGPass::~CVulkanSceneRendererGPass() {
        log::info("Destroying gpass scene_geometry renderer");
    }

    void CVulkanSceneRendererGPass::renderScene(const vk::CVulkanCommandBuffer& commandBuffer, const uint32_t currentFrame) const {
        vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), this->_multipleRenderTargets.colorAlbedoAttachment().image().vkImage(),
                                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                   VK_IMAGE_ASPECT_COLOR_BIT
        );
        vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), this->_multipleRenderTargets.depthAttachment().image().vkImage(),
                                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                   VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                                   VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                   VK_IMAGE_ASPECT_DEPTH_BIT
        );

        vkCmdBeginRendering(commandBuffer.vkCommandBuffer(), &this->_vkRenderingInfo);
        CVulkanSceneRenderer::setViewportAndScissor(commandBuffer, this->_context.swapChain().vkExtent2D());
        this->_sceneContext.renderContext(commandBuffer, this->_descriptorAllocator, this->_entitiesCollection, this->_caches, currentFrame);
        vkCmdEndRendering(commandBuffer.vkCommandBuffer());
    }

    void CVulkanSceneRendererGPass::onResize() {
        this->sceneContext().projection().resize({this->_context.glfwWindow().size().x, this->_context.glfwWindow().size().y});
        this->_multipleRenderTargets.reset();
        this->createRenderingData();
        {
            this->_sceneContext.onResize();
        }
    }

    void CVulkanSceneRendererGPass::createMaterialDescriptors() const {
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
        txt_descriptorSet.setImagesArray(imageViews, *this->_sceneContext.textureSampler(), 0);
    }

    void CVulkanSceneRendererGPass::createRenderingData() {
        {
            this->_vkAttachmentColorInfos = {
                {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = this->_multipleRenderTargets.colorAlbedoAttachment().view().vkImageView(),
                    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = CLEAR_COLOR
                }
            };
        }

        {
            this->_vkAttachmentDepthInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = this->_multipleRenderTargets.depthAttachment().view().vkImageView(),
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
                    {}, {this->_context.swapChain().vkExtent2D()}
                },
                .layerCount = 1,
                .colorAttachmentCount = static_cast<uint32_t>(this->_vkAttachmentColorInfos.size()),
                .pColorAttachments = this->_vkAttachmentColorInfos.data(),
                .pDepthAttachment = &this->_vkAttachmentDepthInfo,
            };
        }
    }

    void CVulkanSceneRendererGPass::initCacheData(std::vector<asset::modelGen::CVulkanMaterialData>& materials, const std::vector<asset::modelGen::CVulkanModelData>& models) const {
        const uint32_t transferQueueFamilyIndex = vk::util::findQueueFamilyIndex(this->_context, vk::CVulkanQueueFamilyBitMasks::TRANSFER);
        const vk::CVulkanCommandPool tempPool{this->_context, transferQueueFamilyIndex, true};
        const vk::CVulkanQueue tempQueue{this->_context, transferQueueFamilyIndex, 0};
        log::info("Loading {} material(s) into cache", materials.size());
        this->_caches.materialsCache().loadMaterials(materials, this->_caches.texturesCache(), tempPool, tempQueue);
        log::info("Loaded {} material(s) into cache", materials.size());

        log::info("Transitioning textures into cache");
        this->_caches.texturesCache().transitTextures(tempPool, tempQueue);
        log::info("Textures transitioned into cache");

        log::info("Loading {} model(s) into cache", models.size());
        this->_caches.modelCache().loadModels(models, tempPool, tempQueue);
        log::info("Loaded {} model(s) into cache", models.size());

        this->createMaterialDescriptors();
    }
}
