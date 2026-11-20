#include "render/glfw_window.h"
#include "control/client_input.h"
#include "render/vulkan_render_core.h"
#include "client_runtime.h"

namespace cvulkan::client {
    CVulkanClientRuntime::CVulkanClientRuntime(const CVulkanClientRuntimeData&& engineData)
        : _engineData(std::move(engineData)) {
        window::createWindow(_engineData);
        input::setupInput(*window::GlfwWindow);
        render::vk::createRenderCore(*window::GlfwWindow);

        this->run();

        this->_initialized = true;
    }

    CVulkanClientRuntime::~CVulkanClientRuntime() {
        if (!this->_initialized) {
            return;
        }
        render::vk::cleanRenderCore();
        input::destroyInput();
        window::destroyWindow();
        this->_initialized = false;
    }

    void CVulkanClientRuntime::run() {
        while (!window::GlfwWindow->shouldBeClosed()) {
            input::updateInput();
            render::vk::runRender();
        }
    }
}