#include <Windows.h>
#include <gl/GL.h>
#include "Screen.h"
#include <iostream>
#include <SDL.h>
#include <stdio.h>

using namespace std;


bool isAppRunning = true;


int main(int argc,char* argv[]) {

	Screen::Instance()->Initialize();



	//=======================

	//MainLoop
	while (isAppRunning)
	{
		
		Screen::Instance()->ClearScreen();
		//Update and render stuff

		glBegin(GL_QUADS);

		glColor3f(1, 0, 0);
		glVertex3f(-0.5f, 0.5f, 0.0f);

		glColor3f(0, 1, 0);
		glVertex3f(1.0f, 0.5f, 0.0f);

		glColor3f(0, 0, 1);
		glVertex3f(0.5f, -0.5f, 0.0f);

		glColor3f(0, 0, 1);
		glVertex3f(-0.5f, -0.5f, 0.0f);

		glEnd();




		Screen::Instance()->Present();
		
	}

	Screen::Instance()->ShutDown();

	//=======================

	system("pause");
	return 0;
}