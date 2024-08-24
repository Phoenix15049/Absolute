#include <gtc/matrix_transform.hpp>
#include "Quad.h"
#include "Shader.h"
#include "Input.h"

Quad::Quad()
{
	//data that represents vertices for quad
	GLfloat vertices[] = { -0.5f,  0.5f, 0.0f,
						    0.5f,  0.5f, 0.0f,
						    0.5f, -0.5f, 0.0f,
					       -0.5f, -0.5f, 0.0f  };

	//data that represents colors for quad
	GLfloat colors[] = { 1.0f, 0.0f, 0.0f, 1.0f,
					     0.0f, 0.0f, 1.0f, 1.0f,
					     0.0f, 1.0f, 0.0f, 1.0f,
					     0.0f, 1.0f, 1.0f, 1.0f  };

	//data that represents normals for quad
	GLfloat normals[] = { 0.0f, 0.0f, 1.0f,
						  0.0f, 0.0f, 1.0f,
						  0.0f, 0.0f, 1.0f,
						  0.0f, 0.0f, 1.0f };

	//data that represents UV coordinates for quad
	GLfloat UVs[] = { 0.0f, 1.0f,
					  1.0f, 1.0f,
					  1.0f, 0.0f,
					  0.0f, 0.0f };

	//our index buffer to control the rendering
	GLuint indices[] = { 0, 1, 3,
					     3, 1, 2 };

	m_buffer.CreateBuffer(6, true);
	m_buffer.FillEBO(indices, sizeof(indices), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::VertexBuffer, vertices, sizeof(vertices), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::ColorBuffer, colors, sizeof(colors), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::TextureBuffer, UVs, sizeof(UVs), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::NormalBuffer, normals, sizeof(normals), Buffer::FillType::Once);

	m_buffer.LinkEBO();
	
	m_texture.Load("Textures/Crate_1.png");

	m_shininess = 50.0f;
	m_position = glm::vec3(0.0f);
	m_ambient = glm::vec3(0.4f, 0.4f, 0.4f);
	m_diffuse = glm::vec3(0.1f, 0.7f, 0.2f);
	m_specular = glm::vec3(0.8f, 0.8f, 0.8f);

}

Quad::~Quad()
{
	m_buffer.DestroyBuffer();
}

void Quad::Update()
{
	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, m_position);
	m_model = glm::rotate(m_model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	m_normal = glm::inverse(glm::mat3(m_model));
}

void Quad::Render(const Shader& shader)
{
	shader.SendUniformData("model", m_model);
	shader.SendUniformData("normal", m_normal);
	
	shader.SendUniformData("isLit", false);
	shader.SendUniformData("isTextured", false);
	
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