//
// Created by ded on 31.10.2026.
//

#pragma once

namespace cvulkan::client::render::entity {
    class CVulkanSceneEntitiesCollection;
}

namespace cvulkan::client::render::scene::imgui {
    struct CVulkanImGuiTextureData;
}

namespace cvulkan::client::render::scene::deferred::context {
    class CVulkanSceneResourcesInitializer;
}

namespace cvulkan::test_logic {
    void initSceneResources(client::render::scene::deferred::context::CVulkanSceneResourcesInitializer& sceneResourcesInitializer);

    void initSceneObjects(client::render::entity::CVulkanSceneEntitiesCollection& entitiesCollection);

    std::vector<client::render::scene::imgui::CVulkanImGuiTextureData> iniImGui();

    void renderImGuiWindow();

    void updateInput(double deltaTime);
}