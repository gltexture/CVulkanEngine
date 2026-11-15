//
// Created by ded on 14.11.2026.
//

#pragma once
#include <source_location>

namespace cvulkan::DEBUG {
    template<typename... Args>
    bool CHECK(std::source_location location, bool condition, std::string_view message, Args&&... args);

    template<typename... Args>
    void ASSERT(std::source_location location, bool condition, std::string_view message, Args&&... args);

    template<typename... Args>
    bool CHECK(bool condition, std::string_view message, Args&&... args) {
        return CHECK(std::source_location::current(), condition, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void ASSERT(bool condition, std::string_view message, Args&&... args) {
        ASSERT(std::source_location::current(), condition, message, std::forward<Args>(args)...);
    }
}

#include "DEBUG.tpp"