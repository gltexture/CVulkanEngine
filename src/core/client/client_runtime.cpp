#include <iostream>

#include "render/client_window.h"
#include "control/client_controls.h"
#include "render/client_render_core.h"

namespace cvulkan::client {
    void initializeClient(const EngineData& engineData) {
        window::create_window(engineData);
        control::setup(*window::glfwWindow);
        renderer::init(*window::glfwWindow);
    }

    void loop() {
        while (!window::glfwWindow->shouldBeClosed()) {
            control::update_input();
            renderer::render();
        }
    }

    void cleanUp() {
        control::cleanUp();
        renderer::clean_up();
        window::cleanUp();
    }
}
