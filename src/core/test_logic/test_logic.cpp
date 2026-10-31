//
// Created by ded on 31.10.2026.
//

#include "test_logic.h"

#include "client/control/client_input.h"
#include "client/render/scene/vulkan_render_loop.h"
#include "client/render/scene/vulkan_render_scene.h"
#include "client/render/scene/vulkan_scene_objects.h"
#include "client/render/scene/camera/view_camera.h"
#include "utility/io_utils.h"


namespace cvulkan::test_logic {
    constexpr static float MOUSE_SENSITIVITY = 0.5f;
    constexpr static float MOVEMENT_SPEED = 5.0f;

    static client::render::scene::context::ModelId SCENE_MODEL_ID {};

    void initSceneResources(client::render::scene::context::CVulkanSceneResourcesInitializer& sceneResourcesInitializer) {
        SCENE_MODEL_ID = sceneResourcesInitializer.loadModelWithMaterials(io_utils::CVulkanResourcePath {"limbo_final/limbo.gltf"});
    }

    void initSceneObjects(client::render::scene::CVulkanSceneEntitiesCollection& entitiesCollection) {
        client::render::entity::CVulkanSceneEntity entity = {"CubeEnt", SCENE_MODEL_ID};
        entity.transform()._position = {0.0f, 0.0f, -6.0f};
        entity.transform()._rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        entitiesCollection.addEntity(std::move(entity));
    }

    void updateInput(const double deltaTime) {
        const auto& input = *client::input::_clientInput;
        const auto& scene = client::render::loop::_sceneRenderer->sceneContext();

        const float move = static_cast<float>(deltaTime) * MOVEMENT_SPEED;
        auto* camera = scene.currentCamera();

        if (camera == nullptr) {
            return;
        }

        if (input.isKeyDown(GLFW_KEY_W)) {
            camera->moveForward(move);
        } else if (input.isKeyDown(GLFW_KEY_S)) {
            camera->moveBackwards(move);
        }

        if (input.isKeyDown(GLFW_KEY_A)) {
            camera->moveLeft(move);
        } else if (input.isKeyDown(GLFW_KEY_D)) {
            camera->moveRight(move);
        }

        if (input.isKeyDown(GLFW_KEY_SPACE)) {
            camera->moveUp(move);
        } else if (input.isKeyDown(GLFW_KEY_LEFT_SHIFT)) {
            camera->moveDown(move);
        }

        constexpr auto rotation = static_cast<float>(std::numbers::pi / 180.0 * MOUSE_SENSITIVITY);

        if (input.isKeyDown(GLFW_KEY_UP)) {
            camera->addRotation(-rotation, 0.0f);
        } else if (input.isKeyDown(GLFW_KEY_DOWN)) {
            camera->addRotation(rotation, 0.0f);
        }

        if (input.isKeyDown(GLFW_KEY_LEFT)) {
            camera->addRotation(0.0f, -rotation);
        } else if (input.isKeyDown(GLFW_KEY_RIGHT)) {
            camera->addRotation(0.0f, rotation);
        }
    }
}
