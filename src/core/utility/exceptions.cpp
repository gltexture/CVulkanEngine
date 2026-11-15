//
// Created by gltex on 09.11.2026.
//

#include "exceptions.h"

#include "CrashCatch.hpp"
#include "io_utils.h"

namespace cvulkan::exc {
    void crash::CCrashReporter::initialize() {
        CrashCatch::Config config{};

        config.appVersion = "1.0.0";
        config.buildConfig = "Release";

        config.dumpFolder = io_utils::_OUT_RESOLVER / io_utils::CVulkanResourcePath{"crash_logs"};

        config.enableTextLog = true;
        config.includeStackTrace = true;

        config.showCrashDialog = false;

        config.additionalNotes = "CVulkanEngine";

        config.onCrash = [](const CrashCatch::CrashContext& ctx) {
            std::cerr << "Crash detected!\n";
            std::cerr << "Log: " << ctx.logFilePath.string() << '\n';
            std::cerr << "Dump: " << ctx.dumpFilePath.string() << '\n';
        };

        CrashCatch::initialize(config);
    }
}