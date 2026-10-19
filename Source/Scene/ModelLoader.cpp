#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include "Core/Log.h"
#include "Core/Platform.h"
#include "Scene/Model.h"

namespace fs = std::filesystem;

namespace
{
	struct VertexKey
	{
		int v = -1;
		int t = -1;
		int n = -1;

		bool operator==(const VertexKey& other) const
		{
			return v == other.v && t == other.t && n == other.n;
		}
	};

	struct VertexKeyHash
	{
		size_t operator()(const VertexKey& key) const
		{
			size_t hash = std::hash<int>()(key.v);
			hash ^= std::hash<int>()(key.t) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= std::hash<int>()(key.n) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			return hash;
		}
	};

	struct ObjState
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> textureCoords;

		std::vector<VertexKey> triangles;
		std::string meshName;
		int materialIndex = -1;
	};

	const char* SkipSpaces(const char* text)
	{
		while (*text == ' ' || *text == '\t')
		{
			text++;
		}

		return text;
	}

	bool IsKeyword(const char* text, const char* keyword)
	{
		while (*keyword)
		{
			if (*text++ != *keyword++)
			{
				return false;
			}
		}

		return *text == ' ' || *text == '\t' || *text == '\0';
	}

	float ReadFloat(const char*& text)
	{
		char* end = nullptr;
		const float value = std::strtof(SkipSpaces(text), &end);
		text = end;
		return value;
	}

	std::string ReadRest(const char* text)
	{
		std::string value = SkipSpaces(text);

		while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || value.back() == '\r'))
		{
			value.pop_back();
		}

		return value;
	}

	std::string ReadLastToken(const char* text)
	{
		const std::string rest = ReadRest(text);
		const size_t space = rest.find_last_of(" \t");
		return (space == std::string::npos) ? rest : rest.substr(space + 1);
	}

	int ResolveIndex(int index, size_t count)
	{
		const int resolved = (index < 0) ? static_cast<int>(count) + index : index - 1;
		return (resolved >= 0 && resolved < static_cast<int>(count)) ? resolved : -2;
	}

	bool ReadVertexKey(const char*& text, const ObjState& state, VertexKey& key)
	{
		char* end = nullptr;

		key.v = ResolveIndex(static_cast<int>(std::strtol(text, &end, 10)), state.positions.size());

		if (end == text || key.v < 0)
		{
			return false;
		}

		text = end;

		if (*text == '/')
		{
			text++;

			if (*text != '/')
			{
				key.t = ResolveIndex(static_cast<int>(std::strtol(text, &end, 10)), state.textureCoords.size());
				text = end;
			}

			if (*text == '/')
			{
				text++;
				key.n = ResolveIndex(static_cast<int>(std::strtol(text, &end, 10)), state.normals.size());
				text = end;
			}
		}

		if (key.t == -2) key.t = -1;
		if (key.n == -2) key.n = -1;

		return true;
	}

	std::string ResolvePath(const std::string& file, std::initializer_list<std::string> directories)
	{
		if (file.empty())
		{
			return std::string();
		}

		std::error_code error;
		const fs::path filePath(file);

		if (fs::is_regular_file(filePath, error))
		{
			return filePath.generic_string();
		}

		for (const auto& directory : directories)
		{
			for (const fs::path& candidate : { fs::path(directory) / filePath, fs::path(directory) / filePath.filename() })
			{
				if (fs::is_regular_file(candidate, error))
				{
					return candidate.lexically_normal().generic_string();
				}
			}
		}

		return std::string();
	}

	void FlushMesh(ObjState& state, ModelData& data)
	{
		if (state.triangles.empty())
		{
			return;
		}

		MeshData mesh;
		mesh.name = state.meshName.empty() ? "Mesh " + std::to_string(data.meshes.size()) : state.meshName;
		mesh.materialIndex = state.materialIndex;

		std::unordered_map<VertexKey, GLuint, VertexKeyHash> map;
		map.reserve(state.triangles.size());
		mesh.indices.reserve(state.triangles.size());

		bool hasMissingNormals = false;

		for (const auto& key : state.triangles)
		{
			auto it = map.find(key);

			if (it != map.end())
			{
				mesh.indices.push_back(it->second);
				continue;
			}

			Vertex vertex;
			vertex.position = state.positions[key.v];
			vertex.normal = (key.n >= 0) ? state.normals[key.n] : glm::vec3(0.0f);
			vertex.uv = (key.t >= 0) ? state.textureCoords[key.t] : glm::vec2(0.0f);
			vertex.tangent = glm::vec4(0.0f);

			hasMissingNormals |= (key.n < 0);

			const GLuint index = static_cast<GLuint>(mesh.vertices.size());
			mesh.vertices.push_back(vertex);
			mesh.indices.push_back(index);
			map.emplace(key, index);
		}

		if (hasMissingNormals)
		{
			mesh.CalculateNormals();
		}

		mesh.CalculateTangents();

		data.meshes.push_back(std::move(mesh));
		state.triangles.clear();
	}

	void LoadMTL(const std::string& path, ModelData& data)
	{
		std::ifstream file(path);

		if (!file)
		{
			Log::Warning("Could not open material library " + path);
			return;
		}

		const std::string directory = Platform::GetDirectory(path);
		const std::string objDirectory = Platform::GetDirectory(data.path);

		auto resolveTexture = [&](const char* text)
		{
			const std::string file = ReadLastToken(text);
			const std::string resolved = ResolvePath(file, { directory, objDirectory, "Textures" });

			if (resolved.empty())
			{
				Log::Warning("Texture not found: " + file);
			}

			return resolved;
		};

		std::string line;
		Material* material = nullptr;

		while (std::getline(file, line))
		{
			const char* text = SkipSpaces(line.c_str());

			if (IsKeyword(text, "newmtl"))
			{
				data.materials.emplace_back();
				material = &data.materials.back();
				material->name = ReadRest(text + 6);
				continue;
			}

			if (!material)
			{
				continue;
			}

			if (IsKeyword(text, "Kd"))
			{
				text += 2;
				material->diffuse.x = ReadFloat(text);
				material->diffuse.y = ReadFloat(text);
				material->diffuse.z = ReadFloat(text);
			}

			else if (IsKeyword(text, "Ks"))
			{
				text += 2;
				material->specular.x = ReadFloat(text);
				material->specular.y = ReadFloat(text);
				material->specular.z = ReadFloat(text);
			}

			else if (IsKeyword(text, "Ke"))
			{
				text += 2;
				material->emissive.x = ReadFloat(text);
				material->emissive.y = ReadFloat(text);
				material->emissive.z = ReadFloat(text);
			}

			else if (IsKeyword(text, "Ns"))
			{
				text += 2;
				material->shininess = ReadFloat(text);
			}

			else if (IsKeyword(text, "d"))
			{
				text += 1;
				material->opacity = ReadFloat(text);
			}

			else if (IsKeyword(text, "Tr"))
			{
				text += 2;
				material->opacity = 1.0f - ReadFloat(text);
			}

			else if (IsKeyword(text, "map_Kd"))
			{
				material->diffusePath = resolveTexture(text + 6);
			}

			else if (IsKeyword(text, "map_Ks"))
			{
				material->specularPath = resolveTexture(text + 6);
			}

			else if (IsKeyword(text, "map_Bump") || IsKeyword(text, "map_bump"))
			{
				material->normalPath = resolveTexture(text + 8);
			}

			else if (IsKeyword(text, "bump") || IsKeyword(text, "norm"))
			{
				material->normalPath = resolveTexture(text + 4);
			}
		}
	}

	int FindMaterial(const ModelData& data, const std::string& name)
	{
		for (size_t i = 0; i < data.materials.size(); i++)
		{
			if (data.materials[i].name == name)
			{
				return static_cast<int>(i);
			}
		}

		return -1;
	}
}

bool ModelLoader::LoadOBJ(const std::string& path, ModelData& data)
{
	const auto startTime = std::chrono::steady_clock::now();

	data.path = path;

	std::ifstream file(path);

	if (!file)
	{
		data.error = "Could not open " + path;
		return false;
	}

	const std::string directory = Platform::GetDirectory(path);

	ObjState state;
	std::string line;
	std::vector<VertexKey> polygon;

	while (std::getline(file, line))
	{
		const char* text = SkipSpaces(line.c_str());

		if (text[0] == 'v' && (text[1] == ' ' || text[1] == '\t'))
		{
			text += 1;
			const float x = ReadFloat(text);
			const float y = ReadFloat(text);
			const float z = ReadFloat(text);
			state.positions.emplace_back(x, y, z);
		}

		else if (text[0] == 'v' && text[1] == 'n')
		{
			text += 2;
			const float x = ReadFloat(text);
			const float y = ReadFloat(text);
			const float z = ReadFloat(text);
			state.normals.emplace_back(x, y, z);
		}

		else if (text[0] == 'v' && text[1] == 't')
		{
			text += 2;
			const float u = ReadFloat(text);
			const float v = ReadFloat(text);
			state.textureCoords.emplace_back(u, v);
		}

		else if (text[0] == 'f' && (text[1] == ' ' || text[1] == '\t'))
		{
			text += 1;
			polygon.clear();

			while (true)
			{
				text = SkipSpaces(text);

				if (*text == '\0' || *text == '\r')
				{
					break;
				}

				VertexKey key;

				if (!ReadVertexKey(text, state, key))
				{
					polygon.clear();
					break;
				}

				polygon.push_back(key);
			}

			for (size_t i = 1; i + 1 < polygon.size(); i++)
			{
				state.triangles.push_back(polygon[0]);
				state.triangles.push_back(polygon[i]);
				state.triangles.push_back(polygon[i + 1]);
			}
		}

		else if (IsKeyword(text, "usemtl"))
		{
			FlushMesh(state, data);
			state.materialIndex = FindMaterial(data, ReadRest(text + 6));
		}

		else if (IsKeyword(text, "mtllib"))
		{
			const std::string library = ReadRest(text + 6);
			const std::string resolved = ResolvePath(library, { directory, "Materials" });

			if (resolved.empty())
			{
				Log::Warning("Material library not found: " + library);
			}

			else
			{
				LoadMTL(resolved, data);
			}
		}

		else if (IsKeyword(text, "o") || IsKeyword(text, "g"))
		{
			FlushMesh(state, data);
			state.meshName = ReadRest(text + 1);
		}
	}

	FlushMesh(state, data);

	if (data.meshes.empty())
	{
		data.error = path + " does not contain any faces.";
		return false;
	}

	int defaultMaterial = -1;

	for (auto& mesh : data.meshes)
	{
		if (mesh.materialIndex < 0)
		{
			if (defaultMaterial < 0)
			{
				data.materials.emplace_back();
				defaultMaterial = static_cast<int>(data.materials.size()) - 1;
			}

			mesh.materialIndex = defaultMaterial;
		}
	}

	for (const auto& material : data.materials)
	{
		for (const std::string* texturePath : { &material.diffusePath, &material.specularPath, &material.normalPath })
		{
			if (!texturePath->empty() && data.images.find(*texturePath) == data.images.end())
			{
				ImageData image;

				if (ImageData::Load(*texturePath, image))
				{
					data.images.emplace(*texturePath, std::move(image));
				}
			}
		}
	}

	data.loadTime = std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count();
	return true;
}
