#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <SDL.h>
#include <SDL_syswm.h>
#include "Core/Platform.h"

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#endif

namespace fs = std::filesystem;

namespace
{
	SDL_Window* s_window = nullptr;

#ifdef _WIN32
	HWND GetOwner()
	{
		if (!s_window)
		{
			return nullptr;
		}

		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);

		return SDL_GetWindowWMInfo(s_window, &info) ? info.info.win.window : nullptr;
	}
#endif
}

void Platform::SetMainWindow(SDL_Window* window)
{
	s_window = window;
}

std::string Platform::OpenFileDialog(const char* title, const char* filter)
{
#ifdef _WIN32
	char buffer[MAX_PATH] = {};

	OPENFILENAMEA dialog = {};
	dialog.lStructSize = sizeof(dialog);
	dialog.hwndOwner = GetOwner();
	dialog.lpstrFilter = filter;
	dialog.lpstrFile = buffer;
	dialog.nMaxFile = MAX_PATH;
	dialog.lpstrTitle = title;
	dialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	return GetOpenFileNameA(&dialog) ? MakeRelative(buffer) : std::string();
#else
	return std::string();
#endif
}

std::string Platform::SaveFileDialog(const char* title, const char* filter, const char* defaultExtension)
{
#ifdef _WIN32
	char buffer[MAX_PATH] = {};

	OPENFILENAMEA dialog = {};
	dialog.lStructSize = sizeof(dialog);
	dialog.hwndOwner = GetOwner();
	dialog.lpstrFilter = filter;
	dialog.lpstrFile = buffer;
	dialog.nMaxFile = MAX_PATH;
	dialog.lpstrTitle = title;
	dialog.lpstrDefExt = defaultExtension;
	dialog.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

	return GetSaveFileNameA(&dialog) ? MakeRelative(buffer) : std::string();
#else
	return std::string();
#endif
}

std::string Platform::MakeRelative(const std::string& path)
{
	if (path.empty())
	{
		return path;
	}

	std::error_code error;
	const fs::path absolute = fs::weakly_canonical(fs::path(path), error);
	const fs::path current = fs::current_path(error);
	const fs::path relative = absolute.lexically_relative(current);

	if (relative.empty() || *relative.begin() == "..")
	{
		return absolute.generic_string();
	}

	return relative.generic_string();
}

std::string Platform::GetFileName(const std::string& path)
{
	return fs::path(path).filename().string();
}

std::string Platform::GetFileStem(const std::string& path)
{
	return fs::path(path).stem().string();
}

std::string Platform::GetDirectory(const std::string& path)
{
	return fs::path(path).parent_path().generic_string();
}

std::string Platform::GetExtension(const std::string& path)
{
	std::string extension = fs::path(path).extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return extension;
}

bool Platform::FileExists(const std::string& path)
{
	std::error_code error;
	return !path.empty() && fs::is_regular_file(fs::path(path), error);
}

bool Platform::CreateDirectories(const std::string& path)
{
	std::error_code error;
	fs::create_directories(fs::path(path), error);
	return !error;
}

std::string Platform::GetTimestamp()
{
	const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm localTime{};
	localtime_s(&localTime, &now);

	char buffer[32];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", &localTime);
	return buffer;
}

void Platform::OpenInExplorer(const std::string& path)
{
#ifdef _WIN32
	std::error_code error;
	const std::string absolute = fs::absolute(fs::path(path), error).string();
	ShellExecuteA(nullptr, "open", absolute.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
#endif
}
