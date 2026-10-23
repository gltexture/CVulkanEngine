//
// Created by ded on 22.10.2026.
//

#pragma once

namespace cvulkan::sys_utils {
#define CLASS_NO_COPY(Type) \
Type(const Type&) = delete; \
Type& operator=(const Type&) = delete;

#define CLASS_NO_COPY_NO_MOVE(Type) \
Type(const Type&) = delete; \
Type& operator=(const Type&) = delete; \
Type(Type&&) = delete; \
Type& operator=(Type&&) = delete;

#define CLASS_NO_COPY_NO_ASSIGN_MOVE(Type) \
Type(const Type&) = delete; \
Type& operator=(const Type&) = delete; \
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

#if defined(CVULKAN_DEBUG_MODE)
    inline constexpr bool debug_mode = true;
#else
    inline constexpr bool debug_mode = false;
#endif

    constexpr bool checkOS(const OSType osTypeToCheck) {
        return osTypeToCheck == osType;
    }

    void utilityInitialize();
}