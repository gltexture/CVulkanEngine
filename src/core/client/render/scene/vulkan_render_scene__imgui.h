//
// Created by gltex on 08.11.2026.
//

#pragma once
#include <imgui.h>

#include "utility/sys_utils.h"
#include "client/render/vulkan_buffers.h"
#include "client/render/vulkan_config.hpp"
#include "utility/io_utils.h"

namespace cvulkan::client::render::cache {
    class CVulkanCacheCollection;
}

namespace cvulkan::client::render::asset {
    class CVulkanTexture2D;
}

namespace cvulkan::client::render::vk {
    class CVulkanDescriptorAllocator;
    class CVulkanContext;
}

namespace cvulkan::client::render::scene::imgui {
    constexpr std::string_view DESC_ID_FONT_TEXTURE = "DESC_ID_FONT_TEXTURE";
    constexpr VkFormat COLOR_FORMAT {VK_FORMAT_R16G16B16A16_SFLOAT};

    struct ImGuiTexturePtr {
        VkDescriptorSet descriptorSet;
    };

    struct CVulkanImGuiTextureData {
    private:
        inline static std::atomic_uint64_t _nextId{0};

    public:
        ImTextureID _id;
        io_utils::CVulkanResourcePath _texturePath;

        explicit CVulkanImGuiTextureData(io_utils::CVulkanResourcePath texturePath) : _id(_nextId.fetch_add(1, std::memory_order_relaxed)), _texturePath(std::move(texturePath)) {}
    };

    class CVulkanImGuiRenderer {
    public:
        CVulkanImGuiRenderer(const vk::CVulkanContext& context, vk::CVulkanDescriptorAllocator& descriptorAllocator, const vk::CVulkanAttachment2D& dstAttachment, cache::CVulkanCacheCollection& caches);
        ~CVulkanImGuiRenderer();

        CLASS_NO_COPY_NO_MOVE(CVulkanImGuiRenderer);

        void renderImGui(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanAttachment2D& dstAttachment, uint32_t imageIndex);

        void onResize(const vk::CVulkanAttachment2D& dstAttachment);

        void loadTextures(std::span<CVulkanImGuiTextureData> textures);

    protected:
        void renderImGuiWindow();

        void createRenderingData(const vk::CVulkanAttachment2D& dstAttachment);
        void initImGui();
        void updateBuffers(uint32_t idx);

    private:
        const vk::CVulkanContext& _context;
        vk::CVulkanDescriptorAllocator& _descriptorAllocator;
        cache::CVulkanCacheCollection& _caches;
        VkRenderingAttachmentInfo _colorAttachmentInfo {};
        VkRenderingInfo _renderingInfo {};
        std::array<std::unique_ptr<vk::CVulkanBuffer>, renderConfig::MAX_FRAMES_IN_FLIGHT> _vtxBuffers{};
        std::array<std::unique_ptr<vk::CVulkanBuffer>, renderConfig::MAX_FRAMES_IN_FLIGHT> _idxBuffers{};
        std::unique_ptr<vk::CVulkanSampler> _fontTextureSampler {};
        std::unique_ptr<asset::CVulkanTexture2D> _fontTexture {};
        std::unique_ptr<vk::CVulkanPipeline> _imguiPipeline {};
        std::unique_ptr<vk::CVulkanDescriptorSetLayout> _descriptorSetLayout_texture {};
        std::unordered_map<ImTextureID, VkDescriptorSet> _guiTextures;
        std::unique_ptr<ImGuiTexturePtr> _texturePtr {};
    };

    namespace imgui_utils {
        inline ImGuiKey getImKey(const int key) {
            switch (key) {
                case GLFW_KEY_TAB: return ImGuiKey_Tab;
                case GLFW_KEY_LEFT: return ImGuiKey_LeftArrow;
                case GLFW_KEY_RIGHT: return ImGuiKey_RightArrow;
                case GLFW_KEY_UP: return ImGuiKey_UpArrow;
                case GLFW_KEY_DOWN: return ImGuiKey_DownArrow;
                case GLFW_KEY_PAGE_UP: return ImGuiKey_PageUp;
                case GLFW_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
                case GLFW_KEY_HOME: return ImGuiKey_Home;
                case GLFW_KEY_END: return ImGuiKey_End;
                case GLFW_KEY_INSERT: return ImGuiKey_Insert;
                case GLFW_KEY_DELETE: return ImGuiKey_Delete;
                case GLFW_KEY_BACKSPACE: return ImGuiKey_Backspace;
                case GLFW_KEY_SPACE: return ImGuiKey_Space;
                case GLFW_KEY_ENTER: return ImGuiKey_Enter;
                case GLFW_KEY_ESCAPE: return ImGuiKey_Escape;
                case GLFW_KEY_APOSTROPHE: return ImGuiKey_Apostrophe;
                case GLFW_KEY_COMMA: return ImGuiKey_Comma;
                case GLFW_KEY_MINUS: return ImGuiKey_Minus;
                case GLFW_KEY_PERIOD: return ImGuiKey_Period;
                case GLFW_KEY_SLASH: return ImGuiKey_Slash;
                case GLFW_KEY_SEMICOLON: return ImGuiKey_Semicolon;
                case GLFW_KEY_EQUAL: return ImGuiKey_Equal;
                case GLFW_KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
                case GLFW_KEY_BACKSLASH: return ImGuiKey_Backslash;
                case GLFW_KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
                case GLFW_KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
                case GLFW_KEY_CAPS_LOCK: return ImGuiKey_CapsLock;
                case GLFW_KEY_SCROLL_LOCK: return ImGuiKey_ScrollLock;
                case GLFW_KEY_NUM_LOCK: return ImGuiKey_NumLock;
                case GLFW_KEY_PRINT_SCREEN: return ImGuiKey_PrintScreen;
                case GLFW_KEY_PAUSE: return ImGuiKey_Pause;
                case GLFW_KEY_KP_0: return ImGuiKey_Keypad0;
                case GLFW_KEY_KP_1: return ImGuiKey_Keypad1;
                case GLFW_KEY_KP_2: return ImGuiKey_Keypad2;
                case GLFW_KEY_KP_3: return ImGuiKey_Keypad3;
                case GLFW_KEY_KP_4: return ImGuiKey_Keypad4;
                case GLFW_KEY_KP_5: return ImGuiKey_Keypad5;
                case GLFW_KEY_KP_6: return ImGuiKey_Keypad6;
                case GLFW_KEY_KP_7: return ImGuiKey_Keypad7;
                case GLFW_KEY_KP_8: return ImGuiKey_Keypad8;
                case GLFW_KEY_KP_9: return ImGuiKey_Keypad9;
                case GLFW_KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
                case GLFW_KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
                case GLFW_KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
                case GLFW_KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
                case GLFW_KEY_KP_ADD: return ImGuiKey_KeypadAdd;
                case GLFW_KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
                case GLFW_KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
                case GLFW_KEY_LEFT_SHIFT: return ImGuiKey_LeftShift;
                case GLFW_KEY_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
                case GLFW_KEY_LEFT_ALT: return ImGuiKey_LeftAlt;
                case GLFW_KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
                case GLFW_KEY_RIGHT_SHIFT: return ImGuiKey_RightShift;
                case GLFW_KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
                case GLFW_KEY_RIGHT_ALT: return ImGuiKey_RightAlt;
                case GLFW_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
                case GLFW_KEY_MENU: return ImGuiKey_Menu;
                case GLFW_KEY_0: return ImGuiKey_0;
                case GLFW_KEY_1: return ImGuiKey_1;
                case GLFW_KEY_2: return ImGuiKey_2;
                case GLFW_KEY_3: return ImGuiKey_3;
                case GLFW_KEY_4: return ImGuiKey_4;
                case GLFW_KEY_5: return ImGuiKey_5;
                case GLFW_KEY_6: return ImGuiKey_6;
                case GLFW_KEY_7: return ImGuiKey_7;
                case GLFW_KEY_8: return ImGuiKey_8;
                case GLFW_KEY_9: return ImGuiKey_9;
                case GLFW_KEY_A: return ImGuiKey_A;
                case GLFW_KEY_B: return ImGuiKey_B;
                case GLFW_KEY_C: return ImGuiKey_C;
                case GLFW_KEY_D: return ImGuiKey_D;
                case GLFW_KEY_E: return ImGuiKey_E;
                case GLFW_KEY_F: return ImGuiKey_F;
                case GLFW_KEY_G: return ImGuiKey_G;
                case GLFW_KEY_H: return ImGuiKey_H;
                case GLFW_KEY_I: return ImGuiKey_I;
                case GLFW_KEY_J: return ImGuiKey_J;
                case GLFW_KEY_K: return ImGuiKey_K;
                case GLFW_KEY_L: return ImGuiKey_L;
                case GLFW_KEY_M: return ImGuiKey_M;
                case GLFW_KEY_N: return ImGuiKey_N;
                case GLFW_KEY_O: return ImGuiKey_O;
                case GLFW_KEY_P: return ImGuiKey_P;
                case GLFW_KEY_Q: return ImGuiKey_Q;
                case GLFW_KEY_R: return ImGuiKey_R;
                case GLFW_KEY_S: return ImGuiKey_S;
                case GLFW_KEY_T: return ImGuiKey_T;
                case GLFW_KEY_U: return ImGuiKey_U;
                case GLFW_KEY_V: return ImGuiKey_V;
                case GLFW_KEY_W: return ImGuiKey_W;
                case GLFW_KEY_X: return ImGuiKey_X;
                case GLFW_KEY_Y: return ImGuiKey_Y;
                case GLFW_KEY_Z: return ImGuiKey_Z;
                case GLFW_KEY_F1: return ImGuiKey_F1;
                case GLFW_KEY_F2: return ImGuiKey_F2;
                case GLFW_KEY_F3: return ImGuiKey_F3;
                case GLFW_KEY_F4: return ImGuiKey_F4;
                case GLFW_KEY_F5: return ImGuiKey_F5;
                case GLFW_KEY_F6: return ImGuiKey_F6;
                case GLFW_KEY_F7: return ImGuiKey_F7;
                case GLFW_KEY_F8: return ImGuiKey_F8;
                case GLFW_KEY_F9: return ImGuiKey_F9;
                case GLFW_KEY_F10: return ImGuiKey_F10;
                case GLFW_KEY_F11: return ImGuiKey_F11;
                case GLFW_KEY_F12: return ImGuiKey_F12;
                default: return ImGuiKey_None;
            }
        }
    }
}
