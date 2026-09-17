//
// Created by ded on 11.09.2026.
//

#include "vulkan_render_scene.h"

#include "vulkan_render_cache.h"
#include "../vulkan_commands.h"
#include "../vulkan_render_loop.h"
#include "../vulkan_synchronization.h"

namespace cvulkan::client::render::scene {
    void CVulkanSceneRenderer::createScene() {
        cache::createCaches(this->_context);
        loadResources();

        {
            this->_vkColorAttachments.resize(this->_context.surface().swapChain().numImages());
            for (uint32_t i = 0; i < this->_context.surface().swapChain().numImages(); ++i) {
                this->_vkColorAttachments[i] = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = this->_context.surface().swapChain().imageViews()[i].vkImageView(),
                    .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = VK_CLEAR_VALUE
                };
            }

            this->_vkRendering.resize(this->_context.surface().swapChain().numImages());
            for (uint32_t i = 0; i < this->_context.surface().swapChain().numImages(); ++i) {
                this->_vkRendering[i] = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                    .renderArea = {
                        {}, this->_context.surface().swapChain().extent(),
                    },
                    .layerCount = 1,
                    .colorAttachmentCount = 1,
                    .pColorAttachments = &this->_vkColorAttachments[i],
                };
            }
        }
    }

    /*
    *             VkUtils.imageBarrier(stack, cmdHandle, swapChainImage,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT);
     */

    void CVulkanSceneRenderer::renderScene(const core::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const {
        utility::imageBarrier(
            commandBuffer.vkCommandBuffer(),
            this->_context.surface().swapChain().imageViews()[imageIndex].vkImage(),
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_NONE,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT
        );

        {
            vkCmdBeginRendering(commandBuffer.vkCommandBuffer(), &this->_vkRendering[imageIndex]);

            vkCmdBindPipeline(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_defaultRenderPipeline.vkPipeline());
            const VkExtent2D extent = this->_context.surface().swapChain().extent();
            const VkViewport viewport {
                .x = 0.0F,
                .y = static_cast<float>(extent.height),
                .width = static_cast<float>(extent.width),
                .height = -static_cast<float>(extent.height),
                .minDepth = 0.0F,
                .maxDepth = 1.0F
            };
            vkCmdSetViewport(commandBuffer.vkCommandBuffer(), 0, 1, &viewport);

            const VkRect2D scissor {
                .offset = {
                    .x = 0,
                    .y = 0
                },
                .extent = extent
            };
            vkCmdSetScissor(commandBuffer.vkCommandBuffer(), 0, 1, &scissor);

            for (const auto& t : cache::modelsCache->models()) {
                for (const auto& mesh : t.second.meshes()) {
                    const VkBuffer vertexBuffer = mesh.verticesBuffer().vkBuffer();
                    const VkDeviceSize offset = 0;
                    vkCmdBindVertexBuffers(commandBuffer.vkCommandBuffer(),0,1, &vertexBuffer, &offset);
                    vkCmdBindIndexBuffer(commandBuffer.vkCommandBuffer(),mesh.indicesBuffer().vkBuffer(), 0, VK_INDEX_TYPE_UINT32);
                    vkCmdDrawIndexed( commandBuffer.vkCommandBuffer(), mesh.numIndices(), 1, 0, 0, 0 );
                }
            }

            vkCmdEndRendering(commandBuffer.vkCommandBuffer());
        }

        utility::imageBarrier(
            commandBuffer.vkCommandBuffer(),
            this->_context.surface().swapChain().imageViews()[imageIndex].vkImage(),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_NONE,
            VK_IMAGE_ASPECT_COLOR_BIT
        );
    }

    void CVulkanSceneRenderer::destroyScene() {
        this->_defaultRenderPipeline.destroyPipeline();
        cache::destroyCaches();
    }

    void CVulkanSceneRenderer::loadResources() {
        const structs::CVulkanRawMeshData rawMeshData = {
            "triangle-mesh",
            {
                -0.5f, -0.5f, 0.0f,
                0.0f, 0.5f, 0.0f,
                0.5f, -0.5f, 0.0f
            },
{0, 1, 2}
        };
        cache::modelsCache->loadModels({
            {"triangle", {rawMeshData}}
        }, this->_renderLoop.commandPools()[0], this->_renderLoop.graphicsQueue());

        {
        shader::compileShaderIfOutOfDate("test.v", shaderc_glsl_vertex_shader);
        shader::compileShaderIfOutOfDate("test.f", shaderc_glsl_fragment_shader);

            std::vector<shader::CVulkanShaderModule> shaderModules = {};

            shaderModules.reserve(2);

            shaderModules.emplace_back(this->_context, VK_SHADER_STAGE_VERTEX_BIT);
            shaderModules.back().createShaderModule("test.v.spv");

            shaderModules.emplace_back(this->_context, VK_SHADER_STAGE_FRAGMENT_BIT);
            shaderModules.back().createShaderModule("test.f.spv");

            structs::CVulkanVertexStruct vertexStruct {this->_context};
            vertexStruct.createVertexStruct();

            const core::CVulkanPipelineBuildInfo buildInfo {shaderModules, vertexStruct.vkPipelineVertexInputStateCreateInfo(), this->_context.surface().vkFormat()};
            this->_defaultRenderPipeline.createPipeline(buildInfo);

            for (auto& shaderModule : shaderModules) {
                shaderModule.destroyShaderModule();
            }

            vertexStruct.destroyVertexStruct();
        }
    }
}
