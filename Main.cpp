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

	GLfloat vertices[] = { -0.5f,0.5f,0.0f,//TRIANGLE 1
						   0.5f,0.5f,0.0f,
						  -0.5f,-0.5f,0.0f,

						  -0.5f,-0.5f,0.0f,//TRIANGLE 2
						   0.5f,0.5f,0.0f,
						   0.5f,-0.5f,0.0f
	};


	GLfloat colors[] = {   1.0f,0.0f,0.0f,//TRIANGLE 1
						   0.0f,0.0f,1.0f,
						   0.0f,1.0f,1.0f,

						   0.0f,1.0f,1.0f,//TRIANGLE 2
						   0.0f,0.0f,1.0f,
						   0.0f,1.0f,0.0f
	};

	GLuint shaderProgramID = Shader::Instance()->GetShaderProgramID();

	GLint vertexID = glGetAttribLocation(shaderProgramID, "vertexIn");
	GLint colorID = glGetAttribLocation(shaderProgramID, "colorIn");


	GLuint vertexVBO;
	GLuint colorVBO;

	glGenBuffers(1, &vertexVBO);
	glGenBuffers(1, &colorVBO);

	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexID,3,GL_FLOAT,GL_FALSE,0,nullptr);
	glEnableVertexAttribArray(vertexID);


	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(colorID, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(colorID);
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
		}

		glDrawArrays(GL_TRIANGLES, 0, 6);



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