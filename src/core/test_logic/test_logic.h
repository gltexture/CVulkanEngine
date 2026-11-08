//
// Created by ded on 31.10.2026.
//

#pragma once

namespace cvulkan::client::render::scene::main {
    class CVulkanSceneEntitiesCollection;
}

namespace cvulkan::client::render::scene::main::context {
    class CVulkanSceneResourcesInitializer;
}

namespace cvulkan::test_logic {
    void initSceneResources(client::render::scene::main::context::CVulkanSceneResourcesInitializer& sceneResourcesInitializer);

    void initSceneObjects(client::render::scene::main::CVulkanSceneEntitiesCollection& entitiesCollection);

    void updateInput(double deltaTime);
}