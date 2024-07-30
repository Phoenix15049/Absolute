#include "Screen.h"
#include <iostream>
#include <stdio.h>
#include "glad.h"
using namespace std;

Screen* Screen::Instance()
{
	static Screen* screen = new Screen;
	return screen;
}

bool Screen::Initialize()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		cout << "INITIALIZE_ERROR on init" << endl;
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);



	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);


	window = SDL_CreateWindow("Absolute",
								SDL_WINDOWPOS_UNDEFINED,
								SDL_WINDOWPOS_UNDEFINED,
								1280, 720, SDL_WINDOW_OPENGL);

	if (!window) {
		cout << "INITIALIZE_ERROR,on creating Window" << endl;
		return false;
	}


	context = SDL_GL_CreateContext(window);
	if (!context) {
		cout << "INITIALIZE_ERROR,on creating Context" << endl;
		return false;
	}






	return true;
}

void Screen::ClearScreen()
{

	glClear(GL_COLOR_BUFFER_BIT);

}

void Screen::Present()
{

	SDL_GL_SwapWindow(window);

}

void Screen::ShutDown()
{
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
}

Screen::Screen()
{
	window = nullptr;
	context = nullptr;
}
