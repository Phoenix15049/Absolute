#pragma once

#include <functional>
#include <string>
#include <vector>
#include <SDL.h>

class Input
{

public:

	static Input* Instance();

	void Update(const std::function<void(const SDL_Event&)>& eventHandler);

	bool IsQuitRequested() const;
	void ClearQuitRequest();

	bool IsKeyHeld(SDL_Scancode key) const;
	bool IsKeyPressed(SDL_Scancode key) const;

	bool IsCtrlHeld() const;
	bool IsShiftHeld() const;
	bool IsAltHeld() const;

	bool IsMouseHeld(int button) const;
	bool IsMousePressed(int button) const;
	bool IsMouseReleased(int button) const;

	int GetMousePositionX() const;
	int GetMousePositionY() const;

	int GetMouseMotionX() const;
	int GetMouseMotionY() const;

	float GetMouseWheel() const;

	const std::vector<std::string>& GetDroppedFiles() const;

private:

	Input();
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	bool m_isQuitRequested;

	Uint8 m_keys[SDL_NUM_SCANCODES];
	Uint8 m_previousKeys[SDL_NUM_SCANCODES];

	Uint32 m_mouseButtons;
	Uint32 m_previousMouseButtons;

	int m_mousePositionX;
	int m_mousePositionY;

	int m_mouseMotionX;
	int m_mouseMotionY;

	float m_mouseWheel;

	std::vector<std::string> m_droppedFiles;

};
