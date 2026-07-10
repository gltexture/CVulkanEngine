#pragma once
#include "logger.h"
#include <spdlog/spdlog.h>

namespace cvulkan::logging
{
	template <typename... Args>
	void trace(const std::string& message, const Args&... args)
	{
		spdlog::trace(fmt::runtime(message), args...);
	}

	template <typename... Args>
	void debug(const std::string& message, const Args&... args)
	{
		spdlog::debug(fmt::runtime(message), args...);
	}

	template <typename... Args>
	void info(const std::string& message, const Args&... args)
	{
		spdlog::info(fmt::runtime(message), args...);
	}

	template <typename... Args>
	void warn(const std::string& message, const Args&... args)
	{
		spdlog::warn(fmt::runtime(message), args...);
	}

	template <typename... Args>
	void error(const std::string& message, const Args&... args)
	{
		spdlog::error(fmt::runtime(message), args...);
	}

	template <typename... Args>
	void critical(const std::string& message, const Args&... args)
	{
		spdlog::critical(fmt::runtime(message), args...);
	}
}