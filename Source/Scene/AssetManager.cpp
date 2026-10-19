#include <chrono>
#include <iomanip>
#include <sstream>
#include "Core/Log.h"
#include "Core/Platform.h"
#include "Scene/AssetManager.h"

AssetManager::~AssetManager()
{
	Shutdown();
}

void AssetManager::LoadModelAsync(const std::string& path, ModelCallback callback)
{
	auto cached = m_models.find(path);

	if (cached != m_models.end())
	{
		if (auto model = cached->second.lock())
		{
			callback(model);
			return;
		}
	}

	for (auto& pending : m_pending)
	{
		if (pending.path == path)
		{
			pending.callbacks.push_back(std::move(callback));
			return;
		}
	}

	PendingModel pending;
	pending.path = path;
	pending.callbacks.push_back(std::move(callback));
	pending.future = std::async(std::launch::async, [path]()
	{
		auto data = std::make_unique<ModelData>();
		ModelLoader::LoadOBJ(path, *data);
		return data;
	});

	Log::Info("Loading " + path + "...");
	m_pending.push_back(std::move(pending));
}

std::shared_ptr<ModelAsset> AssetManager::CreatePrimitive(PrimitiveType type)
{
	std::vector<MeshData> meshes;
	meshes.push_back(Primitives::Create(type));

	auto asset = BuildAsset(meshes);
	asset->name = Primitives::GetName(type);
	asset->path = std::string("primitive:") + asset->name;
	asset->materials.emplace_back();

	if (type == PrimitiveType::Plane)
	{
		asset->materials.back().diffuse = glm::vec3(0.55f);
		asset->materials.back().specular = glm::vec3(0.05f);
	}

	return asset;
}

std::shared_ptr<Texture> AssetManager::LoadTexture(const std::string& path, bool isSRGB)
{
	return GetOrCreateTexture(path, isSRGB, nullptr);
}

void AssetManager::Update()
{
	for (size_t i = 0; i < m_pending.size();)
	{
		PendingModel& pending = m_pending[i];

		if (pending.future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
		{
			i++;
			continue;
		}

		std::unique_ptr<ModelData> data = pending.future.get();
		std::shared_ptr<ModelAsset> model;

		if (data->error.empty())
		{
			model = Upload(*data);
			m_models[pending.path] = model;

			std::ostringstream message;
			message << "Loaded " << pending.path << " (" << model->meshes.size() << " meshes, "
				<< model->triangleCount << " triangles) in " << std::fixed << std::setprecision(2) << data->loadTime << "s";
			Log::Info(message.str());
		}

		else
		{
			Log::Error(data->error);
		}

		const auto callbacks = std::move(pending.callbacks);
		m_pending.erase(m_pending.begin() + i);

		for (const auto& callback : callbacks)
		{
			callback(model);
		}
	}
}

void AssetManager::ApplyAnisotropy()
{
	for (const auto& texture : m_textures)
	{
		texture.second->ApplyAnisotropy();
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void AssetManager::CollectGarbage()
{
	for (auto it = m_textures.begin(); it != m_textures.end();)
	{
		it = (it->second.use_count() == 1) ? m_textures.erase(it) : std::next(it);
	}

	for (auto it = m_models.begin(); it != m_models.end();)
	{
		it = it->second.expired() ? m_models.erase(it) : std::next(it);
	}
}

void AssetManager::Shutdown()
{
	for (auto& pending : m_pending)
	{
		if (pending.future.valid())
		{
			pending.future.wait();
		}
	}

	m_pending.clear();
	m_models.clear();
	m_textures.clear();
}

bool AssetManager::IsLoading() const
{
	return !m_pending.empty();
}

size_t AssetManager::GetPendingCount() const
{
	return m_pending.size();
}

size_t AssetManager::GetModelCount() const
{
	size_t count = 0;

	for (const auto& model : m_models)
	{
		count += model.second.expired() ? 0 : 1;
	}

	return count;
}

size_t AssetManager::GetTextureCount() const
{
	return m_textures.size();
}

std::shared_ptr<ModelAsset> AssetManager::Upload(ModelData& data)
{
	auto asset = BuildAsset(data.meshes);
	asset->path = data.path;
	asset->name = Platform::GetFileStem(data.path);
	asset->materials = std::move(data.materials);

	for (auto& material : asset->materials)
	{
		if (!material.diffusePath.empty())
		{
			material.diffuseMap = GetOrCreateTexture(material.diffusePath, true, &data);
		}

		if (!material.specularPath.empty())
		{
			material.specularMap = GetOrCreateTexture(material.specularPath, false, &data);
		}

		if (!material.normalPath.empty())
		{
			material.normalMap = GetOrCreateTexture(material.normalPath, false, &data);
		}
	}

	return asset;
}

std::shared_ptr<Texture> AssetManager::GetOrCreateTexture(const std::string& path, bool isSRGB, ModelData* data)
{
	const std::string key = path + (isSRGB ? "|srgb" : "|linear");
	auto cached = m_textures.find(key);

	if (cached != m_textures.end())
	{
		return cached->second;
	}

	auto texture = std::make_shared<Texture>();
	bool isCreated = false;

	if (data)
	{
		auto image = data->images.find(path);
		isCreated = (image != data->images.end()) && texture->Create(image->second, isSRGB);
	}

	else
	{
		isCreated = texture->Load(path, isSRGB);
	}

	if (!isCreated)
	{
		return nullptr;
	}

	m_textures.emplace(key, texture);
	return texture;
}

std::shared_ptr<ModelAsset> AssetManager::BuildAsset(std::vector<MeshData>& meshes)
{
	auto asset = std::make_shared<ModelAsset>();
	asset->meshes.reserve(meshes.size());

	for (auto& data : meshes)
	{
		SubMesh subMesh;
		subMesh.name = data.name;
		subMesh.materialIndex = data.materialIndex;
		subMesh.bounds = data.CalculateBounds();
		subMesh.mesh.Create(data);

		subMesh.positions.reserve(data.vertices.size());

		for (const auto& vertex : data.vertices)
		{
			subMesh.positions.push_back(vertex.position);
		}

		subMesh.indices = std::move(data.indices);

		asset->bounds.Expand(subMesh.bounds);
		asset->vertexCount += data.vertices.size();
		asset->triangleCount += subMesh.indices.size() / 3;

		asset->meshes.push_back(std::move(subMesh));
	}

	return asset;
}
