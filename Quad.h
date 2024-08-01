#ifndef QUAD_H
#define QUAD_H

#include "glad.h"
#include "Buffer.h"
class Quad {

public:

	Quad();
	~Quad();

	void Render();

private:
	Buffer m_buffer;

	
};

#endif // !QUAD_H
