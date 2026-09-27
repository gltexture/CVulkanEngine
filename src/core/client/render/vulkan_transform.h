//
// Created by ded on 24.09.2026.
//

#pragma once

namespace cvulkan::client::render::transform {
    class CVulkanSceneProjection {
    public:
        CVulkanSceneProjection(float fov, float zNear, float zFar, const VkExtent2D& extent);
        ~CVulkanSceneProjection() = default;

        void resize(const VkExtent2D& extent);

        [[nodiscard]] const glm::mat4& mat4() const {
            return _projection;
        }

    private:
        glm::mat4 _projection {};
        float _fov;
         float _zNear;
        float _zFar;
    };
}