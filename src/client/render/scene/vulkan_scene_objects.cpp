//
// Created by ded on 24.09.2026.
//

#include "vulkan_scene_objects.h"

#include "core/utility/logging.h"

namespace cvulkan::client::render::entity {
    CVulkanSceneEntity::CVulkanSceneEntity(const std::string_view id, const std::string_view modelId) : _id(id), _modelId(modelId) {
    }

    CVulkanSceneEntity::~CVulkanSceneEntity() {
    }

    CVulkanSceneEntitiesCollection::CVulkanSceneEntitiesCollection() {
        log::info("Destroying scene_geometry entities");
    }

    void CVulkanSceneEntitiesCollection::addEntity(CVulkanSceneEntity&& entity) {
        this->_entities.emplace(entity.id(), std::move(entity));
    }

    bool CVulkanSceneEntitiesCollection::hasEntity(const std::string& id) const {
        return this->_entities.contains(id);
    }
}
