#pragma once

#include <string>
#include <vector>
#include <glm.hpp>
#include <gl.h>
#include "Core/Math.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec4 tangent;
};

struct MeshData
{
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	int materialIndex = 0;

	BoundingBox CalculateBounds() const;
	void CalculateNormals();
	void CalculateTangents();
};

class Mesh
{

public:

	Mesh();
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;

	void Create(const MeshData& data);
	void Destroy();

	void Draw() const;

	GLsizei GetIndexCount() const;

private:

	GLuint m_VAO;
	GLuint m_VBO;
	GLuint m_EBO;
	GLsizei m_indexCount;

};
