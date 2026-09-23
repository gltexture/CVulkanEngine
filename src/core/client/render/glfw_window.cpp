#include "glfw_window.h"

#include <stdexcept>
#include "GLFW/glfw3.h"
#include "engine_context.h"

namespace cvulkan::client::window {
    std::unique_ptr<CVulkanWindow> glfwWindow;

    CVulkanWindow::CVulkanWindow(const EngineData& engineData) {
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

        this->_glfwWindowDescriptor = glfwCreateWindow(engineData.m_default_windowSize.x, engineData.m_default_windowSize.y, engineData.m_appTitle.c_str(), nullptr, nullptr);
        if (this->_glfwWindowDescriptor == nullptr) {
            throw std::runtime_error("failed to create window");
        }
    }

    CVulkanWindow::~CVulkanWindow() {
        if (this->_glfwWindowDescriptor != nullptr) {
            glfwDestroyWindow(this->_glfwWindowDescriptor);
            this->_glfwWindowDescriptor = nullptr;
        }
        glfwTerminate();
    }

    void CVulkanWindow::closeWindow() const {
        glfwSetWindowShouldClose(this->_glfwWindowDescriptor, GLFW_TRUE);
    }

    bool CVulkanWindow::shouldBeClosed() const {
        return glfwWindowShouldClose(this->_glfwWindowDescriptor) == GLFW_TRUE;
    }

    void createWindow(const EngineData& engineData) {
        glfwWindow = std::make_unique<CVulkanWindow>(engineData);
    }

    void destroyWindow() {
        if (glfwWindow != nullptr) {
            glfwWindow.reset();
        }
    }
}
