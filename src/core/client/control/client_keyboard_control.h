#pragma once
#include <functional>
#include <unordered_map>

#include "client/render/client_window.h"

namespace cvulkan::client::control {
    using KeyCallback = std::function<void(int key, int action)>;

    class KeyboardControl {
    public:
        explicit KeyboardControl(const window::Window& window) : m_window{window} {
            this->glfwCallbacks();
        };
        ~KeyboardControl() = default;

        void glfwCallbacks();
        void updateInput();

        void registerCallback(const int key, KeyCallback func) {
            this->m_key_callbacks.insert({key, std::move(func)});
        }

    protected:
        std::unordered_multimap<int, KeyCallback> m_key_callbacks = {};
        const window::Window& m_window;
    };
}
