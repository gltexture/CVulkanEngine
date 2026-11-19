#pragma once
#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "io.h"

namespace cvulkan::log {
    template<typename... Args>
    void trace(const std::string& message, const Args&... args) noexcept;

    template<typename... Args>
    void debug(const std::string& message, const Args&... args) noexcept;

    template<typename... Args>
    void info(const std::string& message, const Args&... args) noexcept;

    template<typename... Args>
    void warn(const std::string& message, const Args&... args) noexcept;

    template<typename... Args>
    void error(const std::string& message, const Args&... args) noexcept;

    template<typename... Args>
    void critical(const std::string& message, const Args&... args) noexcept;

    inline void init() {
        {
            const auto now = std::chrono::system_clock::now();
            const auto time = std::chrono::system_clock::to_time_t(now);

            std::ostringstream filename {};
            filename << "log_" << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S") << ".txt";

            const std::filesystem::path fileFullPath = io::_OUT_RESOLVER / io::LOGS / filename.str();

            std::vector<spdlog::sink_ptr> sinks{};
            const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            const auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(fileFullPath.string(), 10 * 1024 * 1024, 5);

            sinks.push_back(consoleSink);
            sinks.push_back(fileSink);

            auto logger = std::make_shared<spdlog::logger>("CVulkanEngine", sinks.begin(), sinks.end());

            logger->set_level(spdlog::level::trace);
            if constexpr (system::debug_mode) {
                logger->set_level(spdlog::level::debug);
                logger->debug("Debug mode enabled");
            }
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
            logger->flush_on(spdlog::level::err);

            spdlog::set_default_logger(std::move(logger));
        }
    }
}

#include "logging.inl"
