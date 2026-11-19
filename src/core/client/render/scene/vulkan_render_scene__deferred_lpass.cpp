//
// Created by ded on 17.11.2026.
//

#include "vulkan_render_scene__deferred.h"
#include "client/render/assets/asset_shaders.h"
#include "utility/functionality.h"

namespace cvulkan::client::render::scene::deferred::lpass {
    CVulkanSceneRendererLPass::CVulkanSceneRendererLPass(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const mrt::CVulkanMultipleRenderTargets& multipleRenderTargets)
        : _context{context}, _descriptorAllocator{descriptorAllocator}, _multipleRenderTargets{multipleRenderTargets} {
        this->createRenderingData();

        constexpr vk::CVulkanSamplerInfo samplerInfo{
            ._addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            ._borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            ._mipLevels = 1,
            ._anisotropy = true
        };
        this->_textureSampler = std::make_unique<vk::CVulkanSampler>(this->_context, samplerInfo);

        const shader::CVulkanShaderModule shaderV = asset::shaderLoad::compileAndLoadShader(this->_context, io::CVulkanResourcePath{"scene_light/v.vert"}, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
        const shader::CVulkanShaderModule shaderF = asset::shaderLoad::compileAndLoadShader(this->_context, io::CVulkanResourcePath{"scene_light/f.frag"}, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);

        this->_descTextureLayout = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context,
            std::initializer_list {
                vk::CVulkanLayoutInfo{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
                vk::CVulkanLayoutInfo{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, 1, VK_SHADER_STAGE_FRAGMENT_BIT}
            }
        );

        const vk::CVulkanDescriptorSet& descriptorSet = descriptorAllocator.allocateDescriptorSet(DESC_ID_ATT, *this->_descTextureLayout);
        descriptorSet.setImages(fun::packCRefs(multipleRenderTargets.colorAlbedoAttachment().view(), multipleRenderTargets.depthAttachment().view()), *this->_textureSampler, 0);

        const vk::CVulkanPipelineBuildInfo buildInfo{
            fun::packCRefs(*this->_descTextureLayout),
            fun::packCRefs(shaderV, shaderF),
            {},
            vk::buff_util::emptyVertexStruct(), {COLOR_FORMAT}, true};

        this->_renderPipeline = std::make_unique<vk::CVulkanPipeline>(this->_context, buildInfo);
    }

    CVulkanSceneRendererLPass::~CVulkanSceneRendererLPass() {
        log::info("Destroying lpass scene_geometry renderer");
    }

    void CVulkanSceneRendererLPass::createRenderingData() {
        this->_colorAttachment = vk::CVulkanAttachment2D::createSimpleAttachment(this->_context, this->_context.swapChain().vkExtent3D(), COLOR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        this->_vkAttachmentColorInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = this->_colorAttachment->view().vkImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = CLEAR_COLOR
        };
        this->_vkRenderingInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {
                        {}, {this->_context.swapChain().vkExtent2D()}
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &this->_vkAttachmentColorInfo,
        };
    }


    void CVulkanSceneRendererLPass::renderScene(const vk::CVulkanCommandBuffer& commandBuffer, uint32_t currentFrame) const {
        vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), this->_colorAttachment->image().vkImage(),
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        {
            vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), this->_multipleRenderTargets.colorAlbedoAttachment().image().vkImage(),
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

            vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), this->_multipleRenderTargets.depthAttachment().image().vkImage(),
                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT);
        }

        vkCmdBeginRendering(commandBuffer.vkCommandBuffer(), &this->_vkRenderingInfo);
        vkCmdBindPipeline(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_renderPipeline->vkPipeline());

        CVulkanSceneRenderer::setViewportAndScissor(commandBuffer, this->_colorAttachment->image().vkExtent2D());

        const std::vector descriptorSets {
            this->_descriptorAllocator.getDescriptorSet(DESC_ID_ATT, 0).vkDescriptorSet(),
        };
        vkCmdBindDescriptorSets(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_renderPipeline->vkPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
        vkCmdDraw(commandBuffer.vkCommandBuffer(), 3, 1, 0, 0);
        vkCmdEndRendering(commandBuffer.vkCommandBuffer());
    }

    void CVulkanSceneRendererLPass::onResize() {
        this->_colorAttachment.reset();
        this->createRenderingData();

        {
            const vk::CVulkanDescriptorSet& descriptorSet = this->_descriptorAllocator.getDescriptorSet(DESC_ID_ATT, 0);
            descriptorSet.setImages(fun::packCRefs(this->_multipleRenderTargets.colorAlbedoAttachment().view(), this->_multipleRenderTargets.depthAttachment().view()), *this->_textureSampler, 0);
        }
    }
}
