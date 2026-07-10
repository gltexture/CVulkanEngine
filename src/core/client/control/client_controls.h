#pragma once
#include <memory>

#include "client_keyboard_control.h"
#include "client_mouse_control.h"

namespace cvulkan::client::control {
    extern std::unique_ptr<KeyboardControl> g_keyboardControl;
    extern std::unique_ptr<MouseControl> g_mouseControl;

    void setup(const window::Window& window);
    void updateInput();
    void cleanUp();
}
