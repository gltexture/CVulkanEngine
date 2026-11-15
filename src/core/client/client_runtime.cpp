#include "render/glfw_window.h"
#include "control/client_input.h"
#include "render/vulkan_render_core.h"

namespace cvulkan::client {
    void initializeClient(const EngineData& engineData) {
        window::createWindow(engineData);
        input::setupInput(*window::GlfwWindow);
        render::vk::createRenderCore(*window::GlfwWindow);
    }

    void loop() {
        while (!window::GlfwWindow->shouldBeClosed()) {
            input::updateInput();
            render::vk::runRender();
        }
    }

    void cleanUp() {
        render::vk::cleanRenderCore();
        input::destroyInput();
        window::destroyWindow();
    }
}
