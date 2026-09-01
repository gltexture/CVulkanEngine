#include <iostream>

#include "render/client_window.h"
#include "control/client_controls.h"
#include "render/client_renderer.h"

namespace cvulkan::client {
    void initializeClient(const EngineData& engineData) {
        window::createWindow(engineData);
        control::setup(*window::g_window);
        renderer::init();
    }

    void loop() {
        while (!window::shouldBeClosed()) {
            control::updateInput();
            renderer::render();
        }
    }

    void cleanUp() {
        control::cleanUp();
        renderer::cleanUp();
        window::cleanUp();
    }
}
