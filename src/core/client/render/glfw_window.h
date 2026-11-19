#pragma once

#include <memory>

#include "engine_context.h"


#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"
#include "utility/system.h"

namespace cvulkan::client::window {
    class CVulkanWindow {
    public:
        explicit CVulkanWindow(const EngineData& engineData);

        ~CVulkanWindow();

        CLASS_NO_COPY_NO_MOVEASSIGN(CVulkanWindow);

        void closeWindow() const;

        [[nodiscard]] bool shouldBeClosed() const;

        [[nodiscard]] GLFWwindow* glfwWindowDescriptor() const {
            return _glfwWindowDescriptor;
        }

        [[nodiscard]] glm::uvec2 size() const {
            int width = 0;
            int height = 0;
            glfwGetWindowSize(_glfwWindowDescriptor, &width, &height);
            return {width, height};
        }

    private:
        GLFWwindow* _glfwWindowDescriptor;
        glm::uvec2 glfwWindowSize{};
    };

    extern std::unique_ptr<CVulkanWindow> GlfwWindow;

    void createWindow(const EngineData& engineData);

    void destroyWindow();
}
