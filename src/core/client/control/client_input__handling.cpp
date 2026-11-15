//
// Created by ded on 13.11.2026.
//


#include "client_input.h"

namespace cvulkan::client::input {
    void CVulkanMouse::onButtonEvent(const int button, const int action) {
        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) {
            return;
        }
        if (action != GLFW_PRESS && action != GLFW_RELEASE) {
            return;
        }
        this->_eventQueue.push({button, action});

        const auto [begin, end] = this->_buttonCallbacks.equal_range(button);

        std::vector<ButtonCallback> callbacks {};

        for (auto it = begin; it != end; ++it) {
            callbacks.push_back(it->second);
        }

        callbacks.insert(callbacks.end(), _universalButtonCallbacks.begin(), _universalButtonCallbacks.end());

        for (const auto& callback : callbacks) {
            if (callback) {
                callback(button, action);
            }
        }
    }

    void CVulkanKeyboard::onKeyEvent(const int key, const int action) {
        if (key < 0 || key > GLFW_KEY_LAST) {
            return;
        }

        if (action == GLFW_PRESS || action == GLFW_RELEASE) {
            this->_eventQueue.push({key, action});
        }

        const auto [begin, end] = this->_keyCallbacks.equal_range(key);
        std::vector<KeyCallback> callbacks {};

        for (auto it = begin; it != end; ++it) {
            callbacks.push_back(it->second);
        }

        callbacks.insert(callbacks.end(), _universalKeyCallbacks.begin(), _universalKeyCallbacks.end());

        for (const auto& callback : callbacks) {
            if (callback) {
                callback(key, action);
            }
        }
    }

    void CVulkanKeyboard::onCharCallback(const unsigned int codepoint) const {
        for (const auto& callback : this->_charCallbacks) {
            if (callback != nullptr) {
                callback(codepoint);
            }
        }
    }

    void CVulkanMouse::onPositionEvent(const double x, const double y) {
        this->_currentPosition.x = static_cast<int>(x);
        this->_currentPosition.y = static_cast<int>(y);
    }

    void CVulkanMouse::onEnterEvent(const bool entered) {
        this->_insideWindow = entered;
    }

    void CVulkanMouse::onScrollEvent(const double xOffset, const double yOffset) const {
        for (const auto& callback : this->_scrollCallbacks) {
            if (callback != nullptr) {
                callback(xOffset, yOffset);
            }
        }
    }
}