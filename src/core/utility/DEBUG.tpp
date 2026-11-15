//
// Created by ded on 14.11.2026.
//
#pragma once

#include "exceptions.h"
#include "logger.h"
#include <source_location>

#ifdef NDEBUG
    #define CVULKAN_ASSERT(expression) ((void)0)
#else
    #define CVULKAN_ASSERT(expression) assert(expression)
#endif

namespace cvulkan::DEBUG {
    template <typename... Args>
    bool CHECK(const std::source_location location, const bool condition, const std::string_view message, Args&&... args) {
        if (condition) {
            return true;
        }
        logging::error("[File: {}, Line: {}, Function: {}]", location.file_name(), location.line(), location.function_name());
        logging::error(std::vformat(message, std::make_format_args(args...)));
        return false;
    }

    template <typename... Args>
    void ASSERT(const std::source_location location, const bool condition, const std::string_view message, Args&&... args) {
        CVULKAN_ASSERT(condition);
        if (!condition) {
            logging::error("[File: {}, Line: {}, Function: {}]", location.file_name(), location.line(), location.function_name());
            throw exc::CVulkanException(std::vformat(message, std::make_format_args(args...)));
        }
    }
}