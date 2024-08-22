#pragma once

#include <string>
#include "gl.h"

class Texture
{

public:

	Texture();

	void Bind();
	bool Load(const std::string& filename);
	void Unbind();
	void Unload();

private:

	GLuint m_ID;

};