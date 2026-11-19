#pragma once
#include "logging.h"
#include <spdlog/spdlog.h>

#include <string>
#include <utility>
#include <exception>
#include <iostream>

namespace cvulkan::log {
    template<typename Func>
    void safeLog(Func&& func) noexcept {
        try {
            std::forward<Func>(func)();
        }
        catch (const std::exception& e) {
            try {
                std::cerr << "[Logging Error] " << e.what() << '\n';
            }
            catch (...) {
                // Logging fallback failed.
            }
        }
        catch (...) {
            try {
                std::cerr << "[Logging Error] Unknown exception\n";
            }
            catch (...) {
                // Logging fallback failed.
            }
        }
    }

    template<typename... Args>
    void trace(const std::string& message, const Args&... args) noexcept {
        safeLog([&] {
            spdlog::trace(fmt::runtime(message), args...);
        });
    }

    template<typename... Args>
    void debug(const std::string& message, const Args&... args) noexcept {
        safeLog([&] {
            spdlog::debug(fmt::runtime(message), args...);
        });
    }

    template<typename... Args>
    void info(const std::string& message, const Args&... args) noexcept {
        safeLog([&] {
            spdlog::info(fmt::runtime(message), args...);
        });
    }

    template<typename... Args>
    void warn(const std::string& message, const Args&... args) noexcept {
        safeLog([&] {
            spdlog::warn(fmt::runtime(message), args...);
        });
    }

    template<typename... Args>
    void error(const std::string& message, const Args&... args) noexcept {
        safeLog([&] {
            spdlog::error(fmt::runtime(message), args...);
        });
    }

    template<typename... Args>
    void critical(const std::string& message, const Args&... args) noexcept {
        safeLog([&] {
            spdlog::critical(fmt::runtime(message), args...);
        });
    }
}
