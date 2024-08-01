#ifndef QUAD_H
#define QUAD_H

#include <glm.hpp>
#include "glad.h"
#include "Buffer.h"

class Quad {

public:

	Quad();
	~Quad();

	void Update();
	void Render();


private:
	Buffer m_buffer;
	glm::mat4 m_model;
	glm::vec3 m_position;
	
};

#endif // !QUAD_H
