#ifndef SCREEN_H
#define SCREEN_H

#include <SDL.h>

class Screen {

public :
	static Screen* Instance();
public :

	bool Initialize();
	void ClearScreen();
	void Present();
	void ShutDown();
private: 
	Screen();
	Screen(const Screen&);
	Screen& operator =(const Screen&);


private :
	SDL_Window* window;
	SDL_GLContext context;
};

#endif // !SCREEN_H
#pragma once
