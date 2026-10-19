#pragma once

#include <string>

struct SDL_Window;

namespace Platform
{
	void SetMainWindow(SDL_Window* window);

	std::string OpenFileDialog(const char* title, const char* filter);
	std::string SaveFileDialog(const char* title, const char* filter, const char* defaultExtension);

	std::string MakeRelative(const std::string& path);
	std::string GetFileName(const std::string& path);
	std::string GetFileStem(const std::string& path);
	std::string GetDirectory(const std::string& path);
	std::string GetExtension(const std::string& path);
	bool FileExists(const std::string& path);
	bool CreateDirectories(const std::string& path);

	std::string GetTimestamp();
	void OpenInExplorer(const std::string& path);
}
