#include "client_window.h"

#include <memory>
#include <stdexcept>
#include "GLFW/glfw3.h"
#include "engine_context.h"

namespace cvulkan::client::window {
    std::unique_ptr<Window> g_window;

    void createWindow(const EngineData& engineData)
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

        g_window = std::make_unique<Window>(glfw_win, glm::ivec2{engineData.m_default_windowSize.x, engineData.m_default_windowSize.y});
    }

    void cleanUp()
    {
        if (g_window != nullptr)
        {
            glfwDestroyWindow(g_window->m_windowDescriptor);
            g_window.reset();
        }
        glfwTerminate();
    }

    void closeWindow() {
        glfwSetWindowShouldClose(g_window->m_windowDescriptor, GLFW_TRUE);
    }

    [[nodiscard]] bool shouldBeClosed() {
        return glfwWindowShouldClose(g_window->m_windowDescriptor) == GLFW_TRUE;
    }
}
