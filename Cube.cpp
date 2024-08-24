#include <gtc/matrix_transform.hpp>
#include "Cube.h"
#include "Shader.h"
#include "Input.h"

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

	GLfloat colors[] = { 1.0f, 0.0f, 0.0f, 1.0f,
						 1.0f, 0.0f, 0.0f, 1.0f,
						 1.0f, 0.0f, 0.0f, 1.0f,
						 1.0f, 0.0f, 0.0f, 1.0f,      //Front face (red)
										  
						 0.0f, 1.0f, 0.0f, 1.0f,
						 0.0f, 1.0f, 0.0f, 1.0f,
						 0.0f, 1.0f, 0.0f, 1.0f,
						 0.0f, 1.0f, 0.0f, 1.0f,      //Back face (green)
										   
						 0.0f, 0.0f, 1.0f, 1.0f,
						 0.0f, 0.0f, 1.0f, 1.0f,
						 0.0f, 0.0f, 1.0f, 1.0f,
						 0.0f, 0.0f, 1.0f, 1.0f,      //Left face (blue)
										   
						 1.0f, 1.0f, 0.0f, 1.0f,
						 1.0f, 1.0f, 0.0f, 1.0f,
						 1.0f, 1.0f, 0.0f, 1.0f,
						 1.0f, 1.0f, 0.0f, 1.0f,      //Right face (yellow)
										  
						 1.0f, 0.0f, 1.0f, 1.0f,
						 1.0f, 0.0f, 1.0f, 1.0f,
						 1.0f, 0.0f, 1.0f, 1.0f,
						 1.0f, 0.0f, 1.0f, 1.0f,      //Top face (magenta)
										  
						 0.0f, 1.0f, 1.0f, 1.0f,
						 0.0f, 1.0f, 1.0f, 1.0f,
						 0.0f, 1.0f, 1.0f, 1.0f,
						 0.0f, 1.0f, 1.0f, 1.0f };    //Bottom face (cyan)

	GLfloat UVs[] = { 0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,      //Front face 

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,	   //Back face

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,	   //Left face

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,	   //Right face

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,	   //Top face

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f };    //Bottom face

	GLfloat normals[] = { 0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
						  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,      //Front face 

						   0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,
						   0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,	    //Back face

						  -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
						  -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,	    //Left face

						   1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
						   1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,	    //Right face

						   0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
						   0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,	    //Top face

						   0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,
						   0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f };   //Bottom face

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
	m_buffer.FillVBO(Buffer::VBOType::TextureBuffer, UVs, sizeof(UVs), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::NormalBuffer, normals, sizeof(normals), Buffer::FillType::Once);

	m_buffer.LinkEBO();

	m_texture.Load("Textures/Crate_1.png");

	m_shininess = 50.0f;
	m_position = glm::vec3(0.0f);
	m_rotation = glm::vec3(0.0f);
	m_ambient = glm::vec3(0.4f, 0.4f, 0.4f);
	m_diffuse = glm::vec3(0.1f, 0.7f, 0.2f);
	m_specular = glm::vec3(0.8f, 0.8f, 0.8f);
}

Cube::~Cube()
{
	m_buffer.DestroyBuffer();
}

void Cube::Update()
{
	if (Input::Instance()->IsLeftButtonClicked())
	{
		m_rotation.x += Input::Instance()->GetMouseMotionY();
		m_rotation.y += Input::Instance()->GetMouseMotionX();
	}

	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, m_position);
	m_model = glm::rotate(m_model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_model = glm::rotate(m_model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_normal = glm::inverse(glm::mat3(m_model));
}

void Cube::Render(const Shader& shader)
{
	shader.SendUniformData("model", m_model);
	shader.SendUniformData("normal", m_normal);
	
	shader.SendUniformData("isLit", true);
	shader.SendUniformData("isTextured", true);

	shader.SendUniformData("material.shininess", m_shininess);
	shader.SendUniformData("material.ambient", m_ambient.r, m_ambient.g, m_ambient.b);
	shader.SendUniformData("material.diffuse", m_diffuse.r, m_diffuse.g, m_diffuse.b);
	shader.SendUniformData("material.specular", m_specular.r, m_specular.g, m_specular.b);

	m_buffer.LinkVBO(shader, "vertexIn", Buffer::VBOType::VertexBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);
	m_buffer.LinkVBO(shader, "colorIn", Buffer::VBOType::ColorBuffer, Buffer::ComponentType::RGBA, Buffer::DataType::FloatData);
	m_buffer.LinkVBO(shader, "textureIn", Buffer::VBOType::TextureBuffer, Buffer::ComponentType::UV, Buffer::DataType::FloatData);
	m_buffer.LinkVBO(shader, "normalIn", Buffer::VBOType::NormalBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);

	m_texture.Bind();
	m_buffer.Render(Buffer::DrawType::Triangles);
	m_texture.Unbind();
}