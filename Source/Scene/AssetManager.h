#pragma once

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "Renderer/Primitives.h"
#include "Scene/Model.h"

class AssetManager
{

public:

	using ModelCallback = std::function<void(const std::shared_ptr<ModelAsset>&)>;

	AssetManager() = default;
	~AssetManager();

	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

	void LoadModelAsync(const std::string& path, ModelCallback callback);
	std::shared_ptr<ModelAsset> CreatePrimitive(PrimitiveType type);
	std::shared_ptr<Texture> LoadTexture(const std::string& path, bool isSRGB);

	void Update();
	void ApplyAnisotropy();
	void CollectGarbage();
	void Shutdown();

	bool IsLoading() const;
	size_t GetPendingCount() const;
	size_t GetModelCount() const;
	size_t GetTextureCount() const;

private:

	struct PendingModel
	{
		std::string path;
		std::future<std::unique_ptr<ModelData>> future;
		std::vector<ModelCallback> callbacks;
	};

	std::shared_ptr<ModelAsset> Upload(ModelData& data);
	std::shared_ptr<Texture> GetOrCreateTexture(const std::string& path, bool isSRGB, ModelData* data);

	static std::shared_ptr<ModelAsset> BuildAsset(std::vector<MeshData>& meshes);

	std::vector<PendingModel> m_pending;
	std::unordered_map<std::string, std::weak_ptr<ModelAsset>> m_models;
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;

};
