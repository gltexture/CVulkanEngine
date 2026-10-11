#include <iostream>

#include "render/glfw_window.h"
#include "control/client_controls.h"
#include "render/vulkan_render_core.h"

namespace cvulkan::client {
    void initializeClient(const EngineData& engineData) {
        window::createWindow(engineData);
        control::setup(*window::glfwWindow);
        render::vk::createRenderCore(*window::glfwWindow);
    }

    void loop() {
        while (!window::glfwWindow->shouldBeClosed()) {
            control::update_input();
            render::vk::runRender();
        }
    }

    void cleanUp() {
        control::destroyControl();
        render::vk::cleanRenderCore();
        window::destroyWindow();
    }
}
