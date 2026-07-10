#pragma once
#include <string>

namespace cvulkan::logging
{
	template<typename... Args>
	void trace(const std::string& message, const Args&... args);

	template<typename... Args>
	void debug(const std::string& message, const Args&... args);

	template<typename... Args>
	void info(const std::string& message, const Args&... args);

	template<typename... Args>
	void warn(const std::string& message, const Args&... args);

	template<typename... Args>
	void error(const std::string& message, const Args&... args);

	template<typename... Args>
	void critical(const std::string& message, const Args&... args);
}

#include "logger.inl"
