#include <iostream>

#include "render/glfw_window.h"
#include "control/client_input.h"
#include "render/vulkan_render_core.h"

namespace cvulkan::client {
    void initializeClient(const EngineData& engineData) {
        window::createWindow(engineData);
        input::setupInput(*window::glfwWindow);
        render::vk::createRenderCore(*window::glfwWindow);
    }

    void loop() {
        while (!window::glfwWindow->shouldBeClosed()) {
            input::updateInput();
            render::vk::runRender();
        }
    }

    void cleanUp() {
        input::destroyInput();
        render::vk::cleanRenderCore();
        window::destroyWindow();
    }
}
