#pragma once

#include <glm.hpp>
#include <string>
#include "gl.h"

class Shader
{

public:

	enum class ShaderType 
	{ 
		VertexShader,
		FragmentShader
	};

	Shader();
	~Shader();

	GLuint GetShaderProgramID() const;

	bool Create(const std::string& vertexShaderFilename, 
		const std::string& fragmentShaderFilename);

	bool SendUniformData(const std::string& uniformName, GLint data) const;
	bool SendUniformData(const std::string& uniformName, GLuint data) const;
	bool SendUniformData(const std::string& uniformName, GLfloat data) const;
	
	bool SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y) const;
	bool SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y, GLfloat z) const;
	bool SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const;
	
	bool SendUniformData(const std::string& uniformName, const glm::mat4& data) const;
	bool SendUniformData(const std::string& uniformName, const glm::mat3& data) const;

	void Use();
	void Destroy();

private:

	bool LinkProgram();
	bool CompileShaders(const std::string& filename, ShaderType shaderType);
	
	GLuint m_shaderProgramID;
	static GLuint s_vertexShaderID;
	static GLuint s_fragmentShaderID;

};