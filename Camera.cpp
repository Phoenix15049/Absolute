#include <gtc/matrix_transform.hpp>
#include "Camera.h"
#include "Input.h"
#include "Shader.h"

Camera::Camera() {
	m_view = glm::mat4(1.0f);
	m_proj = glm::mat4(1.0f);


	m_position = glm::vec3(0.0f);
	m_direction = glm::vec3(0.0f,0.0f,-1.0f);
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Camera::Set3DView()
{
	GLfloat FOV = 60.0f;
	GLfloat aspectRatio = 1280.0f / 720.0f;

	m_proj = glm::perspective(FOV, aspectRatio, 0.001f, 1000.0f);
	Shader::Instance()->SendUniformData("proj", m_proj);


}

void Camera::Update() {

	if (Input::Instance()->IsKeyPressed()) {

		char kp = Input::Instance()->GetKeyDown();
		if (kp == 'a') {
			m_position.x -= 0.01f;
		}
		if (kp == 'd') {
			m_position.x += 0.01f;
		}
		if (kp == 'w') {
			m_position.z -= 0.01f;
		}
		if (kp == 's') {
			m_position.z += 0.01f;
		}
		if (kp == 'e') {
			m_position.y -= 0.01f;
		}
		if (kp == 'q') {
			m_position.y += 0.01f;
		}
	}


	m_view = glm::lookAt(m_position, m_position + m_direction, m_up);

	Shader::Instance()->SendUniformData("view", m_view);
}