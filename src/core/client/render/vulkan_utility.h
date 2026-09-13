#pragma once
#include <spdlog/spdlog.h>
#include <vulkan/vk_enum_string_helper.h>

#include "util/logger.h"

#define DEBUG_MODE

namespace cvulkan::utility {
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

    inline void vkCheck(const VkResult vk_result, const std::string_view errMsg) {
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

    void imageBarrier(const VkCommandBuffer commandBuffer, const VkImage image, const VkImageLayout oldLayout, const VkImageLayout newLayout, const VkPipelineStageFlags2 srcStageMask, const VkPipelineStageFlags2 dstStageMask,
        const VkAccessFlags2 srcAccessMask, const VkAccessFlags2 dstAccessMask, const VkImageAspectFlags aspectMask);
}
