//
// Created by ded on 22.10.2026.
//

#include "io.h"
#include <Windows.h>

#include "exceptions.h"

namespace cvulkan::io {
#if defined(_WIN32)
    std::filesystem::path getExecutableDirectory() {
        static const std::filesystem::path cachedPath = [] {
            std::vector<wchar_t> buffer(512);
            while (true) {
                const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
                if (length == 0) {
                    throw exc::CVulkanException("Failed to get executable path");
                }
                if (length < buffer.size()) {
                    return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
                }
                buffer.resize(buffer.size() * 2);
            }
        }();
        return cachedPath;
    }
#elif defined(_LINUX)
    //TODO
#endif


    std::filesystem::path CVulkanResourceResolver::resolve(const std::filesystem::path& resourcePath) const {
        const auto relativePath = resourcePath.lexically_normal();
        if (relativePath.empty() || relativePath.is_absolute()) {
            throw exc::CVulkanException("Invalid resource path");
        }
        for (const auto& component: relativePath) {
            if (component == "..") {
                throw exc::CVulkanException("Resource path escapes the root");
            }
        }
        return (this->_root / relativePath).lexically_normal();
    }
}
