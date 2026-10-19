#pragma once

#include <cfloat>
#include <glm.hpp>

struct BoundingBox
{
	glm::vec3 min = glm::vec3(FLT_MAX);
	glm::vec3 max = glm::vec3(-FLT_MAX);

	bool IsValid() const
	{
		return min.x <= max.x && min.y <= max.y && min.z <= max.z;
	}

	void Expand(const glm::vec3& point)
	{
		min = glm::min(min, point);
		max = glm::max(max, point);
	}

	void Expand(const BoundingBox& other)
	{
		if (other.IsValid())
		{
			Expand(other.min);
			Expand(other.max);
		}
	}

	glm::vec3 GetCenter() const
	{
		return (min + max) * 0.5f;
	}

	glm::vec3 GetSize() const
	{
		return max - min;
	}

	float GetRadius() const
	{
		return glm::length(GetSize()) * 0.5f;
	}

	BoundingBox Transformed(const glm::mat4& matrix) const;
};

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

class Frustum
{

public:

	void Extract(const glm::mat4& viewProjection);
	bool Intersects(const BoundingBox& box) const;

private:

	glm::vec4 m_planes[6];

};

namespace Math
{
	bool IntersectRayBox(const Ray& ray, const BoundingBox& box, float& distance);
	bool IntersectRayTriangle(const Ray& ray, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, float& distance);
	bool IntersectRaySphere(const Ray& ray, const glm::vec3& center, float radius, float& distance);
	glm::vec3 DirectionFromAngles(float pitch, float yaw);
}
