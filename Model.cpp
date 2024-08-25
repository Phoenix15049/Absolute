#include <fstream>
#include <iostream>
#include <unordered_map>
#include "Model.h"
#include "Utility.h"
#include "Input.h"
#include <gtc/matrix_transform.hpp>

Model::Model()
{
	m_meshes.reserve(10);
	m_buffers.reserve(10);

	m_shininess = 50.0f;
	m_position = glm::vec3(0.0f);
	m_rotation = glm::vec3(0.0f);
	m_ambient = glm::vec3(0.4f, 0.4f, 0.4f);
	m_diffuse = glm::vec3(0.1f, 0.7f, 0.2f);
	m_specular = glm::vec3(0.8f, 0.8f, 0.8f);
}

bool Model::Load(const std::string& filename)
{
	std::fstream file(filename, std::ios_base::in);

	if (!file)
	{
		std::cout << "Error loading model file" << std::endl;
		return false;
	}

	Mesh rawMesh;
	std::string lastMaterialName;

	std::string line;
	std::string lastName;
	std::vector<Face> faces;

	std::vector<std::string> subStrings;
	subStrings.reserve(10);

	while (!file.eof())
	{
		std::getline(file, line);
		subStrings.clear();

		if (!line.empty() && line[0] != '#')
		{
			Utility::ParseString(line, subStrings, ' ');

			//Vertex data
			if (subStrings[0] == "v")
			{
				rawMesh.vertices.push_back(glm::vec3(std::stof(subStrings[1]),
					std::stof(subStrings[2]),
					std::stof(subStrings[3])));
				continue;
			}

			//Normal data
			if (subStrings[0] == "vn")
			{
				rawMesh.normals.push_back(glm::vec3(std::stof(subStrings[1]),
					std::stof(subStrings[2]),
					std::stof(subStrings[3])));
				continue;
			}

			//Texture coordinate data
			if (subStrings[0] == "vt")
			{
				rawMesh.textureCoords.push_back(glm::vec2(std::stof(subStrings[1]),
					std::stof(subStrings[2])));
				continue;
			}

			//Faces
			if (subStrings[0] == "f")
			{
				Face face;
				std::vector<std::string> numbers;
				numbers.reserve(10);

				for (int i = 1; i <= 3; i++)
				{
					numbers.clear();
					VertexGroup vertexGroup;
					Utility::ParseString(subStrings[i], numbers, '/');

					vertexGroup.v = std::stoi(numbers[0]) - 1;
					vertexGroup.t = std::stoi(numbers[1]) - 1;
					vertexGroup.n = std::stoi(numbers[2]) - 1;

					face.push_back(vertexGroup);
				}

				faces.push_back(face);
				continue;
			}

			//Material group
			if (subStrings[0] == "usemtl")
			{
				if (!m_materials.empty())
				{
					for (const auto& material : m_materials)
					{
						if (material.GetName() == subStrings[1])
						{
							lastMaterialName = subStrings[1];
							break;
						}
					}
				}

				continue;
			}

			//Material file
			if (subStrings[0] == "mtllib")
			{
				Material material;
				material.Load(subStrings[1], m_materials);
				continue;
			}

			//Group data
			if (subStrings[0] == "g" || subStrings[0] == "o")
			{
				if (!faces.empty())
				{
					Mesh mesh;
					mesh.name = lastName;
					mesh.materialName = lastMaterialName;
					SortVertexData(mesh, rawMesh, faces);
					m_meshes.push_back(mesh);
				}

				lastName = subStrings[1];
				faces.clear();
				continue;
			}
		}
	}

	file.close();

	if (!faces.empty())
	{
		Mesh mesh;
		mesh.name = lastName;
		mesh.materialName = lastMaterialName;
		SortVertexData(mesh, rawMesh, faces);
		m_meshes.push_back(mesh);
	}

	FillBuffers();
	return true;
}

void Model::Update()
{
	if (Input::Instance()->IsLeftButtonClicked())
	{
		m_rotation.x += Input::Instance()->GetMouseMotionY();
		m_rotation.y += Input::Instance()->GetMouseMotionX();
	}

	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, m_position);
	m_model = glm::rotate(m_model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_model = glm::rotate(m_model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_normal = glm::inverse(glm::mat3(m_model));
}

void Model::Render(const Shader& shader)
{
	shader.SendUniformData("model", m_model);
	shader.SendUniformData("normal", m_normal);

	shader.SendUniformData("isLit", true);
	shader.SendUniformData("isTextured", false);

	shader.SendUniformData("material.shininess", m_shininess);
	shader.SendUniformData("material.ambient", m_ambient.r, m_ambient.g, m_ambient.b);
	shader.SendUniformData("material.diffuse", m_diffuse.r, m_diffuse.g, m_diffuse.b);
	shader.SendUniformData("material.specular", m_specular.r, m_specular.g, m_specular.b);

	int count = 0;
	for (auto& buffer : m_buffers)
	{
		buffer.LinkEBO();

		buffer.LinkVBO(shader,
			"vertexIn", Buffer::VBOType::VertexBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);
		buffer.LinkVBO(shader,
			"colorIn", Buffer::VBOType::ColorBuffer, Buffer::ComponentType::RGBA, Buffer::DataType::FloatData);
		buffer.LinkVBO(shader,
			"textureIn", Buffer::VBOType::TextureBuffer, Buffer::ComponentType::UV, Buffer::DataType::FloatData);
		buffer.LinkVBO(shader,
			"normalIn", Buffer::VBOType::NormalBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);

		for (auto mat : m_materials)
		{
			if (mat.GetName() == m_meshes[count].materialName)
			{
				mat.SendToShader(shader);

				if (mat.IsTextured())
				{
					shader.SendUniformData("isTextured", true);
					mat.GetDiffuseMap().Bind();
				}

				else
				{
					shader.SendUniformData("isTextured", false);
				}

				break;
			}
		}

		count++;
		buffer.Render(Buffer::DrawType::Triangles);
	}
}

void Model::FillBuffers()
{
	for (auto& mesh : m_meshes)
	{
		Buffer buffer;
		buffer.CreateBuffer(mesh.indices.size(), true);

		buffer.FillEBO(&mesh.indices[0], mesh.indices.size() * sizeof(GLuint), Buffer::FillType::Once);
		
		buffer.FillVBO(Buffer::VBOType::VertexBuffer,
			&mesh.vertices[0].x, mesh.vertices.size() * sizeof(glm::vec3), Buffer::FillType::Once);

		buffer.FillVBO(Buffer::VBOType::TextureBuffer,
			&mesh.textureCoords[0].x, mesh.textureCoords.size() * sizeof(glm::vec2), Buffer::FillType::Once);

		buffer.FillVBO(Buffer::VBOType::NormalBuffer,
			&mesh.normals[0].x, mesh.normals.size() * sizeof(glm::vec3), Buffer::FillType::Once);

		for (const auto& vertex : mesh.vertices)
		{
			glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
			mesh.colors.push_back(color);
		}

		buffer.FillVBO(Buffer::VBOType::ColorBuffer,
			&mesh.colors[0].x, mesh.colors.size() * sizeof(glm::vec4), Buffer::FillType::Once);

		m_buffers.push_back(buffer);
	}
}

void Model::SortVertexData(Mesh& newMesh, const Mesh& oldMesh, const std::vector<Face>& faces)
{
	GLuint count = 0;
	std::unordered_map<VertexGroup, GLuint, HashFunction> map;

	for (const auto& face : faces)
	{
		for (auto i = 0; i < 3; i++)
		{
			auto it = map.find(face[i]);

			if (it == map.end())
			{
				newMesh.vertices.push_back(oldMesh.vertices[face[i].v]);
				newMesh.textureCoords.push_back(oldMesh.textureCoords[face[i].t]);
				newMesh.normals.push_back(oldMesh.normals[face[i].n]);
				newMesh.indices.push_back(count);

				map[face[i]] = count;
				count++;
			}

			else
			{
				newMesh.indices.push_back(it->second);
			}
		}
	}
}