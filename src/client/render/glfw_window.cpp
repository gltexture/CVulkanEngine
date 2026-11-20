#include "glfw_window.h"

#include <stdexcept>
#include "GLFW/glfw3.h"
#include "core/utility/exceptions.h"
#include "client/client_runtime.h"

namespace cvulkan::client::window {
    std::unique_ptr<CVulkanWindow> GlfwWindow;

    CVulkanWindow::CVulkanWindow(const CVulkanClientRuntimeData& engineData) {
        if (!glfwInit()) {
            throw exc::CVulkanException("Failed to initialize GLFW");
        }

        if (!glfwVulkanSupported()) {
            throw exc::CVulkanException("Vulkan is not supported by GLFW on this system");
        }

        if (const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor()); videoMode == nullptr) {
            throw exc::CVulkanException("Failed to get GLFW video mode");
        }

        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

        this->_glfwWindowDescriptor = glfwCreateWindow(engineData._windowWidth, engineData._windowHeight, engineData._title.c_str(), nullptr, nullptr);
        if (this->_glfwWindowDescriptor == nullptr) {
            throw exc::CVulkanException("Failed to create GLFW window");
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

    void createWindow(const CVulkanClientRuntimeData& engineData) {
        GlfwWindow = std::make_unique<CVulkanWindow>(engineData);
    }

    void destroyWindow() {
        if (GlfwWindow != nullptr) {
            GlfwWindow.reset();
        }
    }
}
