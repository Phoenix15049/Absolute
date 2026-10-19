#pragma once

#include <functional>
#include <string>
#include <vector>
#include <SDL.h>
#include "Core/Log.h"

class Application;
class Window;
struct Entity;
struct Light;
struct Material;
struct ImFont;

class Editor
{

public:

	explicit Editor(Application& application);
	~Editor();

	Editor(const Editor&) = delete;
	Editor& operator=(const Editor&) = delete;

	bool Initialize(Window& window);
	void Shutdown();

	void ProcessEvent(const SDL_Event& event);
	void BeginFrame();
	void Draw();
	void EndFrame();

	void ApplyStyle();
	void ShowConfirm(const std::string& title, const std::string& message, std::function<void()> onConfirm, bool isRequired = true);

	bool WantsKeyboard() const;
	bool WantsTextInput() const;

private:

	struct PanelLayout
	{
		float top = 0.0f;
		float bottom = 0.0f;
		float left = 0.0f;
		float right = 0.0f;
		float consoleTop = 0.0f;
	};

	void RebuildFonts();
	void HandleEditorShortcuts();

	void DrawMenuBar();
	void DrawToolbar();
	void DrawHierarchy();
	void DrawInspector();
	void DrawConsole();
	void DrawStatusBar();
	void DrawViewportOverlay();
	void DrawSettingsWindow();
	void DrawControlsWindow();
	void DrawAboutWindow();
	void DrawConfirmDialog();

	void DrawEntityInspector(Entity& entity);
	void DrawLightInspector(Light& light);
	void DrawEnvironmentInspector();
	bool DrawMaterialEditor(Material& material, int index);
	bool DrawTextureSlot(const char* label, std::string& path, bool isSRGB, Material& material, int slot);

	void UpdateLayout();
	void UpdateViewport();

	Application& m_application;

	bool m_isInitialized;
	bool m_areFontsDirty;
	float m_dpiScale;
	ImFont* m_largeFont;

	PanelLayout m_layout;
	float m_viewportLeft;
	float m_viewportTop;
	float m_viewportWidth;
	float m_viewportHeight;

	bool m_showSettings;
	bool m_showControls;
	bool m_showAbout;
	bool m_isUniformScale;

	char m_hierarchyFilter[64];
	char m_consoleFilter[64];
	bool m_showInfo;
	bool m_showWarnings;
	bool m_showErrors;
	bool m_isAutoScroll;

	unsigned int m_logVersion;
	std::vector<LogEntry> m_logEntries;

	bool m_isConfirmPending;
	std::string m_confirmTitle;
	std::string m_confirmMessage;
	std::function<void()> m_onConfirm;

};
