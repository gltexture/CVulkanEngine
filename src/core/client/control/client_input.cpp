#include "client_input.h"

#include <memory>

#include "client/render/glfw_window.h"
#include "utility/logging.h"

#include "client_input.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace cvulkan::client::input {
    namespace {
        std::unordered_map<GLFWwindow*, CVulkanGlfwInputAdapter*> InputAdapters;
    }

    void CVulkanMouse::update() {
        for (auto& state : this->_buttonStates) {
            state._justPressed = false;
            state._justReleased = false;
        }
        while (!this->_eventQueue.empty()) {
            const auto [button, action] = this->_eventQueue.front();
            this->_eventQueue.pop();
            if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) {
                continue;
            }
            auto& [_down, _justPressed, _justReleased] = this->_buttonStates.at(button);
            if (action == GLFW_PRESS) {
                if (!_down) {
                    _down = true;
                    _justPressed = true;
                }
            } else if (action == GLFW_RELEASE) {
                if (_down) {
                    _down = false;
                    _justReleased = true;
                }
            }
        }
        this->_deltaPosition = {};
        if (this->_previousPosition.x >= 0 && this->_previousPosition.y >= 0 && this->_insideWindow) {
            this->_deltaPosition = this->_currentPosition - this->_previousPosition;
        }
        this->_previousPosition = this->_currentPosition;
    }

    void CVulkanMouse::registerButtonCallback(int button, ButtonCallback callback) {
        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) {
            return;
        }
        if (callback) {
            this->_buttonCallbacks.emplace(button, std::move(callback));
        }
    }

    void CVulkanMouse::registerUniversalButtonCallback(ButtonCallback callback) {
        if (callback) {
            this->_universalButtonCallbacks.push_back(std::move(callback));
        }
    }

    void CVulkanMouse::registerScrollCallback(ScrollCallback callback) {
        if (callback) {
            this->_scrollCallbacks.push_back(std::move(callback));
        }
    }

    bool CVulkanMouse::isDown(const int button) const {
        return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && this->_buttonStates[button]._down;
    }

    bool CVulkanMouse::isButtonJustPressed(const int button) const {
        return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && this->_buttonStates[button]._justPressed;
    }

    bool CVulkanMouse::isButtonJustReleased(const int button) const {
        return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && this->_buttonStates[button]._justReleased;
    }

    void CVulkanKeyboard::update() {
        for (auto& state : this->_states) {
            state._justPressed = false;
            state._justReleased = false;
        }
        while (!this->_eventQueue.empty()) {
            const auto [key, action] = this->_eventQueue.front();
            this->_eventQueue.pop();
            if (key < 0 || key > GLFW_KEY_LAST) {
                continue;
            }
            auto& [_down, _justPressed, _justReleased] = this->_states.at(key);
            if (action == GLFW_PRESS) {
                if (!_down) {
                    _down = true;
                    _justPressed = true;
                }
            } else if (action == GLFW_RELEASE) {
                if (_down) {
                    _down = false;
                    _justReleased = true;
                }
            }
        }
    }

    void CVulkanKeyboard::registerKeyCallback(int key, KeyCallback callback) {
        if (key < 0 || key > GLFW_KEY_LAST) {
            return;
        }
        if (callback) {
            this->_keyCallbacks.emplace(key, std::move(callback));
        }
    }

    void CVulkanKeyboard::registerUniversalKeyCallback(KeyCallback callback) {
        if (callback) {
            this->_universalKeyCallbacks.push_back(std::move(callback));
        }
    }

    void CVulkanKeyboard::registerCharCallback(const CharCallback& callback) {
        this->_charCallbacks.push_back(callback);
    }

    bool CVulkanKeyboard::isDown(const int key) const {
        return key >= 0 && key <= GLFW_KEY_LAST && this->_states[key]._down;
    }

    bool CVulkanKeyboard::isJustPressed(const int key) const {
        return key >= 0 && key <= GLFW_KEY_LAST && this->_states[key]._justPressed;
    }

    bool CVulkanKeyboard::isJustReleased(const int key) const {
        return key >= 0 && key <= GLFW_KEY_LAST && this->_states[key]._justReleased;
    }

    CVulkanGlfwInputAdapter::CVulkanGlfwInputAdapter(GLFWwindow* window, CVulkanInput& input) : _window(window), _input(input) {
        if (_window == nullptr) {
            throw std::invalid_argument("CVulkanGlfwInputAdapter: GLFWwindow is null");
        }

        if (InputAdapters.contains(_window)) {
            throw std::runtime_error("CVulkanGlfwInputAdapter: window already has an input adapter");
        }

        InputAdapters.emplace(_window, this);

        this->_previousKeyCallback = glfwSetKeyCallback(_window, &CVulkanGlfwInputAdapter::keyCallback);
        this->_previousMouseButtonCallback = glfwSetMouseButtonCallback(_window, &CVulkanGlfwInputAdapter::mouseButtonCallback);
        this->_previousScrollCallback = glfwSetScrollCallback(_window, &CVulkanGlfwInputAdapter::scrollCallback);
        this->_previousCursorPosCallback = glfwSetCursorPosCallback(_window, &CVulkanGlfwInputAdapter::cursorPosCallback);
        this->_previousCursorEnterCallback = glfwSetCursorEnterCallback(_window, &CVulkanGlfwInputAdapter::cursorEnterCallback);
        this->_previousCharCallback = glfwSetCharCallback(_window, &CVulkanGlfwInputAdapter::charCallback);
    }

    CVulkanGlfwInputAdapter::~CVulkanGlfwInputAdapter() {
        if (_window == nullptr) {
            return;
        }

        glfwSetCharCallback(_window, this->_previousCharCallback);
        glfwSetKeyCallback(_window, this->_previousKeyCallback);
        glfwSetMouseButtonCallback(_window, this->_previousMouseButtonCallback);
        glfwSetScrollCallback(_window, this->_previousScrollCallback);
        glfwSetCursorPosCallback(_window, this->_previousCursorPosCallback);
        glfwSetCursorEnterCallback(_window, this->_previousCursorEnterCallback);

        if (const auto it = InputAdapters.find(_window); it != InputAdapters.end() && it->second == this) {
            InputAdapters.erase(it);
        }
    }

    CVulkanGlfwInputAdapter* CVulkanGlfwInputAdapter::get(GLFWwindow* window) {
        const auto it = InputAdapters.find(window);
        if (it == InputAdapters.end()) {
            return nullptr;
        }
        return it->second;
    }

    void CVulkanGlfwInputAdapter::charCallback(GLFWwindow* window, const unsigned int codepoint) {
        if (const auto* adapter = get(window)) {
            adapter->_input.keyboard().onCharCallback(codepoint);
        }
    }

    void CVulkanGlfwInputAdapter::keyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
        if (const auto* adapter = get(window)) {
            adapter->_input.keyboard().onKeyEvent(key, action);
            if (adapter->_previousKeyCallback != nullptr) {
                adapter->_previousKeyCallback(window, key, scancode, action, mods);
            }
        }
    }

    void CVulkanGlfwInputAdapter::mouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods) {
        if (const auto* adapter = get(window)) {
            adapter->_input.mouse().onButtonEvent(button, action);
            if (adapter->_previousMouseButtonCallback != nullptr) {
                adapter->_previousMouseButtonCallback(window, button, action, mods);
            }
        }
    }

    void CVulkanGlfwInputAdapter::scrollCallback(GLFWwindow* window, const double xOffset, const double yOffset) {
        if (const auto* adapter = get(window)) {
            adapter->_input.mouse().onScrollEvent(xOffset, yOffset);
            if (adapter->_previousScrollCallback != nullptr) {
                adapter->_previousScrollCallback(window, xOffset, yOffset);
            }
        }
    }

    void CVulkanGlfwInputAdapter::cursorPosCallback(GLFWwindow* window, const double x, const double y) {
        if (const auto* adapter = get(window)) {
            adapter->_input.mouse().onPositionEvent(x, y);
            if (adapter->_previousCursorPosCallback != nullptr) {
                adapter->_previousCursorPosCallback(window, x, y);
            }
        }
    }

    void CVulkanGlfwInputAdapter::cursorEnterCallback(GLFWwindow* window, const int entered) {
        if (const auto* adapter = get(window)) {
            adapter->_input.mouse().onEnterEvent(entered == GLFW_TRUE);
            if (adapter->_previousCursorEnterCallback != nullptr) {
                adapter->_previousCursorEnterCallback(window, entered);
            }
        }
    }

    CVulkanInput::CVulkanInput(const window::CVulkanWindow& window) {
        this->_glfwAdapter = std::make_unique<CVulkanGlfwInputAdapter>(window.glfwWindowDescriptor(), *this);
    }

    void CVulkanInput::update() {
        this->_keyboard.update();
        this->_mouse.update();
    }

    std::unique_ptr<CVulkanInput> ClientInput{};

    void setupInput(const window::CVulkanWindow& window) {
        ClientInput = std::make_unique<CVulkanInput>(window);
    }

    void updateInput() {
        if (ClientInput == nullptr) {
            return;
        }
        glfwPollEvents();
        ClientInput->update();
    }

    void destroyInput() {
        ClientInput.reset();
    }
}
