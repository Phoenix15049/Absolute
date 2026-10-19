#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <glm.hpp>
#include "Core/Math.h"
#include "Renderer/Primitives.h"
#include "Scene/Model.h"

class AssetManager;

struct Transform
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	glm::mat4 GetMatrix() const;
};

struct Entity
{
	uint32_t id = 0;
	std::string name;
	std::string source;

	Transform transform;
	std::shared_ptr<ModelAsset> model;

	bool visible = true;
	bool castShadows = true;

	BoundingBox GetWorldBounds() const;
};

enum class LightType
{
	Directional,
	Point,
	Spot
};

struct Light
{
	uint32_t id = 0;
	std::string name;
	LightType type = LightType::Point;

	glm::vec3 position = glm::vec3(0.0f, 2.0f, 0.0f);
	float pitch = -50.0f;
	float yaw = 30.0f;

	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0f;
	float range = 10.0f;
	float innerAngle = 20.0f;
	float outerAngle = 30.0f;

	bool enabled = true;
	bool castShadows = true;

	glm::vec3 GetDirection() const;
};

struct Environment
{
	glm::vec3 skyTop = glm::vec3(0.22f, 0.40f, 0.72f);
	glm::vec3 skyHorizon = glm::vec3(0.72f, 0.80f, 0.90f);
	glm::vec3 ground = glm::vec3(0.22f, 0.20f, 0.18f);
	float ambientIntensity = 0.6f;
	float skyIntensity = 1.0f;
};

struct Selection
{
	enum class Type
	{
		None,
		Entity,
		Light
	};

	Type type = Type::None;
	uint32_t id = 0;

	bool IsEntity(uint32_t entityID) const
	{
		return type == Type::Entity && id == entityID;
	}

	bool IsLight(uint32_t lightID) const
	{
		return type == Type::Light && id == lightID;
	}

	void Clear()
	{
		type = Type::None;
		id = 0;
	}
};

class Scene
{

public:

	Entity& CreateEntity(const std::string& name);
	Light& CreateLight(const std::string& name, LightType type);

	Entity* FindEntity(uint32_t id);
	Light* FindLight(uint32_t id);
	const Entity* FindEntity(uint32_t id) const;
	const Light* FindLight(uint32_t id) const;

	void RemoveEntity(uint32_t id);
	void RemoveLight(uint32_t id);
	void Clear();

	std::vector<Entity>& GetEntities();
	const std::vector<Entity>& GetEntities() const;
	std::vector<Light>& GetLights();
	const std::vector<Light>& GetLights() const;
	Environment& GetEnvironment();
	const Environment& GetEnvironment() const;

	BoundingBox GetBounds() const;

	bool Save(const std::string& filename) const;
	bool Load(const std::string& filename, AssetManager& assets);

	bool IsDirty() const;
	void SetDirty(bool isDirty);

	void AssignModel(uint32_t entityID, const std::shared_ptr<ModelAsset>& model);

private:

	uint32_t NextID();

	std::vector<Entity> m_entities;
	std::vector<Light> m_lights;
	Environment m_environment;

	bool m_isDirty = false;
	static uint32_t s_nextID;

};
