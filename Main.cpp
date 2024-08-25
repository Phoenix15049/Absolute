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
#include "Model.h"

bool isAppRunning = true;

int main(int argc, char* argv[])
{
	Screen::Instance()->Initialize();
	
	Shader lightShader;
	lightShader.Create("Shaders/Main.vert", "Shaders/Main.frag");
	lightShader.Use();

	float xPos = 0.0f;
	float yPos = 0.0f;

	//================================================================

	Quad quad;
	Cube cube;
	Model model;
	model.Load("Models/Armchair.obj");

	Camera camera;
	camera.Set3DView();

	Light light;

	//================================================================

	while (isAppRunning)
	{

		Screen::Instance()->ClearScreen();

		Input::Instance()->Update();

		char keyPressed = Input::Instance()->GetKeyDown();

		isAppRunning = !Input::Instance()->IsXClicked();

		camera.Update();
		camera.SendToShader(lightShader);

		light.Update();
		light.Render(lightShader);
		light.SendToShader(lightShader);

		//quad.Update();
		//quad.Render(lightShader);

		//cube.Update();
		//cube.Render(lightShader);

		model.Update();
		model.Render(lightShader);

		Screen::Instance()->Present();

	}

	lightShader.Destroy();	

	Screen::Instance()->Shutdown();	

	return 0;
}