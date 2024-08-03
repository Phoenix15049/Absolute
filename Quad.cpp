#include "Quad.h"
#include "Shader.h"
#include <gtc/matrix_transform.hpp>
#include "Input.h"
Quad::Quad() {

	

	

	GLfloat vertices[] = { -0.5f,0.5f,0.0f,//TRIANGLE 1
						   0.5f,0.5f,0.0f,
						  -0.5f,-0.5f,0.0f,

						  -0.5f,-0.5f,0.0f,//TRIANGLE 2
						   0.5f,0.5f,0.0f,
						   0.5f,-0.5f,0.0f
	};


	GLfloat colors[] = {   1.0f,1.0f,1.0f,//TRIANGLE 1
						   1.0f,1.0f,1.0f,
						   1.0f,1.0f,1.0f,//BOTH WHITE

						   1.0f,1.0f,1.0f,//TRIANGLE 2
						   1.0f,1.0f,1.0f,
						   1.0f,1.0f,1.0f
	};


	GLfloat UVs[] = { 0.0f,1.0f,//TRIANGLE 1
					  1.0f,1.0f,
					  0.0f,0.0f,

					  0.0f,0.0f,//TRIANGLE 2
					  1.0f,1.0f,
					  1.0f,0.0f
	};
	
	m_buffer.CreateBuffer(6); 
	
	m_buffer.FillVBO(Buffer::VERTEX_BUFFER, vertices, sizeof(vertices), Buffer::SINGLE);
	m_buffer.FillVBO(Buffer::COLOR_BUFFER, colors, sizeof(colors), Buffer::SINGLE);
	m_buffer.FillVBO(Buffer::TEXTURE_BUFFER, UVs, sizeof(UVs), Buffer::SINGLE);


	m_buffer.LinkBuffer("vertexIn", Buffer::VERTEX_BUFFER, Buffer::XYZ, Buffer::FLOAT);
	m_buffer.LinkBuffer("colorIn", Buffer::COLOR_BUFFER, Buffer::RGB, Buffer::FLOAT);
	m_buffer.LinkBuffer("textureIn", Buffer::TEXTURE_BUFFER, Buffer::UV, Buffer::FLOAT);


	m_texture.Load("Textures/1.png");
	m_shininess = 50.0f;
	m_position = glm::vec3(0.0f);

	m_ambient = glm::vec3(0.4f, 0.4f, 0.4f);
	m_diffuse = glm::vec3(0.1f, 0.7f, 0.2f);
	m_specular = glm::vec3(0.8f, 0.8f, 0.8f);
	

}

Quad::~Quad() {
	m_buffer.DestroyBuffer();
}

void Quad::Update()
{
	/*if (Input::Instance()->IsKeyPressed()) {

		char kp = Input::Instance()->GetKeyDown();
		if (kp == 'j') {
			m_position.x -= 0.01f;
		}
		if (kp == 'l') {
			m_position.x += 0.01f;
		}
		if (kp == 'i') {
			m_position.z -= 0.01f;
		}
		if (kp == 'k') {
			m_position.z += 0.01f;
		}
		if (kp == 'o') {
			m_position.y -= 0.01f;
		}
		if (kp == 'u') {
			m_position.y += 0.01f;
		}
	}*/


	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, m_position);
	m_model = glm::rotate(m_model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//m_model = glm::scale(m_model, glm::vec3(0.5f, 0.5f, 1.0f));

}

void Quad::Render() {



	Shader::Instance()->SendUniformData("model", m_model);
	Shader::Instance()->SendUniformData("isLit", true);
	Shader::Instance()->SendUniformData("isTextured", true);
	
	Shader::Instance()->SendUniformData("material.shininess", m_shininess);
	Shader::Instance()->SendUniformData("material.ambient", m_ambient.r, m_ambient.g, m_ambient.b);
	Shader::Instance()->SendUniformData("material.diffuse", m_diffuse.r, m_diffuse.g, m_diffuse.b);
	Shader::Instance()->SendUniformData("material.specular", m_specular.r, m_specular.g, m_specular.b);


	m_texture.Bind();
	m_buffer.Render(Buffer::TRIANGLES);
	m_texture.Unbind();
}