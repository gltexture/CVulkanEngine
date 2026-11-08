//
// Created by ded on 06.11.2026.
//

#include "vulkan_render_scene__swapchain.h"

#include "vulkan_render_scene__main.h"
#include "client/render/vulkan_buffers.h"
#include "client/render/vulkan_commands.h"
#include "client/render/vulkan_images.h"
#include "client/render/vulkan_pipeline.h"
#include "client/render/vulkan_render_core.h"
#include "client/render/assets/asset_shaders.h"
#include "utility/fun_utils.h"
#include "utility/io_utils.h"

namespace cvulkan::client::render::scene::swapchain {
    CVulkanSwapchainRenderer::CVulkanSwapchainRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const vk::CVulkanAttachment2D& srcAttachment)
        : _context(context), _descriptorAllocator(descriptorAllocator) {
        this->createRenderingData();

        this->_textureSampler = std::make_unique<vk::CVulkanSampler>(this->_context, vk::CVulkanSamplerInfo {
            VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_INT_OPAQUE_BLACK, 1, true
        });

        this->_descriptorSetLayout_texture = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, vk::CVulkanLayoutInfo{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT});
        const vk::CVulkanDescriptorSet& descriptorSet = this->_descriptorAllocator.allocateDescriptorSets(DESC_ID_ATT, 1, *this->_descriptorSetLayout_texture).at(0);
        descriptorSet.setImage(srcAttachment.view(), *this->_textureSampler, 0);

        const shader::CVulkanShaderModule shaderV = asset::shaderLoad::compileAndLoadShader(this->_context, io_utils::CVulkanResourcePath{"swapchain/v.vert"}, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
        const shader::CVulkanShaderModule shaderF = asset::shaderLoad::compileAndLoadShader(this->_context, io_utils::CVulkanResourcePath{"swapchain/f.frag"}, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);

        const vk::CVulkanPipelineBuildInfo buildInfo{
            fun_utils::packCRefs(*this->_descriptorSetLayout_texture),
            fun_utils::packCRefs(shaderV, shaderF),
            {},
            vk::buffutil::emptyVertexStruct(), this->_context.surface().vkFormat(), false};

        this->_swapchainPipeline = std::make_unique<vk::CVulkanPipeline>(this->_context, buildInfo);
    }

    CVulkanSwapchainRenderer::~CVulkanSwapchainRenderer() {
        logging::info("Destroying swapchain scene renderer");
    }

    void CVulkanSwapchainRenderer::renderSwapchain(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanAttachment2D& srcAttachment, const uint32_t imageIndex) const {
        vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), this->_context.swapChain().imageViews().at(imageIndex).vkImage(),
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

        vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), srcAttachment.image().vkImage(),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

        vkCmdBeginRendering(commandBuffer.vkCommandBuffer(), &this->_renderingInfo.at(imageIndex));
        vkCmdBindPipeline(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_swapchainPipeline->vkPipeline());

        main::CVulkanSceneRenderer::setDefaultViewportAndScissor(commandBuffer, this->_context.swapChain().extent());

        std::vector<VkDescriptorSet> descriptorSets{};
        descriptorSets.reserve(1);
        descriptorSets.push_back(this->_descriptorAllocator.getDescriptorSet(DESC_ID_ATT, 0).vkDescriptorSet());

        vkCmdBindDescriptorSets(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_swapchainPipeline->vkPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
        vkCmdDraw(commandBuffer.vkCommandBuffer(), 3, 1, 0, 0);
        vkCmdEndRendering(commandBuffer.vkCommandBuffer());

        vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), this->_context.swapChain().imageViews().at(imageIndex).vkImage(),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE,
            VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void CVulkanSwapchainRenderer::onResize(const vk::CVulkanAttachment2D& srcAttachment) {
        this->createRenderingData();
        const vk::CVulkanDescriptorSet& descriptorSet = this->_descriptorAllocator.getDescriptorSet(DESC_ID_ATT, 0);
        descriptorSet.setImage(srcAttachment.view(), *this->_textureSampler, 0);
    }

    void CVulkanSwapchainRenderer::createRenderingData() {
        for (uint32_t i = 0; i < this->_colorAttachmentsInfo.size(); i++) {
            this->_colorAttachmentsInfo[i] = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = this->_context.swapChain().imageViews().at(i).vkImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = CLEAR_COLOR
            };
        }

        for (uint32_t i = 0; i < this->_renderingInfo.size(); i++) {
            this->_renderingInfo[i] = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .renderArea = {
                        {}, {this->_context.swapChain().extent().width, this->_context.swapChain().extent().height}
                },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &this->_colorAttachmentsInfo[i]
            };
        }
    }
}
