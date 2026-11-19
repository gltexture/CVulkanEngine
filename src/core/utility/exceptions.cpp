//
// Created by gltex on 09.11.2026.
//

#include "exceptions.h"

#include "CrashCatch.hpp"
#include "io.h"

namespace cvulkan::exc {
    void crash::CCrashReporter::initialize() {
        CrashCatch::Config config{};

        config.appVersion = "1.0.0";
        config.buildConfig = "Release";

        config.dumpFolder = io::_OUT_RESOLVER / io::CRASH_LOGS;

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