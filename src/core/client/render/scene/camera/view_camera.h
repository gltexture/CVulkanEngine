//
// Created by ded on 30.10.2026.
//

#pragma once

namespace cvulkan::client::render::vk {
    class CVulkanContext;
}

namespace cvulkan::client::render::camera {
    class CVulkanCamera {
    public:
        explicit CVulkanCamera(const vk::CVulkanContext& context) : _context(context) { }
        ~CVulkanCamera() = default;

        void addRotation(float x, float y);
        void moveBackwards(float inc);
        void moveDown(float inc);
        void moveForward(float inc);
        void moveLeft(float inc);
        void moveRight(float inc);
        void moveUp(float inc);
        void setPosition(float x, float y, float z);
        void setRotation(float x, float y);

        [[nodiscard]] glm::vec3 direction() const {
            return _direction;
        }

        [[nodiscard]] glm::vec3 position() const {
            return _position;
        }

        [[nodiscard]] glm::vec3 right() const {
            return _right;
        }

        [[nodiscard]] glm::vec2 rotation() const {
            return _rotation;
        }

        [[nodiscard]] glm::vec3 up() const {
            return _up;
        }

        [[nodiscard]] glm::mat4 viewMatrix() const {
            return _viewMatrix;
        }

    protected:
        void recalculateMatrix();

    private:
        const vk::CVulkanContext& _context;
        glm::vec3 _direction;
        glm::vec3 _position;
        glm::vec3 _right;
        glm::vec2 _rotation;
        glm::vec3 _up;
        glm::mat4 _viewMatrix {1.0f};
    };
}
