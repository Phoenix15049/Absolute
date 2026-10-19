#include <algorithm>
#include <cmath>
#include <gtc/matrix_transform.hpp>
#include "Core/Input.h"
#include "Core/Settings.h"
#include "Scene/Camera.h"

Camera::Camera()
{
	m_aspectRatio = 16.0f / 9.0f;
	m_nearClip = 0.05f;
	m_farClip = 1000.0f;
	m_view = glm::mat4(1.0f);
	m_projection = glm::mat4(1.0f);

	Reset();
}

void Camera::Reset()
{
	m_position = glm::vec3(0.0f, 2.0f, 6.0f);
	m_yaw = 0.0f;
	m_pitch = -15.0f;
	m_pivotDistance = 6.0f;
	m_speedMultiplier = 1.0f;
	m_focusTime = 0.0f;
	m_focusTarget = glm::vec3(0.0f);
	m_focusDistance = 0.0f;
	m_isNavigating = false;
}

void Camera::Update(float deltaTime, const Settings& settings, bool hasMouse, bool hasKeyboard)
{
	const Input* input = Input::Instance();

	const bool isLooking = input->IsMouseHeld(SDL_BUTTON_RIGHT);
	const bool isOrbiting = input->IsMouseHeld(SDL_BUTTON_LEFT) && input->IsAltHeld();
	const bool isPanning = input->IsMouseHeld(SDL_BUTTON_MIDDLE);
	const bool isMouseActive = isLooking || isOrbiting || isPanning;

	if (!isMouseActive)
	{
		m_isNavigating = false;
	}

	else if (hasMouse && (input->IsMousePressed(SDL_BUTTON_RIGHT) || input->IsMousePressed(SDL_BUTTON_LEFT) ||
		input->IsMousePressed(SDL_BUTTON_MIDDLE)))
	{
		m_isNavigating = true;
	}

	const float motionX = static_cast<float>(input->GetMouseMotionX());
	const float motionY = static_cast<float>(input->GetMouseMotionY()) * (settings.invertY ? -1.0f : 1.0f);

	if (m_isNavigating && (motionX != 0.0f || motionY != 0.0f))
	{
		m_focusTime = 0.0f;

		if (isLooking || isOrbiting)
		{
			const glm::vec3 pivot = m_position + GetForward() * m_pivotDistance;

			m_yaw += motionX * settings.mouseSensitivity;
			m_pitch = std::clamp(m_pitch - motionY * settings.mouseSensitivity, -89.0f, 89.0f);

			if (isOrbiting && !isLooking)
			{
				m_position = pivot - GetForward() * m_pivotDistance;
			}
		}

		else if (isPanning)
		{
			const float panSpeed = std::max(m_pivotDistance, 1.0f) * 0.0015f;
			m_position += (-GetRight() * motionX + GetUp() * motionY) * panSpeed;
		}
	}

	const float wheel = input->GetMouseWheel();

	if (hasMouse && wheel != 0.0f)
	{
		m_focusTime = 0.0f;

		if (isLooking)
		{
			m_speedMultiplier = std::clamp(m_speedMultiplier * (wheel > 0.0f ? 1.2f : 1.0f / 1.2f), 0.05f, 20.0f);
		}

		else
		{
			const float step = std::max(m_pivotDistance, 0.5f) * 0.15f * wheel;
			const float newDistance = std::max(m_pivotDistance - step, 0.1f);
			m_position += GetForward() * (m_pivotDistance - newDistance);
			m_pivotDistance = newDistance;
		}
	}

	const bool canMove = (hasKeyboard || m_isNavigating) && !input->IsCtrlHeld();

	if (canMove)
	{
		glm::vec3 movement(0.0f);

		if (input->IsKeyHeld(SDL_SCANCODE_W)) movement += GetForward();
		if (input->IsKeyHeld(SDL_SCANCODE_S)) movement -= GetForward();
		if (input->IsKeyHeld(SDL_SCANCODE_D)) movement += GetRight();
		if (input->IsKeyHeld(SDL_SCANCODE_A)) movement -= GetRight();
		if (input->IsKeyHeld(SDL_SCANCODE_E)) movement += glm::vec3(0.0f, 1.0f, 0.0f);
		if (input->IsKeyHeld(SDL_SCANCODE_Q)) movement -= glm::vec3(0.0f, 1.0f, 0.0f);

		if (glm::dot(movement, movement) > 0.0f)
		{
			float speed = settings.cameraSpeed * m_speedMultiplier;

			if (input->IsShiftHeld())
			{
				speed *= 3.0f;
			}

			m_position += glm::normalize(movement) * speed * deltaTime;
			m_focusTime = 0.0f;
		}
	}

	if (m_focusTime > 0.0f)
	{
		const float t = 1.0f - std::exp(-deltaTime * 12.0f);
		const glm::vec3 target = m_focusTarget - GetForward() * m_focusDistance;

		m_position = glm::mix(m_position, target, t);
		m_pivotDistance = glm::mix(m_pivotDistance, m_focusDistance, t);
		m_focusTime -= deltaTime;
	}

	UpdateMatrices(settings);
}

void Camera::Focus(const BoundingBox& bounds)
{
	if (!bounds.IsValid())
	{
		return;
	}

	const float radius = std::max(bounds.GetRadius(), 0.25f);

	m_focusTarget = bounds.GetCenter();
	m_focusDistance = radius * 2.2f;
	m_focusTime = 0.6f;
}

void Camera::SetAspectRatio(float aspectRatio)
{
	m_aspectRatio = aspectRatio;
}

const glm::vec3& Camera::GetPosition() const
{
	return m_position;
}

glm::vec3 Camera::GetForward() const
{
	return Math::DirectionFromAngles(m_pitch, m_yaw);
}

glm::vec3 Camera::GetRight() const
{
	return glm::normalize(glm::cross(GetForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
}

glm::vec3 Camera::GetUp() const
{
	return glm::normalize(glm::cross(GetRight(), GetForward()));
}

const glm::mat4& Camera::GetView() const
{
	return m_view;
}

const glm::mat4& Camera::GetProjection() const
{
	return m_projection;
}

glm::mat4 Camera::GetViewProjection() const
{
	return m_projection * m_view;
}

float Camera::GetNearClip() const
{
	return m_nearClip;
}

float Camera::GetFarClip() const
{
	return m_farClip;
}

bool Camera::IsNavigating() const
{
	return m_isNavigating;
}

Ray Camera::ScreenPointToRay(float x, float y, float width, float height) const
{
	const glm::vec2 ndc((x / width) * 2.0f - 1.0f, 1.0f - (y / height) * 2.0f);
	const glm::mat4 inverse = glm::inverse(GetViewProjection());

	glm::vec4 nearPoint = inverse * glm::vec4(ndc, -1.0f, 1.0f);
	glm::vec4 farPoint = inverse * glm::vec4(ndc, 1.0f, 1.0f);
	nearPoint /= nearPoint.w;
	farPoint /= farPoint.w;

	return { glm::vec3(nearPoint), glm::normalize(glm::vec3(farPoint - nearPoint)) };
}

void Camera::UpdateMatrices(const Settings& settings)
{
	m_nearClip = settings.nearClip;
	m_farClip = settings.farClip;

	m_view = glm::lookAt(m_position, m_position + GetForward(), glm::vec3(0.0f, 1.0f, 0.0f));
	m_projection = glm::perspective(glm::radians(settings.fieldOfView), m_aspectRatio, m_nearClip, m_farClip);
}
