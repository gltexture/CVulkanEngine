//
// Created by ded on 31.10.2026.
//

#include "test_logic.h"

#include <numbers>

#include "imgui.h"
#include "client/control/client_input.h"
#include "client/render/scene/vulkan_render_loop.h"
#include "client/render/scene/vulkan_render_scene__deferred.h"
#include "client/render/scene/vulkan_scene_objects.h"
#include "client/render/scene/camera/view_camera.h"
#include "client/render/scene/vulkan_render_scene__imgui.h"
#include "utility/io.h"

namespace cvulkan::test_logic {
    constexpr static float MOUSE_SENSITIVITY = 0.5f;
    constexpr static float MOVEMENT_SPEED = 5.0f;

    static client::render::scene::deferred::context::ModelId SCENE_MODEL_ID{};

    void initSceneResources(client::render::scene::deferred::context::CVulkanSceneResourcesInitializer& sceneResourcesInitializer) {
        SCENE_MODEL_ID = sceneResourcesInitializer.loadModelWithMaterials(io::CVulkanResourcePath{"limbo_final/limbo.gltf"});
    }

    void initSceneObjects(client::render::entity::CVulkanSceneEntitiesCollection& entitiesCollection) {
        client::render::entity::CVulkanSceneEntity entity = {"CubeEnt", SCENE_MODEL_ID};
        entity.transform()._position = {0.0f, 0.0f, -6.0f};
        entity.transform()._rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        entitiesCollection.addEntity(std::move(entity));
    }

    void updateInput(const double deltaTime) {
        const auto& input = *client::input::ClientInput;
        const auto& scene = client::render::loop::RenderLoop->deferredSceneRenderer().geometryPassRendering().sceneContext();

        const float move = static_cast<float>(deltaTime) * MOVEMENT_SPEED;
        auto* camera = scene.currentCamera();

        if (camera == nullptr) {
            return;
        }

        if (input.keyboard().isDown(GLFW_KEY_W)) {
            camera->moveForward(move);
        } else if (input.keyboard().isDown(GLFW_KEY_S)) {
            camera->moveBackwards(move);
        }

        if (input.keyboard().isDown(GLFW_KEY_A)) {
            camera->moveLeft(move);
        } else if (input.keyboard().isDown(GLFW_KEY_D)) {
            camera->moveRight(move);
        }

        if (input.keyboard().isDown(GLFW_KEY_SPACE)) {
            camera->moveUp(move);
        } else if (input.keyboard().isDown(GLFW_KEY_LEFT_SHIFT)) {
            camera->moveDown(move);
        }

        constexpr auto rotation = static_cast<float>(std::numbers::pi / 180.0 * MOUSE_SENSITIVITY);

        if (input.keyboard().isDown(GLFW_KEY_UP)) {
            camera->addRotation(-rotation, 0.0f);
        } else if (input.keyboard().isDown(GLFW_KEY_DOWN)) {
            camera->addRotation(rotation, 0.0f);
        }

        if (input.keyboard().isDown(GLFW_KEY_LEFT)) {
            camera->addRotation(0.0f, -rotation);
        } else if (input.keyboard().isDown(GLFW_KEY_RIGHT)) {
            camera->addRotation(0.0f, rotation);
        }
    }

    std::vector<client::render::scene::imgui::CVulkanImGuiTextureData> iniImGui() {
        return {};
    }

    void renderImGuiWindow() {
        ImGuiIO& io = ImGui::GetIO();

        io.AddMousePosEvent(client::input::ClientInput->mouse().currentPosition().x, client::input::ClientInput->mouse().currentPosition().y);
        io.AddMouseButtonEvent(0, client::input::ClientInput->mouse().isDown(0));
        io.AddMouseButtonEvent(1, client::input::ClientInput->mouse().isDown(1));
        io.AddMouseButtonEvent(2, client::input::ClientInput->mouse().isDown(2));

        //const auto& descAlloc = client::render::loop::RenderLoop->descriptorAllocator();
        //const auto& gpass = client::render::loop::RenderLoop->deferredSceneRenderer().geometryPassRendering();
        //const VkDescriptorSet descriptorSet = descAlloc.getDescriptorSet(client::render::scene::deferred::lpass::DESC_ID_ATT, 0).vkDescriptorSet();
        //ImGui::Begin("MRT Debug");
        //constexpr ImVec2 imageSize{400.0f, 300.0f};
        //ImGui::Text("Albedo");
        //ImGui::Image(descriptorSet, imageSize);
        //ImGui::End();
    }
}
