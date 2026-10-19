#include <cmath>
#include <gtc/constants.hpp>
#include "Renderer/Primitives.h"

namespace
{
	void AddQuad(MeshData& data, const glm::vec3& center, const glm::vec3& right, const glm::vec3& up, const glm::vec2& uvScale)
	{
		const GLuint start = static_cast<GLuint>(data.vertices.size());
		const glm::vec3 normal = glm::normalize(glm::cross(right, up));

		const glm::vec3 corners[] = { center - right - up, center + right - up, center + right + up, center - right + up };
		const glm::vec2 uvs[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		for (int i = 0; i < 4; i++)
		{
			data.vertices.push_back({ corners[i], normal, uvs[i] * uvScale, glm::vec4(0.0f) });
		}

		data.indices.insert(data.indices.end(), { start, start + 1, start + 2, start, start + 2, start + 3 });
	}
}

const char* Primitives::GetName(PrimitiveType type)
{
	switch (type)
	{
		case PrimitiveType::Cube: return "Cube";
		case PrimitiveType::Sphere: return "Sphere";
		case PrimitiveType::Plane: return "Plane";
		case PrimitiveType::Cylinder: return "Cylinder";
	}

	return "Primitive";
}

bool Primitives::FromSource(const std::string& source, PrimitiveType& type)
{
	for (PrimitiveType candidate : { PrimitiveType::Cube, PrimitiveType::Sphere, PrimitiveType::Plane, PrimitiveType::Cylinder })
	{
		if (source == std::string("primitive:") + GetName(candidate))
		{
			type = candidate;
			return true;
		}
	}

	return false;
}

MeshData Primitives::CreateCube()
{
	MeshData data;
	data.name = "Cube";

	const glm::vec3 x(0.5f, 0.0f, 0.0f);
	const glm::vec3 y(0.0f, 0.5f, 0.0f);
	const glm::vec3 z(0.0f, 0.0f, 0.5f);
	const glm::vec2 uv(1.0f);

	AddQuad(data, z, x, y, uv);
	AddQuad(data, -z, -x, y, uv);
	AddQuad(data, x, -z, y, uv);
	AddQuad(data, -x, z, y, uv);
	AddQuad(data, y, x, -z, uv);
	AddQuad(data, -y, x, z, uv);

	data.CalculateTangents();
	return data;
}

MeshData Primitives::CreateSphere(int segments, int rings)
{
	MeshData data;
	data.name = "Sphere";

	for (int ring = 0; ring <= rings; ring++)
	{
		const float v = static_cast<float>(ring) / rings;
		const float phi = v * glm::pi<float>();

		for (int segment = 0; segment <= segments; segment++)
		{
			const float u = static_cast<float>(segment) / segments;
			const float theta = u * glm::two_pi<float>();

			const glm::vec3 normal(std::sin(phi) * std::sin(theta), std::cos(phi), std::sin(phi) * std::cos(theta));
			data.vertices.push_back({ normal * 0.5f, normal, glm::vec2(u, 1.0f - v), glm::vec4(0.0f) });
		}
	}

	const GLuint stride = segments + 1;

	for (int ring = 0; ring < rings; ring++)
	{
		for (int segment = 0; segment < segments; segment++)
		{
			const GLuint a = ring * stride + segment;
			const GLuint b = a + stride;

			data.indices.insert(data.indices.end(), { a, b, a + 1, a + 1, b, b + 1 });
		}
	}

	data.CalculateTangents();
	return data;
}

MeshData Primitives::CreatePlane(float size, float tiling)
{
	MeshData data;
	data.name = "Plane";

	const float half = size * 0.5f;
	AddQuad(data, glm::vec3(0.0f), glm::vec3(half, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -half), glm::vec2(tiling));

	data.CalculateTangents();
	return data;
}

MeshData Primitives::CreateCylinder(int segments)
{
	MeshData data;
	data.name = "Cylinder";

	for (int segment = 0; segment <= segments; segment++)
	{
		const float u = static_cast<float>(segment) / segments;
		const float theta = u * glm::two_pi<float>();
		const glm::vec3 normal(std::sin(theta), 0.0f, std::cos(theta));

		data.vertices.push_back({ normal * 0.5f + glm::vec3(0.0f, -0.5f, 0.0f), normal, glm::vec2(u, 0.0f), glm::vec4(0.0f) });
		data.vertices.push_back({ normal * 0.5f + glm::vec3(0.0f, 0.5f, 0.0f), normal, glm::vec2(u, 1.0f), glm::vec4(0.0f) });
	}

	for (int segment = 0; segment < segments; segment++)
	{
		const GLuint a = segment * 2;
		data.indices.insert(data.indices.end(), { a, a + 2, a + 1, a + 1, a + 2, a + 3 });
	}

	for (float side : { -1.0f, 1.0f })
	{
		const GLuint center = static_cast<GLuint>(data.vertices.size());
		const glm::vec3 normal(0.0f, side, 0.0f);

		data.vertices.push_back({ glm::vec3(0.0f, 0.5f * side, 0.0f), normal, glm::vec2(0.5f), glm::vec4(0.0f) });

		for (int segment = 0; segment <= segments; segment++)
		{
			const float theta = static_cast<float>(segment) / segments * glm::two_pi<float>();
			const glm::vec3 offset(std::sin(theta) * 0.5f, 0.5f * side, std::cos(theta) * 0.5f);

			data.vertices.push_back({ offset, normal, glm::vec2(offset.x, offset.z) + 0.5f, glm::vec4(0.0f) });
		}

		for (int segment = 0; segment < segments; segment++)
		{
			const GLuint a = center + 1 + segment;

			if (side > 0.0f)
			{
				data.indices.insert(data.indices.end(), { center, a, a + 1 });
			}

			else
			{
				data.indices.insert(data.indices.end(), { center, a + 1, a });
			}
		}
	}

	data.CalculateTangents();
	return data;
}

MeshData Primitives::Create(PrimitiveType type)
{
	switch (type)
	{
		case PrimitiveType::Sphere: return CreateSphere();
		case PrimitiveType::Plane: return CreatePlane();
		case PrimitiveType::Cylinder: return CreateCylinder();
		default: return CreateCube();
	}
}
