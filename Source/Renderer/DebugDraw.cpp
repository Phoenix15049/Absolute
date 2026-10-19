#include <cmath>
#include <cstddef>
#include <gtc/constants.hpp>
#include "Renderer/DebugDraw.h"
#include "Renderer/Shader.h"

DebugDraw::DebugDraw()
{
	m_VAO = 0;
	m_VBO = 0;
	m_capacity = 0;
}

DebugDraw::~DebugDraw()
{
	Destroy();
}

void DebugDraw::Initialize()
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), reinterpret_cast<void*>(offsetof(DebugVertex, position)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), reinterpret_cast<void*>(offsetof(DebugVertex, color)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugDraw::Destroy()
{
	if (m_VBO != 0)
	{
		glDeleteBuffers(1, &m_VBO);
		m_VBO = 0;
	}

	if (m_VAO != 0)
	{
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
	}

	m_capacity = 0;
}

void DebugDraw::AddLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color)
{
	m_lines.push_back({ start, color });
	m_lines.push_back({ end, color });
}

void DebugDraw::AddBox(const BoundingBox& box, const glm::vec4& color)
{
	if (!box.IsValid())
	{
		return;
	}

	const glm::vec3& a = box.min;
	const glm::vec3& b = box.max;

	const glm::vec3 corners[] =
	{
		{ a.x, a.y, a.z }, { b.x, a.y, a.z }, { b.x, a.y, b.z }, { a.x, a.y, b.z },
		{ a.x, b.y, a.z }, { b.x, b.y, a.z }, { b.x, b.y, b.z }, { a.x, b.y, b.z }
	};

	for (int i = 0; i < 4; i++)
	{
		AddLine(corners[i], corners[(i + 1) % 4], color);
		AddLine(corners[i + 4], corners[(i + 1) % 4 + 4], color);
		AddLine(corners[i], corners[i + 4], color);
	}
}

void DebugDraw::AddCircle(const glm::vec3& center, const glm::vec3& normal, float radius, const glm::vec4& color, int segments)
{
	const glm::vec3 axis = std::abs(normal.y) < 0.99f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
	const glm::vec3 u = glm::normalize(glm::cross(normal, axis)) * radius;
	const glm::vec3 v = glm::normalize(glm::cross(normal, u)) * radius;

	glm::vec3 previous = center + u;

	for (int i = 1; i <= segments; i++)
	{
		const float angle = static_cast<float>(i) / segments * glm::two_pi<float>();
		const glm::vec3 current = center + u * std::cos(angle) + v * std::sin(angle);

		AddLine(previous, current, color);
		previous = current;
	}
}

void DebugDraw::AddCone(const glm::vec3& apex, const glm::vec3& direction, float length, float angle, const glm::vec4& color)
{
	const glm::vec3 forward = glm::normalize(direction);
	const float radius = std::tan(glm::radians(angle)) * length;
	const glm::vec3 baseCenter = apex + forward * length;

	AddCircle(baseCenter, forward, radius, color, 32);

	const glm::vec3 axis = std::abs(forward.y) < 0.99f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
	const glm::vec3 u = glm::normalize(glm::cross(forward, axis)) * radius;
	const glm::vec3 v = glm::normalize(glm::cross(forward, u)) * radius;

	AddLine(apex, baseCenter + u, color);
	AddLine(apex, baseCenter - u, color);
	AddLine(apex, baseCenter + v, color);
	AddLine(apex, baseCenter - v, color);
}

void DebugDraw::AddPoint(const glm::vec3& point, const glm::vec4& color)
{
	m_points.push_back({ point, color });
}

void DebugDraw::Render(const Shader& shader, const glm::mat4& viewProjection, float pointSize)
{
	if (m_lines.empty() && m_points.empty())
	{
		return;
	}

	shader.Use();
	shader.SetUniform("viewProjection", viewProjection);
	shader.SetUniform("pointSize", pointSize);

	glBindVertexArray(m_VAO);

	if (!m_lines.empty())
	{
		Upload(m_lines);
		shader.SetUniform("isPoint", false);
		glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_lines.size()));
	}

	if (!m_points.empty())
	{
		Upload(m_points);
		shader.SetUniform("isPoint", true);
		glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_points.size()));
	}

	glBindVertexArray(0);
	Clear();
}

void DebugDraw::Clear()
{
	m_lines.clear();
	m_points.clear();
}

void DebugDraw::Upload(const std::vector<DebugVertex>& vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	const size_t size = vertices.size() * sizeof(DebugVertex);

	if (size > m_capacity)
	{
		m_capacity = size * 2;
		glBufferData(GL_ARRAY_BUFFER, m_capacity, nullptr, GL_STREAM_DRAW);
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices.data());
}
