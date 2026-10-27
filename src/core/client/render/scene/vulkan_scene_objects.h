//
// Created by ded on 24.09.2026.
//

#pragma once
#include <string>


#include "glm/gtc/quaternion.hpp"
#include "utility/sys_utils.h"

namespace cvulkan::client::render::entity {
    struct CVulkanEntityTransform {
        glm::vec3 _position{0.0f, 0.0f, 0.0f};
        glm::quat _rotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 _scale{1.0f};

        [[nodiscard]] glm::mat4 modelMatrix() const {
            return glm::translate(glm::mat4{1.0f}, this->_position) * glm::mat4_cast(this->_rotation) * glm::scale(glm::mat4{1.0f}, this->_scale);
        }
    };

    class CVulkanSceneEntity {
    public:
        CVulkanSceneEntity(std::string_view id, std::string_view modelId);
        ~CVulkanSceneEntity();

        CLASS_NO_COPY(CVulkanSceneEntity);
        CVulkanSceneEntity(CVulkanSceneEntity&& other) = default;

        [[nodiscard]] const std::string& id() const {
            return _id;
        }

        [[nodiscard]] const std::string& modelId() const {
            return _modelId;
        }

        [[nodiscard]] const CVulkanEntityTransform& transform() const {
            return _transform;
        }

        [[nodiscard]] CVulkanEntityTransform& transform() {
            return _transform;
        }

    private:
        std::string _id;
        std::string _modelId;
        CVulkanEntityTransform _transform;
    };
}
