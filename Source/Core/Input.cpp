#include <cstring>
#include "Core/Input.h"

Input* Input::Instance()
{
	static Input input;
	return &input;
}

Input::Input()
{
	m_isQuitRequested = false;

	std::memset(m_keys, 0, sizeof(m_keys));
	std::memset(m_previousKeys, 0, sizeof(m_previousKeys));

	m_mouseButtons = 0;
	m_previousMouseButtons = 0;

	m_mousePositionX = 0;
	m_mousePositionY = 0;

	m_mouseMotionX = 0;
	m_mouseMotionY = 0;

	m_mouseWheel = 0.0f;
}

void Input::Update(const std::function<void(const SDL_Event&)>& eventHandler)
{
	std::memcpy(m_previousKeys, m_keys, sizeof(m_keys));
	m_previousMouseButtons = m_mouseButtons;

	m_mouseMotionX = 0;
	m_mouseMotionY = 0;
	m_mouseWheel = 0.0f;
	m_droppedFiles.clear();

	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		if (eventHandler)
		{
			eventHandler(event);
		}

		switch (event.type)
		{
			case SDL_QUIT:
				m_isQuitRequested = true;
				break;

			case SDL_MOUSEMOTION:
				m_mouseMotionX += event.motion.xrel;
				m_mouseMotionY += event.motion.yrel;
				break;

			case SDL_MOUSEWHEEL:
				m_mouseWheel += event.wheel.preciseY;
				break;

			case SDL_DROPFILE:
				m_droppedFiles.push_back(event.drop.file);
				SDL_free(event.drop.file);
				break;
		}
	}

	int totalKeys = 0;
	const Uint8* keys = SDL_GetKeyboardState(&totalKeys);
	std::memcpy(m_keys, keys, totalKeys < SDL_NUM_SCANCODES ? totalKeys : SDL_NUM_SCANCODES);

	m_mouseButtons = SDL_GetMouseState(&m_mousePositionX, &m_mousePositionY);
}

bool Input::IsQuitRequested() const
{
	return m_isQuitRequested;
}

void Input::ClearQuitRequest()
{
	m_isQuitRequested = false;
}

bool Input::IsKeyHeld(SDL_Scancode key) const
{
	return m_keys[key] != 0;
}

bool Input::IsKeyPressed(SDL_Scancode key) const
{
	return m_keys[key] != 0 && m_previousKeys[key] == 0;
}

bool Input::IsCtrlHeld() const
{
	return IsKeyHeld(SDL_SCANCODE_LCTRL) || IsKeyHeld(SDL_SCANCODE_RCTRL);
}

bool Input::IsShiftHeld() const
{
	return IsKeyHeld(SDL_SCANCODE_LSHIFT) || IsKeyHeld(SDL_SCANCODE_RSHIFT);
}

bool Input::IsAltHeld() const
{
	return IsKeyHeld(SDL_SCANCODE_LALT) || IsKeyHeld(SDL_SCANCODE_RALT);
}

bool Input::IsMouseHeld(int button) const
{
	return (m_mouseButtons & SDL_BUTTON(button)) != 0;
}

bool Input::IsMousePressed(int button) const
{
	return (m_mouseButtons & SDL_BUTTON(button)) && !(m_previousMouseButtons & SDL_BUTTON(button));
}

bool Input::IsMouseReleased(int button) const
{
	return !(m_mouseButtons & SDL_BUTTON(button)) && (m_previousMouseButtons & SDL_BUTTON(button));
}

int Input::GetMousePositionX() const
{
	return m_mousePositionX;
}

int Input::GetMousePositionY() const
{
	return m_mousePositionY;
}

int Input::GetMouseMotionX() const
{
	return m_mouseMotionX;
}

int Input::GetMouseMotionY() const
{
	return m_mouseMotionY;
}

float Input::GetMouseWheel() const
{
	return m_mouseWheel;
}

const std::vector<std::string>& Input::GetDroppedFiles() const
{
	return m_droppedFiles;
}
