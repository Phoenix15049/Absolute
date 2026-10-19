#pragma once

#include <string>
#include <utility>

enum class ToneMapping
{
	None,
	Reinhard,
	ACES
};

enum class RenderMode
{
	Lit,
	Albedo,
	Normals,
	Lighting
};

enum class UITheme
{
	Dark,
	Light,
	Classic
};

struct Settings
{
	int windowWidth = 1600;
	int windowHeight = 900;
	bool maximized = false;
	bool fullscreen = false;
	bool vsync = true;
	int fpsLimit = 0;
	bool pauseWhenUnfocused = true;

	int msaaSamples = 4;
	bool fxaa = false;
	int anisotropy = 8;
	bool frustumCulling = true;
	bool wireframe = false;
	RenderMode renderMode = RenderMode::Lit;

	bool shadows = true;
	int shadowResolution = 2048;
	int shadowSoftness = 1;
	float shadowBias = 1.5f;

	ToneMapping toneMapping = ToneMapping::ACES;
	float exposure = 1.0f;
	float gamma = 2.2f;
	bool bloom = true;
	float bloomThreshold = 1.0f;
	float bloomIntensity = 0.6f;
	float bloomRadius = 1.0f;
	bool vignette = true;

	float fieldOfView = 60.0f;
	float nearClip = 0.05f;
	float farClip = 1000.0f;
	float cameraSpeed = 5.0f;
	float mouseSensitivity = 0.15f;
	bool invertY = false;

	bool showGrid = true;
	bool showLightIcons = true;
	bool showBounds = false;
	bool showStats = true;
	bool showAxisGizmo = true;
	bool showHierarchy = true;
	bool showInspector = true;
	bool showConsole = true;
	float hierarchyWidth = 280.0f;
	float inspectorWidth = 380.0f;
	float consoleHeight = 190.0f;
	float uiScale = 1.0f;
	UITheme theme = UITheme::Dark;

	bool Load(const std::string& filename);
	bool Save(const std::string& filename) const;

	template<typename Visitor>
	void Visit(Visitor&& visit)
	{
		visit("windowWidth", windowWidth);
		visit("windowHeight", windowHeight);
		visit("maximized", maximized);
		visit("fullscreen", fullscreen);
		visit("vsync", vsync);
		visit("fpsLimit", fpsLimit);
		visit("pauseWhenUnfocused", pauseWhenUnfocused);
		visit("msaaSamples", msaaSamples);
		visit("fxaa", fxaa);
		visit("anisotropy", anisotropy);
		visit("frustumCulling", frustumCulling);
		visit("wireframe", wireframe);
		visit("renderMode", renderMode);
		visit("shadows", shadows);
		visit("shadowResolution", shadowResolution);
		visit("shadowSoftness", shadowSoftness);
		visit("shadowBias", shadowBias);
		visit("toneMapping", toneMapping);
		visit("exposure", exposure);
		visit("gamma", gamma);
		visit("bloom", bloom);
		visit("bloomThreshold", bloomThreshold);
		visit("bloomIntensity", bloomIntensity);
		visit("bloomRadius", bloomRadius);
		visit("vignette", vignette);
		visit("fieldOfView", fieldOfView);
		visit("nearClip", nearClip);
		visit("farClip", farClip);
		visit("cameraSpeed", cameraSpeed);
		visit("mouseSensitivity", mouseSensitivity);
		visit("invertY", invertY);
		visit("showGrid", showGrid);
		visit("showLightIcons", showLightIcons);
		visit("showBounds", showBounds);
		visit("showStats", showStats);
		visit("showAxisGizmo", showAxisGizmo);
		visit("showHierarchy", showHierarchy);
		visit("showInspector", showInspector);
		visit("showConsole", showConsole);
		visit("hierarchyWidth", hierarchyWidth);
		visit("inspectorWidth", inspectorWidth);
		visit("consoleHeight", consoleHeight);
		visit("uiScale", uiScale);
		visit("theme", theme);
	}

	template<typename Visitor>
	void Visit(Visitor&& visit) const
	{
		const_cast<Settings*>(this)->Visit(std::forward<Visitor>(visit));
	}
};
