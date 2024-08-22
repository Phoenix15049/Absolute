#include <gtc/matrix_transform.hpp>
#include "Cube.h"
#include "Shader.h"






Cube::Cube()
{
	GLfloat vertices[] = { -0.5f,  0.5f,  0.5f,
							0.5f,  0.5f,  0.5f,
							0.5f, -0.5f,  0.5f,
						   -0.5f, -0.5f,  0.5f,       //Front face

							0.5f,  0.5f, -0.5f,
						   -0.5f,  0.5f, -0.5f,
						   -0.5f, -0.5f, -0.5f,
							0.5f, -0.5f, -0.5f,       //Back face

						   -0.5f,  0.5f, -0.5f,
						   -0.5f,  0.5f,  0.5f,
						   -0.5f, -0.5f,  0.5f,
						   -0.5f, -0.5f, -0.5f,       //Left face

							0.5f,  0.5f,  0.5f,
							0.5f,  0.5f, -0.5f,
							0.5f, -0.5f, -0.5f,
							0.5f, -0.5f,  0.5f,       //Right face

						   -0.5f,  0.5f, -0.5f,
							0.5f,  0.5f, -0.5f,
							0.5f,  0.5f,  0.5f,
						   -0.5f,  0.5f,  0.5f,       //Top face

						   -0.5f, -0.5f,  0.5f,
							0.5f, -0.5f,  0.5f,
							0.5f, -0.5f, -0.5f,
						   -0.5f, -0.5f, -0.5f };    //Bottom face




	GLfloat colors[] = { 1.0f, 0.0f, 0.0f,
						 1.0f, 0.0f, 0.0f,
						 1.0f, 0.0f, 0.0f,
						 1.0f, 0.0f, 0.0f,       //Front face (red)

						 0.0f, 1.0f, 0.0f,
						 0.0f, 1.0f, 0.0f,
						 0.0f, 1.0f, 0.0f,
						 0.0f, 1.0f, 0.0f,       //Back face (green)

						 0.0f, 0.0f, 1.0f,
						 0.0f, 0.0f, 1.0f,
						 0.0f, 0.0f, 1.0f,
						 0.0f, 0.0f, 1.0f,       //Left face (blue)

						 1.0f, 1.0f, 0.0f,
						 1.0f, 1.0f, 0.0f,
						 1.0f, 1.0f, 0.0f,
						 1.0f, 1.0f, 0.0f,       //Right face (yellow)

						 1.0f, 0.0f, 1.0f,
						 1.0f, 0.0f, 1.0f,
						 1.0f, 0.0f, 1.0f,
						 1.0f, 0.0f, 1.0f,       //Top face (magenta)

						 0.0f, 1.0f, 1.0f,
						 0.0f, 1.0f, 1.0f,
						 0.0f, 1.0f, 1.0f, 
						 0.0f, 1.0f, 1.0f };    //Bottom face (cyan)




	GLuint indices[] = { 0,  1,  3,  3,  1,  2,      //Front face
						 4,  5,  7,  7,  5,  6,      //Back face 
						 8,  9, 11, 11,  9, 10,      //Left face
						12, 13, 15, 15, 13, 14,      //Right face
						16, 17, 19, 19, 17, 18,      //Top face
						20, 21, 23, 23, 21, 22 };    //Bottom face

	m_buffer.CreateBuffer(36, true);
	m_buffer.FillEBO(indices, sizeof(indices), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::VertexBuffer, vertices, sizeof(vertices), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::ColorBuffer, colors, sizeof(colors), Buffer::FillType::Once);

	m_buffer.LinkEBO();
	m_buffer.LinkVBO("vertexIn", Buffer::VBOType::VertexBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);
	m_buffer.LinkVBO("colorIn", Buffer::VBOType::ColorBuffer, Buffer::ComponentType::RGB, Buffer::DataType::FloatData);

	m_position = glm::vec3(0.0f);
}

Cube::~Cube()
{
	m_buffer.DestroyBuffer();
}

void Cube::Update()
{
	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, m_position);
}

void Cube::Render()
{
	Shader::Instance()->SendUniformData("model", m_model);
	Shader::Instance()->SendUniformData("isLit", false);
	Shader::Instance()->SendUniformData("isTextured", false);
	m_buffer.Render(Buffer::DrawType::Triangles);
}