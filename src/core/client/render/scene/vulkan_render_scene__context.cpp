#include "vulkan_render_scene.h"
#include "vulkan_scene_objects.h"
#include "test_logic/test_logic.h"
#include "camera/view_camera.h"
#include "client/render/vulkan_config.hpp"
#include "client/render/assets/asset_models.h"
#include "client/render/assets/asset_shaders.h"
#include "utility/io_utils.h"

namespace cvulkan::client::render::scene::context {
    ModelId CVulkanSceneResourcesInitializer::loadModelWithMaterials(const io_utils::CVulkanResourcePath& resourcePath) {
        asset::modelLoad::CVulkanLoadedModelData cubeModel = asset::modelLoad::loadModelWithMaterials(resourcePath);
        this->_loadedResourcesData._loadedModels.push_back(std::move(cubeModel));
        return this->_loadedResourcesData._loadedModels.back()._modelData._id;
    }

    CVulkanSceneContext::CVulkanSceneContext(const vk::CVulkanContext &context) : _context{context}, _projection({glm::radians(60.0f), 0.1f, 1024.0f, {context.swapChain().extent()}}) {
    }

    void CVulkanSceneContext::loadResources(vk::CVulkanDescriptorAllocator& descriptorAllocator, CVulkanSceneResourcesInitializer& sceneResourcesInitializer) {
        this->_descVertexUniformLayout = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, std::initializer_list<vk::CVulkanLayoutInfo>{
                {
                    ._descType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    ._binding = 0,
                    ._descCount = 1,
                    ._stage = VK_SHADER_STAGE_VERTEX_BIT
                }
            });
        this->_descFragmentStorageLayout = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, std::initializer_list<vk::CVulkanLayoutInfo>{
            {
                ._descType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                ._binding = 0,
                ._descCount = 1,
                ._stage = VK_SHADER_STAGE_FRAGMENT_BIT
            }
        });
        this->_descTextureLayout = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, std::initializer_list<vk::CVulkanLayoutInfo>{
            {
                ._descType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                ._binding = 0,
                ._descCount = renderConfig::MAX_TEXTURES_IN_CACHE,
                ._stage = VK_SHADER_STAGE_FRAGMENT_BIT
            }
        });

        this->_bufferProjMatrix = std::make_unique<vk::CVulkanBuffer>(vk::buffutil::allocateHostVisibleBuffer(this->_context, descriptorAllocator, sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, std::string{DESC_ID_PRJ}, *this->_descVertexUniformLayout));
        this->_bufferViewMatrices = std::make_unique<std::vector<vk::CVulkanBuffer>>(vk::buffutil::allocateHostVisibleBuffers(this->_context, descriptorAllocator, sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, renderConfig::MAX_IMAGES_IN_FLIGHT, std::string{DESC_ID_VIEW}, *this->_descVertexUniformLayout));
        vk::buffutil::copyMatrixToBuffer(*this->_bufferProjMatrix, this->projection().mat4Projection(), 0);

        constexpr vk::CVulkanSamplerInfo samplerInfo {
            ._addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            ._borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            ._mipLevels = 1,
            ._anisotropy = true
        };
        this->_defaultTextureSampler = std::make_unique<vk::CVulkanSampler>(this->_context, samplerInfo);

        const shader::CVulkanShaderModule shaderV = asset::shaderLoad::compileAndLoadShader(this->_context, io_utils::CVulkanResourcePath{"scene/v.vert"}, VK_SHADER_STAGE_VERTEX_BIT);
        const shader::CVulkanShaderModule shaderF = asset::shaderLoad::compileAndLoadShader(this->_context, io_utils::CVulkanResourcePath{"scene/f.frag"}, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::vector<vk::Ref_CVulkanShaderModule> shaderModules  {};
        shaderModules.reserve(2);
        shaderModules.push_back(std::cref(shaderV));
        shaderModules.push_back(std::cref(shaderF));

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
        std::vector<vk::Ref_CVulkanPushConstantRange> pushConstantRanges {};
        pushConstantRanges.reserve(2);
        pushConstantRanges.push_back(std::cref(push1));
        pushConstantRanges.push_back(std::cref(push2));

        std::vector<vk::Ref_CVulkanDescriptorSetLayout> descriptorSetLayouts {};
        descriptorSetLayouts.reserve(3);
        descriptorSetLayouts.push_back(std::cref(*this->_descVertexUniformLayout));
        descriptorSetLayouts.push_back(std::cref(*this->_descVertexUniformLayout));
        descriptorSetLayouts.push_back(std::cref(*this->_descFragmentStorageLayout));
        descriptorSetLayouts.push_back(std::cref(*this->_descTextureLayout));

        const structs::CVulkanVertexStruct vertexStruct{this->_context};
        const vk::CVulkanPipelineBuildInfo buildInfo{descriptorSetLayouts, shaderModules, pushConstantRanges, vertexStruct.vkPipelineVertexInputStateCreateInfo(), this->_context.surface().vkFormat(), VK_FORMAT_D16_UNORM, true};

        this->_defaultRenderPipeline = std::make_unique<vk::CVulkanPipeline>(this->_context, buildInfo);

        {
            test_logic::initSceneResources(sceneResourcesInitializer);
        }

        this->_currentCamera = std::make_unique<camera::CVulkanCamera>(this->_context);
    }

    void CVulkanSceneContext::initializeObjects(CVulkanSceneEntitiesCollection& entitiesCollection) {
       {
           test_logic::initSceneObjects(entitiesCollection);
       }
    }

    void CVulkanSceneContext::renderContext(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanDescriptorAllocator& descriptorAllocator, const CVulkanSceneEntitiesCollection& entitiesCollection, const cache::CVulkanCacheCollection& caches, const uint32_t currentFrame) const {
        std::vector<VkDescriptorSet> descriptorSets {};
        descriptorSets.reserve(3);
        descriptorSets.push_back(descriptorAllocator.getDescriptorSet(std::string {DESC_ID_PRJ}, 0).vkDescriptorSet());
        descriptorSets.push_back(descriptorAllocator.getDescriptorSet(std::string {DESC_ID_VIEW}, 0).vkDescriptorSet());
        descriptorSets.push_back(descriptorAllocator.getDescriptorSet(std::string {DESC_ID_MAT}, 0).vkDescriptorSet());
        descriptorSets.push_back(descriptorAllocator.getDescriptorSet(std::string {DESC_ID_TEXT}, 0).vkDescriptorSet());

        vk::buffutil::copyMatrixToBuffer(this->_bufferViewMatrices->at(currentFrame), this->_currentCamera->viewMatrix(), 0);

        vkCmdBindPipeline(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_defaultRenderPipeline->vkPipeline());
        vkCmdBindDescriptorSets(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_defaultRenderPipeline->vkPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

        this->renderEntities(commandBuffer, entitiesCollection, caches, false);
        this->renderEntities(commandBuffer, entitiesCollection, caches, true);
    }

    void CVulkanSceneContext::renderEntities(const vk::CVulkanCommandBuffer& commandBuffer, const CVulkanSceneEntitiesCollection& entitiesCollection, const cache::CVulkanCacheCollection& caches, const bool transparent) const {
        for (const auto& entity : entitiesCollection.entities() | std::views::values) {
            const auto& model = caches.modelCache().getModel(entity.modelId());

            const glm::mat4 modelMat = entity.transform().modelMatrix();
            vkCmdPushConstants(commandBuffer.vkCommandBuffer(), this->_defaultRenderPipeline->vkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(modelMat), &modelMat);
            for (uint32_t j = 0; j < model->meshes().size(); j++) {
                const auto& mesh = model->meshes().at(j);
                const VkBuffer vertexBuffer = mesh.verticesBuffer().vkBuffer();
                constexpr VkDeviceSize offset = 0;

                const std::string materialId = mesh.materialId();
                const structs::CVulkanMaterial* material = caches.materialsCache().getMaterial(materialId);
                if (material == nullptr) {
                    logging::error("Mesh {} in model '{}' has no material", j, model->id());
                    continue;
                }
                if (material->_hasTransparency != transparent) {
                    continue;
                }
                const uint32_t materialIdx = caches.materialsCache().getPosition(materialId);
                vkCmdPushConstants(commandBuffer.vkCommandBuffer(), this->_defaultRenderPipeline->vkPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(modelMat), sizeof(uint32_t), &materialIdx);
                vkCmdBindVertexBuffers(commandBuffer.vkCommandBuffer(), 0, 1, &vertexBuffer, &offset);
                vkCmdBindIndexBuffer(commandBuffer.vkCommandBuffer(), mesh.indicesBuffer().vkBuffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(commandBuffer.vkCommandBuffer(), mesh.numIndices(), 1, 0, 0, 0);
            }
        }
    }

    void CVulkanSceneContext::onResize() {
        vk::buffutil::copyMatrixToBuffer(*this->_bufferProjMatrix, this->projection().mat4Projection(), 0);
    }
}
