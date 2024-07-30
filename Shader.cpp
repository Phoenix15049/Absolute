#include <fstream>
#include "Shader.h"
#include <iostream>
using namespace std;
Shader* Shader::Instance() {
	static Shader* shaderObject = new Shader();
	return shaderObject;
}

Shader::Shader() {
	m_shaderProgramID = 0;
	m_vertexShaderID = 0;
	m_fragmentShaderID = 0;
}
bool Shader::CreateProgram()
{


	m_shaderProgramID = glCreateProgram();


	//BIFA
	if (m_shaderProgramID == 0) {
		cout << "ERROR CREATING SHADER PROGRAM" << endl;
		return false;
	}
	//BIFA


	return true;
}

bool Shader::CreateShaders()
{
	m_vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	m_fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	//BIFA
	if (m_vertexShaderID == 0) {
		cout << "ERROR CREATING VERTEX SHADER" << endl;
		return false;
	}
	else if (m_fragmentShaderID == 0) {
		cout << "ERROR CREATING FRAGMENT SHADER" << endl;
		return false;
	}
	//BIFA
	return true;
}

bool Shader::CompileShaders(const string& filename,ShaderType shaderType)
{
	fstream file;
	string text;
	string sourceCode;
	GLuint shaderID;

	shaderID = (shaderType == ShaderType::VERTEX_SHADER) ? m_vertexShaderID : m_fragmentShaderID;

	file.open(filename);
	
	//BIFA
	if (!file) {
		cout << "ERROR READING SHADER FILE : " << filename << endl;
		return false;
	}
	//BIFA

	while (!file.eof()) {
		getline(file, text);
		sourceCode += text + "\n";
	}
	file.close();
	
	const GLchar* finalSourceCode = reinterpret_cast <const GLchar*>( sourceCode.c_str());

	glShaderSource(shaderID,1,&finalSourceCode,nullptr);

	glCompileShader(shaderID);

	GLint errorCode;

	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &errorCode);
	
	//BIFA
	if (errorCode == GL_TRUE) {
		cout << "SHADER COMPILATION SUCCESSFUL" << endl;
	}
	else {

		GLchar errorMessage[1000];
		GLsizei bufferSize = 1000;
		glGetShaderInfoLog(shaderID, bufferSize, &bufferSize, errorMessage);
		cout << errorMessage << endl;
	}
	//BIFA



	return true;
}

void Shader::AttachShaders()
{
	glAttachShader(m_shaderProgramID, m_vertexShaderID);
	glAttachShader(m_shaderProgramID, m_fragmentShaderID);
	
}

bool Shader::LinkProgram()
{
	glLinkProgram(m_shaderProgramID);

	glUseProgram(m_shaderProgramID);



	//BIFA

	GLint errorCode;

	glGetProgramiv(m_shaderProgramID, GL_LINK_STATUS, &errorCode);


	if (errorCode == GL_TRUE) {
		cout << "SHADER LINKING SUCCESSFUL" << endl;
	}
	else {

		GLchar errorMessage[1000];
		GLsizei bufferSize = 1000;
		glGetProgramInfoLog(m_shaderProgramID, bufferSize, &bufferSize, errorMessage);
		cout << errorMessage << endl;
	}
	//BIFA



	return true;
}

void Shader::DetachShaders()
{
	glDetachShader(m_shaderProgramID, m_vertexShaderID);
	glDetachShader(m_shaderProgramID, m_fragmentShaderID);
}

void Shader::DestroyShaders()
{
	glDeleteShader(m_vertexShaderID);
	glDeleteShader(m_fragmentShaderID);
}

void Shader::DestroyProgram()
{
	glDeleteProgram(m_shaderProgramID);
}

bool Shader::SendUniformData(const string& uniformName, GLint data)
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());
	
	//BIFA

	if (ID == -1) {
		cout << "SHADER VARIABLE : " << uniformName << " not found or not used."<<endl;
		return false;
	}
	glUniform1i(ID, data);
	return true;

	//BIFA
}

bool Shader::SendUniformData(const string& uniformName, GLuint data)
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	//BIFA

	if (ID == -1) {
		cout << "SHADER VARIABLE : " << uniformName << " not found or not used." << endl;
		return false;
	}
	glUniform1ui(ID, data);
	return true;
}

bool Shader::SendUniformData(const string& uniformName, GLfloat data)
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	//BIFA

	if (ID == -1) {
		cout << "SHADER VARIABLE : " << uniformName << " not found or not used." << endl;
		return false;
	}
	glUniform1f(ID, data);
	return true;
}

bool Shader::SendUniformData(const string& uniformName, GLfloat x, GLfloat y)
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	//BIFA

	if (ID == -1) {
		cout << "SHADER VARIABLE : " << uniformName << " not found or not used." << endl;
		return false;
	}
	glUniform2f(ID, x,y);
	return true;
}

bool Shader::SendUniformData(const string& uniformName, GLfloat x, GLfloat y, GLfloat z)
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	//BIFA

	if (ID == -1) {
		cout << "SHADER VARIABLE : " << uniformName << " not found or not used." << endl;
		return false;
	}
	glUniform3f(ID, x, y,z);
	return true;
}

bool Shader::SendUniformData(const string& uniformName, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	//BIFA

	if (ID == -1) {
		cout << "SHADER VARIABLE : " << uniformName << " not found or not used." << endl;
		return false;
	}
	glUniform4f(ID, x, y, z,w);
	return true;
}

