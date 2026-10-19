#pragma once

#include <glm.hpp>
#include "Core/Math.h"

struct Settings;

class Camera
{

public:

	Camera();

	void Update(float deltaTime, const Settings& settings, bool hasMouse, bool hasKeyboard);
	void Focus(const BoundingBox& bounds);
	void Reset();

	void SetAspectRatio(float aspectRatio);

	const glm::vec3& GetPosition() const;
	glm::vec3 GetForward() const;
	glm::vec3 GetRight() const;
	glm::vec3 GetUp() const;

	const glm::mat4& GetView() const;
	const glm::mat4& GetProjection() const;
	glm::mat4 GetViewProjection() const;

	float GetNearClip() const;
	float GetFarClip() const;
	bool IsNavigating() const;

	Ray ScreenPointToRay(float x, float y, float width, float height) const;

private:

	void UpdateMatrices(const Settings& settings);

	glm::vec3 m_position;
	float m_yaw;
	float m_pitch;
	float m_pivotDistance;
	float m_speedMultiplier;

	glm::vec3 m_focusTarget;
	float m_focusDistance;
	float m_focusTime;

	float m_aspectRatio;
	float m_nearClip;
	float m_farClip;
	bool m_isNavigating;

	glm::mat4 m_view;
	glm::mat4 m_projection;

};
