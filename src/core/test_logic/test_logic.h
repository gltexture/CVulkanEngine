//
// Created by ded on 31.10.2026.
//

#pragma once

namespace cvulkan::client::render::scene {
    class CVulkanSceneEntitiesCollection;
}

namespace cvulkan::client::render::scene::context {
    class CVulkanSceneResourcesInitializer;
}

namespace cvulkan::test_logic {
    void initSceneResources(client::render::scene::context::CVulkanSceneResourcesInitializer& sceneResourcesInitializer);
    void initSceneObjects(client::render::scene::CVulkanSceneEntitiesCollection& entitiesCollection);
    void updateInput(double deltaTime);
}