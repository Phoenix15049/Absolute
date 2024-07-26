#include <iostream>
#include <SDL.h>
#include <stdio.h>
using namespace std;



SDL_Window* window = nullptr;
SDL_GLContext context = nullptr;

int main(int argc,char* argv[]) {

	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		cout << "ERROR in INIT"<<endl;
	}
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,32);
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
		cout << "ERROR creating Window"<<endl;
	}


	context = SDL_GL_CreateContext(window);
	if (!context) {
		cout << "ERROR creating Context" << endl;
	}



	//=======================

	//MainLoop

	//=======================








	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);


	system("pause");
	return 0;
}