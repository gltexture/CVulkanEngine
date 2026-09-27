//
// Created by ded on 24.09.2026.
//

#include "vulkan_transform.h"
#include <glm/ext/matrix_clip_space.hpp>

namespace cvulkan::client::render::transform {
    CVulkanSceneProjection::CVulkanSceneProjection(const float fov, const float zNear, const float zFar, const VkExtent2D& extent) : _fov(fov), _zNear(zNear), _zFar(zFar) {
        this->resize(extent);
    }

    void CVulkanSceneProjection::resize(const VkExtent2D& extent) {
        this->_projection = glm::perspective(this->_fov, static_cast<float>(extent.width) / static_cast<float>(extent.height), this->_zNear, this->_zFar);
    }
}
