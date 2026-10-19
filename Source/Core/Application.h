#pragma once

#include <array>
#include <memory>
#include <string>
#include "Core/Settings.h"
#include "Core/Window.h"
#include "Renderer/Primitives.h"
#include "Renderer/Renderer.h"
#include "Scene/AssetManager.h"
#include "Scene/Camera.h"
#include "Scene/Scene.h"

class Editor;

class Application
{

public:

	static const int FrameHistorySize = 120;

	Application();
	~Application();

	bool Initialize();
	void Run();
	void Shutdown();

	void NewScene();
	void CreateDefaultScene();
	void OpenScene();
	void OpenScene(const std::string& path);
	void SaveScene(bool saveAs);
	void ImportModel();
	void ImportModel(const std::string& path);
	void AddPrimitive(PrimitiveType type);
	void AddLight(LightType type);
	void DuplicateSelection();
	void DeleteSelection();
	void FocusSelection();
	void TakeScreenshot();
	void CaptureWindowAndQuit(const std::string& filename);
	void ReloadShaders();
	void ApplySettings();
	void ResetSettings();
	void RequestQuit();
	void Quit();

	Scene& GetScene();
	Settings& GetSettings();
	Camera& GetCamera();
	Selection& GetSelection();
	AssetManager& GetAssets();
	const RenderStats& GetRenderStats() const;

	const std::string& GetScenePath() const;
	float GetFrameTime() const;
	float GetFPS() const;
	const std::array<float, FrameHistorySize>& GetFrameHistory() const;
	int GetFrameHistoryOffset() const;

	void SetViewport(const Viewport& viewport, bool isHovered);

private:

	void HandleShortcuts();
	void HandlePicking();
	void HandleDroppedFiles();
	void UpdateTitle();
	void SaveWindowCapture();
	glm::vec3 GetSpawnPosition() const;

	Window m_window;
	Settings m_settings;
	Renderer m_renderer;
	AssetManager m_assets;
	Scene m_scene;
	Camera m_camera;
	Selection m_selection;
	std::unique_ptr<Editor> m_editor;

	Viewport m_viewport;
	bool m_isViewportHovered;
	bool m_isRunning;
	bool m_isQuitConfirmed;
	std::string m_scenePath;
	std::string m_title;
	std::string m_capturePath;
	int m_captureFrames;

	float m_frameTime;
	float m_fps;
	std::array<float, FrameHistorySize> m_frameHistory;
	int m_frameHistoryOffset;

	int m_mouseDownX;
	int m_mouseDownY;

};
