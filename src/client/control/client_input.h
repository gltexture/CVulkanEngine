#pragma once
#include <memory>
#include <queue>

#include "core/utility/system.h"

namespace cvulkan::client::window {
    class CVulkanWindow;
}

namespace cvulkan::client::input {
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

    class CVulkanMouse {
    public:
        using ButtonCallback = std::function<void(int, int)>;
        using ScrollCallback = std::function<void(double, double)>;

        CVulkanMouse() = default;

        void update();

        void onButtonEvent(int button, int action);
        void onPositionEvent(double x, double y);
        void onEnterEvent(bool entered);
        void onScrollEvent(double xOffset, double yOffset) const;

        void registerButtonCallback(int button, ButtonCallback callback);
        void registerUniversalButtonCallback(ButtonCallback callback);
        void registerScrollCallback(ScrollCallback callback);

        [[nodiscard]] bool isDown(int button) const;
        [[nodiscard]] bool isButtonJustPressed(int button) const;
        [[nodiscard]] bool isButtonJustReleased(int button) const;

        [[nodiscard]] glm::ivec2 previousPosition() const {
            return _previousPosition;
        }

        [[nodiscard]] glm::ivec2 currentPosition() const {
            return _currentPosition;
        }

        [[nodiscard]] glm::ivec2 deltaPosition() const {
            return _deltaPosition;
        }

        [[nodiscard]] bool insideWindow() const {
            return _insideWindow;
        }

    private:
        std::array<CVulkanInputState, GLFW_MOUSE_BUTTON_LAST + 1> _buttonStates{};

        glm::ivec2 _previousPosition{};
        glm::ivec2 _currentPosition{};
        glm::ivec2 _deltaPosition{};

        bool _insideWindow{};

        std::unordered_multimap<int, ButtonCallback> _buttonCallbacks{};
        std::vector<ButtonCallback> _universalButtonCallbacks{};
        std::vector<ScrollCallback> _scrollCallbacks{};

        std::queue<CVulkanEventPress> _eventQueue{};
    };

    class CVulkanKeyboard {
    public:
        using KeyCallback = std::function<void(int, int)>;
        using CharCallback = std::function<void(int)>;

        CVulkanKeyboard() = default;

        void update();

        void onKeyEvent(int key, int action);
        void onCharCallback(unsigned int codepoint) const;

        void registerCharCallback(const CharCallback& callback);
        void registerKeyCallback(int key, KeyCallback callback);
        void registerUniversalKeyCallback(KeyCallback callback);

        [[nodiscard]] bool isDown(int key) const;
        [[nodiscard]] bool isJustPressed(int key) const;
        [[nodiscard]] bool isJustReleased(int key) const;

    private:
        std::array<CVulkanInputState, GLFW_KEY_LAST + 1> _states{};

        std::unordered_multimap<int, KeyCallback> _keyCallbacks{};
        std::vector<KeyCallback> _universalKeyCallbacks{};
        std::vector<CharCallback> _charCallbacks{};

        std::queue<CVulkanEventPress> _eventQueue{};
    };

    class CVulkanInput;

    class CVulkanGlfwInputAdapter {
    public:
        CVulkanGlfwInputAdapter(GLFWwindow* window, CVulkanInput& input);
        ~CVulkanGlfwInputAdapter();

        CLASS_NO_COPY_NO_MOVE(CVulkanGlfwInputAdapter)

    private:
        GLFWwindow* _window{};
        CVulkanInput& _input;

        GLFWkeyfun _previousKeyCallback{};
        GLFWmousebuttonfun _previousMouseButtonCallback{};
        GLFWscrollfun _previousScrollCallback{};
        GLFWcursorposfun _previousCursorPosCallback{};
        GLFWcursorenterfun _previousCursorEnterCallback{};
        GLFWcharfun _previousCharCallback{};

        static CVulkanGlfwInputAdapter* get(GLFWwindow* window);

        static void charCallback(GLFWwindow* window, unsigned int codepoint);
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
        static void cursorPosCallback(GLFWwindow* window, double x, double y);
        static void cursorEnterCallback(GLFWwindow* window, int entered);
    };

    class CVulkanInput {
    public:
        explicit CVulkanInput(const window::CVulkanWindow& window);

        ~CVulkanInput() = default;

        CLASS_NO_COPY_NO_MOVE(CVulkanInput)

        void update();

        [[nodiscard]] CVulkanKeyboard& keyboard() noexcept {
            return _keyboard;
        }

        [[nodiscard]] const CVulkanKeyboard& keyboard() const noexcept {
            return _keyboard;
        }

        [[nodiscard]] CVulkanMouse& mouse() noexcept {
            return _mouse;
        }

        [[nodiscard]] const CVulkanMouse& mouse() const noexcept {
            return _mouse;
        }

    private:
        CVulkanKeyboard _keyboard;
        CVulkanMouse _mouse;

        std::unique_ptr<CVulkanGlfwInputAdapter> _glfwAdapter;
    };

    extern std::unique_ptr<CVulkanInput> ClientInput;

    void setupInput(const window::CVulkanWindow& window);

    void updateInput();

    void destroyInput();
}
