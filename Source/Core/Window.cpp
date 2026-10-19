#include <SDL_image.h>
#include <gl.h>
#include "Core/Log.h"
#include "Core/Window.h"

Window::Window()
{
	m_window = nullptr;
	m_context = nullptr;
}

Window::~Window()
{
	Destroy();
}

bool Window::Create(const std::string& title, int width, int height, bool maximized)
{
	SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
	{
		Log::Error(std::string("Error initializing SDL: ") + SDL_GetError());
		return false;
	}

	const int imageFlags = IMG_INIT_JPG | IMG_INIT_PNG;

	if ((IMG_Init(imageFlags) & imageFlags) != imageFlags)
	{
		Log::Warning(std::string("SDL_image could not initialize every format: ") + IMG_GetError());
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;

	if (maximized)
	{
		flags |= SDL_WINDOW_MAXIMIZED;
	}

	m_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);

	if (!m_window)
	{
		Log::Error(std::string("Error creating window: ") + SDL_GetError());
		return false;
	}

	SDL_SetWindowMinimumSize(m_window, 800, 500);

	m_context = SDL_GL_CreateContext(m_window);

	if (!m_context)
	{
		Log::Error(std::string("Error creating OpenGL 3.3 context: ") + SDL_GetError());
		return false;
	}

	if (!gladLoaderLoadGL())
	{
		Log::Error("Error loading OpenGL functions.");
		return false;
	}

	Log::Info(std::string("OpenGL ") + reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	Log::Info(std::string("GPU: ") + reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

	return true;
}

void Window::Destroy()
{
	if (m_context)
	{
		SDL_GL_DeleteContext(m_context);
		m_context = nullptr;
	}

	if (m_window)
	{
		SDL_DestroyWindow(m_window);
		m_window = nullptr;

		IMG_Quit();
		SDL_Quit();
	}
}

void Window::SwapBuffers()
{
	SDL_GL_SwapWindow(m_window);
}

void Window::SetTitle(const std::string& title)
{
	SDL_SetWindowTitle(m_window, title.c_str());
}

void Window::SetVSync(bool enabled)
{
	SDL_GL_SetSwapInterval(enabled ? 1 : 0);
}

void Window::SetFullscreen(bool enabled)
{
	SDL_SetWindowFullscreen(m_window, enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

bool Window::IsMinimized() const
{
	return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED) != 0;
}

bool Window::IsMaximized() const
{
	return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MAXIMIZED) != 0;
}

bool Window::IsFocused() const
{
	return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

void Window::GetWindowSize(int& width, int& height) const
{
	SDL_GetWindowSize(m_window, &width, &height);
}

void Window::GetDrawableSize(int& width, int& height) const
{
	SDL_GL_GetDrawableSize(m_window, &width, &height);
}

SDL_Window* Window::GetHandle() const
{
	return m_window;
}

SDL_GLContext Window::GetContext() const
{
	return m_context;
}
