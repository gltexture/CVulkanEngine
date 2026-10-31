//
// Created by ded on 30.10.2026.
//

#include "view_camera.h"

#include "glm/ext/matrix_transform.hpp"

namespace cvulkan::client::render::camera {
    void CVulkanCamera::addRotation(const float x, const float y) {
        this->_rotation += glm::vec2 {x, y};
        this->recalculateMatrix();
    }

    void CVulkanCamera::moveBackwards(const float inc) {
        this->_position -= this->_direction * inc;
        this->recalculateMatrix();
    }

    void CVulkanCamera::moveDown(const float inc) {
        this->_position -= this->_up * inc;
        this->recalculateMatrix();
    }

    void CVulkanCamera::moveForward(const float inc) {
        this->_position += this->_direction * inc;
        this->recalculateMatrix();
    }

    void CVulkanCamera::moveLeft(const float inc) {
        this->_position -= this->_right * inc;
        this->recalculateMatrix();
    }

    void CVulkanCamera::moveRight(const float inc) {
        this->_position += this->_right * inc;
        this->recalculateMatrix();
    }

    void CVulkanCamera::moveUp(const float inc) {
        this->_position += this->_up * inc;
        this->recalculateMatrix();
    }

    void CVulkanCamera::setPosition(const float x, const float y, const float z) {
        this->_position = glm::vec3 {x, y, z};
        this->recalculateMatrix();
    }

    void CVulkanCamera::setRotation(const float x, const float y) {
        this->_rotation = glm::vec2 {x, y};
        this->recalculateMatrix();
    }

    void CVulkanCamera::recalculateMatrix() {
        this->_viewMatrix = glm::mat4 {1.0f};
        this->_viewMatrix = glm::rotate(this->_viewMatrix, this->_rotation.x, glm::vec3 {1.0f, 0.0f, 0.0f});
        this->_viewMatrix = glm::rotate(this->_viewMatrix, this->_rotation.y, glm::vec3 {0.0f, 1.0f, 0.0f});
        this->_viewMatrix = glm::translate(this->_viewMatrix, -this->_position);

        this->_direction = glm::normalize(glm::vec3 {
            -this->_viewMatrix[0][2],
            -this->_viewMatrix[1][2],
            -this->_viewMatrix[2][2]
        });

        this->_right = glm::normalize(glm::vec3 {
            this->_viewMatrix[0][0],
            this->_viewMatrix[1][0],
            this->_viewMatrix[2][0]
        });

        this->_up = glm::normalize(glm::vec3 {
            this->_viewMatrix[0][1],
            this->_viewMatrix[1][1],
            this->_viewMatrix[2][1]
        });
    }
}
