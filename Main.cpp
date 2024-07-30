#include "Input.h"
#include "Screen.h"
#include <iostream>
#include <SDL.h>
#include <stdio.h>

#include "glad.h"
#include "Shader.h"
using namespace std;


bool isAppRunning = true;


int main(int argc,char* argv[]) {

	Screen::Instance()->Initialize();


	if (!Shader::Instance()->CreateProgram()) {
		return 0;//CLOSING PROGRAM IF ERROR HAPPENS
	}
	if (!Shader::Instance()->CreateShaders()) {
		return 0;
	}
	 
	Shader::Instance()->CompileShaders("Shaders/Main.vert",Shader::ShaderType::VERTEX_SHADER);
	Shader::Instance()->CompileShaders("Shaders/Main.frag", Shader::ShaderType::FRAGMENT_SHADER);

	Shader::Instance()->AttachShaders();
	Shader::Instance()->LinkProgram();


	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;


	Shader::Instance()->SendUniformData("time", 20);


	//=======================

	//MainLoop
	while (isAppRunning)
	{
		
		Screen::Instance()->ClearScreen();
		//Update and render stuff

		Input::Instance()->Update();
		
		
		isAppRunning = !Input::Instance()->IsXClicked();
		
		if (Input::Instance()->IsKeyPressed()) {

			char kp = Input::Instance()->GetKeyDown();
			if (kp == 'a') {
				x=x-0.01f;
			}
			if (kp == 'd') {
				x = x + 0.01f;
			}
			if (kp == 'w') {
				y = y + 0.01f;
			}
			if (kp == 's') {
				y = y - 0.01f;
			}
			if (kp == 'e') {
				z = z + 0.01f;
			}
			if (kp == 'q') {
				z = z - 0.01f;
			}

		}

		glBegin(GL_QUADS);

		glColor3f(1, 0, 0);
		glVertex3f(-0.5f+x, 0.5f+y, 0.0f+z);

		glColor3f(0, 1, 0);
		glVertex3f(1.0f+x, 0.5f+y, 0.0f+z);

		glColor3f(0, 0, 1);
		glVertex3f(0.5f+x, -0.5f+y, 0.0f+z);

		glColor3f(0, 0, 1);
		glVertex3f(-0.5f+x, -0.5f+y, 0.0f+z);

		glEnd();




		Screen::Instance()->Present();
		
	}


	Shader::Instance()->DetachShaders();
	Shader::Instance()->DestroyShaders();
	Shader::Instance()->DestroyProgram();
	Screen::Instance()->ShutDown();

	//=======================

	system("pause");
	return 0;
}