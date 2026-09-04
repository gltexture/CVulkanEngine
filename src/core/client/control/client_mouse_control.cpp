#include "client_mouse_control.h"

namespace cvulkan::client::control {
    void MouseControl::glfwCallbacks() {
        glfwSetWindowUserPointer(m_window.glfw_window_descriptor(), this);
        glfwSetCursorPosCallback(this->m_window.glfw_window_descriptor(), [](GLFWwindow* window, const double x, const double y) {
            auto* mouse = static_cast<MouseControl*>(glfwGetWindowUserPointer(window));
            glm::ivec2& pos = mouse->m_current_mouse_position;
            pos.x = static_cast<int>(x);
            pos.y = static_cast<int>(y);
        });
        glfwSetCursorEnterCallback(this->m_window.glfw_window_descriptor(), [](GLFWwindow* window, const int entered) {
            auto* mouse = static_cast<MouseControl*>(glfwGetWindowUserPointer(window));
            bool& inWin = mouse->m_in_window;
            inWin = entered;
        });
        glfwSetMouseButtonCallback(this->m_window.glfw_window_descriptor(), [](GLFWwindow* window, const int button, const int action, const int mods) {
            auto* mouse = static_cast<MouseControl*>(glfwGetWindowUserPointer(window));
            bool& mouse0 = mouse->m_mouse0_pressed;
            bool& mouse1 = mouse->m_mouse1_pressed;
            bool& mouse2 = mouse->m_mouse2_pressed;
            mouse0 = button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS;
            mouse1 = button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS;
            mouse2 = button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS;
        });
    }

    void MouseControl::updateInput() {
        this->m_delta_mouse_position.x = 0;
        this->m_delta_mouse_position.y = 0;
        if (this->m_prevFrame_mouse_position.x >= 0 && this->m_prevFrame_mouse_position.y >= 0 && this->m_in_window) {
            this->m_delta_mouse_position.x = this->m_current_mouse_position.x - this->m_prevFrame_mouse_position.x;
            this->m_delta_mouse_position.y = this->m_current_mouse_position.y - this->m_prevFrame_mouse_position.y;
        }
        this->m_prevFrame_mouse_position.x = this->m_current_mouse_position.x;
        this->m_prevFrame_mouse_position.y = this->m_current_mouse_position.y;
    }
}
