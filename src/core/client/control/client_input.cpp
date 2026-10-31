#include "client_input.h"

#include <memory>

#include "client/render/glfw_window.h"
#include "utility/logger.h"

namespace cvulkan::client::input {
    std::unique_ptr<CVulkanClientInput> _clientInput;

    CVulkanKeyboardInput::CVulkanKeyboardInput(const window::CVulkanWindow& window) : _glfwWindow{window} {
    }

    void CVulkanKeyboardInput::updateInput() {
    }

    CVulkanClientInput::CVulkanClientInput(const window::CVulkanWindow& glfwWindow) : _glfwWindow(glfwWindow), _keyboardControl(glfwWindow), _mouseControl(glfwWindow) {
        GLFWwindow* window = this->_glfwWindow.glfwWindowDescriptor();
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, [](GLFWwindow* mWindow, const int key, int scancode, const int action, int mods) {
            auto* input = static_cast<CVulkanClientInput*>(glfwGetWindowUserPointer(mWindow));
            input->onKeyboardEvent(key, action);
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow* mWindow, const int button, const int action, int mods) {
            auto* input = static_cast<CVulkanClientInput*>(glfwGetWindowUserPointer(mWindow));
            input->onMouseButtonEvent(button, action);
        });
    }

    CVulkanMouseInput::CVulkanMouseInput(const window::CVulkanWindow& window) : _glfwWindow{window} {
        glfwSetWindowUserPointer(this->_glfwWindow.glfwWindowDescriptor(), this);
        glfwSetCursorPosCallback(this->_glfwWindow.glfwWindowDescriptor(), [](GLFWwindow* mWindow, const double x, const double y) {
            auto* mouse = static_cast<CVulkanMouseInput *>(glfwGetWindowUserPointer(mWindow));
            mouse->_currentMousePosition.x = static_cast<int>(x);
            mouse->_currentMousePosition.y = static_cast<int>(y);
        });
        glfwSetCursorEnterCallback(this->_glfwWindow.glfwWindowDescriptor(), [](GLFWwindow* mWindow, const int entered) {
            auto* mouse = static_cast<CVulkanMouseInput *>(glfwGetWindowUserPointer(mWindow));
            mouse->_inWindow = entered;
        });
    }

    void CVulkanMouseInput::updateInput() {
        this->_deltaMousePosition.x = 0;
        this->_deltaMousePosition.y = 0;
        if (this->_prevFrameMousePosition.x >= 0 && this->_prevFrameMousePosition.y >= 0 && this->_inWindow) {
            this->_deltaMousePosition.x = this->_currentMousePosition.x - this->_prevFrameMousePosition.x;
            this->_deltaMousePosition.y = this->_currentMousePosition.y - this->_prevFrameMousePosition.y;
        }
        this->_prevFrameMousePosition.x = this->_currentMousePosition.x;
        this->_prevFrameMousePosition.y = this->_currentMousePosition.y;
    }

    void CVulkanClientInput::updateInput() {
        {
            for (auto& state : this->_mouseButtonStates) {
                state._justPressed = false;
                state._justReleased = false;
            }

            while (!this->_mouse_eventQueue.empty()) {
                const auto& [_keyId, _action] = this->_mouse_eventQueue.front();
                this->_mouse_eventQueue.pop();

                if (_keyId < 0 || _keyId > GLFW_MOUSE_BUTTON_LAST) {
                    return;
                }

                auto& [_down, _justPressed, _justReleased] = this->_mouseButtonStates.at(_keyId);

                if (_action == GLFW_PRESS) {
                    _down = true;
                    _justPressed = true;
                } else if (_action == GLFW_RELEASE) {
                    _down = false;
                    _justReleased = true;
                }
            }
        }

        {
            for (auto& state : this->_keyStates) {
                state._justPressed = false;
                state._justReleased = false;
            }

            while (!this->_key_eventQueue.empty()) {
                const auto& [_key, _action] = this->_key_eventQueue.front();
                this->_key_eventQueue.pop();

                if (_key < 0 || _key > GLFW_KEY_LAST) {
                    return;
                }

                auto& [_down, _justPressed, _justReleased] = this->_keyStates.at(_key);

                if (_action == GLFW_PRESS) {
                    _down = true;
                    _justPressed = true;
                } else if (_action == GLFW_RELEASE) {
                    _down = false;
                    _justReleased = true;
                }
            }
        }

        {
            this->_mouseControl.updateInput();
            this->_keyboardControl.updateInput();
        }
    }

    bool CVulkanClientInput::isKeyDown(const int key) const {
        return key >= 0 && key <= GLFW_KEY_LAST && this->_keyStates[key]._down;
    }

    bool CVulkanClientInput::isKeyJustPressed(const int key) const {
        return key >= 0 && key <= GLFW_KEY_LAST && this->_keyStates[key]._justPressed;
    }

    bool CVulkanClientInput::isKeyJustReleased(const int key) const {
        return key >= 0 && key <= GLFW_KEY_LAST && this->_keyStates[key]._justReleased;
    }

    bool CVulkanClientInput::isMouseButtonDown(const int button) const {
        return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && this->_mouseButtonStates[button]._down;
    }

    bool CVulkanClientInput::isMouseButtonJustPressed(const int button) const {
        return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && this->_mouseButtonStates[button]._justPressed;
    }

    bool CVulkanClientInput::isMouseButtonJustReleased(const int button) const {
        return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && this->_mouseButtonStates[button]._justReleased;
    }

    glm::ivec2 CVulkanClientInput::getMouseDelta() const {
        return this->_mouseControl.deltaMousePosition();
    }

    void CVulkanClientInput::onKeyboardEvent(const int key, const int action) {
        if (key < 0 || key > GLFW_KEY_LAST) {
            return;
        }
        if (action == GLFW_PRESS || action == GLFW_RELEASE) {
            this->_key_eventQueue.push({key, action});
        }
    }

    void CVulkanClientInput::onMouseButtonEvent(const int button, const int action) {
        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) {
            return;
        }
        if (action == GLFW_PRESS || action == GLFW_RELEASE) {
            this->_mouse_eventQueue.push({button, action});
        }
    }

    void setupInput(const window::CVulkanWindow& window) {
        _clientInput = std::make_unique<CVulkanClientInput>(window);
    }

    void updateInput() {
        glfwPollEvents();
        _clientInput->updateInput();
    }

    void destroyInput() {
        if (_clientInput != nullptr) {
            _clientInput.reset();
        }
    }
}
