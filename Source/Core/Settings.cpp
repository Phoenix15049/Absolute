#include <fstream>
#include <sstream>
#include <type_traits>
#include <unordered_map>
#include "Core/Log.h"
#include "Core/Settings.h"

namespace
{
	template<typename T>
	void ReadValue(const std::string& text, T& value)
	{
		std::istringstream stream(text);

		if constexpr (std::is_enum_v<T>)
		{
			int number = 0;

			if (stream >> number)
			{
				value = static_cast<T>(number);
			}
		}

		else
		{
			T result{};

			if (stream >> result)
			{
				value = result;
			}
		}
	}

	template<typename T>
	std::string WriteValue(const T& value)
	{
		if constexpr (std::is_enum_v<T>)
		{
			return std::to_string(static_cast<int>(value));
		}

		else
		{
			std::ostringstream stream;
			stream << value;
			return stream.str();
		}
	}
}

bool Settings::Load(const std::string& filename)
{
	std::ifstream file(filename);

	if (!file)
	{
		return false;
	}

	std::unordered_map<std::string, std::string> values;
	std::string line;

	while (std::getline(file, line))
	{
		const size_t separator = line.find('=');

		if (line.empty() || line[0] == '#' || separator == std::string::npos)
		{
			continue;
		}

		values[line.substr(0, separator)] = line.substr(separator + 1);
	}

	Visit([&values](const char* key, auto& value)
	{
		auto it = values.find(key);

		if (it != values.end())
		{
			ReadValue(it->second, value);
		}
	});

	Log::Info("Settings loaded from " + filename);
	return true;
}

bool Settings::Save(const std::string& filename) const
{
	std::ofstream file(filename);

	if (!file)
	{
		Log::Error("Could not save settings to " + filename);
		return false;
	}

	file << "# Absolute Engine settings" << std::endl;

	Visit([&file](const char* key, const auto& value)
	{
		file << key << "=" << WriteValue(value) << std::endl;
	});

	return true;
}
