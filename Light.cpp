#include <gtc/matrix_transform.hpp>
#include "Input.h"
#include "Light.h"
#include "Shader.h"

Light::Light()
{
	m_ambient = glm::vec3(1.0f);
	m_diffuse = glm::vec3(1.0f);
	m_specular = glm::vec3(1.0f);
	m_position = glm::vec3(0.0f, 1.0f, 0.0f);

	GLfloat vertices[] = { 0.0f, 0.0f, 0.0f };
	GLfloat colors[] = { 1.0f, 1.0f, 1.0f };

	m_buffer.CreateBuffer(1);

	m_buffer.FillVBO(Buffer::VBOType::VertexBuffer, vertices, sizeof(vertices), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::ColorBuffer, colors, sizeof(colors), Buffer::FillType::Once);

	m_buffer.LinkVBO("vertexIn", Buffer::VBOType::VertexBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);
	m_buffer.LinkVBO("colorIn", Buffer::VBOType::ColorBuffer, Buffer::ComponentType::RGB, Buffer::DataType::FloatData);
}



Light::~Light()
{
	m_buffer.DestroyBuffer();
}

void Light::Update()
{
	if (Input::Instance()->IsKeyPressed())
	{



		if (Input::Instance()->GetKeyDown() == 'j')
		{
			m_position.x -= 0.01f;
		}

		else if (Input::Instance()->GetKeyDown() == 'l')
		{
			m_position.x += 0.01f;
		}

		else if (Input::Instance()->GetKeyDown() == 'i')
		{
			m_position.z -= 0.01f;
		}

		else if (Input::Instance()->GetKeyDown() == 'k')
		{
			m_position.z += 0.01f;
		}

		else if (Input::Instance()->GetKeyDown() == 'u')
		{
			m_position.y += 0.01f;
		}

		else if (Input::Instance()->GetKeyDown() == 'o')
		{
			m_position.y -= 0.01f;
		}

	}

	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, m_position);

}

void Light::Render()
{
	Shader::Instance()->SendUniformData("model", m_model);
	Shader::Instance()->SendUniformData("isLit", false);
	Shader::Instance()->SendUniformData("isTextured", false);
	
	glPointSize(20.0f);
	m_buffer.Render(Buffer::DrawType::Points);
}

void Light::SendToShader()
{

	Shader::Instance()->SendUniformData("light.position", m_position.x, m_position.y, m_position.z);
	Shader::Instance()->SendUniformData("light.ambient", m_ambient.r, m_ambient.g, m_ambient.b);
	Shader::Instance()->SendUniformData("light.diffuse", m_diffuse.r, m_diffuse.g, m_diffuse.b);
	Shader::Instance()->SendUniformData("light.specular", m_specular.r, m_specular.g, m_specular.b);

}