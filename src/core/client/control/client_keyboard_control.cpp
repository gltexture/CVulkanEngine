#include "client_keyboard_control.h"

namespace cvulkan::client::control {
    void KeyboardControl::glfwCallbacks() {
        glfwSetWindowUserPointer(m_window.m_windowDescriptor, this);
        glfwSetKeyCallback(this->m_window.m_windowDescriptor, [](GLFWwindow* window, const int key, int scancode, const int action, int mods) {
            auto* keyboard = static_cast<KeyboardControl*>(glfwGetWindowUserPointer(window));
            if (keyboard->m_key_callbacks.contains(key)) {
                auto [fst, snd] = keyboard->m_key_callbacks.equal_range(key);
                for (auto it = fst; it != snd; ++it){
                    it->second(key, action);
                }
            }
        });
    }

    void KeyboardControl::updateInput() {
    }
}
