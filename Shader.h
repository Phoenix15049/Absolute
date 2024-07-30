#ifndef SHADER_H
#define SHADER_H

#include<string>
#include"glad.h"

class Shader
{
public:
	static Shader* Instance();

public :
	enum class ShaderType {
		VERTEX_SHADER,
		FRAGMENT_SHADER
	};
public:
	bool CreateProgram();
	bool CreateShaders();

	bool CompileShaders(const std::string& filename,ShaderType shaderType);
	void AttachShaders();
	bool LinkProgram();

	void DetachShaders();
	void DestroyShaders();
	void DestroyProgram();



private:
	Shader();
	Shader(const Shader&);
	Shader& operator=(Shader&);



private:
	GLuint m_shaderProgramID;
	GLuint m_vertexShaderID;
	GLuint m_fragmentShaderID;
};



#endif // !SHADER_H
