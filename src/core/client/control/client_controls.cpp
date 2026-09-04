#include "client_controls.h"

#include <memory>

#include "client_keyboard_control.h"
#include "client_mouse_control.h"
#include "util/logger.h"

namespace cvulkan::client::control {
    std::unique_ptr<KeyboardControl> g_keyboardControl;
    std::unique_ptr<MouseControl> g_mouseControl;

    void setup(const window::CVWindow& window) {
        logging::info("Init control:Start");
        g_keyboardControl = std::make_unique<KeyboardControl>(window);
        g_mouseControl = std::make_unique<MouseControl>(window);
        logging::info("Init control:End");
    }

    void update_input() {
        glfwPollEvents();
        if (g_keyboardControl != nullptr) {
            g_keyboardControl->updateInput();
        }
        if (g_mouseControl != nullptr) {
            g_mouseControl->updateInput();
        }
    }

    void cleanUp() {
        g_keyboardControl.reset();
        g_mouseControl.reset();
    }
}
