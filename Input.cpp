#include "Input.h"

Input* Input::Instance()
{
	static Input* input = new Input;
	return input;
}

bool Input::IsXClicked()
{
	return m_isXClicked;
}

bool Input::IsKeyPressed()
{
	return m_isKeyPressed;
}

char Input::GetKeyUp()
{
	return m_keyUp;
}

char Input::GetKeyDown()
{
	return m_keyDown;
}

bool Input::IsLeftButtonClicked()
{
	return m_isLeftButtonClicked;
}

bool Input::IsRightButtonClicked()
{
	return m_isRightButtonClicked;
}

bool Input::IsMiddleButtonClicked()
{
	return m_isMiddleButtonClicked;
}

int Input::GetMousePositionX()
{
	return m_mousePositionX;
}

int Input::GetMousePositionY()
{
	return m_mousePositionY;
}

int Input::GetMouseMotionX()
{
	return m_mouseMotionX;
}

int Input::GetMouseMotionY()
{
	return m_mouseMotionY;
}

void Input::Update()
{
}











Input::Input()
{
	m_keyUp = 0;
	m_keyDown = 0;

	m_isXClicked = false;
	m_isKeyPressed = false;

	m_isLeftButtonClicked = false;
	m_isRightButtonClicked = false;
	m_isMiddleButtonClicked = false;

	m_mousePositionX = 0;
	m_mousePositionY = 0;

	m_mouseMotionX = 0;
	m_mouseMotionY = 0;

}
