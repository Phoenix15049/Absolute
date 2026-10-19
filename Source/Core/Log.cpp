#include <chrono>
#include <ctime>
#include <deque>
#include <iostream>
#include <mutex>
#include "Core/Log.h"

namespace
{
	const size_t MaxEntries = 1000;

	std::mutex s_mutex;
	std::deque<LogEntry> s_entries;
	unsigned int s_version = 0;

	std::string CurrentTime()
	{
		const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm localTime{};
		localtime_s(&localTime, &now);

		char buffer[16];
		std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &localTime);
		return buffer;
	}

	void Add(LogLevel level, const std::string& message)
	{
		static const char* prefixes[] = { "[Info] ", "[Warning] ", "[Error] " };

		std::lock_guard<std::mutex> lock(s_mutex);

		std::cout << prefixes[static_cast<int>(level)] << message << std::endl;

		s_entries.push_back({ level, CurrentTime(), message });
		s_version++;

		if (s_entries.size() > MaxEntries)
		{
			s_entries.pop_front();
		}
	}
}

void Log::Info(const std::string& message)
{
	Add(LogLevel::Info, message);
}

void Log::Warning(const std::string& message)
{
	Add(LogLevel::Warning, message);
}

void Log::Error(const std::string& message)
{
	Add(LogLevel::Error, message);
}

void Log::Clear()
{
	std::lock_guard<std::mutex> lock(s_mutex);
	s_entries.clear();
	s_version++;
}

size_t Log::GetCount()
{
	std::lock_guard<std::mutex> lock(s_mutex);
	return s_entries.size();
}

unsigned int Log::GetVersion()
{
	std::lock_guard<std::mutex> lock(s_mutex);
	return s_version;
}

void Log::ForEach(const std::function<void(const LogEntry&)>& callback)
{
	std::lock_guard<std::mutex> lock(s_mutex);

	for (const auto& entry : s_entries)
	{
		callback(entry);
	}
}
