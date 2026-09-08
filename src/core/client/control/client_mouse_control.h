#pragma once
#include "client/render/glfw_window.h"
#include "glm/vec2.hpp"

namespace cvulkan::client::control {
    class MouseControl {
    public:
        explicit MouseControl(const window::CVWindow& window) : m_window{window} {
            this->glfwCallbacks();
        };
        ~MouseControl() = default;

        MouseControl(const MouseControl&) = delete;
        MouseControl& operator=(const MouseControl&) = delete;

        [[nodiscard]] const glm::ivec2& prevMousePosition() const {
            return this->m_prevFrame_mouse_position;
        }

        [[nodiscard]] const glm::ivec2& currentMousePosition() const {
            return this->m_current_mouse_position;
        }

        [[nodiscard]] const glm::ivec2& deltaMousePosition() const {
            return this->m_delta_mouse_position;
        }

        [[nodiscard]] bool inWindow() const {
            return this->m_in_window;
        }

        [[nodiscard]] bool mouse0Pressed() const {
            return this->m_mouse0_pressed;
        }

        [[nodiscard]] bool mouse1Pressed() const {
            return this->m_mouse1_pressed;
        }

        [[nodiscard]] bool mouse2Pressed() const {
            return this->m_mouse2_pressed;
        }

        void glfwCallbacks();
        void updateInput();

    protected:
        const window::CVWindow& m_window;
        glm::ivec2 m_prevFrame_mouse_position{};
        glm::ivec2 m_current_mouse_position{};
        glm::ivec2 m_delta_mouse_position{};
        bool m_in_window{};
        bool m_mouse0_pressed{};
        bool m_mouse1_pressed{};
        bool m_mouse2_pressed{};
    };
}