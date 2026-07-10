#pragma once
#include <memory>

#include "engine_context.h"
#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

namespace cvulkan::client::window {
    struct Window
    {
        GLFWwindow* m_windowDescriptor;
        const glm::uvec2 m_windowSize;
    };

    extern std::unique_ptr<Window> g_window;

    void closeWindow();
    [[nodiscard]] bool shouldBeClosed();

    void createWindow(const EngineData& engineData);
    void cleanUp();
}
