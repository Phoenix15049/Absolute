#pragma once

#include <functional>
#include <string>

enum class LogLevel
{
	Info,
	Warning,
	Error
};

struct LogEntry
{
	LogLevel level;
	std::string time;
	std::string message;
};

namespace Log
{
	void Info(const std::string& message);
	void Warning(const std::string& message);
	void Error(const std::string& message);

	void Clear();
	size_t GetCount();
	unsigned int GetVersion();
	void ForEach(const std::function<void(const LogEntry&)>& callback);
}
