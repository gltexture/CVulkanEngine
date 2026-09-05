#pragma once

#include <memory>

#include "engine_context.h"

#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

namespace cvulkan::client::window {
    class CVWindow {
    public:
        explicit CVWindow(GLFWwindow* glfw_win) : glfwWindowDescriptor(glfw_win) {};
        ~CVWindow() = default;

        void closeWindow() const;
        [[nodiscard]] bool shouldBeClosed() const;

        [[nodiscard]] GLFWwindow *glfw_window_descriptor() const {
            return glfwWindowDescriptor;
        }

        [[nodiscard]] glm::uvec2 size() const {
            int width = 0;
            int height = 0;
            glfwGetWindowSize(glfwWindowDescriptor, &width, &height);
            return {width, height};
        }

    private:
        GLFWwindow *glfwWindowDescriptor;
        glm::uvec2 glfwWindowSize = {};
    };

    extern std::unique_ptr<CVWindow> glfwWindow;
    void create_window(const EngineData& engineData);
    void cleanUp();
}
