#include <iostream>

#include "render/glfw_window.h"
#include "control/client_controls.h"
#include "render/vulkan_render_core.h"

namespace cvulkan::client {
    void initializeClient(const EngineData& engineData) {
        window::createWindow(engineData);
        control::setup(*window::glfwWindow);
        render::core::createRenderCore(*window::glfwWindow);
    }

    void loop() {
        while (!window::glfwWindow->shouldBeClosed()) {
            control::update_input();
            render::core::runRender();
        }
    }

    void cleanUp() {
        control::destroyControl();
        render::core::cleanRenderCore();
        window::destroyWindow();
    }
}
