#pragma once

#include <vector>
#include <glm.hpp>
#include <gl.h>
#include "Core/Math.h"

class Shader;

class DebugDraw
{

public:

	DebugDraw();
	~DebugDraw();

	DebugDraw(const DebugDraw&) = delete;
	DebugDraw& operator=(const DebugDraw&) = delete;

	void Initialize();
	void Destroy();

	void AddLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);
	void AddBox(const BoundingBox& box, const glm::vec4& color);
	void AddCircle(const glm::vec3& center, const glm::vec3& normal, float radius, const glm::vec4& color, int segments = 48);
	void AddCone(const glm::vec3& apex, const glm::vec3& direction, float length, float angle, const glm::vec4& color);
	void AddPoint(const glm::vec3& point, const glm::vec4& color);

	void Render(const Shader& shader, const glm::mat4& viewProjection, float pointSize);
	void Clear();

private:

	struct DebugVertex
	{
		glm::vec3 position;
		glm::vec4 color;
	};

	void Upload(const std::vector<DebugVertex>& vertices);

	std::vector<DebugVertex> m_lines;
	std::vector<DebugVertex> m_points;

	GLuint m_VAO;
	GLuint m_VBO;
	size_t m_capacity;

};
