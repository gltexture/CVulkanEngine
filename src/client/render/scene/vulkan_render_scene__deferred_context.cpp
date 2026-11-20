#include "vulkan_render_scene__deferred.h"
#include "vulkan_scene_objects.h"
#include "client/test_logic/test_logic.h"
#include "camera/view_camera.h"
#include "client/render/vulkan_config.hpp"
#include "client/render/assets/asset_models.h"
#include "client/render/assets/asset_shaders.h"
#include "core/utility/functionality.h"
#include "core/utility/io.h"

namespace cvulkan::client::render::scene::deferred::context {
    ModelId CVulkanSceneResourcesInitializer::loadModelWithMaterials(const io::CVulkanResourcePath& resourcePath) {
        asset::modelLoad::CVulkanLoadedModelData cubeModel = asset::modelLoad::loadModelWithMaterials(resourcePath);
        this->_loadedResourcesData._loadedModels.push_back(std::move(cubeModel));
        return this->_loadedResourcesData._loadedModels.back()._modelData._id;
    }

    CVulkanSceneContext::CVulkanSceneContext(const vk::CVulkanContext& context) : _context{context}, _projection({glm::radians(60.0f), 0.1f, 1024.0f, {context.swapChain().vkExtent2D()}}) {
    }

    void CVulkanSceneContext::loadResources(vk::CVulkanDescriptorAllocator& descriptorAllocator, CVulkanSceneResourcesInitializer& sceneResourcesInitializer) {
        this->_descVertexUniformLayout = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, vk::CVulkanLayoutInfo {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, VK_SHADER_STAGE_VERTEX_BIT});
        this->_descFragmentStorageLayout = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, vk::CVulkanLayoutInfo {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT});
        this->_descTextureLayout = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, vk::CVulkanLayoutInfo {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, renderConfig::MAX_TEXTURES_IN_CACHE, VK_SHADER_STAGE_FRAGMENT_BIT});

        this->_bufferProjMatrix = std::make_unique<vk::CVulkanBuffer>(vk::buff_util::allocateHostVisibleBuffer(this->_context, descriptorAllocator, sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, gpass::DESC_ID_PRJ, *this->_descVertexUniformLayout));
        this->_bufferViewMatrices = std::make_unique<std::vector<vk::CVulkanBuffer>>(vk::buff_util::allocateHostVisibleBuffers(this->_context, descriptorAllocator, sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, renderConfig::MAX_FRAMES_IN_FLIGHT, gpass::DESC_ID_VIEW, *this->_descVertexUniformLayout));
        vk::buff_util::copyMatrixToBuffer(*this->_bufferProjMatrix, this->projection().mat4Projection(), 0);

        constexpr vk::CVulkanSamplerInfo samplerInfo{
            ._addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            ._borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            ._mipLevels = 1,
            ._anisotropy = true
        };
        this->_textureSampler = std::make_unique<vk::CVulkanSampler>(this->_context, samplerInfo);

        const shader::CVulkanShaderModule shaderV = asset::shaderLoad::compileAndLoadShader(this->_context, io::CVulkanResourcePath{"scene_geometry/v.vert"}, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
        const shader::CVulkanShaderModule shaderF = asset::shaderLoad::compileAndLoadShader(this->_context, io::CVulkanResourcePath{"scene_geometry/f.frag"}, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);

        constexpr vk::CVulkanPushConstantRange push1 {
            ._stage = VK_SHADER_STAGE_VERTEX_BIT,
            ._offset = 0,
            ._size = sizeof(glm::mat4)
        };
        constexpr vk::CVulkanPushConstantRange push2 {
            ._stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            ._offset = sizeof(glm::mat4),
            ._size = sizeof(uint32_t)
        };

        const structs::CVulkanMeshVertexBufferStruct vertexStruct{this->_context};
        const vk::CVulkanPipelineBuildInfo buildInfo{
            fun::packCRefs(*this->_descVertexUniformLayout, *this->_descVertexUniformLayout, *this->_descFragmentStorageLayout, *this->_descTextureLayout),
            fun::packCRefs(shaderV, shaderF),
            fun::packCRefs(push1, push2),
            vertexStruct.vkPipelineVertexInputStateCreateInfo(), {mrt::ALBEDO_FORMAT}, mrt::DEPTH_FORMAT, true};

        this->_renderPipeline = std::make_unique<vk::CVulkanPipeline>(this->_context, buildInfo);

        {
            test_logic::initSceneResources(sceneResourcesInitializer);
        }

        this->_currentCamera = std::make_unique<camera::CVulkanCamera>(this->_context);
    }

    void CVulkanSceneContext::initializeObjects(entity::CVulkanSceneEntitiesCollection& entitiesCollection) {
        {
            test_logic::initSceneObjects(entitiesCollection);
        }
    }

    void CVulkanSceneContext::renderContext(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanDescriptorAllocator& descriptorAllocator, const entity::CVulkanSceneEntitiesCollection& entitiesCollection,
                                            const cache::CVulkanCacheCollection& caches, const
                                            uint32_t currentFrame) const {
        std::vector<VkDescriptorSet> descriptorSets{};
        descriptorSets.reserve(3);
        descriptorSets.push_back(descriptorAllocator.getDescriptorSet(gpass::DESC_ID_PRJ, 0).vkDescriptorSet());
        descriptorSets.push_back(descriptorAllocator.getDescriptorSet(gpass::DESC_ID_VIEW, currentFrame).vkDescriptorSet());
        descriptorSets.push_back(descriptorAllocator.getDescriptorSet(gpass::DESC_ID_MAT, 0).vkDescriptorSet());
        descriptorSets.push_back(descriptorAllocator.getDescriptorSet(gpass::DESC_ID_TEXT, 0).vkDescriptorSet());

        vk::buff_util::copyMatrixToBuffer(this->_bufferViewMatrices->at(currentFrame), this->_currentCamera->viewMatrix(), 0);

        vkCmdBindPipeline(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_renderPipeline->vkPipeline());
        vkCmdBindDescriptorSets(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_renderPipeline->vkPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

        this->renderEntities(commandBuffer, entitiesCollection, caches, false);
        this->renderEntities(commandBuffer, entitiesCollection, caches, true);
    }

    void CVulkanSceneContext::renderEntities(const vk::CVulkanCommandBuffer& commandBuffer, const entity::CVulkanSceneEntitiesCollection& entitiesCollection, const cache::CVulkanCacheCollection& caches, const bool transparent) const {
        for (const auto& entity: entitiesCollection.entities() | std::views::values) {
            const auto& model = caches.modelCache().getModel(entity.modelId());

            const glm::mat4 modelMat = entity.transform().modelMatrix();
            vkCmdPushConstants(commandBuffer.vkCommandBuffer(), this->_renderPipeline->vkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(modelMat), &modelMat);
            for (uint32_t j = 0; j < model->meshes().size(); j++) {
                const auto& mesh = model->meshes().at(j);
                const VkBuffer vertexBuffer = mesh.verticesBuffer().vkBuffer();
                constexpr VkDeviceSize offset = 0;

                const std::string materialId = mesh.materialId();
                const structs::CVulkanMaterial* material = caches.materialsCache().getMaterial(materialId);
                if (material == nullptr) {
                    log::error("Mesh {} in model '{}' has no material", j, model->id());
                    continue;
                }
                if (material->_hasTransparency != transparent) {
                    continue;
                }
                const uint32_t materialIdx = caches.materialsCache().getPosition(materialId);
                vkCmdPushConstants(commandBuffer.vkCommandBuffer(), this->_renderPipeline->vkPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(modelMat), sizeof(uint32_t), &materialIdx);
                vkCmdBindVertexBuffers(commandBuffer.vkCommandBuffer(), 0, 1, &vertexBuffer, &offset);
                vkCmdBindIndexBuffer(commandBuffer.vkCommandBuffer(), mesh.indicesBuffer().vkBuffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(commandBuffer.vkCommandBuffer(), mesh.numIndices(), 1, 0, 0, 0);
            }
        }
    }

    void CVulkanSceneContext::onResize() {
        vk::buff_util::copyMatrixToBuffer(*this->_bufferProjMatrix, this->projection().mat4Projection(), 0);
    }
}
