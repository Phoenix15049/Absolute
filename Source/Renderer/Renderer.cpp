#include <algorithm>
#include <cmath>
#include <cstring>
#include <SDL.h>
#include <SDL_image.h>
#include <gtc/matrix_transform.hpp>
#include "Core/Log.h"
#include "Core/Settings.h"
#include "Renderer/Renderer.h"
#include "Scene/Camera.h"
#include "Scene/Scene.h"

namespace
{
	const int MaxDirectionalLights = 4;
	const int MaxPointLights = 32;
	const int MaxSpotLights = 16;

	const GLuint FrameBinding = 0;
	const GLuint LightBinding = 1;

	struct FrameUniforms
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewProjection;
		glm::mat4 inverseViewProjection;
		glm::mat4 lightSpace;
		glm::vec4 cameraPosition;
		glm::vec4 skyTop;
		glm::vec4 skyHorizon;
		glm::vec4 groundColor;
		glm::vec4 shadowParams;
		glm::ivec4 options;
	};

	struct DirectionalLightData
	{
		glm::vec4 direction;
		glm::vec4 color;
	};

	struct PointLightData
	{
		glm::vec4 positionRange;
		glm::vec4 colorIntensity;
	};

	struct SpotLightData
	{
		glm::vec4 positionRange;
		glm::vec4 directionOuter;
		glm::vec4 colorIntensity;
		glm::vec4 innerCone;
	};

	struct LightUniforms
	{
		glm::ivec4 counts;
		DirectionalLightData directional[MaxDirectionalLights];
		PointLightData point[MaxPointLights];
		SpotLightData spot[MaxSpotLights];
	};

	glm::vec3 ToLinear(const glm::vec3& color)
	{
		return glm::pow(glm::max(color, glm::vec3(0.0f)), glm::vec3(2.2f));
	}

	const Light* FindShadowLight(const Scene& scene)
	{
		for (const auto& light : scene.GetLights())
		{
			if (light.enabled && light.type == LightType::Directional && light.castShadows)
			{
				return &light;
			}
		}

		return nullptr;
	}

	const Light* FindSun(const Scene& scene)
	{
		for (const auto& light : scene.GetLights())
		{
			if (light.enabled && light.type == LightType::Directional)
			{
				return &light;
			}
		}

		return nullptr;
	}
}

Renderer::Renderer()
{
	m_bloomFBO = 0;
	m_shadowTexelSize = 0.0f;
	m_frameUBO = 0;
	m_lightUBO = 0;
	m_emptyVAO = 0;
	m_timerQueries[0] = m_timerQueries[1] = 0;
	m_timerIndex = 0;
	m_isTimerActive[0] = m_isTimerActive[1] = false;
}

Renderer::~Renderer()
{
	Shutdown();
}

bool Renderer::Initialize()
{
	const bool isCreated =
		m_litShader.Create("Shaders/Lit.vert", "Shaders/Lit.frag") &&
		m_shadowShader.Create("Shaders/Shadow.vert", "Shaders/Shadow.frag") &&
		m_skyShader.Create("Shaders/Fullscreen.vert", "Shaders/Sky.frag") &&
		m_gridShader.Create("Shaders/Grid.vert", "Shaders/Grid.frag") &&
		m_postShader.Create("Shaders/Fullscreen.vert", "Shaders/Post.frag") &&
		m_fxaaShader.Create("Shaders/Fullscreen.vert", "Shaders/FXAA.frag") &&
		m_bloomDownShader.Create("Shaders/Fullscreen.vert", "Shaders/BloomDown.frag") &&
		m_bloomUpShader.Create("Shaders/Fullscreen.vert", "Shaders/BloomUp.frag") &&
		m_debugShader.Create("Shaders/Debug.vert", "Shaders/Debug.frag");

	if (!isCreated)
	{
		return false;
	}

	SetupShaders();

	glGenBuffers(1, &m_frameUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_frameUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(FrameUniforms), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, FrameBinding, m_frameUBO);

	glGenBuffers(1, &m_lightUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_lightUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightUniforms), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, LightBinding, m_lightUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenVertexArrays(1, &m_emptyVAO);
	glGenFramebuffers(1, &m_bloomFBO);
	glGenQueries(2, m_timerQueries);

	m_whiteTexture.CreateSolid(255, 255, 255, 255);
	m_flatNormalTexture.CreateSolid(128, 128, 255, 255);

	m_debugDraw.Initialize();

	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	return true;
}

void Renderer::Shutdown()
{
	DestroyBloom();

	if (m_bloomFBO != 0)
	{
		glDeleteFramebuffers(1, &m_bloomFBO);
		m_bloomFBO = 0;
	}

	if (m_frameUBO != 0)
	{
		glDeleteBuffers(1, &m_frameUBO);
		glDeleteBuffers(1, &m_lightUBO);
		m_frameUBO = 0;
		m_lightUBO = 0;
	}

	if (m_emptyVAO != 0)
	{
		glDeleteVertexArrays(1, &m_emptyVAO);
		m_emptyVAO = 0;
	}

	if (m_timerQueries[0] != 0)
	{
		glDeleteQueries(2, m_timerQueries);
		m_timerQueries[0] = m_timerQueries[1] = 0;
	}

	m_sceneTarget.Destroy();
	m_resolveTarget.Destroy();
	m_ldrTarget.Destroy();
	m_shadowTarget.Destroy();

	m_whiteTexture.Destroy();
	m_flatNormalTexture.Destroy();
	m_debugDraw.Destroy();

	m_litShader.Destroy();
	m_shadowShader.Destroy();
	m_skyShader.Destroy();
	m_gridShader.Destroy();
	m_postShader.Destroy();
	m_fxaaShader.Destroy();
	m_bloomDownShader.Destroy();
	m_bloomUpShader.Destroy();
	m_debugShader.Destroy();
}

bool Renderer::ReloadShaders()
{
	bool isReloaded = true;

	for (Shader* shader : { &m_litShader, &m_shadowShader, &m_skyShader, &m_gridShader, &m_postShader,
		&m_fxaaShader, &m_bloomDownShader, &m_bloomUpShader, &m_debugShader })
	{
		isReloaded &= shader->Reload();
	}

	SetupShaders();

	if (isReloaded)
	{
		Log::Info("Shaders reloaded.");
	}

	return isReloaded;
}

void Renderer::RequestScreenshot(const std::string& filename)
{
	m_screenshotPath = filename;
}

const RenderStats& Renderer::GetStats() const
{
	return m_stats;
}

void Renderer::Render(const Scene& scene, const Camera& camera, const Settings& settings,
	const Viewport& viewport, const Selection& selection)
{
	const float gpuTime = m_stats.gpuTime;
	m_stats = RenderStats();
	m_stats.gpuTime = gpuTime;

	if (viewport.width <= 0 || viewport.height <= 0)
	{
		return;
	}

	glBeginQuery(GL_TIME_ELAPSED, m_timerQueries[m_timerIndex]);

	UpdateTargets(viewport, settings);
	CollectDrawItems(scene, camera, settings, selection);

	glm::mat4 lightSpace(1.0f);
	const bool hasShadows = RenderShadows(scene, settings, lightSpace);

	UpdateUniforms(scene, camera, settings, lightSpace, hasShadows);

	m_sceneTarget.Bind();
	glDepthMask(GL_TRUE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderSky(scene);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	if (settings.wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	RenderItems(m_opaqueItems, false);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (settings.showGrid)
	{
		RenderGrid();
	}

	if (settings.wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	RenderItems(m_transparentItems, true);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (m_resolveTarget.IsValid())
	{
		m_sceneTarget.BlitColorTo(m_resolveTarget);
	}

	if (settings.bloom)
	{
		RenderBloom(settings);
	}

	RenderPost(settings, viewport);

	if (!m_screenshotPath.empty())
	{
		SaveScreenshot(viewport);
	}

	RenderGizmos(scene, camera, settings, viewport, selection);

	glEndQuery(GL_TIME_ELAPSED);
	m_isTimerActive[m_timerIndex] = true;
	m_timerIndex = 1 - m_timerIndex;

	if (m_isTimerActive[m_timerIndex])
	{
		GLint isAvailable = 0;
		glGetQueryObjectiv(m_timerQueries[m_timerIndex], GL_QUERY_RESULT_AVAILABLE, &isAvailable);

		if (isAvailable)
		{
			GLuint64 elapsed = 0;
			glGetQueryObjectui64v(m_timerQueries[m_timerIndex], GL_QUERY_RESULT, &elapsed);
			m_stats.gpuTime = static_cast<float>(elapsed / 1000000.0);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void Renderer::SetupShaders()
{
	for (const Shader* shader : { &m_litShader, &m_skyShader, &m_gridShader })
	{
		shader->BindUniformBlock("FrameData", FrameBinding);
	}

	m_litShader.BindUniformBlock("LightData", LightBinding);

	m_litShader.SetSampler("diffuseMap", 0);
	m_litShader.SetSampler("specularMap", 1);
	m_litShader.SetSampler("normalMap", 2);
	m_litShader.SetSampler("shadowMap", 3);

	m_postShader.SetSampler("sceneTexture", 0);
	m_postShader.SetSampler("bloomTexture", 1);

	m_fxaaShader.SetSampler("inputTexture", 0);
	m_bloomDownShader.SetSampler("sourceTexture", 0);
	m_bloomUpShader.SetSampler("sourceTexture", 0);

	glUseProgram(0);
}

void Renderer::UpdateTargets(const Viewport& viewport, const Settings& settings)
{
	static GLint maxSamples = -1;

	if (maxSamples < 0)
	{
		glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	}

	const int samples = std::clamp(settings.msaaSamples, 0, static_cast<int>(maxSamples));

	FramebufferSpec sceneSpec;
	sceneSpec.width = viewport.width;
	sceneSpec.height = viewport.height;
	sceneSpec.colorFormat = GL_RGBA16F;
	sceneSpec.samples = samples > 1 ? samples : 0;
	sceneSpec.depth = DepthAttachment::Renderbuffer;

	if (!(m_sceneTarget.GetSpec() == sceneSpec))
	{
		m_sceneTarget.Create(sceneSpec);
	}

	if (sceneSpec.samples > 0)
	{
		FramebufferSpec resolveSpec = sceneSpec;
		resolveSpec.samples = 0;
		resolveSpec.depth = DepthAttachment::None;

		if (!(m_resolveTarget.GetSpec() == resolveSpec))
		{
			m_resolveTarget.Create(resolveSpec);
		}
	}

	else if (m_resolveTarget.IsValid())
	{
		m_resolveTarget.Destroy();
	}

	if (settings.fxaa)
	{
		FramebufferSpec ldrSpec;
		ldrSpec.width = viewport.width;
		ldrSpec.height = viewport.height;
		ldrSpec.colorFormat = GL_RGBA8;

		if (!(m_ldrTarget.GetSpec() == ldrSpec))
		{
			m_ldrTarget.Create(ldrSpec);
		}
	}

	else if (m_ldrTarget.IsValid())
	{
		m_ldrTarget.Destroy();
	}

	FramebufferSpec shadowSpec;
	shadowSpec.width = settings.shadows ? std::clamp(settings.shadowResolution, 256, 8192) : 1;
	shadowSpec.height = shadowSpec.width;
	shadowSpec.depth = DepthAttachment::ShadowTexture;

	if (!(m_shadowTarget.GetSpec() == shadowSpec))
	{
		m_shadowTarget.Create(shadowSpec);
	}

	if (!settings.bloom)
	{
		DestroyBloom();
		return;
	}

	const int bloomWidth = std::max(viewport.width / 2, 1);
	const int bloomHeight = std::max(viewport.height / 2, 1);

	if (!m_bloomMips.empty() && m_bloomMips[0].width == bloomWidth && m_bloomMips[0].height == bloomHeight)
	{
		return;
	}

	DestroyBloom();

	int width = bloomWidth;
	int height = bloomHeight;

	for (int i = 0; i < 6 && width >= 8 && height >= 8; i++)
	{
		BloomMip mip;
		mip.width = width;
		mip.height = height;

		glGenTextures(1, &mip.texture);
		glBindTexture(GL_TEXTURE_2D, mip.texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		m_bloomMips.push_back(mip);

		width /= 2;
		height /= 2;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::DestroyBloom()
{
	for (auto& mip : m_bloomMips)
	{
		glDeleteTextures(1, &mip.texture);
	}

	m_bloomMips.clear();
}

void Renderer::CollectDrawItems(const Scene& scene, const Camera& camera, const Settings& settings, const Selection& selection)
{
	m_opaqueItems.clear();
	m_transparentItems.clear();

	m_frustum.Extract(camera.GetViewProjection());

	for (const auto& entity : scene.GetEntities())
	{
		if (!entity.visible || !entity.model)
		{
			continue;
		}

		const glm::mat4 model = entity.transform.GetMatrix();
		const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
		const bool isSelected = selection.IsEntity(entity.id);

		for (const auto& subMesh : entity.model->meshes)
		{
			const BoundingBox bounds = subMesh.bounds.Transformed(model);

			if (settings.frustumCulling && !m_frustum.Intersects(bounds))
			{
				m_stats.culledMeshes++;
				continue;
			}

			const int materialIndex = std::clamp(subMesh.materialIndex, 0, static_cast<int>(entity.model->materials.size()) - 1);
			const Material* material = &entity.model->materials[materialIndex];
			const glm::vec3 offset = bounds.GetCenter() - camera.GetPosition();

			DrawItem item{ &subMesh.mesh, material, model, normalMatrix, glm::dot(offset, offset), isSelected };

			(material->IsTransparent() ? m_transparentItems : m_opaqueItems).push_back(item);
			m_stats.visibleMeshes++;
		}
	}

	std::sort(m_opaqueItems.begin(), m_opaqueItems.end(), [](const DrawItem& a, const DrawItem& b)
	{
		return (a.material != b.material) ? a.material < b.material : a.distance < b.distance;
	});

	std::sort(m_transparentItems.begin(), m_transparentItems.end(), [](const DrawItem& a, const DrawItem& b)
	{
		return a.distance > b.distance;
	});
}

bool Renderer::RenderShadows(const Scene& scene, const Settings& settings, glm::mat4& lightSpace)
{
	const Light* light = FindShadowLight(scene);

	if (!settings.shadows || !light)
	{
		return false;
	}

	m_shadowItems.clear();
	BoundingBox casters;

	for (const auto& entity : scene.GetEntities())
	{
		if (!entity.visible || !entity.castShadows || !entity.model)
		{
			continue;
		}

		const glm::mat4 model = entity.transform.GetMatrix();

		for (const auto& subMesh : entity.model->meshes)
		{
			const int materialIndex = std::clamp(subMesh.materialIndex, 0, static_cast<int>(entity.model->materials.size()) - 1);
			const Material* material = &entity.model->materials[materialIndex];

			if (material->opacity < 0.5f)
			{
				continue;
			}

			casters.Expand(subMesh.bounds.Transformed(model));
			m_shadowItems.push_back({ &subMesh.mesh, material, model, glm::mat3(1.0f), 0.0f, false });
		}
	}

	if (!casters.IsValid())
	{
		return false;
	}

	const float size = static_cast<float>(m_shadowTarget.GetSpec().width);
	const glm::vec3 center = casters.GetCenter();
	const float radius = std::max(casters.GetRadius(), 0.5f);
	const glm::vec3 direction = glm::normalize(light->GetDirection());
	const glm::vec3 up = std::abs(direction.y) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

	const glm::mat4 view = glm::lookAt(center - direction * radius * 2.0f, center, up);
	glm::mat4 projection = glm::ortho(-radius, radius, -radius, radius, 0.01f, radius * 4.0f);

	glm::vec4 origin = projection * view * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	origin *= size * 0.5f;
	const glm::vec2 offset = (glm::round(glm::vec2(origin)) - glm::vec2(origin)) * (2.0f / size);
	projection[3][0] += offset.x;
	projection[3][1] += offset.y;

	lightSpace = projection * view;
	m_shadowTexelSize = (radius * 2.0f) / size;

	m_shadowTarget.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.5f, 4.0f);

	m_shadowShader.Use();

	for (const auto& item : m_shadowItems)
	{
		m_shadowShader.SetUniform("lightSpaceModel", lightSpace * item.model);
		item.mesh->Draw();

		m_stats.drawCalls++;
		m_stats.shadowCasters++;
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	return true;
}

void Renderer::UpdateUniforms(const Scene& scene, const Camera& camera, const Settings& settings,
	const glm::mat4& lightSpace, bool hasShadows)
{
	const Environment& environment = scene.GetEnvironment();

	FrameUniforms frame;
	frame.view = camera.GetView();
	frame.projection = camera.GetProjection();
	frame.viewProjection = camera.GetViewProjection();
	frame.inverseViewProjection = glm::inverse(frame.viewProjection);
	frame.lightSpace = lightSpace;
	frame.cameraPosition = glm::vec4(camera.GetPosition(), static_cast<float>(SDL_GetTicks()) / 1000.0f);
	frame.skyTop = glm::vec4(ToLinear(environment.skyTop), environment.skyIntensity);
	frame.skyHorizon = glm::vec4(ToLinear(environment.skyHorizon), environment.ambientIntensity);
	frame.groundColor = glm::vec4(ToLinear(environment.ground), 0.0f);
	frame.shadowParams = glm::vec4(hasShadows ? 1.0f : 0.0f, settings.shadowBias * m_shadowTexelSize,
		static_cast<float>(std::clamp(settings.shadowSoftness, 0, 3)), 0.0f);
	frame.options = glm::ivec4(static_cast<int>(settings.renderMode), 0, 0, 0);

	LightUniforms lights;
	std::memset(&lights, 0, sizeof(lights));

	const Light* shadowLight = hasShadows ? FindShadowLight(scene) : nullptr;
	int directionalCount = 0;
	int pointCount = 0;
	int spotCount = 0;

	if (shadowLight)
	{
		lights.directional[directionalCount++] = { glm::vec4(shadowLight->GetDirection(), 1.0f),
			glm::vec4(ToLinear(shadowLight->color), shadowLight->intensity) };
	}

	for (const auto& light : scene.GetLights())
	{
		if (!light.enabled || &light == shadowLight)
		{
			continue;
		}

		if (light.type == LightType::Directional && directionalCount < MaxDirectionalLights)
		{
			lights.directional[directionalCount++] = { glm::vec4(light.GetDirection(), 0.0f), glm::vec4(ToLinear(light.color), light.intensity) };
		}

		else if (light.type == LightType::Point && pointCount < MaxPointLights)
		{
			lights.point[pointCount++] = { glm::vec4(light.position, light.range), glm::vec4(ToLinear(light.color), light.intensity) };
		}

		else if (light.type == LightType::Spot && spotCount < MaxSpotLights)
		{
			const float outer = std::cos(glm::radians(std::max(light.outerAngle, 0.1f)));
			const float inner = std::cos(glm::radians(std::min(light.innerAngle, light.outerAngle - 0.05f)));

			lights.spot[spotCount++] = { glm::vec4(light.position, light.range), glm::vec4(light.GetDirection(), outer),
				glm::vec4(ToLinear(light.color), light.intensity), glm::vec4(inner, 0.0f, 0.0f, 0.0f) };
		}
	}

	lights.counts = glm::ivec4(directionalCount, pointCount, spotCount, 0);
	m_stats.lights = directionalCount + pointCount + spotCount;

	glBindBuffer(GL_UNIFORM_BUFFER, m_frameUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(frame), &frame);

	glBindBuffer(GL_UNIFORM_BUFFER, m_lightUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lights), &lights);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::RenderSky(const Scene& scene)
{
	const Light* sun = FindSun(scene);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);

	m_skyShader.Use();
	m_skyShader.SetUniform("sunDirection", sun ? sun->GetDirection() : glm::vec3(0.0f, -1.0f, 0.0f));
	m_skyShader.SetUniform("sunColor", sun ? ToLinear(sun->color) * sun->intensity : glm::vec3(0.0f));

	DrawFullscreen();

	glDepthMask(GL_TRUE);
}

void Renderer::RenderItems(const std::vector<DrawItem>& items, bool isTransparent)
{
	if (items.empty())
	{
		return;
	}

	m_litShader.Use();
	m_litShader.SetUniform("isTransparent", isTransparent);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_shadowTarget.GetDepthTexture());

	if (isTransparent)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
	}

	else
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}

	const Material* currentMaterial = nullptr;

	for (const auto& item : items)
	{
		if (item.material != currentMaterial)
		{
			BindMaterial(*item.material);
			currentMaterial = item.material;
		}

		m_litShader.SetUniform("model", item.model);
		m_litShader.SetUniform("normalMatrix", item.normalMatrix);
		m_litShader.SetUniform("highlightColor", item.isSelected ? glm::vec3(1.0f, 0.45f, 0.1f) * 0.35f : glm::vec3(0.0f));

		item.mesh->Draw();

		m_stats.drawCalls++;
		m_stats.triangles += item.mesh->GetIndexCount() / 3;
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void Renderer::RenderGrid()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1.0f, -1.0f);

	m_gridShader.Use();
	m_gridShader.SetUniform("gridExtent", 150.0f);

	glBindVertexArray(m_emptyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	m_stats.drawCalls++;
}

void Renderer::RenderBloom(const Settings& settings)
{
	if (m_bloomMips.empty())
	{
		return;
	}

	GLuint source = m_resolveTarget.IsValid() ? m_resolveTarget.GetColorTexture() : m_sceneTarget.GetColorTexture();

	glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFBO);
	glDisable(GL_BLEND);

	m_bloomDownShader.Use();
	m_bloomDownShader.SetUniform("threshold", settings.bloomThreshold);

	for (size_t i = 0; i < m_bloomMips.size(); i++)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_bloomMips[i].texture, 0);
		glViewport(0, 0, m_bloomMips[i].width, m_bloomMips[i].height);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, source);
		m_bloomDownShader.SetUniform("isFirstPass", i == 0);

		DrawFullscreen();
		source = m_bloomMips[i].texture;
	}

	m_bloomUpShader.Use();
	m_bloomUpShader.SetUniform("radius", settings.bloomRadius);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	for (size_t i = m_bloomMips.size() - 1; i > 0; i--)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_bloomMips[i - 1].texture, 0);
		glViewport(0, 0, m_bloomMips[i - 1].width, m_bloomMips[i - 1].height);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_bloomMips[i].texture);

		DrawFullscreen();
	}

	glDisable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderPost(const Settings& settings, const Viewport& viewport)
{
	const GLuint sceneTexture = m_resolveTarget.IsValid() ? m_resolveTarget.GetColorTexture() : m_sceneTarget.GetColorTexture();
	const bool hasBloom = settings.bloom && !m_bloomMips.empty();

	if (m_ldrTarget.IsValid())
	{
		m_ldrTarget.Bind();
	}

	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
	}

	glDisable(GL_BLEND);

	m_postShader.Use();
	m_postShader.SetUniform("toneMapping", static_cast<GLint>(settings.toneMapping));
	m_postShader.SetUniform("exposure", settings.exposure);
	m_postShader.SetUniform("gamma", std::max(settings.gamma, 0.1f));
	m_postShader.SetUniform("isBloomEnabled", hasBloom);
	m_postShader.SetUniform("bloomIntensity", settings.bloomIntensity);
	m_postShader.SetUniform("isVignetteEnabled", settings.vignette);
	m_postShader.SetUniform("isLumaOutput", m_ldrTarget.IsValid());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, hasBloom ? m_bloomMips[0].texture : m_whiteTexture.GetID());

	DrawFullscreen();

	if (m_ldrTarget.IsValid())
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(viewport.x, viewport.y, viewport.width, viewport.height);

		m_fxaaShader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_ldrTarget.GetColorTexture());

		DrawFullscreen();
	}
}

void Renderer::RenderGizmos(const Scene& scene, const Camera& camera, const Settings& settings,
	const Viewport& viewport, const Selection& selection)
{
	const glm::vec4 selectionColor(1.0f, 0.55f, 0.1f, 1.0f);

	for (const auto& entity : scene.GetEntities())
	{
		if (selection.IsEntity(entity.id))
		{
			m_debugDraw.AddBox(entity.GetWorldBounds(), selectionColor);
		}

		else if (settings.showBounds && entity.visible)
		{
			m_debugDraw.AddBox(entity.GetWorldBounds(), glm::vec4(0.4f, 0.8f, 1.0f, 0.6f));
		}
	}

	for (const auto& light : scene.GetLights())
	{
		const bool isSelected = selection.IsLight(light.id);
		const glm::vec4 color = light.enabled ? glm::vec4(glm::min(light.color, glm::vec3(1.0f)), 1.0f) : glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);

		if (settings.showLightIcons || isSelected)
		{
			m_debugDraw.AddPoint(light.position, isSelected ? selectionColor : color);
		}

		if (!isSelected)
		{
			continue;
		}

		const glm::vec4 shapeColor(1.0f, 0.85f, 0.3f, 0.9f);
		const glm::vec3 direction = light.GetDirection();

		if (light.type == LightType::Point)
		{
			m_debugDraw.AddCircle(light.position, glm::vec3(1.0f, 0.0f, 0.0f), light.range, shapeColor);
			m_debugDraw.AddCircle(light.position, glm::vec3(0.0f, 1.0f, 0.0f), light.range, shapeColor);
			m_debugDraw.AddCircle(light.position, glm::vec3(0.0f, 0.0f, 1.0f), light.range, shapeColor);
		}

		else if (light.type == LightType::Spot)
		{
			m_debugDraw.AddCone(light.position, direction, light.range, light.outerAngle, shapeColor);
			m_debugDraw.AddCone(light.position, direction, light.range, light.innerAngle, glm::vec4(shapeColor.r, shapeColor.g, shapeColor.b, 0.4f));
		}

		else
		{
			m_debugDraw.AddLine(light.position, light.position + direction * 2.0f, shapeColor);
			m_debugDraw.AddCircle(light.position, direction, 0.35f, shapeColor, 24);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_debugDraw.Render(m_debugShader, camera.GetViewProjection(), 14.0f);

	glDisable(GL_BLEND);
}

void Renderer::SaveScreenshot(const Viewport& viewport)
{
	const int width = viewport.width;
	const int height = viewport.height;
	const size_t rowSize = static_cast<size_t>(width) * 4;

	std::vector<unsigned char> pixels(rowSize * height);
	std::vector<unsigned char> flipped(rowSize * height);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glReadBuffer(GL_BACK);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(viewport.x, viewport.y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	for (int row = 0; row < height; row++)
	{
		unsigned char* destination = &flipped[row * rowSize];
		std::memcpy(destination, &pixels[(height - 1 - row) * rowSize], rowSize);

		for (size_t i = 3; i < rowSize; i += 4)
		{
			destination[i] = 255;
		}
	}

	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(flipped.data(), width, height, 32,
		static_cast<int>(rowSize), SDL_PIXELFORMAT_RGBA32);

	if (surface && IMG_SavePNG(surface, m_screenshotPath.c_str()) == 0)
	{
		Log::Info("Screenshot saved to " + m_screenshotPath);
	}

	else
	{
		Log::Error("Could not save screenshot: " + std::string(IMG_GetError()));
	}

	SDL_FreeSurface(surface);
	m_screenshotPath.clear();
}

void Renderer::BindMaterial(const Material& material)
{
	m_litShader.SetUniform("material.diffuse", ToLinear(material.diffuse));
	m_litShader.SetUniform("material.specular", ToLinear(material.specular));
	m_litShader.SetUniform("material.emissive", ToLinear(material.emissive) * material.emissiveStrength);
	m_litShader.SetUniform("material.shininess", material.shininess);
	m_litShader.SetUniform("material.opacity", material.opacity);
	m_litShader.SetUniform("material.normalStrength", material.normalStrength);

	(material.diffuseMap ? *material.diffuseMap : m_whiteTexture).Bind(0);
	(material.specularMap ? *material.specularMap : m_whiteTexture).Bind(1);
	(material.normalMap ? *material.normalMap : m_flatNormalTexture).Bind(2);

	if (material.doubleSided)
	{
		glDisable(GL_CULL_FACE);
	}

	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
}

void Renderer::DrawFullscreen() const
{
	glBindVertexArray(m_emptyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}
