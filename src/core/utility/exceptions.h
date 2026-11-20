//
// Created by gltex on 09.11.2026.
//

#pragma once

#include <stdexcept>
#include <stacktrace>
#include <string>
#include <utility>
#include <spdlog/fmt/fmt.h>

namespace cvulkan::exc {
    class CVulkanException : public std::runtime_error {
    public:
        explicit CVulkanException(const std::string& message)
            : std::runtime_error(message) , _stacktrace(std::stacktrace::current()) {
        }

        template<typename... Args>
        explicit CVulkanException(const fmt::format_string<Args...> message, Args&&... args)
            : CVulkanException(fmt::format(message, std::forward<Args>(args)...)) {
        }

        [[nodiscard]]
        const std::stacktrace& getStacktrace() const noexcept {
            return _stacktrace;
        }

    private:
        std::stacktrace _stacktrace;
    };

    namespace crash {
        class CCrashReporter {
        public:
            static void initialize();
        };
    }
}
