#pragma once

#include <string>
#include <vector>
#include <glm.hpp>
#include <gl.h>
#include "Core/Math.h"
#include "Renderer/DebugDraw.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"

class Camera;
class Scene;
struct Material;
class Mesh;
struct Selection;
struct Settings;

struct Viewport
{
	int x = 0;
	int y = 0;
	int width = 1;
	int height = 1;
};

struct RenderStats
{
	int drawCalls = 0;
	int triangles = 0;
	int visibleMeshes = 0;
	int culledMeshes = 0;
	int shadowCasters = 0;
	int lights = 0;
	float gpuTime = 0.0f;
};

class Renderer
{

public:

	Renderer();
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	bool Initialize();
	void Shutdown();
	bool ReloadShaders();

	void Render(const Scene& scene, const Camera& camera, const Settings& settings,
		const Viewport& viewport, const Selection& selection);

	void RequestScreenshot(const std::string& filename);

	const RenderStats& GetStats() const;

private:

	struct DrawItem
	{
		const Mesh* mesh;
		const Material* material;
		glm::mat4 model;
		glm::mat3 normalMatrix;
		float distance;
		bool isSelected;
	};

	struct BloomMip
	{
		GLuint texture = 0;
		int width = 0;
		int height = 0;
	};

	void SetupShaders();
	void UpdateTargets(const Viewport& viewport, const Settings& settings);
	void DestroyBloom();

	void CollectDrawItems(const Scene& scene, const Camera& camera, const Settings& settings, const Selection& selection);
	bool RenderShadows(const Scene& scene, const Settings& settings, glm::mat4& lightSpace);
	void UpdateUniforms(const Scene& scene, const Camera& camera, const Settings& settings, const glm::mat4& lightSpace, bool hasShadows);

	void RenderSky(const Scene& scene);
	void RenderItems(const std::vector<DrawItem>& items, bool isTransparent);
	void RenderGrid();
	void RenderBloom(const Settings& settings);
	void RenderPost(const Settings& settings, const Viewport& viewport);
	void RenderGizmos(const Scene& scene, const Camera& camera, const Settings& settings,
		const Viewport& viewport, const Selection& selection);
	void SaveScreenshot(const Viewport& viewport);

	void BindMaterial(const Material& material);
	void DrawFullscreen() const;

	Shader m_litShader;
	Shader m_shadowShader;
	Shader m_skyShader;
	Shader m_gridShader;
	Shader m_postShader;
	Shader m_fxaaShader;
	Shader m_bloomDownShader;
	Shader m_bloomUpShader;
	Shader m_debugShader;

	Framebuffer m_sceneTarget;
	Framebuffer m_resolveTarget;
	Framebuffer m_ldrTarget;
	Framebuffer m_shadowTarget;

	std::vector<BloomMip> m_bloomMips;
	GLuint m_bloomFBO;
	float m_shadowTexelSize;

	GLuint m_frameUBO;
	GLuint m_lightUBO;
	GLuint m_emptyVAO;
	GLuint m_timerQueries[2];
	int m_timerIndex;
	bool m_isTimerActive[2];

	Texture m_whiteTexture;
	Texture m_flatNormalTexture;

	DebugDraw m_debugDraw;

	std::vector<DrawItem> m_opaqueItems;
	std::vector<DrawItem> m_transparentItems;
	std::vector<DrawItem> m_shadowItems;

	Frustum m_frustum;
	RenderStats m_stats;
	std::string m_screenshotPath;

};
