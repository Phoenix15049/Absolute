#pragma once

#include <string>
#include <SDL.h>

class Window
{

public:

	Window();
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	bool Create(const std::string& title, int width, int height, bool maximized);
	void Destroy();

	void SwapBuffers();

	void SetTitle(const std::string& title);
	void SetVSync(bool enabled);
	void SetFullscreen(bool enabled);

	bool IsMinimized() const;
	bool IsMaximized() const;
	bool IsFocused() const;

	void GetWindowSize(int& width, int& height) const;
	void GetDrawableSize(int& width, int& height) const;

	SDL_Window* GetHandle() const;
	SDL_GLContext GetContext() const;

private:

	SDL_Window* m_window;
	SDL_GLContext m_context;

};
