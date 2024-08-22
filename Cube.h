#pragma once

#include <glm.hpp>
#include "Buffer.h"

class Cube
{

public:

	Cube();
	~Cube();

	void Update();
	void Render();

private:

	Buffer m_buffer;

	glm::mat4 m_model;
	glm::vec3 m_position;

};