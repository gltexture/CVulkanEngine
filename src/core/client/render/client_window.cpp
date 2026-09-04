#include "client_window.h"

#include <stdexcept>
#include "GLFW/glfw3.h"
#include "engine_context.h"

namespace cvulkan::client::window {
    std::unique_ptr<CVWindow> glfwWindow = {};

    void CVWindow::closeWindow() const {
        glfwSetWindowShouldClose(this->glfwWindowDescriptor, GLFW_TRUE);
    }

    bool CVWindow::shouldBeClosed() const {
        return glfwWindowShouldClose(this->glfwWindowDescriptor) == GLFW_TRUE;
    }

    void create_window(const EngineData& engineData)
    {
        if (!glfwInit()) {
            throw std::runtime_error("failed to initialize glfw");
        }

        if (!glfwVulkanSupported()) {
            throw std::runtime_error("Vulkan support not available");
        }

        if (const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor()); videoMode == nullptr) {
            throw std::runtime_error("failed to get video mode");
        }

        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

        GLFWwindow* glfw_win = glfwCreateWindow(engineData.m_default_windowSize.x, engineData.m_default_windowSize.y, engineData.m_appTitle.c_str(), nullptr, nullptr);
        if (glfw_win == nullptr) {
            throw std::runtime_error("failed to create window");
        }

        glfwWindow = std::make_unique<CVWindow>(glfw_win);
    }

    void cleanUp() {
        if (glfwWindow != nullptr && glfwWindow->glfw_window_descriptor() != nullptr) {
            glfwDestroyWindow(glfwWindow->glfw_window_descriptor());
            glfwWindow.reset();
        }
        glfwTerminate();
    }
}
