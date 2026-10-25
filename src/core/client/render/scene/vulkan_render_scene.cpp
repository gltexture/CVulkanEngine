//
// Created by ded on 11.09.2026.
//

#include "vulkan_render_scene.h"

#include <ranges>

#include "vulkan_render_cache.h"
#include "vulkan_scene_objects.h"
#include "client/render/vulkan_commands.h"
#include "client/render/vulkan_config.hpp"
#include "client/render/vulkan_images.h"
#include "client/render/vulkan_shaders.h"
#include "client/render/assets/asset_shaders.h"
#include "client/render/assets/asset_models.h"
#include "shaderc/shaderc.h"

namespace cvulkan::client::render::scene {
    static constexpr VkClearValue CLEAR_COLOR  {0.5f, 0.7f, 0.9f, 1.0f};
    static constexpr VkClearValue CLEAR_DEPTH  {1.0f, 0.0f};

    static constexpr std::string_view DESC_ID_MAT = "SCN_DESC_ID_MAT";
    static constexpr std::string_view DESC_ID_PRJ = "SCN_DESC_ID_PRJ";
    static constexpr std::string_view DESC_ID_TEXT = "SCN_DESC_ID_TEXT";

    /*
    *             VkUtils.imageBarrier(stack, cmdHandle, swapChainImage,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT);
     */

    void CVulkanSceneRenderer::renderScene(const vk::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const {
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

            vkCmdBindPipeline(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_defaultRenderPipeline->vkPipeline());
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

            std::vector<VkDescriptorSet> descriptorSets {};
            descriptorSets.reserve(3);
            descriptorSets.push_back(this->_descriptorAllocator.getDescriptorSet(std::string {DESC_ID_PRJ}, 0).vkDescriptorSet());
            descriptorSets.push_back(this->_descriptorAllocator.getDescriptorSet(std::string {DESC_ID_MAT}, 0).vkDescriptorSet());
            descriptorSets.push_back(this->_descriptorAllocator.getDescriptorSet(std::string {DESC_ID_TEXT}, 0).vkDescriptorSet());
            vkCmdBindDescriptorSets(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_defaultRenderPipeline->vkPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

            for (const auto& entity : this->_entitiesCollection.entities() | std::views::values) {
                const auto& model = this->caches().modelCache().getModel(entity.modelId());

                const glm::mat4 modelMat = entity.transform().modelMatrix();
                vkCmdPushConstants(commandBuffer.vkCommandBuffer(), this->_defaultRenderPipeline->vkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(modelMat), &modelMat);
                for (uint32_t j = 0; j < model->meshes().size(); j++) {
                    const auto& mesh = model->meshes().at(j);
                    const VkBuffer vertexBuffer = mesh.verticesBuffer().vkBuffer();
                    constexpr VkDeviceSize offset = 0;

                    const std::string materialId = mesh.materialId();
                    const structs::CVulkanMaterial* material = this->_caches.materialsCache().getMaterial(materialId);
                    if (material == nullptr) {
                        logging::error("Mesh {} in model '{}' has no material", j, model->id());
                        continue;
                    }
                    const uint32_t materialIdx = this->_caches.materialsCache().getPosition(materialId);
                    vkCmdPushConstants(commandBuffer.vkCommandBuffer(), this->_defaultRenderPipeline->vkPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(modelMat), sizeof(uint32_t), &materialIdx);
                    vkCmdBindVertexBuffers(commandBuffer.vkCommandBuffer(), 0, 1, &vertexBuffer, &offset);
                    vkCmdBindIndexBuffer(commandBuffer.vkCommandBuffer(), mesh.indicesBuffer().vkBuffer(), 0, VK_INDEX_TYPE_UINT32);
                    vkCmdDrawIndexed(commandBuffer.vkCommandBuffer(), mesh.numIndices(), 1, 0, 0, 0);
                }
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

    void CVulkanSceneRenderer::resize() {
        this->_vkColorAttachmentsInfo.clear();
        this->_vkDepthAttachmentsInfo.clear();
        this->_depthAttachments.clear();
        this->_vkRenderingInfo.clear();
        this->createRenderData();

        {
            vk::buffutil::copyMatrixToBuffer(*this->_bufferProjMatrix, this->projection().mat4Projection(), 0);
        }
    }

    void CVulkanSceneRenderer::loadMaterials() {
        const vk::CVulkanDescriptorSet& mat_descriptorSet = this->_descriptorAllocator.allocateDescriptorSets(std::string {DESC_ID_MAT}, 1, *this->_descFragmentStorageLayout).front();
        const vk::CVulkanBuffer& materials = *this->_caches.materialsCache().materialsBuffer();
        mat_descriptorSet.setBuffer(materials, materials.vkRequestedSize(), this->_descFragmentStorageLayout->layoutInfoArray().front()._binding, this->_descFragmentStorageLayout->layoutInfoArray().front()._descType);

        const auto& texturesMap = this->_caches.texturesCache().indexedTexturesMap().map();
        std::vector<const vk::CVulkanImage2DView*> imageViews;
        imageViews.reserve(texturesMap.size());
        std::ranges::transform(texturesMap | std::views::values, std::back_inserter(imageViews),
            [](const auto& t) {
                return t.imageView();
            });

        const vk::CVulkanDescriptorSet& txt_descriptorSet = this->_descriptorAllocator.allocateDescriptorSets(std::string {DESC_ID_TEXT}, 1, *this->_descTextureLayout).front();
        txt_descriptorSet.setImagesArray(imageViews, *this->_defaultTextureSampler, 0);
    }

    void CVulkanSceneRenderer::createRenderData() {
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

    void CVulkanSceneRenderer::initCacheData(const std::vector<asset::modelGen::CVulkanMaterialData>& materials, const std::vector<asset::modelGen::CVulkanModelData>& models) {
        uint32_t transferQueueFamilyIndex = UINT32_MAX; //TODO
        for (const auto& t: this->_context.device().queueFamiliesRegistry().registeredData()) {
            if (t._bitMask & vk::CVulkanQueueFamilyBitMasks::GRAPHICS) {
                transferQueueFamilyIndex = t._queueFamilyIndex;
                break;
            }
        }
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

        this->loadMaterials();
    }

    void CVulkanSceneRenderer::loadResources() {
        {
            this->_descVertexUniformLayout = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, std::initializer_list<vk::CVulkanLayoutInfo>{
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, VK_SHADER_STAGE_VERTEX_BIT}
            });
            this->_descFragmentStorageLayout = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, std::initializer_list<vk::CVulkanLayoutInfo>{
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT}
            });
            this->_descTextureLayout = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, std::initializer_list<vk::CVulkanLayoutInfo>{
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, renderConfig::MAX_TEXTURES_IN_CACHE, VK_SHADER_STAGE_FRAGMENT_BIT}
            });

            this->_bufferProjMatrix = std::make_unique<vk::CVulkanBuffer>(vk::buffutil::allocateHostVisibleBuffer(this->_context, this->_descriptorAllocator, sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, std::string{DESC_ID_PRJ}, *this->_descVertexUniformLayout));
            vk::buffutil::copyMatrixToBuffer(*this->_bufferProjMatrix, this->projection().mat4Projection(), 0);

            constexpr vk::CVulkanSamplerInfo samplerInfo {VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_INT_OPAQUE_BLACK, 1, true};
            this->_defaultTextureSampler = std::make_unique<vk::CVulkanSampler>(this->_context, samplerInfo);

            shader::CVulkanShaderModule shaderV = asset::shaderLoad::compileAndLoadShader(this->_context, io_utils::CVulkanResourcePath{"scene/v.vert"}, shaderc_glsl_vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
            shader::CVulkanShaderModule shaderF = asset::shaderLoad::compileAndLoadShader(this->_context, io_utils::CVulkanResourcePath{"scene/f.frag"}, shaderc_glsl_fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);
            std::vector<vk::Ref_CVulkanShaderModule> shaderModules  {};
            shaderModules.reserve(2);
            shaderModules.push_back(std::cref(shaderV));
            shaderModules.push_back(std::cref(shaderF));

            vk::CVulkanPushConstantRange push1 {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4)};
            vk::CVulkanPushConstantRange push2 {VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), sizeof(uint32_t)};
            std::vector<vk::Ref_CVulkanPushConstantRange> pushConstantRanges {};
            pushConstantRanges.reserve(2);
            pushConstantRanges.push_back(std::cref(push1));
            pushConstantRanges.push_back(std::cref(push2));

            std::vector<vk::Ref_CVulkanDescriptorSetLayout> descriptorSetLayouts {};
            descriptorSetLayouts.reserve(3);
            descriptorSetLayouts.push_back(std::cref(*this->_descVertexUniformLayout));
            descriptorSetLayouts.push_back(std::cref(*this->_descFragmentStorageLayout));
            descriptorSetLayouts.push_back(std::cref(*this->_descTextureLayout));

            const structs::CVulkanVertexStruct vertexStruct{this->_context};
            const vk::CVulkanPipelineBuildInfo buildInfo{descriptorSetLayouts, shaderModules, pushConstantRanges, vertexStruct.vkPipelineVertexInputStateCreateInfo(), this->_context.surface().vkFormat(), VK_FORMAT_D16_UNORM};

            this->_defaultRenderPipeline = std::make_unique<vk::CVulkanPipeline>(this->_context, buildInfo);
        }
    }

    CVulkanSceneRenderer::CVulkanSceneRenderer(const vk::CVulkanContext& context, const loop::CVulkanRenderLoop& renderLoop) :
        _context{context},
        _caches{context},
        _renderLoop(renderLoop),
        _projection({glm::radians(60.0f), 0.1f, 1024.0f, {context.swapChain().extent()}}),
        _descriptorAllocator{context} {
        this->loadResources();

        const asset::modelLoad::CVulkanLoadedModelData loadedModelData = asset::modelLoad::loadModelWithMaterials(io_utils::CVulkanResourcePath {"cube/cube.obj"});

        this->createRenderData();
        this->initCacheData(loadedModelData._materialsData, {loadedModelData._modelData});

        {
            entity::CVulkanSceneEntity entity = {"CubeEnt", loadedModelData._modelData._id};
            entity.transform()._position = {0.0f, 0.0f, -6.0f};
            entity.transform()._rotation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            this->_entitiesCollection.addEntity(std::move(entity));
        }
    }

    CVulkanSceneRenderer::~CVulkanSceneRenderer() {
    }

    CVulkanSceneEntitiesCollection::CVulkanSceneEntitiesCollection() {
    }

    void CVulkanSceneEntitiesCollection::addEntity(entity::CVulkanSceneEntity&& entity) {
        this->_entities.emplace(entity.id(), std::move(entity));
    }

    bool CVulkanSceneEntitiesCollection::hasEntity(const std::string& id) const {
        return this->_entities.contains(id);
    }
}
