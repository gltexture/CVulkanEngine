//
// Created by ded on 11.09.2026.
//

#include "vulkan_render_scene.h"

#include "vulkan_render_cache.h"
#include "client/render/vulkan_commands.h"
#include "client/render/vulkan_render_loop.h"
#include "shaderc/shaderc.h"

namespace cvulkan::client::render::scene {
    static constexpr VkClearValue VK_CLEAR_VALUE = {0.5f, 0.7f, 0.9f, 1.0f};

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
            this->_context.swapChain().imageViews()[imageIndex].vkImage(),
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

            vkCmdBindPipeline(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_defaultRenderPipeline->vkPipeline());
            const VkExtent2D extent = this->_context.swapChain().extent();
            const VkViewport viewport{
                .x = 0.0F,
                .y = static_cast<float>(extent.height),
                .width = static_cast<float>(extent.width),
                .height = -static_cast<float>(extent.height),
                .minDepth = 0.0F,
                .maxDepth = 1.0F
            };
            vkCmdSetViewport(commandBuffer.vkCommandBuffer(), 0, 1, &viewport);

            const VkRect2D scissor{
                .offset = {
                    .x = 0,
                    .y = 0
                },
                .extent = extent
            };
            vkCmdSetScissor(commandBuffer.vkCommandBuffer(), 0, 1, &scissor);

            for (const auto& t : this->caches().modelCache().models()) {
                for (const auto& mesh: t.second.meshes()) {
                    const VkBuffer vertexBuffer = mesh.verticesBuffer().vkBuffer();
                    const VkDeviceSize offset = 0;
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
        this->_caches.modelCache().loadModels({
            {"triangle", {rawMeshData}}
        }, this->_renderLoop.commandPools()[0], this->_renderLoop.graphicsQueue());

        {
            shader::compileShaderIfOutOfDate("test.v", shaderc_glsl_vertex_shader);
            shader::compileShaderIfOutOfDate("test.f", shaderc_glsl_fragment_shader);

            std::vector<shader::CVulkanShaderModule> shaderModules = {};

            shaderModules.reserve(2);

            shaderModules.emplace_back(this->_context, "test.v.spv", VK_SHADER_STAGE_VERTEX_BIT);
            shaderModules.emplace_back(this->_context, "test.f.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

            const structs::CVulkanVertexStruct vertexStruct{this->_context};
            const core::CVulkanPipelineBuildInfo buildInfo{shaderModules, vertexStruct.vkPipelineVertexInputStateCreateInfo(), this->_context.surface().vkFormat()};

            this->_defaultRenderPipeline = std::make_unique<core::CVulkanPipeline>(this->_context, buildInfo);
        }
    }

    CVulkanSceneRenderer::CVulkanSceneRenderer(const core::CVulkanContext& context, const loop::CVulkanRenderLoop& renderLoop)
        : _context(context), _caches(context), _renderLoop(renderLoop) {
        loadResources();

        {
            this->_vkColorAttachments.resize(this->_context.swapChain().numImages());
            for (uint32_t i = 0; i < this->_context.swapChain().numImages(); ++i) {
                this->_vkColorAttachments[i] = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = this->_context.swapChain().imageViews()[i].vkImageView(),
                    .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = VK_CLEAR_VALUE
                };
            }

            this->_vkRendering.resize(this->_context.swapChain().numImages());
            for (uint32_t i = 0; i < this->_context.swapChain().numImages(); ++i) {
                this->_vkRendering[i] = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                    .renderArea = {
                            {}, this->_context.swapChain().extent(),
                        },
                        .layerCount = 1,
                        .colorAttachmentCount = 1,
                        .pColorAttachments = &this->_vkColorAttachments[i],
                    };
            }
        }
    }

    CVulkanSceneRenderer::~CVulkanSceneRenderer() {
    }
}
