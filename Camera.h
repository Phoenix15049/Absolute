#pragma once

#include <glm.hpp>
#include "gl.h"

class Camera
{

public:

	Camera();

	void Set3DView();
	void Update();

protected:

	glm::mat4 m_view;
	glm::mat4 m_proj;

	glm::vec3 m_position;
	glm::vec3 m_direction;
	glm::vec3 m_up;

};