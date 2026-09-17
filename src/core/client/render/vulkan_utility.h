#pragma once
#include <filesystem>
#include <spdlog/spdlog.h>
#include <vulkan/vk_enum_string_helper.h>

#include "util/logger.h"

#define DEBUG_MODE

namespace cvulkan::utility {
    inline const std::filesystem::path RESOURCES_FOLDER {"resources"};
    inline const std::filesystem::path SHADERS_FOLDER = RESOURCES_FOLDER / "shaders";
    inline const std::filesystem::path SHADERS_FOLDER_SPV = RESOURCES_FOLDER / "shaders" / "compiled";

#define CVULKAN_NO_COPY(Type) \
    Type(const Type&) = delete; \
    Type& operator=(const Type&) = delete;

#define CVULKAN_NO_COPY_NO_MOVE(Type) \
    Type(const Type&) = delete; \
    Type& operator=(const Type&) = delete; \
    Type(Type&&) noexcept = default; \
    Type& operator=(Type&&) = delete;

    enum OSType {
        MAC,
        WIN,
        LINUX,
        UNDEFINED
    };

#if defined(_WIN32)
    inline constexpr OSType osType = WIN;
#elif defined(__linux__)
    inline constexpr OSType osType = LINUX;
#elif defined(__APPLE__)
    inline constexpr OSType osType = MAC;
#else
    inline constexpr OSType osType = UNDEFINED;
#endif

#if defined(DEBUG_MODE)
    inline constexpr bool debug_mode = true;
#else
    inline constexpr bool debug_mode = false;
#endif

    constexpr bool checkOS(const OSType osTypeToCheck) {
        return osTypeToCheck == osType;
    }

    inline void vkCheck(const VkResult vk_result, std::string_view errMsg) {
        if (vk_result != VK_SUCCESS) {
            throw std::runtime_error{std::format("{} -> {}", string_VkResult(vk_result), errMsg)};
        }
    }

    inline void vkCheck(const VkResult vk_result) {
        vkCheck(vk_result, std::string_view("Unknown"));
    }

    inline void utilityInitialize() {
        if constexpr (debug_mode) {
            spdlog::set_level(spdlog::level::debug);
            logging::debug("DEBUG MODE");
        }
    }

    void imageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask,
        VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask, VkImageAspectFlags aspectMask);
}
