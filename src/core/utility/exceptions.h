//
// Created by gltex on 09.11.2026.
//

#pragma once

namespace cvulkan::exc {
    class CVulkanException : public std::runtime_error {
    public:
        explicit CVulkanException(const std::string& message)
            : std::runtime_error(message), _stacktrace(std::stacktrace::current(1)) {
        }

        [[nodiscard]]
        const std::stacktrace& getStacktrace() const noexcept {
            return _stacktrace;
        }

    private:
        std::stacktrace _stacktrace;
    };
}
