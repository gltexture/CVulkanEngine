//
// Created by ded on 14.11.2026.
//
#pragma once

#include "exceptions.h"
#include "logging.h"
#include <source_location>

#ifdef NDEBUG
    #define CVULKAN_ASSERT(expression) ((void)0)
#else
    #define CVULKAN_ASSERT(expression) assert(expression)
#endif

namespace cvulkan::DEBUG {
    template <typename... Args>
    bool CHECK(const std::source_location location, const bool condition, const fmt::format_string<Args...> message, Args&&... args) {
        if (condition) {
            return true;
        }
        log::error("[File: {}, Line: {}, Function: {}]", location.file_name(), location.line(), location.function_name());
        log::error("{}", fmt::format(message, std::forward<Args>(args)...));
        return false;
    }

    template <typename... Args>
    void ASSERT(const std::source_location location, const bool condition, const fmt::format_string<Args...> message, Args&&... args) {
        CVULKAN_ASSERT(condition);
        if (condition) {
            return;
        }
        log::error("[File: {}, Line: {}, Function: {}]", location.file_name(), location.line(), location.function_name());
        throw exc::CVulkanException(fmt::format(message, std::forward<Args>(args)...));
    }
}