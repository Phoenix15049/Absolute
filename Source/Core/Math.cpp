#include <algorithm>
#include <cmath>
#include "Core/Math.h"

BoundingBox BoundingBox::Transformed(const glm::mat4& matrix) const
{
	if (!IsValid())
	{
		return *this;
	}

	const glm::vec3 center = glm::vec3(matrix * glm::vec4(GetCenter(), 1.0f));
	const glm::vec3 extents = GetSize() * 0.5f;
	const glm::mat3 absolute = glm::mat3(glm::abs(glm::vec3(matrix[0])),
		glm::abs(glm::vec3(matrix[1])),
		glm::abs(glm::vec3(matrix[2])));
	const glm::vec3 newExtents = absolute * extents;

	BoundingBox result;
	result.min = center - newExtents;
	result.max = center + newExtents;
	return result;
}

void Frustum::Extract(const glm::mat4& m)
{
	const glm::vec4 row0(m[0][0], m[1][0], m[2][0], m[3][0]);
	const glm::vec4 row1(m[0][1], m[1][1], m[2][1], m[3][1]);
	const glm::vec4 row2(m[0][2], m[1][2], m[2][2], m[3][2]);
	const glm::vec4 row3(m[0][3], m[1][3], m[2][3], m[3][3]);

	m_planes[0] = row3 + row0;
	m_planes[1] = row3 - row0;
	m_planes[2] = row3 + row1;
	m_planes[3] = row3 - row1;
	m_planes[4] = row3 + row2;
	m_planes[5] = row3 - row2;

	for (auto& plane : m_planes)
	{
		plane /= glm::length(glm::vec3(plane));
	}
}

bool Frustum::Intersects(const BoundingBox& box) const
{
	for (const auto& plane : m_planes)
	{
		const glm::vec3 positive(plane.x >= 0.0f ? box.max.x : box.min.x,
			plane.y >= 0.0f ? box.max.y : box.min.y,
			plane.z >= 0.0f ? box.max.z : box.min.z);

		if (glm::dot(glm::vec3(plane), positive) + plane.w < 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool Math::IntersectRayBox(const Ray& ray, const BoundingBox& box, float& distance)
{
	const glm::vec3 inverse = 1.0f / ray.direction;
	const glm::vec3 t0 = (box.min - ray.origin) * inverse;
	const glm::vec3 t1 = (box.max - ray.origin) * inverse;

	const glm::vec3 smaller = glm::min(t0, t1);
	const glm::vec3 larger = glm::max(t0, t1);

	const float nearest = std::max(std::max(smaller.x, smaller.y), smaller.z);
	const float farthest = std::min(std::min(larger.x, larger.y), larger.z);

	if (farthest < 0.0f || nearest > farthest)
	{
		return false;
	}

	distance = nearest > 0.0f ? nearest : 0.0f;
	return true;
}

bool Math::IntersectRayTriangle(const Ray& ray, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, float& distance)
{
	const glm::vec3 edge1 = b - a;
	const glm::vec3 edge2 = c - a;
	const glm::vec3 p = glm::cross(ray.direction, edge2);
	const float determinant = glm::dot(edge1, p);

	if (std::abs(determinant) < 1e-8f)
	{
		return false;
	}

	const float inverse = 1.0f / determinant;
	const glm::vec3 s = ray.origin - a;
	const float u = glm::dot(s, p) * inverse;

	if (u < 0.0f || u > 1.0f)
	{
		return false;
	}

	const glm::vec3 q = glm::cross(s, edge1);
	const float v = glm::dot(ray.direction, q) * inverse;

	if (v < 0.0f || u + v > 1.0f)
	{
		return false;
	}

	const float t = glm::dot(edge2, q) * inverse;

	if (t <= 0.0f)
	{
		return false;
	}

	distance = t;
	return true;
}

bool Math::IntersectRaySphere(const Ray& ray, const glm::vec3& center, float radius, float& distance)
{
	const glm::vec3 offset = ray.origin - center;
	const float b = glm::dot(offset, ray.direction);
	const float c = glm::dot(offset, offset) - radius * radius;
	const float discriminant = b * b - c;

	if (discriminant < 0.0f)
	{
		return false;
	}

	const float t = -b - std::sqrt(discriminant);

	if (t < 0.0f)
	{
		return false;
	}

	distance = t;
	return true;
}

glm::vec3 Math::DirectionFromAngles(float pitch, float yaw)
{
	const float pitchRadians = glm::radians(pitch);
	const float yawRadians = glm::radians(yaw);

	return glm::normalize(glm::vec3(std::cos(pitchRadians) * std::sin(yawRadians),
		std::sin(pitchRadians),
		-std::cos(pitchRadians) * std::cos(yawRadians)));
}
