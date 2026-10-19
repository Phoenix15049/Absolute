#include <algorithm>
#include <fstream>
#include <sstream>
#include <gtc/matrix_transform.hpp>
#include "Core/Log.h"
#include "Scene/AssetManager.h"
#include "Scene/Scene.h"

uint32_t Scene::s_nextID = 1;

namespace
{
	struct MaterialOverride
	{
		size_t index = 0;
		glm::vec3 diffuse = glm::vec3(0.8f);
		glm::vec3 specular = glm::vec3(0.25f);
		glm::vec3 emissive = glm::vec3(0.0f);
		float emissiveStrength = 1.0f;
		float shininess = 32.0f;
		float opacity = 1.0f;
		std::string diffusePath;
	};

	std::ostream& operator<<(std::ostream& stream, const glm::vec3& value)
	{
		return stream << value.x << " " << value.y << " " << value.z;
	}

	std::istream& operator>>(std::istream& stream, glm::vec3& value)
	{
		return stream >> value.x >> value.y >> value.z;
	}

	std::string ReadRest(std::istringstream& stream)
	{
		std::string value;
		std::getline(stream >> std::ws, value);

		while (!value.empty() && (value.back() == '\r' || value.back() == ' '))
		{
			value.pop_back();
		}

		return value;
	}

	void ApplyOverrides(const std::shared_ptr<ModelAsset>& model, const std::vector<MaterialOverride>& overrides, AssetManager& assets)
	{
		if (!model)
		{
			return;
		}

		for (const auto& entry : overrides)
		{
			if (entry.index >= model->materials.size())
			{
				continue;
			}

			Material& material = model->materials[entry.index];
			material.diffuse = entry.diffuse;
			material.specular = entry.specular;
			material.emissive = entry.emissive;
			material.emissiveStrength = entry.emissiveStrength;
			material.shininess = entry.shininess;
			material.opacity = entry.opacity;

			if (!entry.diffusePath.empty() && entry.diffusePath != material.diffusePath)
			{
				material.diffusePath = entry.diffusePath;
				material.diffuseMap = assets.LoadTexture(entry.diffusePath, true);
			}
		}
	}
}

glm::mat4 Transform::GetMatrix() const
{
	glm::mat4 matrix = glm::translate(glm::mat4(1.0f), position);
	matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	return glm::scale(matrix, scale);
}

BoundingBox Entity::GetWorldBounds() const
{
	return model ? model->bounds.Transformed(transform.GetMatrix()) : BoundingBox();
}

glm::vec3 Light::GetDirection() const
{
	return Math::DirectionFromAngles(pitch, yaw);
}

Entity& Scene::CreateEntity(const std::string& name)
{
	m_entities.emplace_back();
	m_entities.back().id = NextID();
	m_entities.back().name = name;
	m_isDirty = true;
	return m_entities.back();
}

Light& Scene::CreateLight(const std::string& name, LightType type)
{
	m_lights.emplace_back();
	m_lights.back().id = NextID();
	m_lights.back().name = name;
	m_lights.back().type = type;
	m_isDirty = true;
	return m_lights.back();
}

Entity* Scene::FindEntity(uint32_t id)
{
	auto it = std::find_if(m_entities.begin(), m_entities.end(), [id](const Entity& entity) { return entity.id == id; });
	return (it != m_entities.end()) ? &*it : nullptr;
}

Light* Scene::FindLight(uint32_t id)
{
	auto it = std::find_if(m_lights.begin(), m_lights.end(), [id](const Light& light) { return light.id == id; });
	return (it != m_lights.end()) ? &*it : nullptr;
}

const Entity* Scene::FindEntity(uint32_t id) const
{
	return const_cast<Scene*>(this)->FindEntity(id);
}

const Light* Scene::FindLight(uint32_t id) const
{
	return const_cast<Scene*>(this)->FindLight(id);
}

void Scene::RemoveEntity(uint32_t id)
{
	m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
		[id](const Entity& entity) { return entity.id == id; }), m_entities.end());
	m_isDirty = true;
}

void Scene::RemoveLight(uint32_t id)
{
	m_lights.erase(std::remove_if(m_lights.begin(), m_lights.end(),
		[id](const Light& light) { return light.id == id; }), m_lights.end());
	m_isDirty = true;
}

void Scene::Clear()
{
	m_entities.clear();
	m_lights.clear();
	m_environment = Environment();
	m_isDirty = false;
}

std::vector<Entity>& Scene::GetEntities()
{
	return m_entities;
}

const std::vector<Entity>& Scene::GetEntities() const
{
	return m_entities;
}

std::vector<Light>& Scene::GetLights()
{
	return m_lights;
}

const std::vector<Light>& Scene::GetLights() const
{
	return m_lights;
}

Environment& Scene::GetEnvironment()
{
	return m_environment;
}

const Environment& Scene::GetEnvironment() const
{
	return m_environment;
}

BoundingBox Scene::GetBounds() const
{
	BoundingBox bounds;

	for (const auto& entity : m_entities)
	{
		if (entity.visible)
		{
			bounds.Expand(entity.GetWorldBounds());
		}
	}

	return bounds;
}

bool Scene::Save(const std::string& filename) const
{
	std::ofstream file(filename);

	if (!file)
	{
		Log::Error("Could not save scene to " + filename);
		return false;
	}

	file << "# Absolute Engine scene" << std::endl << std::endl;

	file << "environment" << std::endl;
	file << "skyTop " << m_environment.skyTop << std::endl;
	file << "skyHorizon " << m_environment.skyHorizon << std::endl;
	file << "ground " << m_environment.ground << std::endl;
	file << "ambient " << m_environment.ambientIntensity << std::endl;
	file << "skyIntensity " << m_environment.skyIntensity << std::endl;
	file << "end" << std::endl << std::endl;

	for (const auto& entity : m_entities)
	{
		file << "entity" << std::endl;
		file << "name " << entity.name << std::endl;
		file << "source " << entity.source << std::endl;
		file << "position " << entity.transform.position << std::endl;
		file << "rotation " << entity.transform.rotation << std::endl;
		file << "scale " << entity.transform.scale << std::endl;
		file << "visible " << entity.visible << std::endl;
		file << "castShadows " << entity.castShadows << std::endl;

		if (entity.model)
		{
			for (size_t i = 0; i < entity.model->materials.size(); i++)
			{
				const Material& material = entity.model->materials[i];

				file << "material " << i
					<< " " << material.diffuse
					<< " " << material.specular
					<< " " << material.emissive
					<< " " << material.emissiveStrength
					<< " " << material.shininess
					<< " " << material.opacity << std::endl;

				if (!material.diffusePath.empty())
				{
					file << "materialTexture " << i << " " << material.diffusePath << std::endl;
				}
			}
		}

		file << "end" << std::endl << std::endl;
	}

	for (const auto& light : m_lights)
	{
		file << "light" << std::endl;
		file << "name " << light.name << std::endl;
		file << "type " << static_cast<int>(light.type) << std::endl;
		file << "position " << light.position << std::endl;
		file << "angles " << light.pitch << " " << light.yaw << std::endl;
		file << "color " << light.color << std::endl;
		file << "intensity " << light.intensity << std::endl;
		file << "range " << light.range << std::endl;
		file << "cone " << light.innerAngle << " " << light.outerAngle << std::endl;
		file << "enabled " << light.enabled << std::endl;
		file << "castShadows " << light.castShadows << std::endl;
		file << "end" << std::endl << std::endl;
	}

	return true;
}

bool Scene::Load(const std::string& filename, AssetManager& assets)
{
	std::ifstream file(filename);

	if (!file)
	{
		Log::Error("Could not open scene " + filename);
		return false;
	}

	Clear();

	enum class Section { None, Environment, Entity, Light };

	Section section = Section::None;
	Entity entity;
	Light light;
	std::vector<MaterialOverride> overrides;

	std::string line;

	while (std::getline(file, line))
	{
		std::istringstream stream(line);
		std::string key;
		stream >> key;

		if (key.empty() || key[0] == '#')
		{
			continue;
		}

		if (section == Section::None)
		{
			if (key == "environment") section = Section::Environment;
			else if (key == "entity") { section = Section::Entity; entity = Entity(); overrides.clear(); }
			else if (key == "light") { section = Section::Light; light = Light(); }
			continue;
		}

		if (key == "end")
		{
			if (section == Section::Entity)
			{
				Entity& created = CreateEntity(entity.name);
				const uint32_t id = created.id;
				entity.id = id;
				created = entity;

				PrimitiveType primitive = PrimitiveType::Cube;

				if (Primitives::FromSource(entity.source, primitive))
				{
					created.model = assets.CreatePrimitive(primitive);
					ApplyOverrides(created.model, overrides, assets);
				}

				else if (!entity.source.empty())
				{
					assets.LoadModelAsync(entity.source, [this, id, overrides, &assets](const std::shared_ptr<ModelAsset>& model)
					{
						ApplyOverrides(model, overrides, assets);
						AssignModel(id, model);
					});
				}
			}

			else if (section == Section::Light)
			{
				Light& created = CreateLight(light.name, light.type);
				light.id = created.id;
				created = light;
			}

			section = Section::None;
			continue;
		}

		if (section == Section::Environment)
		{
			if (key == "skyTop") stream >> m_environment.skyTop;
			else if (key == "skyHorizon") stream >> m_environment.skyHorizon;
			else if (key == "ground") stream >> m_environment.ground;
			else if (key == "ambient") stream >> m_environment.ambientIntensity;
			else if (key == "skyIntensity") stream >> m_environment.skyIntensity;
		}

		else if (section == Section::Entity)
		{
			if (key == "name") entity.name = ReadRest(stream);
			else if (key == "source") entity.source = ReadRest(stream);
			else if (key == "position") stream >> entity.transform.position;
			else if (key == "rotation") stream >> entity.transform.rotation;
			else if (key == "scale") stream >> entity.transform.scale;
			else if (key == "visible") stream >> entity.visible;
			else if (key == "castShadows") stream >> entity.castShadows;
			else if (key == "material")
			{
				MaterialOverride entry;
				stream >> entry.index >> entry.diffuse >> entry.specular >> entry.emissive
					>> entry.emissiveStrength >> entry.shininess >> entry.opacity;

				if (stream)
				{
					overrides.push_back(entry);
				}
			}
			else if (key == "materialTexture")
			{
				size_t index = 0;
				stream >> index;
				const std::string path = ReadRest(stream);

				for (auto& entry : overrides)
				{
					if (entry.index == index)
					{
						entry.diffusePath = path;
					}
				}
			}
		}

		else if (section == Section::Light)
		{
			int type = 0;

			if (key == "name") light.name = ReadRest(stream);
			else if (key == "type" && (stream >> type)) light.type = static_cast<LightType>(std::clamp(type, 0, 2));
			else if (key == "position") stream >> light.position;
			else if (key == "angles") stream >> light.pitch >> light.yaw;
			else if (key == "color") stream >> light.color;
			else if (key == "intensity") stream >> light.intensity;
			else if (key == "range") stream >> light.range;
			else if (key == "cone") stream >> light.innerAngle >> light.outerAngle;
			else if (key == "enabled") stream >> light.enabled;
			else if (key == "castShadows") stream >> light.castShadows;
		}
	}

	m_isDirty = false;
	Log::Info("Scene loaded from " + filename);
	return true;
}

bool Scene::IsDirty() const
{
	return m_isDirty;
}

void Scene::SetDirty(bool isDirty)
{
	m_isDirty = isDirty;
}

void Scene::AssignModel(uint32_t entityID, const std::shared_ptr<ModelAsset>& model)
{
	if (Entity* entity = FindEntity(entityID))
	{
		entity->model = model;
	}
}

uint32_t Scene::NextID()
{
	return s_nextID++;
}
