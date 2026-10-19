#pragma once

#include <string>
#include <unordered_map>
#include <glm.hpp>
#include <gl.h>

class Shader
{

public:

	Shader();
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	bool Create(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
	bool Reload();
	void Destroy();

	void Use() const;
	GLuint GetID() const;

	void BindUniformBlock(const char* blockName, GLuint binding) const;
	void SetSampler(const char* uniformName, GLint unit) const;

	void SetUniform(const char* uniformName, bool value) const;
	void SetUniform(const char* uniformName, GLint value) const;
	void SetUniform(const char* uniformName, GLfloat value) const;
	void SetUniform(const char* uniformName, const glm::vec2& value) const;
	void SetUniform(const char* uniformName, const glm::vec3& value) const;
	void SetUniform(const char* uniformName, const glm::vec4& value) const;
	void SetUniform(const char* uniformName, const glm::mat3& value) const;
	void SetUniform(const char* uniformName, const glm::mat4& value) const;

private:

	static GLuint CompileShader(const std::string& filename, GLenum type);
	static GLuint LinkProgram(GLuint vertexShaderID, GLuint fragmentShaderID, const std::string& name);

	GLint GetUniformLocation(const char* uniformName) const;

	GLuint m_programID;

	std::string m_vertexShaderFilename;
	std::string m_fragmentShaderFilename;

	mutable std::unordered_map<std::string, GLint> m_uniformLocations;

};
