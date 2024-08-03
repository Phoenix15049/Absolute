#include "Input.h"
#include "Screen.h"
#include <iostream>
#include <SDL.h>
#include <stdio.h>
#include "glad.h"
#include "Shader.h"
#include "Quad.h"
#include <glm.hpp>
#include "Camera.h"
#include <SDL_image.h>
#include "Light.h"

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
	
	
	
	Quad quad;
	Camera camera;
	camera.Set3DView();
	Light light;


	float x = 0.0f;
	float y = 0.0f;
	
	//=======================

	//MainLoop
	while (isAppRunning)
	{
		
		Screen::Instance()->ClearScreen();
		//Update and render stuff

		Input::Instance()->Update();
		
		
		isAppRunning = !Input::Instance()->IsXClicked();
		
		/**/
		camera.Update();
		light.Update();
		light.Render();
		light.SendToShader();
		quad.Update();
		quad.Render();
		
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