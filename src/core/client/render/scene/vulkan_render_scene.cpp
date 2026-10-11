//
// Created by ded on 11.09.2026.
//

#include "vulkan_render_scene.h"

#include "vulkan_render_cache.h"
#include "vulkan_scene_objects.h"
#include "client/render/vulkan_commands.h"
#include "client/render/vulkan_images.h"
#include "client/render/vulkan_render_loop.h"
#include "client/render/assets/asset_shaders.h"
#include "shaderc/shaderc.h"

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

    void CVulkanSceneRenderer::renderScene(const vk::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const {
        utility::imageBarrier(
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
        utility::imageBarrier(
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

            for (const auto& [_, entity] : this->_entitiesCollection.entities()) {
                const auto& model = this->caches().modelCache().getModel(entity.modelId());
                const CVulkanPushConstants pushConstants {
                    .projection = this->_projection.mat4(),
                    .model = entity.transform().modelMatrix()
                };
                vkCmdPushConstants(commandBuffer.vkCommandBuffer(), this->_defaultRenderPipeline->vkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(CVulkanPushConstants), &pushConstants);
                for (const auto& mesh : model.meshes()) {
                    const VkBuffer vertexBuffer = mesh.verticesBuffer().vkBuffer();
                    constexpr VkDeviceSize offset = 0;
                    vkCmdBindVertexBuffers(commandBuffer.vkCommandBuffer(), 0, 1, &vertexBuffer, &offset);
                    vkCmdBindIndexBuffer(commandBuffer.vkCommandBuffer(), mesh.indicesBuffer().vkBuffer(), 0, VK_INDEX_TYPE_UINT32);
                    vkCmdDrawIndexed(commandBuffer.vkCommandBuffer(), mesh.numIndices(), 1, 0, 0, 0);
                }
            }

            vkCmdEndRendering(commandBuffer.vkCommandBuffer());
        }

        utility::imageBarrier(
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

    void CVulkanSceneRenderer::loadResources() {
        const structs::CVulkanRawMeshData rawMeshData  {
            "Cube-mesh",
            {
            -0.5f, 0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
        },
        {
            0.0f, 0.0f,
            0.5f, 0.0f,
            1.0f, 0.0f,
            1.0f, 0.5f,
            1.0f, 1.0f,
            0.5f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.5f,
        },
            {
            // Front face
            0, 1, 3, 3, 1, 2,
            // Top Face
            4, 0, 3, 5, 4, 3,
            // Right face
            3, 2, 7, 5, 3, 7,
            // Left face
            6, 1, 0, 6, 0, 4,
            // Bottom face
            2, 1, 6, 2, 6, 7,
            // Back face
            7, 6, 4, 7, 4, 5,
            }
        };
        this->_caches.modelCache().loadModels({
            {"Cube", {rawMeshData}}
        }, this->_renderLoop.commandPools()[0], this->_renderLoop.graphicsQueue());

        {
            asset::shaderLoad::compileShaderIfOutOfDate("test.v", shaderc_glsl_vertex_shader);
            asset::shaderLoad::compileShaderIfOutOfDate("test.f", shaderc_glsl_fragment_shader);

            std::vector<shader::CVulkanShaderModule> shaderModules  {};
            shaderModules.reserve(2);
            shaderModules.emplace_back(this->_context, "test.v.spv", VK_SHADER_STAGE_VERTEX_BIT);
            shaderModules.emplace_back(this->_context, "test.f.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

            std::vector<vk::CVulkanPushConstantRange> pushConstantRanges {};
            pushConstantRanges.reserve(1);
            pushConstantRanges.emplace_back(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(CVulkanPushConstants));

            const structs::CVulkanVertexStruct vertexStruct{this->_context};
            const vk::CVulkanPipelineBuildInfo buildInfo{shaderModules, pushConstantRanges, vertexStruct.vkPipelineVertexInputStateCreateInfo(), this->_context.surface().vkFormat(), VK_FORMAT_D16_UNORM};

            this->_defaultRenderPipeline = std::make_unique<vk::CVulkanPipeline>(this->_context, buildInfo);

            {
                entity::CVulkanSceneEntity entity = {"CubeEnt", "Cube"};
                entity.transform()._position = {0.0f, 0.0f, -2.0f};
                entity.transform()._rotation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                this->_entitiesCollection.addEntity(std::move(entity));
            }
        }
    }

    CVulkanSceneRenderer::CVulkanSceneRenderer(const vk::CVulkanContext& context, const loop::CVulkanRenderLoop& renderLoop)
        : _context(context), _caches(context), _renderLoop(renderLoop), _projection({glm::radians(60.0f), 0.1f, 1024.0f, {context.swapChain().extent()}}) {
        this->loadResources();
        this->createRenderData();
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
