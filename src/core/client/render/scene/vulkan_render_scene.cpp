//
// Created by ded on 11.09.2026.
//

#include "vulkan_render_scene.h"

#include "../vulkan_commands.h"
#include "../vulkan_render_loop.h"
#include "../vulkan_synchronization.h"

namespace cvulkan::client::renderScene {
    void CVulkanSceneRenderer::initScene() {
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
                    {}, this->_context.surface().swapChain().swapChainExtent(),
                },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &this->_vkColorAttachments[i],
            };
        }
    }

    /*
    *             VkUtils.imageBarrier(stack, cmdHandle, swapChainImage,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT);
     */

    void CVulkanSceneRenderer::renderScene(const renderCore::CVulkanCommandBuffer& commandBuffer, const uint32_t& imageIndex) const {
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

    }
}
