#ifndef SCREEN_H
#define SCREEN_H

#include <SDL.h>

class Screen {

public :

	bool Initialize();
	void ClearScreen();
	void Present();
	void ShutDown();



private :
	SDL_Window* window = nullptr;
	SDL_GLContext context = nullptr;



};

#endif // !SCREEN_H
#pragma once
