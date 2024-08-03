#ifndef LIGHT_H
#define LIGHT_H

#include <glm.hpp>
#include "Buffer.h"

class Light {

public:

	Light();
	~Light();

	
	void Update();
	void Render();
	void SendToShader();

protected:

	Buffer m_buffer;

	glm::mat4 m_model;
	glm::vec3 m_position;

	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;

};

#endif // !LIGHT_H
