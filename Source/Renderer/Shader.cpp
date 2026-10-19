#include <fstream>
#include <vector>
#include "Core/Log.h"
#include "Renderer/Shader.h"

Shader::Shader()
{
	m_programID = 0;
}

Shader::~Shader()
{
	Destroy();
}

bool Shader::Create(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename)
{
	m_vertexShaderFilename = vertexShaderFilename;
	m_fragmentShaderFilename = fragmentShaderFilename;

	return Reload();
}

bool Shader::Reload()
{
	const GLuint vertexShaderID = CompileShader(m_vertexShaderFilename, GL_VERTEX_SHADER);

	if (vertexShaderID == 0)
	{
		return false;
	}

	const GLuint fragmentShaderID = CompileShader(m_fragmentShaderFilename, GL_FRAGMENT_SHADER);

	if (fragmentShaderID == 0)
	{
		glDeleteShader(vertexShaderID);
		return false;
	}

	const GLuint programID = LinkProgram(vertexShaderID, fragmentShaderID, m_vertexShaderFilename);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	if (programID == 0)
	{
		return false;
	}

	Destroy();
	m_programID = programID;

	return true;
}

void Shader::Destroy()
{
	if (m_programID != 0)
	{
		glDeleteProgram(m_programID);
		m_programID = 0;
	}

	m_uniformLocations.clear();
}

void Shader::Use() const
{
	glUseProgram(m_programID);
}

GLuint Shader::GetID() const
{
	return m_programID;
}

void Shader::BindUniformBlock(const char* blockName, GLuint binding) const
{
	const GLuint index = glGetUniformBlockIndex(m_programID, blockName);

	if (index != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(m_programID, index, binding);
	}
}

void Shader::SetSampler(const char* uniformName, GLint unit) const
{
	Use();
	SetUniform(uniformName, unit);
}

void Shader::SetUniform(const char* uniformName, bool value) const
{
	SetUniform(uniformName, static_cast<GLint>(value));
}

void Shader::SetUniform(const char* uniformName, GLint value) const
{
	const GLint location = GetUniformLocation(uniformName);

	if (location != -1)
	{
		glUniform1i(location, value);
	}
}

void Shader::SetUniform(const char* uniformName, GLfloat value) const
{
	const GLint location = GetUniformLocation(uniformName);

	if (location != -1)
	{
		glUniform1f(location, value);
	}
}

void Shader::SetUniform(const char* uniformName, const glm::vec2& value) const
{
	const GLint location = GetUniformLocation(uniformName);

	if (location != -1)
	{
		glUniform2f(location, value.x, value.y);
	}
}

void Shader::SetUniform(const char* uniformName, const glm::vec3& value) const
{
	const GLint location = GetUniformLocation(uniformName);

	if (location != -1)
	{
		glUniform3f(location, value.x, value.y, value.z);
	}
}

void Shader::SetUniform(const char* uniformName, const glm::vec4& value) const
{
	const GLint location = GetUniformLocation(uniformName);

	if (location != -1)
	{
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}
}

void Shader::SetUniform(const char* uniformName, const glm::mat3& value) const
{
	const GLint location = GetUniformLocation(uniformName);

	if (location != -1)
	{
		glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);
	}
}

void Shader::SetUniform(const char* uniformName, const glm::mat4& value) const
{
	const GLint location = GetUniformLocation(uniformName);

	if (location != -1)
	{
		glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
	}
}

namespace
{
	bool ReadSource(const std::string& filename, std::string& sourceCode, int depth)
	{
		std::ifstream file(filename);

		if (!file || depth > 8)
		{
			Log::Error("Could not read shader file: " + filename);
			return false;
		}

		const size_t slash = filename.find_last_of("/\\");
		const std::string directory = (slash == std::string::npos) ? std::string() : filename.substr(0, slash + 1);

		std::string line;

		while (std::getline(file, line))
		{
			const size_t include = line.find("#include");

			if (include != std::string::npos)
			{
				const size_t first = line.find('"', include);
				const size_t last = line.find('"', first + 1);

				if (first != std::string::npos && last != std::string::npos)
				{
					if (!ReadSource(directory + line.substr(first + 1, last - first - 1), sourceCode, depth + 1))
					{
						return false;
					}

					continue;
				}
			}

			sourceCode += line;
			sourceCode += '\n';
		}

		return true;
	}
}

GLuint Shader::CompileShader(const std::string& filename, GLenum type)
{
	std::string sourceCode;

	if (!ReadSource(filename, sourceCode, 0))
	{
		return 0;
	}

	const GLchar* source = sourceCode.c_str();

	const GLuint shaderID = glCreateShader(type);
	glShaderSource(shaderID, 1, &source, nullptr);
	glCompileShader(shaderID);

	GLint status = GL_FALSE;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE)
	{
		GLint length = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> message(length > 1 ? length : 1, '\0');
		glGetShaderInfoLog(shaderID, static_cast<GLsizei>(message.size()), nullptr, message.data());

		Log::Error("Error compiling " + filename + ":\n" + message.data());

		glDeleteShader(shaderID);
		return 0;
	}

	return shaderID;
}

GLuint Shader::LinkProgram(GLuint vertexShaderID, GLuint fragmentShaderID, const std::string& name)
{
	const GLuint programID = glCreateProgram();

	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);
	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);

	GLint status = GL_FALSE;
	glGetProgramiv(programID, GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		GLint length = 0;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> message(length > 1 ? length : 1, '\0');
		glGetProgramInfoLog(programID, static_cast<GLsizei>(message.size()), nullptr, message.data());

		Log::Error("Error linking " + name + ":\n" + message.data());

		glDeleteProgram(programID);
		return 0;
	}

	return programID;
}

GLint Shader::GetUniformLocation(const char* uniformName) const
{
	auto it = m_uniformLocations.find(uniformName);

	if (it != m_uniformLocations.end())
	{
		return it->second;
	}

	const GLint location = glGetUniformLocation(m_programID, uniformName);
	m_uniformLocations.emplace(uniformName, location);
	return location;
}
