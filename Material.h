#pragma once

#include <string>
#include <vector>
#include "gl.h"
#include <glm.hpp>
#include "Shader.h"
#include "Texture.h"

class Material
{

public:

	Material();

	const std::string& GetName() const;

	bool IsTextured() const;

	const Texture& GetNormalMap() const;
	const Texture& GetAmbientMap() const;
	const Texture& GetDiffuseMap() const;
	const Texture& GetSpecularMap() const;

	void SetShininess(GLfloat shininess);
	void SetAmbient(const glm::vec3& ambient);
	void SetDiffuse(const glm::vec3& diffuse);
	void SetSpecular(const glm::vec3& specular);
	
	bool Load(const std::string& filename, std::vector<Material>& materials);

	void SendToShader(const Shader& shader) const;

private:

	bool m_isTextured;
	std::string m_name;

	Texture m_normalMap;
	Texture m_ambientMap;
	Texture m_diffuseMap;
	Texture m_specularMap;

	GLfloat m_shininess;
	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;

};