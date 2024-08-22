#include <iostream>
#include <SDL.h>
#include "Screen.h"
#include "Input.h"
#include "gl.h"
#include "Shader.h"
#include "Quad.h"
#include "Camera.h"
#include "Light.h"
#include "Cube.h"



bool isAppRunning = true;

int main(int argc, char* argv[])
{

	Screen::Instance()->Initialize();
	
	if (!Shader::Instance()->CreateProgram())
	{
		return 0;
	}

	if (!Shader::Instance()->CreateShaders())
	{
		return 0;
	}
	
	if (!Shader::Instance()->CompileShaders("Shaders/Main.vert", Shader::ShaderType::VERTEX_SHADER))
	{
		//CONSOLE ERROR CATCHING 
	}

	if (!Shader::Instance()->CompileShaders("Shaders/Main.frag", Shader::ShaderType::FRAGMENT_SHADER))
	{
		//CONSOLE ERROR CATCHING 
	}

	Shader::Instance()->AttachShaders();
	
	if (!Shader::Instance()->LinkProgram())
	{
		//CONSOLE ERROR CATCHING 
	}

	float xPos = 0.0f;
	float yPos = 0.0f;

	//==================================================================================

	Quad quad;
	Cube cube;
	Camera camera;
	camera.Set3DView();

	Light light;

	//==================================================================================

	while (isAppRunning)
	{

		Screen::Instance()->ClearScreen();

		Input::Instance()->Update();

		char keyPressed = Input::Instance()->GetKeyDown();

		isAppRunning = !Input::Instance()->IsXClicked();

		camera.Update();

		light.Update();
		light.Render();
		light.SendToShader();

		//quad.Update();
		//quad.Render();

		cube.Update();
		cube.Render();

		Screen::Instance()->Present();

	}

	Shader::Instance()->DetachShaders();
	Shader::Instance()->DestroyShaders();
	Shader::Instance()->DestroyProgram();

	Screen::Instance()->Shutdown();	

	return 0;
}