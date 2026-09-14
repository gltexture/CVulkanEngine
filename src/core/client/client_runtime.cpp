#include <iostream>

#include "render/glfw_window.h"
#include "control/client_controls.h"
#include "render/vulkan_render_core.h"

namespace cvulkan::client {
    void initializeClient(const EngineData& engineData) {
        window::create_window(engineData);
        control::setup(*window::glfwWindow);
        renderCore::initRenderCore(*window::glfwWindow);
    }

    void loop() {
        while (!window::glfwWindow->shouldBeClosed()) {
            control::update_input();
            renderCore::runRender();
        }
    }

    void cleanUp() {
        control::cleanUp();
        renderCore::cleanRenderCore();
        window::cleanUp();
    }
}
