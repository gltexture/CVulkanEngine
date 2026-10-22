//
// Created by ded on 22.10.2026.
//

#include "io_utils.h"
#include <Windows.h>

namespace cvulkan::io_utils {
#if defined(_WIN32)
    std::filesystem::path getExecutableDirectory() {
        static const std::filesystem::path cachedPath = [] {
            std::vector<wchar_t> buffer(512);
            while (true) {
                const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
                if (length == 0) {
                    throw std::runtime_error("Failed to get executable path");
                }
                if (length < buffer.size()) {
                    return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
                }
                buffer.resize(buffer.size() * 2);
            }
        }();
        return cachedPath;
    }
#endif

    std::filesystem::path CVulkanResourceResolver::resolve(const CVulkanResourcePath& resourcePath) const {
        const auto relativePath = resourcePath.path().lexically_normal();
        if (relativePath.empty() || relativePath.is_absolute()) {
            throw std::invalid_argument("Invalid resource path");
        }
        for (const auto& component : relativePath) {
            if (component == "..") {
                throw std::invalid_argument("Resource path escapes the root");
            }
        }
        return (this->_root / relativePath).lexically_normal();
    }
}
