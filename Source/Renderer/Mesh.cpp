#include <cmath>
#include <cstddef>
#include <utility>
#include "Renderer/Mesh.h"

BoundingBox MeshData::CalculateBounds() const
{
	BoundingBox bounds;

	for (const auto& vertex : vertices)
	{
		bounds.Expand(vertex.position);
	}

	return bounds;
}

void MeshData::CalculateNormals()
{
	for (auto& vertex : vertices)
	{
		vertex.normal = glm::vec3(0.0f);
	}

	for (size_t i = 0; i + 2 < indices.size(); i += 3)
	{
		Vertex& a = vertices[indices[i]];
		Vertex& b = vertices[indices[i + 1]];
		Vertex& c = vertices[indices[i + 2]];

		const glm::vec3 faceNormal = glm::cross(b.position - a.position, c.position - a.position);

		a.normal += faceNormal;
		b.normal += faceNormal;
		c.normal += faceNormal;
	}

	for (auto& vertex : vertices)
	{
		const float length = glm::length(vertex.normal);
		vertex.normal = (length > 0.0f) ? vertex.normal / length : glm::vec3(0.0f, 1.0f, 0.0f);
	}
}

void MeshData::CalculateTangents()
{
	std::vector<glm::vec3> tangents(vertices.size(), glm::vec3(0.0f));
	std::vector<glm::vec3> bitangents(vertices.size(), glm::vec3(0.0f));

	for (size_t i = 0; i + 2 < indices.size(); i += 3)
	{
		const GLuint ia = indices[i];
		const GLuint ib = indices[i + 1];
		const GLuint ic = indices[i + 2];

		const glm::vec3 edge1 = vertices[ib].position - vertices[ia].position;
		const glm::vec3 edge2 = vertices[ic].position - vertices[ia].position;
		const glm::vec2 deltaUV1 = vertices[ib].uv - vertices[ia].uv;
		const glm::vec2 deltaUV2 = vertices[ic].uv - vertices[ia].uv;

		const float determinant = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;

		if (std::abs(determinant) < 1e-12f)
		{
			continue;
		}

		const float inverse = 1.0f / determinant;
		const glm::vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * inverse;
		const glm::vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * inverse;

		for (GLuint index : { ia, ib, ic })
		{
			tangents[index] += tangent;
			bitangents[index] += bitangent;
		}
	}

	for (size_t i = 0; i < vertices.size(); i++)
	{
		const glm::vec3& normal = vertices[i].normal;
		glm::vec3 tangent = tangents[i] - normal * glm::dot(normal, tangents[i]);

		if (glm::dot(tangent, tangent) < 1e-12f)
		{
			const glm::vec3 axis = std::abs(normal.y) < 0.99f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
			tangent = glm::cross(axis, normal);
		}

		tangent = glm::normalize(tangent);
		const float handedness = glm::dot(glm::cross(normal, tangent), bitangents[i]) < 0.0f ? -1.0f : 1.0f;

		vertices[i].tangent = glm::vec4(tangent, handedness);
	}
}

Mesh::Mesh()
{
	m_VAO = 0;
	m_VBO = 0;
	m_EBO = 0;
	m_indexCount = 0;
}

Mesh::~Mesh()
{
	Destroy();
}

Mesh::Mesh(Mesh&& other) noexcept : Mesh()
{
	*this = std::move(other);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
	if (this != &other)
	{
		Destroy();

		m_VAO = other.m_VAO;
		m_VBO = other.m_VBO;
		m_EBO = other.m_EBO;
		m_indexCount = other.m_indexCount;

		other.m_VAO = 0;
		other.m_VBO = 0;
		other.m_EBO = 0;
		other.m_indexCount = 0;
	}

	return *this;
}

void Mesh::Create(const MeshData& data)
{
	Destroy();

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(Vertex), data.vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(GLuint), data.indices.data(), GL_STATIC_DRAW);

	const GLsizei stride = sizeof(Vertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, position)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, normal)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, uv)));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, tangent)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_indexCount = static_cast<GLsizei>(data.indices.size());
}

void Mesh::Destroy()
{
	if (m_EBO != 0)
	{
		glDeleteBuffers(1, &m_EBO);
		m_EBO = 0;
	}

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

	m_indexCount = 0;
}

void Mesh::Draw() const
{
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
}

GLsizei Mesh::GetIndexCount() const
{
	return m_indexCount;
}
