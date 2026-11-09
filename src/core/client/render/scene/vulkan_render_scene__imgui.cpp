//
// Created by gltex on 08.11.2026.
//

#include "vulkan_render_scene__imgui.h"

#include <imgui.h>

#include "client/render/vulkan_commands.h"
#include "client/render/assets/asset_shaders.h"
#include "client/render/assets/asset_textures.h"
#include "utility/fun_utils.h"
#include "utility/io_utils.h"

namespace cvulkan::client::render::scene::imgui {
    CVulkanImGuiRenderer::CVulkanImGuiRenderer(const vk::CVulkanContext &context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const vk::CVulkanAttachment2D& dstAttachment, const cache::CVulkanCacheCollection& caches)
        : _context{context}, _descriptorAllocator{descriptorAllocator}, _caches{caches} {
        this->createRenderingData(dstAttachment);

        this->_descriptorSetLayout_texture = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, vk::CVulkanLayoutInfo{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT});
        const vk::CVulkanDescriptorSet& descriptorSet = this->_descriptorAllocator.allocateDescriptorSets(DESC_ID_TEXTURE, 1, *this->_descriptorSetLayout_texture).at(0);

        const shader::CVulkanShaderModule shaderV = asset::shaderLoad::compileAndLoadShader(this->_context, io_utils::CVulkanResourcePath{"imgui/v.vert"}, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
        const shader::CVulkanShaderModule shaderF = asset::shaderLoad::compileAndLoadShader(this->_context, io_utils::CVulkanResourcePath{"imgui/f.frag"}, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);

        const vk::CVulkanPipelineBuildInfo buildInfo{
            fun_utils::packCRefs(*this->_descriptorSetLayout_texture),
            fun_utils::packCRefs(shaderV, shaderF),
            {},
            vk::buffutil::emptyVertexStruct(), COLOR_FORMAT, false};

        this->_imguiPipeline = std::make_unique<vk::CVulkanPipeline>(this->_context, buildInfo);
        this->initImGui();

    }

    CVulkanImGuiRenderer::~CVulkanImGuiRenderer() {
        logging::info("Destroying imgui scene renderer");
    }

    void CVulkanImGuiRenderer::renderImGui(const vk::CVulkanCommandBuffer& commandBuffer) const {
    }

    void CVulkanImGuiRenderer::onResize() {
    }

    void CVulkanImGuiRenderer::loadTextures(const std::span<CVulkanImGuiTextureData> textures) {
        for (const auto& texture : textures) {
            this->_descriptorAllocator.allocateDescriptorSets(texture._texturePath, 1, &this->_descriptorSetLayout_texture);
        }
    }

    void CVulkanImGuiRenderer::createRenderingData(const vk::CVulkanAttachment2D& dstAttachment) {
        this->_colorAttachmentInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = dstAttachment.view().vkImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };

        this->_renderingInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {
                .offset = {.x = 0, .y = 0},
                .extent = dstAttachment.image().extent2D()
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &this->_colorAttachmentInfo
        };
    }

    void CVulkanImGuiRenderer::initImGui() {
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = {
            static_cast<float>(this->_context.swapChain().extent().width),
            static_cast<float>(this->_context.swapChain().extent().height)
        };
        io.DisplayFramebufferScale = {1.0f, 1.0f};

        unsigned char* out_pixels = nullptr;
        int out_width = 0;
        int out_height = 0;
        int out_bytes_per_pixel = 0;
        io.Fonts->GetTexDataAsRGBA32(&out_pixels, &out_width, &out_height, &out_bytes_per_pixel);

        asset::CVulkanImageSrc imageSrc {out_pixels, (static_cast<size_t>(out_width) * out_height * out_bytes_per_pixel), {out_width, out_height}, 4};
        imageSrc._dataHandle = false;

        this->_fontTexture = std::make_unique<asset::CVulkanTexture2D>(this->_context, "IMGUI_FONT_TEXTURE", imageSrc, VK_FORMAT_R8G8B8A8_SRGB);

        const uint32_t transferQueueFamilyIndex = vk::util::findQueueFamilyIndex(this->_context, vk::CVulkanQueueFamilyBitMasks::TRANSFER);
        const vk::CVulkanCommandPool tempPool{this->_context, transferQueueFamilyIndex, true};
        const vk::CVulkanQueue tempQueue{this->_context, transferQueueFamilyIndex, 0};
        const vk::CVulkanCommandBuffer tempCommandBuffer {this->_context, tempPool, true, true};

        tempCommandBuffer.beginRecording();
        this->_fontTexture->recordTextureTransition(tempCommandBuffer);
        tempCommandBuffer.endRecording();
        tempCommandBuffer.submitAndWait(tempQueue);
    }
}
