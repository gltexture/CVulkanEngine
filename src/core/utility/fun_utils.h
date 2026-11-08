//
// Created by ded on 06.11.2026.
//

#pragma once

namespace cvulkan::fun_utils {
    template <typename T, typename... Args>
    requires(std::same_as<T, Args> && ...)
    std::array<std::reference_wrapper<const T>, 1 + sizeof...(Args)> packCRefs(const T& first, const Args&... args) {
        return {
            std::cref(first),
            std::cref(args)...
        };
    }

    /*
    *template<typename T, typename... Args>
    requires (std::same_as<T, Args> && ...)
auto makeRefs(const T& first, const Args&... args) {
    std::array<std::reference_wrapper<const T>, 1 + sizeof...(Args)> refs{
        std::cref(first), std::cref(args)...
    };
    return refs;
}
     */
}