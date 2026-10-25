//
// Created by ded on 22.10.2026.
//

#include "sys_utils.h"

#include "logger.h"

namespace cvulkan::sys_utils {
    void utilityInitialize() {
        if constexpr (debug_mode) {
            spdlog::set_level(spdlog::level::debug);
            logging::info("Debug mode enabled");
        }
    }
}
