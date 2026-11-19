//
// Created by ded on 04.11.2026.
//

#include "vulkan_render_scene__post.h"

#include "vulkan_render_scene__deferred.h"
#include "vulkan_render_structs.h"
#include "client/render/vulkan_commands.h"
#include "client/render/vulkan_config.hpp"
#include "client/render/vulkan_images.h"
#include "client/render/vulkan_render_core.h"
#include "client/render/vulkan_spec_const.h"
#include "client/render/assets/asset_shaders.h"
#include "utility/functionality.h"
#include "utility/io.h"

namespace cvulkan::client::render::scene::post {
    CVulkanPostFXRenderer::CVulkanPostFXRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const vk::CVulkanAttachment2D& srcAttachment)
        : _context(context), _descriptorAllocator(descriptorAllocator) {
        this->createRenderingData();

        this->_textureSampler = std::make_unique<vk::CVulkanSampler>(this->_context, vk::CVulkanSamplerInfo {
            VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_INT_OPAQUE_BLACK, 1, true
        });

        this->_descriptorSetLayout_texture = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, vk::CVulkanLayoutInfo{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT});
        const vk::CVulkanDescriptorSet& descriptorSet = this->_descriptorAllocator.allocateDescriptorSets(DESC_ID_ATT, 1, *this->_descriptorSetLayout_texture).at(0);
        descriptorSet.setImage(srcAttachment.view(), *this->_textureSampler, 0);

        const vk::CVulkanDescriptorSetLayout descriptorSetLayout_data {this->_context, vk::CVulkanLayoutInfo{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT}};
        this->_srcSizeBuffer = std::make_unique<vk::CVulkanBuffer>(vk::buff_util::allocateHostVisibleBuffer(this->_context, this->_descriptorAllocator, sizeof(glm::vec2), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, DESC_ID_SCREEN_SIZE, descriptorSetLayout_data));
        this->setSrcSizeBuffer();
        
        const vk::CVulkanSpecConstants<uint32_t> specConst {renderConfig::FXAA ? 1 : 0};
        const shader::CVulkanShaderModule shaderV = asset::shaderLoad::compileAndLoadShader(this->_context, io::CVulkanResourcePath{"post/v.vert"}, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
        const shader::CVulkanShaderModule shaderF = asset::shaderLoad::compileAndLoadShader(this->_context, io::CVulkanResourcePath{"post/f.frag"}, VK_SHADER_STAGE_FRAGMENT_BIT, &specConst.vkSpecializationInfo());

        const vk::CVulkanPipelineBuildInfo buildInfo {
            fun::packCRefs(*this->_descriptorSetLayout_texture, descriptorSetLayout_data),
            fun::packCRefs(shaderV, shaderF),
            {},
            vk::buff_util::emptyVertexStruct(), {COLOR_FORMAT}, false};

        this->_postPipeline = std::make_unique<vk::CVulkanPipeline>(this->_context, buildInfo);
    }

    CVulkanPostFXRenderer::~CVulkanPostFXRenderer() {
        log::info("Destroying postfx scene_geometry renderer");
    }

    void CVulkanPostFXRenderer::renderPostFX(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanAttachment2D& srcAttachment) const {
        vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), srcAttachment.image().vkImage(),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), this->_colorAttachment->image().vkImage(),
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        vkCmdBeginRendering(commandBuffer.vkCommandBuffer(), &this->_renderingInfo);
        vkCmdBindPipeline(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_postPipeline->vkPipeline());

        deferred::CVulkanSceneRenderer::setViewportAndScissor(commandBuffer, this->_context.swapChain().vkExtent2D());

        std::vector<VkDescriptorSet> descriptorSets{};
        descriptorSets.reserve(2);
        descriptorSets.push_back(this->_descriptorAllocator.getDescriptorSet(DESC_ID_ATT, 0).vkDescriptorSet());
        descriptorSets.push_back(this->_descriptorAllocator.getDescriptorSet(DESC_ID_SCREEN_SIZE, 0).vkDescriptorSet());

        vkCmdBindDescriptorSets(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_postPipeline->vkPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
        vkCmdDraw(commandBuffer.vkCommandBuffer(), 3, 1, 0, 0);
        vkCmdEndRendering(commandBuffer.vkCommandBuffer());
    }

    void CVulkanPostFXRenderer::onResize(const vk::CVulkanAttachment2D& srcAttachment) {
        this->_colorAttachment.reset();
        this->createRenderingData();

        this->_descriptorAllocator.getDescriptorSet(DESC_ID_ATT, 0).setImage(srcAttachment.view(), *this->_textureSampler, 0);
        this->setSrcSizeBuffer();
    }

    void CVulkanPostFXRenderer::setSrcSizeBuffer() const {
        const vk::CVulkanBufferMapped mapped{*this->_srcSizeBuffer};
        const glm::vec2 srcSize = {this->_context.swapChain().vkExtent2D().width, this->_context.swapChain().vkExtent2D().height};
        std::memcpy(mapped.data(), &srcSize, sizeof(glm::vec2));
    }

    void CVulkanPostFXRenderer::createRenderingData() {
        this->_colorAttachment = (vk::CVulkanAttachment2D::createSimpleAttachment(this->_context, this->_context.swapChain().vkExtent3D(), COLOR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));

        this->_colorAttachmentInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = this->_colorAttachment->view().vkImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = CLEAR_COLOR
        };

        this->_renderingInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {{0, 0}, this->_colorAttachment->image().vkExtent2D()},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &this->_colorAttachmentInfo
        };
    }
}
