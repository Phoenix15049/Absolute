#include <fstream>
#include <iostream>
#include "Material.h"
#include "Utility.h"

Material::Material()
{
	m_isTextured = false;
	m_shininess = 0.0f;
	m_ambient = glm::vec3(0.0f);
	m_diffuse = glm::vec3(0.0f);
	m_specular = glm::vec3(0.0f);
}

const std::string& Material::GetName() const
{
	return m_name;
}

bool Material::IsTextured() const
{
	return m_isTextured;
}

const Texture& Material::GetNormalMap() const
{
	return m_normalMap;
}

const Texture& Material::GetAmbientMap() const
{
	return m_ambientMap;
}

const Texture& Material::GetDiffuseMap() const
{
	return m_diffuseMap;
}

const Texture& Material::GetSpecularMap() const
{
	return m_specularMap;
}

void Material::SetShininess(GLfloat shininess)
{
	m_shininess = shininess;
}

void Material::SetAmbient(const glm::vec3& ambient)
{
	m_ambient.r = ambient.r;
	m_ambient.g = ambient.g;
	m_ambient.b = ambient.b;
}

void Material::SetDiffuse(const glm::vec3& diffuse)
{
	m_diffuse.r = diffuse.r;
	m_diffuse.g = diffuse.g;
	m_diffuse.b = diffuse.b;
}

void Material::SetSpecular(const glm::vec3& specular)
{
	m_specular.r = specular.r;
	m_specular.g = specular.g;
	m_specular.b = specular.b;
}

bool Material::Load(const std::string& filename, std::vector<Material>& materials)
{
	std::fstream file("Materials/" + filename, std::ios_base::in);

	if (!file)
	{
		std::cout << "Error loading material file" << std::endl;
		return false;
	}

	std::string line;
	std::vector<std::string> subStrings;

	while (!file.eof())
	{
		getline(file, line);
		subStrings.clear();

		if (!line.empty() && line[0] != '#')
		{
			Utility::ParseString(line, subStrings, ' ');

			//New material group
			if (subStrings[0] == "newmtl")
			{
				materials.push_back(Material());
				materials.back().m_name = subStrings[1];
				continue;
			}

			//Read ambient data
			if (subStrings[0] == "Ka")
			{
				materials.back().SetAmbient(glm::vec3(std::stof(subStrings[1]),
					std::stof(subStrings[2]),
					std::stof(subStrings[3])));
				continue;
			}

			//Read diffuse data
			if (subStrings[0] == "Kd")
			{
				materials.back().SetDiffuse(glm::vec3(std::stof(subStrings[1]),
					std::stof(subStrings[2]),
					std::stof(subStrings[3])));
				continue;
			}

			//Read specular data
			if (subStrings[0] == "Ks")
			{
				materials.back().SetSpecular(glm::vec3(std::stof(subStrings[1]),
					std::stof(subStrings[2]),
					std::stof(subStrings[3])));
				continue;
			}

			//Read specular exponent or shininess value
			if (subStrings[0] == "Ns")
			{
				materials.back().SetShininess(std::stof(subStrings[1]));
				continue;
			}

			//Read ambient texture file
			if (subStrings[0] == "map_Ka")
			{
				materials.back().m_ambientMap.Load("Textures/" + subStrings[1]);
				materials.back().m_isTextured = true;
				continue;
			}

			//Read diffuse texture file
			if (subStrings[0] == "map_Kd")
			{
				materials.back().m_diffuseMap.Load("Textures/" + subStrings[1]);
				materials.back().m_isTextured = true;
				continue;
			}

			//Read specular texture file
			if (subStrings[0] == "map_Ks")
			{
				materials.back().m_specularMap.Load("Textures/" + subStrings[1]);
				materials.back().m_isTextured = true;
				continue;
			}

			//Read normal texture file
			if (subStrings[0] == "map_Ns" || subStrings[0] == "bump")
			{
				materials.back().m_normalMap.Load("Textures/" + subStrings[1]);
				materials.back().m_isTextured = true;
				continue;
			}
		}
	}

	file.close();
	return true;
}

void Material::SendToShader(const Shader& shader) const
{
	shader.SendUniformData("material.shininess", m_shininess);
	shader.SendUniformData("material.ambient", m_ambient.r, m_ambient.g, m_ambient.b);
	shader.SendUniformData("material.diffuse", m_diffuse.r, m_diffuse.g, m_diffuse.b);
	shader.SendUniformData("material.specular", m_specular.r, m_specular.g, m_specular.b);
}