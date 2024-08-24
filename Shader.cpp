#include <fstream>
#include <iostream>
#include "Shader.h"

GLuint Shader::s_vertexShaderID = 0;
GLuint Shader::s_fragmentShaderID = 0;

Shader::Shader()
{
	m_shaderProgramID = 0;
}

Shader::~Shader()
{
	//TODO - Find a better place to put these
	//glDeleteShader(s_vertexShaderID);
	//glDeleteShader(s_fragmentShaderID);
}

GLuint Shader::GetShaderProgramID() const
{
	return m_shaderProgramID;
}

bool Shader::Create(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename)
{
	m_shaderProgramID = glCreateProgram();

	if (m_shaderProgramID == 0)
	{
		std::cout << "Error creating shader program." << std::endl;
		return false;
	}

	if (s_vertexShaderID == 0)
	{
		s_vertexShaderID = glCreateShader(GL_VERTEX_SHADER);

		if (s_vertexShaderID == 0)
		{
			std::cout << "Error creating vertex shader object." << std::endl;
			return false;
		}
	}

	if (s_fragmentShaderID == 0)
	{
		s_fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		if (s_fragmentShaderID == 0)
		{
			std::cout << "Error creating fragment shader object." << std::endl;
			return false;
		}
	}

	if (!CompileShaders(vertexShaderFilename, ShaderType::VertexShader))
	{
		return false;
	}

	if (!CompileShaders(fragmentShaderFilename, ShaderType::FragmentShader))
	{
		return false;
	}

	if (!LinkProgram())
	{
		return false;
	}

	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLint data) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform1i(ID, data);
	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLuint data) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform1ui(ID, data);
	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat data) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform1f(ID, data);
	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform2f(ID, x, y);
	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y, GLfloat z) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform3f(ID, x, y, z);
	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform4f(ID, x, y, z, w);
	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, const glm::mat4& data) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniformMatrix4fv(ID, 1, GL_FALSE, &data[0][0]);
	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, const glm::mat3& data) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniformMatrix3fv(ID, 1, GL_TRUE, &data[0][0]);
	return true;
}

void Shader::Use()
{
	glUseProgram(m_shaderProgramID);
}

void Shader::Destroy()
{
	glDeleteProgram(m_shaderProgramID);
}

bool Shader::LinkProgram()
{
	glAttachShader(m_shaderProgramID, s_vertexShaderID);
	glAttachShader(m_shaderProgramID, s_fragmentShaderID);
	glLinkProgram(m_shaderProgramID);
	glDetachShader(m_shaderProgramID, s_vertexShaderID);
	glDetachShader(m_shaderProgramID, s_fragmentShaderID);

	GLint errorCode;
	glGetProgramiv(m_shaderProgramID, GL_LINK_STATUS, &errorCode);

	if (errorCode == GL_TRUE)
	{
		std::cout << "Shader linking successful!" << std::endl;
	}

	else
	{
		GLchar errorMessage[1000];
		GLsizei bufferSize = 1000;

		glGetProgramInfoLog(m_shaderProgramID, bufferSize, &bufferSize, errorMessage);
		std::cout << errorMessage << std::endl;
	}

	return true;
}

bool Shader::CompileShaders(const std::string& filename, ShaderType shaderType)
{
	std::fstream file;
	std::string text;
	std::string sourceCode;
	GLuint shaderID;

	shaderID = (shaderType == ShaderType::VertexShader) ? s_vertexShaderID : s_fragmentShaderID;

	file.open(filename);

	if (!file)
	{
		std::cout << "Error reading shader file: " << filename << std::endl;
		return false;
	}

	while (!file.eof())
	{
		std::getline(file, text);
		sourceCode += text + "\n";
	}

	file.close();

	const GLchar* finalSourceCode = reinterpret_cast<const GLchar*>(sourceCode.c_str());
	glShaderSource(shaderID, 1, &finalSourceCode, nullptr);

	glCompileShader(shaderID);

	GLint errorCode;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &errorCode);

	if (errorCode == GL_TRUE)
	{
		std::cout << "Shader compilation successful!" << std::endl;
	}

	else
	{
		GLchar errorMessage[1000];
		GLsizei bufferSize = 1000;

		glGetShaderInfoLog(shaderID, bufferSize, &bufferSize, errorMessage);
		std::cout << errorMessage << std::endl;
	}

	return true;
}