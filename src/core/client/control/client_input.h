#pragma once
#include <memory>
#include <queue>

#include "utility/sys_utils.h"

namespace cvulkan::client::window {
    class CVulkanWindow;
}

namespace cvulkan::client::input {
    using GLFW_KeyCallback = std::function<void(int key, int action)>;
    using GLFW_MouseCallback = std::function<void(int button, int action)>;

    using GLFW_Key = int;
    using GLFW_Action = int;

    struct CVulkanInputState {
        bool _down = false;
        bool _justPressed = false;
        bool _justReleased = false;
    };

    struct CVulkanEventPress {
        GLFW_Key _key;
        GLFW_Action _action;
    };

    class CVulkanMouseInput {
    public:
        explicit CVulkanMouseInput(const window::CVulkanWindow& window);

        ~CVulkanMouseInput() = default;

        CLASS_NO_COPY_NO_MOVE(CVulkanMouseInput)

        void updateInput();

        [[nodiscard]] glm::ivec2 prevMousePosition() const {
            return this->_prevFrameMousePosition;
        }

        [[nodiscard]] glm::ivec2 currentMousePosition() const {
            return this->_currentMousePosition;
        }

        [[nodiscard]] glm::ivec2 deltaMousePosition() const {
            return this->_deltaMousePosition;
        }

        [[nodiscard]] bool inWindow() const {
            return this->_inWindow;
        }

    protected:
        const window::CVulkanWindow& _glfwWindow;
        glm::ivec2 _prevFrameMousePosition{};
        glm::ivec2 _currentMousePosition{};
        glm::ivec2 _deltaMousePosition{};
        bool _inWindow{};
    };

    class CVulkanKeyboardInput {
    public:
        explicit CVulkanKeyboardInput(const window::CVulkanWindow& window);
        ~CVulkanKeyboardInput() = default;

        CLASS_NO_COPY_NO_MOVE(CVulkanKeyboardInput)

        void updateInput();

        //void registerCallback(const int key, GLFW_KeyCallback func) {
        //    this->_keyCallbacks.insert({key, std::move(func)});
        //}

    protected:
        const window::CVulkanWindow& _glfwWindow;
        //std::unordered_multimap<int, GLFW_KeyCallback> _keyCallbacks = {};
    };

    class CVulkanClientInput {
    public:
        explicit CVulkanClientInput(const window::CVulkanWindow& glfwWindow);

        CLASS_NO_COPY_NO_MOVE(CVulkanClientInput)

        void updateInput();

        [[nodiscard]] bool isKeyDown(int key) const;
        [[nodiscard]] bool isKeyJustPressed(int key) const;
        [[nodiscard]] bool isKeyJustReleased(int key) const;

        [[nodiscard]] bool isMouseButtonDown(int button) const;
        [[nodiscard]] bool isMouseButtonJustPressed(int button) const;
        [[nodiscard]] bool isMouseButtonJustReleased(int button) const;

        [[nodiscard]] glm::ivec2 getMouseDelta() const;

    protected:
        void onKeyboardEvent(int key, int action);
        void onMouseButtonEvent(int button, int action);

    private:
        const window::CVulkanWindow& _glfwWindow;

        CVulkanKeyboardInput _keyboardControl;
        CVulkanMouseInput _mouseControl;

        std::array<CVulkanInputState, GLFW_KEY_LAST + 1> _keyStates {};
        std::array<CVulkanInputState, GLFW_MOUSE_BUTTON_LAST + 1> _mouseButtonStates {};

        std::queue<CVulkanEventPress> _key_eventQueue {};
        std::queue<CVulkanEventPress> _mouse_eventQueue {};
    };

    extern std::unique_ptr<CVulkanClientInput> _clientInput;

    void setupInput(const window::CVulkanWindow& window);
    void updateInput();
    void destroyInput();
}
