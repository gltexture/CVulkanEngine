//
// Created by ded on 19.11.2026.
//

#include "vulkan_imgui_window_renderer.h"

#include "imgui.h"
#include "client/render/vulkan_images.h"
#include "client/render/scene/cvulkan_render_mrt.h"

namespace cvulkan::client::render::debug::imgui {
    CVulkanImguiDebugWindowRenderer::CVulkanImguiDebugWindowRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const scene::mrt::CVulkanMultipleRenderTargets& multipleRenderTargets)
        : _context(context), _descriptorAllocator(descriptorAllocator), _multipleRenderTargets(multipleRenderTargets) {
        this->_textureLayout = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, vk::CVulkanLayoutInfo {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT});

        const std::vector<vk::CVulkanDescriptorSet>& descriptorSets = descriptorAllocator.allocateDescriptorSets(DESC_ID_ALBEDO_TEXTURE_IMGUI, 2, *this->_textureLayout);
        const vk::CVulkanDescriptorSet& _albedoTextureDescriptorSet = descriptorSets.at(0);
        const vk::CVulkanDescriptorSet& _depthTextureDescriptorSet = descriptorSets.at(1);

        constexpr vk::CVulkanSamplerInfo samplerInfo{
            ._addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            ._borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            ._mipLevels = 1,
            ._anisotropy = true
        };
        this->_textureSampler = std::make_unique<vk::CVulkanSampler>(this->_context, samplerInfo);

        {
            _albedoTextureDescriptorSet.setImage(multipleRenderTargets.colorAlbedoAttachment().view(), *this->_textureSampler);
            _depthTextureDescriptorSet.setImage(multipleRenderTargets.depthAttachment().view(), *this->_textureSampler);
        }
    }

    CVulkanImguiDebugWindowRenderer::~CVulkanImguiDebugWindowRenderer() {
    }

    void CVulkanImguiDebugWindowRenderer::renderDebugWindow() {
        const vk::CVulkanDescriptorSet& _albedoTextureDescriptorSet = this->_descriptorAllocator.getDescriptorSet(DESC_ID_ALBEDO_TEXTURE_IMGUI, 0);
        const vk::CVulkanDescriptorSet& _depthTextureDescriptorSet = this->_descriptorAllocator.getDescriptorSet(DESC_ID_ALBEDO_TEXTURE_IMGUI, 1);

        ImGui::Begin("MRT Debug");
        constexpr ImVec2 imageSize{400.0f, 300.0f};
        ImGui::Text("Albedo");
        ImGui::Image(_albedoTextureDescriptorSet.vkDescriptorSet(), imageSize);
        ImGui::Text("Depth");
        ImGui::Image(_depthTextureDescriptorSet.vkDescriptorSet(), imageSize);
        ImGui::End();
    }

    void CVulkanImguiDebugWindowRenderer::onResize() {
        {
            const vk::CVulkanDescriptorSet& _albedoTextureDescriptorSet = this->_descriptorAllocator.getDescriptorSet(DESC_ID_ALBEDO_TEXTURE_IMGUI, 0);
            const vk::CVulkanDescriptorSet& _depthTextureDescriptorSet = this->_descriptorAllocator.getDescriptorSet(DESC_ID_ALBEDO_TEXTURE_IMGUI, 1);
            _albedoTextureDescriptorSet.setImage(this->_multipleRenderTargets.colorAlbedoAttachment().view(), *this->_textureSampler);
            _depthTextureDescriptorSet.setImage(this->_multipleRenderTargets.depthAttachment().view(), *this->_textureSampler);
        }
    }

    void createDebugImguiRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const scene::mrt::CVulkanMultipleRenderTargets& multipleRenderTargets) {
        ImguiDebugWindowRenderer = std::make_unique<CVulkanImguiDebugWindowRenderer>(context, descriptorAllocator, multipleRenderTargets);
    }

    void destroyDebugImguiRenderer() {
        ImguiDebugWindowRenderer.reset();
    }
}
