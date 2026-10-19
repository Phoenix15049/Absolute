#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm.hpp>
#include "Core/Math.h"
#include "Renderer/Mesh.h"
#include "Renderer/Texture.h"

struct Material
{
	std::string name = "Default";

	glm::vec3 diffuse = glm::vec3(0.8f);
	glm::vec3 specular = glm::vec3(0.25f);
	glm::vec3 emissive = glm::vec3(0.0f);
	float emissiveStrength = 1.0f;
	float shininess = 32.0f;
	float opacity = 1.0f;
	float normalStrength = 1.0f;
	bool doubleSided = false;

	std::string diffusePath;
	std::string specularPath;
	std::string normalPath;

	std::shared_ptr<Texture> diffuseMap;
	std::shared_ptr<Texture> specularMap;
	std::shared_ptr<Texture> normalMap;

	bool IsTransparent() const
	{
		return opacity < 0.999f;
	}
};

struct SubMesh
{
	std::string name;
	Mesh mesh;
	int materialIndex = 0;
	BoundingBox bounds;

	std::vector<glm::vec3> positions;
	std::vector<GLuint> indices;
};

struct ModelAsset
{
	std::string path;
	std::string name;

	std::vector<SubMesh> meshes;
	std::vector<Material> materials;

	BoundingBox bounds;
	size_t vertexCount = 0;
	size_t triangleCount = 0;
};

struct ModelData
{
	std::string path;
	std::vector<MeshData> meshes;
	std::vector<Material> materials;
	std::unordered_map<std::string, ImageData> images;
	std::string error;
	double loadTime = 0.0;
};

namespace ModelLoader
{
	bool LoadOBJ(const std::string& path, ModelData& data);
}
