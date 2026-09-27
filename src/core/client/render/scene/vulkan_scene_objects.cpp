//
// Created by ded on 24.09.2026.
//

#include "vulkan_scene_objects.h"

namespace cvulkan::client::render::entity {
    CVulkanSceneEntity::CVulkanSceneEntity(const std::string& id, const std::string& modelId) : _id(id), _modelId(modelId) {
    }

    CVulkanSceneEntity::~CVulkanSceneEntity() {
    }
}
