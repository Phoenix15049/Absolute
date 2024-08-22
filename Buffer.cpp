#include "Buffer.h"
#include "Shader.h"

Buffer::Buffer()
{
	m_VAO = 0;
	m_EBO = 0;
	m_vertexVBO = 0;
	m_colorVBO = 0;
	m_textureVBO = 0;
	m_totalVertices = 0;
	m_hasEBO = false;
}

void Buffer::CreateBuffer(GLuint totalVertices, bool hasEBO)
{
	glGenBuffers(1, &m_vertexVBO);
	glGenBuffers(1, &m_colorVBO);
	glGenBuffers(1, &m_textureVBO);
	glGenVertexArrays(1, &m_VAO);

	if (hasEBO)
	{
		glGenBuffers(1, &m_EBO);
		m_hasEBO = hasEBO;
	}

	m_totalVertices = totalVertices;
}

void Buffer::FillEBO(const GLuint* data, GLsizeiptr bufferSize, FillType fill)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, data, static_cast<GLenum>(fill));
}

void Buffer::FillVBO(VBOType vboType, GLfloat* data, GLsizeiptr bufferSize, FillType fillType)
{
	glBindVertexArray(m_VAO);

		if (vboType == VBOType::VertexBuffer)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
		}

		else if(vboType == VBOType::ColorBuffer)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
		}

		else
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_textureVBO);
		}

		glBufferData(GL_ARRAY_BUFFER, bufferSize, data, static_cast<GLenum>(fillType));

	glBindVertexArray(0);
}

void Buffer::LinkEBO()
{
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBindVertexArray(0);
}

void Buffer::LinkVBO(const std::string& attribute, VBOType vboType, ComponentType componentType, DataType dataType)
{
	
	GLuint shaderProgramID = Shader::Instance()->GetShaderProgramID();

	GLint ID = glGetAttribLocation(shaderProgramID, attribute.c_str());

	glBindVertexArray(m_VAO);

		if (vboType == VBOType::VertexBuffer)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
		}

		else if (vboType == VBOType::ColorBuffer)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
		}

		else
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_textureVBO);
		}

		glVertexAttribPointer(ID, static_cast<GLint>(componentType), GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(ID);

	glBindVertexArray(0);

}

void Buffer::Render(DrawType drawType)
{
	glBindVertexArray(m_VAO);

	if (m_hasEBO)
	{
		glDrawElements(static_cast<GLenum>(drawType),
			m_totalVertices, GL_UNSIGNED_INT, nullptr);
	}

	else
	{
		glDrawArrays(static_cast<GLenum>(drawType), 0, m_totalVertices);
	}

	glBindVertexArray(0);
}

void Buffer::DestroyBuffer()
{
	glDeleteBuffers(1, &m_vertexVBO);
	glDeleteBuffers(1, &m_colorVBO);
	glDeleteBuffers(1, &m_textureVBO);
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_EBO);
}