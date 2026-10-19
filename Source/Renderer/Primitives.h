#pragma once

#include <string>
#include "Renderer/Mesh.h"

enum class PrimitiveType
{
	Cube,
	Sphere,
	Plane,
	Cylinder
};

namespace Primitives
{
	const char* GetName(PrimitiveType type);
	bool FromSource(const std::string& source, PrimitiveType& type);

	MeshData CreateCube();
	MeshData CreateSphere(int segments = 48, int rings = 24);
	MeshData CreatePlane(float size = 10.0f, float tiling = 5.0f);
	MeshData CreateCylinder(int segments = 48);
	MeshData Create(PrimitiveType type);
}
