#pragma once

#include <glm.hpp>
#include "gl.h"
#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"

class Quad
{

public:

	Quad();
	~Quad();

	void Update();
	void Render(const Shader& shader);

private:

	Buffer m_buffer;	
	Texture m_texture;

	glm::mat4 m_model;
	glm::mat3 m_normal;
	glm::vec3 m_position;

	float m_shininess;
	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;

};