//
// Created by gltex on 08.11.2026.
//

#include "vulkan_render_scene__imgui.h"

#include <imgui.h>

#include "vulkan_render_cache.h"
#include "vulkan_render_scene__main.h"
#include "client/control/client_input.h"
#include "client/render/vulkan_commands.h"
#include "client/render/assets/asset_shaders.h"
#include "client/render/assets/asset_textures.h"
#include "test_logic/test_logic.h"
#include "utility/fun_utils.h"
#include "utility/io_utils.h"

namespace cvulkan::client::render::scene::imgui {
    CVulkanImGuiRenderer::CVulkanImGuiRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const vk::CVulkanAttachment2D& dstAttachment, cache::CVulkanCacheCollection& caches)
        : _context{context}, _descriptorAllocator{descriptorAllocator}, _caches{caches} {
        this->createRenderingData(dstAttachment);

        this->_fontTextureSampler = std::make_unique<vk::CVulkanSampler>(this->_context, vk::CVulkanSamplerInfo {
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_INT_OPAQUE_BLACK, 1, false
        });

        this->initImGui();

        this->_descriptorSetLayout_texture = std::make_unique<vk::CVulkanDescriptorSetLayout>(this->_context, vk::CVulkanLayoutInfo{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT});
        const vk::CVulkanDescriptorSet& descriptorSet = this->_descriptorAllocator.allocateDescriptorSets(DESC_ID_FONT_TEXTURE, 1, *this->_descriptorSetLayout_texture).at(0);
        descriptorSet.setImage(this->_fontTexture->imageView(), *this->_fontTextureSampler, this->_descriptorSetLayout_texture->layoutInfo()._binding);
        this->_texturePtr = std::make_unique<ImGuiTexturePtr>(descriptorSet.vkDescriptorSet());
        ImGui::GetIO().Fonts->SetTexID(reinterpret_cast<ImTextureID>(_texturePtr.get()));
        this->_guiTextures.emplace(reinterpret_cast<ImTextureID>(_texturePtr.get()), descriptorSet.vkDescriptorSet());

        const shader::CVulkanShaderModule shaderV = asset::shaderLoad::compileAndLoadShader(this->_context, io_utils::CVulkanResourcePath{"imgui/v.vert"}, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
        const shader::CVulkanShaderModule shaderF = asset::shaderLoad::compileAndLoadShader(this->_context, io_utils::CVulkanResourcePath{"imgui/f.frag"}, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);

        constexpr vk::CVulkanPushConstantRange pushConstantRange1 {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec2)};
        const auto bufferStruct = structs::CVulkanImGuiVertexBufferStruct {context};

        const vk::CVulkanPipelineBuildInfo buildInfo{
            fun_utils::packCRefs(*this->_descriptorSetLayout_texture),
            fun_utils::packCRefs(shaderV, shaderF),
            fun_utils::packCRefs(pushConstantRange1),
            bufferStruct.vkPipelineVertexInputStateCreateInfo(), COLOR_FORMAT, true};

        this->_imguiPipeline = std::make_unique<vk::CVulkanPipeline>(this->_context, buildInfo);

        {
            std::vector<CVulkanImGuiTextureData> imGuiTextureData = test_logic::iniImGui();
            this->loadTextures(imGuiTextureData);
        }
    }

    CVulkanImGuiRenderer::~CVulkanImGuiRenderer() {

    }

    void CVulkanImGuiRenderer::renderImGuiWindow() {
        ImGui::NewFrame();
        {
            test_logic::renderImGuiWindow();
        }
        ImGui::EndFrame();
        ImGui::Render();
    }

    void CVulkanImGuiRenderer::renderImGui(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanAttachment2D& dstAttachment, const uint32_t imageIndex) {
        this->renderImGuiWindow();
        this->updateBuffers(imageIndex);
        if (this->_vtxBuffers.at(imageIndex) == nullptr) {
            return;
        }

        const ImDrawData* imDrawData = ImGui::GetDrawData();

        if (imDrawData == nullptr || imDrawData->CmdListsCount == 0 || imDrawData->TotalVtxCount == 0 || imDrawData->TotalIdxCount == 0) {
            return;
        }

        vkCmdBeginRendering(commandBuffer.vkCommandBuffer(), &this->_renderingInfo);
        vkCmdBindPipeline(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_imguiPipeline->vkPipeline());

        main::CVulkanSceneRenderer::setViewport(commandBuffer, dstAttachment.image().extent2D());

        constexpr VkDeviceSize offset = 0;
        const VkBuffer vertexBuffer = this->_vtxBuffers.at(imageIndex)->vkBuffer();
        const VkBuffer indexBuffer = this->_idxBuffers.at(imageIndex)->vkBuffer();

        static_assert(sizeof(ImDrawIdx) == 2 || sizeof(ImDrawIdx) == 4);

        constexpr VkIndexType indexType = sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
        vkCmdBindVertexBuffers(commandBuffer.vkCommandBuffer(), 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer.vkCommandBuffer(), indexBuffer, 0, indexType);

        const ImGuiIO& io = ImGui::GetIO();
        const glm::vec2 scale {2.0f / io.DisplaySize.x, -2.0f / io.DisplaySize.y};
        vkCmdPushConstants(commandBuffer.vkCommandBuffer(), this->_imguiPipeline->vkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec2), &scale);

        int32_t offsetVtx = 0;
        uint32_t offsetIdx = 0;

        for (int i = 0; i < imDrawData->CmdListsCount; i++) {
            const ImDrawList* cmdList = imDrawData->CmdLists[i];
            for (int j = 0; j < cmdList->CmdBuffer.Size; j++) {
                const ImDrawCmd& drawCmd = cmdList->CmdBuffer[j];

                ImTextureID textureId = drawCmd.GetTexID();
                const VkDescriptorSet& descriptorSet = this->_guiTextures.at(textureId);
                vkCmdBindDescriptorSets(commandBuffer.vkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->_imguiPipeline->vkPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

                const ImVec2 clipMin {
                    (drawCmd.ClipRect.x - imDrawData->DisplayPos.x) * imDrawData->FramebufferScale.x,
                    (drawCmd.ClipRect.y - imDrawData->DisplayPos.y) * imDrawData->FramebufferScale.y
                };
                const ImVec2 clipMax {
                    (drawCmd.ClipRect.z - imDrawData->DisplayPos.x) * imDrawData->FramebufferScale.x,
                    (drawCmd.ClipRect.w - imDrawData->DisplayPos.y) * imDrawData->FramebufferScale.y
                };
                const VkRect2D scissor {
                    .offset = {
                        .x = static_cast<int32_t>(clipMin.x),
                        .y = static_cast<int32_t>(clipMin.y)
                    },
                    .extent = {
                        .width = static_cast<uint32_t>(clipMax.x - clipMin.x),
                        .height = static_cast<uint32_t>(clipMax.y - clipMin.y)
                    }
                };

                vkCmdSetScissor(commandBuffer.vkCommandBuffer(), 0, 1, &scissor);
                vkCmdDrawIndexed(commandBuffer.vkCommandBuffer(), drawCmd.ElemCount, 1, offsetIdx + drawCmd.IdxOffset, offsetVtx + static_cast<int32_t>(drawCmd.VtxOffset), 0);
            }

            offsetIdx += static_cast<uint32_t>(cmdList->IdxBuffer.Size);
            offsetVtx += cmdList->VtxBuffer.Size;
        }

        vkCmdEndRendering(commandBuffer.vkCommandBuffer());
    }

    void CVulkanImGuiRenderer::onResize(const vk::CVulkanAttachment2D& dstAttachment) {
        ImGuiIO& io = ImGui::GetIO();
        const auto& [width, height] = this->_context.swapChain().extent();
        io.DisplaySize = ImVec2(
            static_cast<float>(width),
            static_cast<float>(height)
        );
        this->createRenderingData(dstAttachment);
    }

    void CVulkanImGuiRenderer::loadTextures(const std::span<CVulkanImGuiTextureData> textures) {
        for (const auto& [_id, _texturePath] : textures) {
            const std::filesystem::path textureFullPath = io_utils::_RS_RESOLVER / _texturePath;
            const asset::CVulkanTexture2D& texture2D = this->_caches.texturesCache().loadTexture(textureFullPath.string(), textureFullPath, VK_FORMAT_R8G8B8A8_SRGB, false);
            const vk::CVulkanDescriptorSet& descriptorSet = this->_descriptorAllocator.allocateDescriptorSet(textureFullPath.string(), *this->_descriptorSetLayout_texture);
            descriptorSet.setImage(texture2D.imageView(), *this->_fontTextureSampler, this->_descriptorSetLayout_texture->layoutInfo()._binding);
            this->_guiTextures.emplace(_id, descriptorSet.vkDescriptorSet());
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
        imageSrc._genMipMaps = false;

        this->_fontTexture = std::make_unique<asset::CVulkanTexture2D>(this->_context, "IMGUI_FONT_TEXTURE", imageSrc, VK_FORMAT_R8G8B8A8_SRGB);

        const uint32_t transferQueueFamilyIndex = vk::util::findQueueFamilyIndex(this->_context, vk::CVulkanQueueFamilyBitMasks::TRANSFER);
        const vk::CVulkanCommandPool tempPool{this->_context, transferQueueFamilyIndex, true};
        const vk::CVulkanQueue tempQueue{this->_context, transferQueueFamilyIndex, 0};
        const vk::CVulkanCommandBuffer tempCommandBuffer {this->_context, tempPool, true, true};

        tempCommandBuffer.beginRecording();
        this->_fontTexture->recordTextureTransition(tempCommandBuffer);
        tempCommandBuffer.endRecording();
        tempCommandBuffer.submitAndWait(tempQueue);

        input::ClientInput->keyboard().registerUniversalKeyCallback([](const int key, const int action) {
            ImGuiIO& _io = ImGui::GetIO();
            if (!_io.WantCaptureKeyboard) {
                return;
            }
            const ImGuiKey imKey = imgui_utils::getImKey(key);
            if (imKey == ImGuiKey_None) {
                return;
            }
            if (action == GLFW_PRESS) {
                _io.AddKeyEvent(imKey, true);
            } else if (action == GLFW_RELEASE) {
                _io.AddKeyEvent(imKey, false);
            }
        });

        input::ClientInput->keyboard().registerCharCallback([](const unsigned int codepoint) {
            ImGui::GetIO().AddInputCharacter(codepoint);
        });

        input::ClientInput->mouse().registerUniversalButtonCallback([](const int button, const int action) {
            ImGuiIO& _io = ImGui::GetIO();
            if (!_io.WantCaptureMouse) {
                return;
            }
            if (action == GLFW_PRESS) {
                _io.AddMouseButtonEvent(button, true);
            } else if (action == GLFW_RELEASE) {
                _io.AddMouseButtonEvent(button, false);
            }
        });

        input::ClientInput->mouse().registerScrollCallback([](const double xOffset, const double yOffset) {
            ImGuiIO& _io = ImGui::GetIO();
            if (!_io.WantCaptureMouse) {
                return;
            }
            _io.AddMouseWheelEvent(static_cast<float>(xOffset), static_cast<float>(yOffset));
        });
    }

    void CVulkanImGuiRenderer::updateBuffers(const uint32_t idx) {
        const ImDrawData* imDrawData = ImGui::GetDrawData();
        if (imDrawData == nullptr) {
            return;
        }

        const VkDeviceSize vertexBufferSize = static_cast<VkDeviceSize>(imDrawData->TotalVtxCount) * sizeof(ImDrawVert);
        const VkDeviceSize indexBufferSize = static_cast<VkDeviceSize>(imDrawData->TotalIdxCount) * sizeof(ImDrawIdx);

        if (vertexBufferSize == 0 || indexBufferSize == 0) {
            return;
        }

        if (const vk::CVulkanBuffer* vtxBuffer = this->_vtxBuffers.at(idx).get(); vtxBuffer == nullptr || vertexBufferSize > vtxBuffer->vkRequestedSize()) {
            if (vtxBuffer != nullptr) {
                this->_vtxBuffers.at(idx).reset();
            }
            this->_vtxBuffers.at(idx) = std::make_unique<vk::CVulkanBuffer>(this->_context, vertexBufferSize,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        }

        if (const vk::CVulkanBuffer* idxBuffer = this->_idxBuffers.at(idx).get(); idxBuffer == nullptr || indexBufferSize > idxBuffer->vkRequestedSize()) {
            if (idxBuffer != nullptr) {
                this->_idxBuffers.at(idx).reset();
            }
            this->_idxBuffers.at(idx) = std::make_unique<vk::CVulkanBuffer>(this->_context, indexBufferSize,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        }

        const auto& vtxBuffer = this->_vtxBuffers.at(idx);
        const auto& idxBuffer = this->_idxBuffers.at(idx);

        const vk::CVulkanBufferMapped vtxMapped {*vtxBuffer};
        const vk::CVulkanBufferMapped idxMapped {*idxBuffer};

        size_t vtxOffset = 0;
        size_t idxOffset = 0;
        for (int i = 0; i < imDrawData->CmdListsCount; i++) {
            const ImVector<ImDrawVert>& imDrawVts = imDrawData->CmdLists[i]->VtxBuffer;
            const ImVector<ImDrawIdx>& imDrawIds = imDrawData->CmdLists[i]->IdxBuffer;

            const size_t vtxSize = static_cast<size_t>(imDrawVts.Size) * sizeof(ImDrawVert);
            const size_t idxSize = static_cast<size_t>(imDrawIds.Size) * sizeof(ImDrawIdx);

            std::memcpy(static_cast<std::byte*>(vtxMapped.data()) + vtxOffset, imDrawVts.Data, vtxSize);
            std::memcpy(static_cast<std::byte*>(idxMapped.data()) + idxOffset, imDrawIds.Data, idxSize);

            vtxOffset += vtxSize;
            idxOffset += idxSize;
        }

        vtxBuffer->flush();
        idxBuffer->flush();
    }
}
