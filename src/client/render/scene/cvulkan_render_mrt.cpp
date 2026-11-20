//
// Created by ded on 16.11.2026.
//

#include "cvulkan_render_mrt.h"

namespace cvulkan::client::render::scene::mrt {
    CVulkanMultipleRenderTargets::CVulkanMultipleRenderTargets(const vk::CVulkanContext& context) : _context{context} {
        this->createPtrs();
    }

    void CVulkanMultipleRenderTargets::reset() {
        this->_colorAlbedoAttachment.reset();
        this->_depthAttachment.reset();
        this->createPtrs();
    }

    void CVulkanMultipleRenderTargets::createPtrs() {
        this->_colorAlbedoAttachment = vk::CVulkanAttachment2D::createSimpleAttachment(this->_context, this->extent3D(), ALBEDO_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        this->_depthAttachment = vk::CVulkanAttachment2D::createSimpleAttachment(this->_context, this->extent3D(), DEPTH_FORMAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}
