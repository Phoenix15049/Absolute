#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <gl.h>
#include "Core/Application.h"
#include "Core/Input.h"
#include "Core/Platform.h"
#include "Core/Window.h"
#include "Editor/Editor.h"

namespace
{
	const char* TextureFilter = "Images (*.png;*.jpg;*.jpeg;*.tga;*.bmp)\0*.png;*.jpg;*.jpeg;*.tga;*.bmp\0All Files (*.*)\0*.*\0";
	const char* ModelFilter = "Wavefront OBJ (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";

	const ImVec4 AccentColor(0.26f, 0.59f, 0.98f, 1.0f);
	const ImVec4 EntityColor(0.45f, 0.75f, 1.0f, 1.0f);
	const ImVec4 LightColor(1.0f, 0.82f, 0.35f, 1.0f);
	const ImVec4 WarningColor(1.0f, 0.78f, 0.3f, 1.0f);
	const ImVec4 ErrorColor(1.0f, 0.4f, 0.4f, 1.0f);

	float LabelWidth()
	{
		return std::min(ImGui::GetFontSize() * 7.0f, ImGui::GetWindowWidth() * 0.34f);
	}

	void PropertyLabel(const char* label)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);
		ImGui::SameLine(LabelWidth());
		ImGui::SetNextItemWidth(-FLT_MIN);
	}

	bool PropertyCheckbox(const char* label, bool& value)
	{
		PropertyLabel(label);
		ImGui::PushID(label);
		const bool isChanged = ImGui::Checkbox("##value", &value);
		ImGui::PopID();
		return isChanged;
	}

	bool PropertyFloat(const char* label, float& value, float speed, float minimum, float maximum, const char* format = "%.2f")
	{
		PropertyLabel(label);
		ImGui::PushID(label);
		const bool isChanged = ImGui::DragFloat("##value", &value, speed, minimum, maximum, format, ImGuiSliderFlags_AlwaysClamp);
		ImGui::PopID();
		return isChanged;
	}

	bool PropertySlider(const char* label, float& value, float minimum, float maximum, const char* format = "%.2f", ImGuiSliderFlags flags = 0)
	{
		PropertyLabel(label);
		ImGui::PushID(label);
		const bool isChanged = ImGui::SliderFloat("##value", &value, minimum, maximum, format, flags | ImGuiSliderFlags_AlwaysClamp);
		ImGui::PopID();
		return isChanged;
	}

	bool PropertyColor(const char* label, glm::vec3& value)
	{
		PropertyLabel(label);
		ImGui::PushID(label);
		const bool isChanged = ImGui::ColorEdit3("##value", &value.x, ImGuiColorEditFlags_Float);
		ImGui::PopID();
		return isChanged;
	}

	bool PropertyCombo(const char* label, int& value, const char* const items[], int count)
	{
		PropertyLabel(label);
		ImGui::PushID(label);
		const bool isChanged = ImGui::Combo("##value", &value, items, count);
		ImGui::PopID();
		return isChanged;
	}

	bool PropertyVector(const char* label, glm::vec3& value, float resetValue, float speed = 0.05f)
	{
		static const ImVec4 axisColors[] =
		{
			ImVec4(0.75f, 0.2f, 0.2f, 1.0f),
			ImVec4(0.25f, 0.6f, 0.25f, 1.0f),
			ImVec4(0.2f, 0.4f, 0.8f, 1.0f)
		};

		static const char* axisNames[] = { "X", "Y", "Z" };

		bool isChanged = false;

		ImGui::PushID(label);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);
		ImGui::SameLine(LabelWidth());

		const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
		const float buttonWidth = ImGui::GetFontSize() * 0.95f;
		const float fieldWidth = (ImGui::GetContentRegionAvail().x - (buttonWidth + spacing) * 3.0f) / 3.0f;

		for (int i = 0; i < 3; i++)
		{
			ImGui::PushID(i);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ImGui::GetStyle().FramePadding.y));
			ImGui::PushStyleColor(ImGuiCol_Button, axisColors[i]);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(axisColors[i].x + 0.1f, axisColors[i].y + 0.1f, axisColors[i].z + 0.1f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, axisColors[i]);

			if (ImGui::Button(axisNames[i], ImVec2(buttonWidth, 0.0f)))
			{
				value[i] = resetValue;
				isChanged = true;
			}

			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();
			ImGui::SetItemTooltip("Reset %s", axisNames[i]);

			ImGui::SameLine(0.0f, 0.0f);
			ImGui::SetNextItemWidth(fieldWidth);
			isChanged |= ImGui::DragFloat("##value", &value[i], speed, 0.0f, 0.0f, "%.2f");

			if (i < 2)
			{
				ImGui::SameLine(0.0f, spacing);
			}

			ImGui::PopID();
		}

		ImGui::PopID();
		return isChanged;
	}

	bool ToggleButton(const char* label, bool& value, const char* tooltip)
	{
		if (value)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		}

		const bool isClicked = ImGui::Button(label);

		if (value)
		{
			ImGui::PopStyleColor();
		}

		ImGui::SetItemTooltip("%s", tooltip);

		if (isClicked)
		{
			value = !value;
		}

		return isClicked;
	}

	void ToolbarSeparator()
	{
		ImGui::SameLine();
		ImGui::TextDisabled("|");
		ImGui::SameLine();
	}

	void DrawDot(const ImVec4& color)
	{
		const float size = ImGui::GetFontSize();
		const ImVec2 position = ImGui::GetCursorScreenPos();

		ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(position.x + size * 0.5f, position.y + ImGui::GetFrameHeight() * 0.5f),
			size * 0.22f, ImGui::GetColorU32(color));

		ImGui::Dummy(ImVec2(size, ImGui::GetFrameHeight()));
		ImGui::SameLine(0.0f, 2.0f);
	}

	void InfoRow(const char* label, const char* format, ...)
	{
		ImGui::TextDisabled("%s", label);
		ImGui::SameLine(LabelWidth());

		va_list args;
		va_start(args, format);
		ImGui::TextV(format, args);
		va_end(args);
	}

	char Spinner()
	{
		return "|/-\\"[static_cast<int>(ImGui::GetTime() * 8.0) % 4];
	}

	void SetDarkTheme(ImGuiStyle& style)
	{
		ImGui::StyleColorsDark(&style);

		ImVec4* colors = style.Colors;
		colors[ImGuiCol_Text] = ImVec4(0.92f, 0.93f, 0.95f, 1.0f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.52f, 0.56f, 1.0f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.115f, 0.13f, 1.0f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.095f, 0.11f, 1.0f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.105f, 0.12f, 0.98f);
		colors[ImGuiCol_Border] = ImVec4(0.20f, 0.21f, 0.24f, 1.0f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.17f, 0.18f, 0.20f, 1.0f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.23f, 0.26f, 1.0f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.27f, 0.31f, 1.0f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.085f, 0.10f, 1.0f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.11f, 0.13f, 1.0f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.08f, 0.085f, 0.10f, 1.0f);
		colors[ImGuiCol_Header] = ImVec4(0.20f, 0.22f, 0.26f, 1.0f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.30f, 0.40f, 1.0f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.40f, 0.70f, 1.0f);
		colors[ImGuiCol_Button] = ImVec4(0.19f, 0.20f, 0.23f, 1.0f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.32f, 0.44f, 1.0f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.42f, 0.75f, 1.0f);
		colors[ImGuiCol_CheckMark] = AccentColor;
		colors[ImGuiCol_SliderGrab] = AccentColor;
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.68f, 1.0f, 1.0f);
		colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.21f, 0.24f, 1.0f);
		colors[ImGuiCol_Tab] = ImVec4(0.14f, 0.15f, 0.17f, 1.0f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.32f, 0.44f, 1.0f);
		colors[ImGuiCol_TabSelected] = ImVec4(0.20f, 0.30f, 0.46f, 1.0f);
		colors[ImGuiCol_PlotLines] = AccentColor;
		colors[ImGuiCol_PlotHistogram] = AccentColor;
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.55f);
	}
}

Editor::Editor(Application& application) : m_application(application)
{
	m_isInitialized = false;
	m_areFontsDirty = true;
	m_dpiScale = 1.0f;
	m_largeFont = nullptr;

	m_viewportLeft = 0.0f;
	m_viewportTop = 0.0f;
	m_viewportWidth = 1.0f;
	m_viewportHeight = 1.0f;

	m_showSettings = false;
	m_showControls = false;
	m_showAbout = false;
	m_isUniformScale = true;

	m_hierarchyFilter[0] = '\0';
	m_consoleFilter[0] = '\0';
	m_showInfo = true;
	m_showWarnings = true;
	m_showErrors = true;
	m_isAutoScroll = true;
	m_logVersion = 0;

	m_isConfirmPending = false;
}

Editor::~Editor()
{
	Shutdown();
}

bool Editor::Initialize(Window& window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	float dpi = 96.0f;

	if (SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(window.GetHandle()), &dpi, nullptr, nullptr) == 0 && dpi > 0.0f)
	{
		m_dpiScale = std::clamp(dpi / 96.0f, 1.0f, 3.0f);
	}

	if (!ImGui_ImplSDL2_InitForOpenGL(window.GetHandle(), window.GetContext()) || !ImGui_ImplOpenGL3_Init("#version 330 core"))
	{
		return false;
	}

	m_isInitialized = true;
	ApplyStyle();
	return true;
}

void Editor::Shutdown()
{
	if (!m_isInitialized)
	{
		return;
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	m_isInitialized = false;
}

void Editor::ProcessEvent(const SDL_Event& event)
{
	ImGui_ImplSDL2_ProcessEvent(&event);
}

void Editor::BeginFrame()
{
	if (m_areFontsDirty)
	{
		RebuildFonts();
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void Editor::Draw()
{
	Settings& settings = m_application.GetSettings();

	HandleEditorShortcuts();

	DrawMenuBar();
	UpdateLayout();
	DrawToolbar();

	if (settings.showHierarchy)
	{
		DrawHierarchy();
	}

	if (settings.showInspector)
	{
		DrawInspector();
	}

	if (settings.showConsole)
	{
		DrawConsole();
	}

	DrawStatusBar();
	UpdateViewport();
	DrawViewportOverlay();

	if (m_showSettings)
	{
		DrawSettingsWindow();
	}

	if (m_showControls)
	{
		DrawControlsWindow();
	}

	if (m_showAbout)
	{
		DrawAboutWindow();
	}

	DrawConfirmDialog();
}

void Editor::EndFrame()
{
	ImGui::Render();

	const ImGuiIO& io = ImGui::GetIO();
	glViewport(0, 0, static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x),
		static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y));

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::ApplyStyle()
{
	if (!m_isInitialized)
	{
		return;
	}

	const Settings& settings = m_application.GetSettings();

	ImGuiStyle style;

	switch (settings.theme)
	{
		case UITheme::Light:
			ImGui::StyleColorsLight(&style);
			break;

		case UITheme::Classic:
			ImGui::StyleColorsClassic(&style);
			break;

		default:
			SetDarkTheme(style);
			break;
	}

	style.WindowRounding = 4.0f;
	style.ChildRounding = 4.0f;
	style.FrameRounding = 3.0f;
	style.PopupRounding = 4.0f;
	style.GrabRounding = 3.0f;
	style.TabRounding = 3.0f;
	style.ScrollbarRounding = 6.0f;
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.FramePadding = ImVec2(6.0f, 4.0f);
	style.ItemSpacing = ImVec2(8.0f, 5.0f);
	style.ItemInnerSpacing = ImVec2(5.0f, 4.0f);
	style.IndentSpacing = 14.0f;
	style.ScrollbarSize = 12.0f;
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.SeparatorTextBorderSize = 1.0f;

	style.ScaleAllSizes(m_dpiScale * std::clamp(settings.uiScale, 0.75f, 2.0f));
	ImGui::GetStyle() = style;

	m_areFontsDirty = true;
}

void Editor::ShowConfirm(const std::string& title, const std::string& message, std::function<void()> onConfirm, bool isRequired)
{
	if (!isRequired)
	{
		onConfirm();
		return;
	}

	m_confirmTitle = title;
	m_confirmMessage = message;
	m_onConfirm = std::move(onConfirm);
	m_isConfirmPending = true;
}

bool Editor::WantsKeyboard() const
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

bool Editor::WantsTextInput() const
{
	return ImGui::GetIO().WantTextInput;
}

void Editor::RebuildFonts()
{
	const float scale = m_dpiScale * std::clamp(m_application.GetSettings().uiScale, 0.75f, 2.0f);

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();

	const char* regularPath = "C:/Windows/Fonts/segoeui.ttf";
	const char* boldPath = "C:/Windows/Fonts/seguisb.ttf";

	ImFontConfig config;
	config.OversampleH = 2;
	config.OversampleV = 2;

	if (Platform::FileExists(regularPath))
	{
		io.Fonts->AddFontFromFileTTF(regularPath, std::round(16.0f * scale), &config);
		m_largeFont = io.Fonts->AddFontFromFileTTF(Platform::FileExists(boldPath) ? boldPath : regularPath, std::round(26.0f * scale), &config);
	}

	else
	{
		ImFontConfig defaultConfig;
		defaultConfig.SizePixels = std::round(13.0f * scale);
		io.Fonts->AddFontDefault(&defaultConfig);

		defaultConfig.SizePixels = std::round(20.0f * scale);
		m_largeFont = io.Fonts->AddFontDefault(&defaultConfig);
	}

	io.Fonts->Build();

	ImGui_ImplOpenGL3_DestroyFontsTexture();
	ImGui_ImplOpenGL3_CreateFontsTexture();

	m_areFontsDirty = false;
}

void Editor::HandleEditorShortcuts()
{
	const Input* input = Input::Instance();

	if (WantsTextInput())
	{
		return;
	}

	if (input->IsCtrlHeld() && input->IsKeyPressed(SDL_SCANCODE_COMMA))
	{
		m_showSettings = !m_showSettings;
	}

	if (input->IsKeyPressed(SDL_SCANCODE_F1))
	{
		m_showControls = !m_showControls;
	}
}

void Editor::DrawMenuBar()
{
	Settings& settings = m_application.GetSettings();
	Selection& selection = m_application.GetSelection();
	const bool hasSelection = selection.type != Selection::Type::None;

	if (!ImGui::BeginMainMenuBar())
	{
		return;
	}

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New Scene", "Ctrl+N"))
		{
			ShowConfirm("New Scene", "Discard the current scene and start a new one?",
				[this]() { m_application.NewScene(); }, m_application.GetScene().IsDirty());
		}

		if (ImGui::MenuItem("Sample Scene"))
		{
			ShowConfirm("Sample Scene", "Discard the current scene and load the sample scene?",
				[this]() { m_application.CreateDefaultScene(); }, m_application.GetScene().IsDirty());
		}

		if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
		{
			m_application.OpenScene();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
		{
			m_application.SaveScene(false);
		}

		if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
		{
			m_application.SaveScene(true);
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Import Model...", "Ctrl+I"))
		{
			m_application.ImportModel();
		}

		if (ImGui::MenuItem("Take Screenshot", "F12"))
		{
			m_application.TakeScreenshot();
		}

		if (ImGui::MenuItem("Open Screenshots Folder"))
		{
			Platform::CreateDirectories("Screenshots");
			Platform::OpenInExplorer("Screenshots");
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Settings...", "Ctrl+,"))
		{
			m_showSettings = true;
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Exit", "Alt+F4"))
		{
			m_application.RequestQuit();
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, hasSelection))
		{
			m_application.DuplicateSelection();
		}

		if (ImGui::MenuItem("Delete", "Del", false, hasSelection))
		{
			m_application.DeleteSelection();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Focus Selection", "F"))
		{
			m_application.FocusSelection();
		}

		if (ImGui::MenuItem("Deselect", "Esc", false, hasSelection))
		{
			selection.Clear();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Reset Camera"))
		{
			m_application.GetCamera().Reset();
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Add"))
	{
		if (ImGui::MenuItem("Model...", "Ctrl+I"))
		{
			m_application.ImportModel();
		}

		ImGui::SeparatorText("Primitives");

		for (PrimitiveType type : { PrimitiveType::Cube, PrimitiveType::Sphere, PrimitiveType::Plane, PrimitiveType::Cylinder })
		{
			if (ImGui::MenuItem(Primitives::GetName(type)))
			{
				m_application.AddPrimitive(type);
			}
		}

		ImGui::SeparatorText("Lights");

		if (ImGui::MenuItem("Directional Light"))
		{
			m_application.AddLight(LightType::Directional);
		}

		if (ImGui::MenuItem("Point Light"))
		{
			m_application.AddLight(LightType::Point);
		}

		if (ImGui::MenuItem("Spot Light"))
		{
			m_application.AddLight(LightType::Spot);
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View"))
	{
		ImGui::MenuItem("Hierarchy", nullptr, &settings.showHierarchy);
		ImGui::MenuItem("Inspector", nullptr, &settings.showInspector);
		ImGui::MenuItem("Console", nullptr, &settings.showConsole);
		ImGui::MenuItem("Statistics", nullptr, &settings.showStats);
		ImGui::MenuItem("Axis Gizmo", nullptr, &settings.showAxisGizmo);

		ImGui::Separator();

		ImGui::MenuItem("Grid", "", &settings.showGrid);
		ImGui::MenuItem("Light Icons", nullptr, &settings.showLightIcons);
		ImGui::MenuItem("Bounding Boxes", nullptr, &settings.showBounds);
		ImGui::MenuItem("Wireframe", nullptr, &settings.wireframe);

		if (ImGui::BeginMenu("Render Mode"))
		{
			static const char* modes[] = { "Lit", "Albedo", "Normals", "Lighting Only" };

			for (int i = 0; i < 4; i++)
			{
				if (ImGui::MenuItem(modes[i], nullptr, static_cast<int>(settings.renderMode) == i))
				{
					settings.renderMode = static_cast<RenderMode>(i);
				}
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Fullscreen", "F11", &settings.fullscreen))
		{
			m_application.ApplySettings();
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Tools"))
	{
		if (ImGui::MenuItem("Reload Shaders", "F5"))
		{
			m_application.ReloadShaders();
		}

		if (ImGui::MenuItem("Clear Console"))
		{
			Log::Clear();
		}

		if (ImGui::MenuItem("Release Unused Assets"))
		{
			m_application.GetAssets().CollectGarbage();
			Log::Info("Unused assets released.");
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Help"))
	{
		if (ImGui::MenuItem("Controls", "F1"))
		{
			m_showControls = true;
		}

		if (ImGui::MenuItem("About Absolute Engine"))
		{
			m_showAbout = true;
		}

		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void Editor::UpdateLayout()
{
	Settings& settings = m_application.GetSettings();
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float frameHeight = ImGui::GetFrameHeight();

	const float width = viewport->Size.x;
	const float height = viewport->Size.y;

	settings.hierarchyWidth = std::clamp(settings.hierarchyWidth, 160.0f, width * 0.35f);
	settings.inspectorWidth = std::clamp(settings.inspectorWidth, 220.0f, width * 0.4f);

	m_layout.top = viewport->Pos.y + frameHeight + frameHeight + ImGui::GetStyle().WindowPadding.y * 2.0f;
	m_layout.bottom = viewport->Pos.y + height - frameHeight;
	m_layout.left = viewport->Pos.x + (settings.showHierarchy ? settings.hierarchyWidth : 0.0f);
	m_layout.right = viewport->Pos.x + width - (settings.showInspector ? settings.inspectorWidth : 0.0f);

	const float available = m_layout.bottom - m_layout.top;
	settings.consoleHeight = std::clamp(settings.consoleHeight, 80.0f, std::max(available * 0.6f, 80.0f));
	m_layout.consoleTop = settings.showConsole ? m_layout.bottom - settings.consoleHeight : m_layout.bottom;
}

void Editor::UpdateViewport()
{
	const ImGuiIO& io = ImGui::GetIO();

	m_viewportLeft = m_layout.left;
	m_viewportTop = m_layout.top;
	m_viewportWidth = std::max(m_layout.right - m_layout.left, 1.0f);
	m_viewportHeight = std::max(m_layout.consoleTop - m_layout.top, 1.0f);

	const float scaleX = io.DisplayFramebufferScale.x;
	const float scaleY = io.DisplayFramebufferScale.y;

	Viewport viewport;
	viewport.x = static_cast<int>(m_viewportLeft * scaleX);
	viewport.y = static_cast<int>((io.DisplaySize.y - (m_viewportTop + m_viewportHeight)) * scaleY);
	viewport.width = std::max(static_cast<int>(m_viewportWidth * scaleX), 1);
	viewport.height = std::max(static_cast<int>(m_viewportHeight * scaleY), 1);

	const ImVec2 mouse = io.MousePos;
	const bool isInside = mouse.x >= m_viewportLeft && mouse.x < m_viewportLeft + m_viewportWidth &&
		mouse.y >= m_viewportTop && mouse.y < m_viewportTop + m_viewportHeight;

	m_application.SetViewport(viewport, isInside && !io.WantCaptureMouse);
}

void Editor::DrawToolbar()
{
	Settings& settings = m_application.GetSettings();
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float frameHeight = ImGui::GetFrameHeight();

	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + frameHeight));
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, m_layout.top - viewport->Pos.y - frameHeight));

	const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::Begin("##Toolbar", nullptr, flags);
	ImGui::PopStyleVar();

	if (ImGui::Button("Import"))
	{
		m_application.ImportModel();
	}

	ImGui::SetItemTooltip("Import an OBJ model (Ctrl+I)");
	ToolbarSeparator();

	for (PrimitiveType type : { PrimitiveType::Cube, PrimitiveType::Sphere, PrimitiveType::Plane, PrimitiveType::Cylinder })
	{
		if (ImGui::Button(Primitives::GetName(type)))
		{
			m_application.AddPrimitive(type);
		}

		ImGui::SetItemTooltip("Add a %s", Primitives::GetName(type));
		ImGui::SameLine();
	}

	ImGui::TextDisabled("|");
	ImGui::SameLine();

	if (ImGui::Button("Point Light"))
	{
		m_application.AddLight(LightType::Point);
	}

	ImGui::SameLine();

	if (ImGui::Button("Spot Light"))
	{
		m_application.AddLight(LightType::Spot);
	}

	ImGui::SameLine();

	if (ImGui::Button("Sun"))
	{
		m_application.AddLight(LightType::Directional);
	}

	ImGui::SetItemTooltip("Add a directional light");
	ToolbarSeparator();

	static const char* modes[] = { "Lit", "Albedo", "Normals", "Lighting Only" };
	int mode = static_cast<int>(settings.renderMode);
	ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.5f);

	if (ImGui::Combo("##RenderMode", &mode, modes, 4))
	{
		settings.renderMode = static_cast<RenderMode>(mode);
	}

	ImGui::SetItemTooltip("Render mode");
	ImGui::SameLine();

	ToggleButton("Grid", settings.showGrid, "Show grid");
	ImGui::SameLine();
	ToggleButton("Wireframe", settings.wireframe, "Wireframe rendering");
	ImGui::SameLine();
	ToggleButton("Shadows", settings.shadows, "Directional light shadows");
	ImGui::SameLine();
	ToggleButton("Bloom", settings.bloom, "HDR bloom");
	ToolbarSeparator();

	if (ImGui::Button("Focus"))
	{
		m_application.FocusSelection();
	}

	ImGui::SetItemTooltip("Focus the selection (F)");
	ImGui::SameLine();

	if (ImGui::Button("Screenshot"))
	{
		m_application.TakeScreenshot();
	}

	ImGui::SetItemTooltip("Save the viewport as PNG (F12)");
	ImGui::SameLine();

	if (ImGui::Button("Settings"))
	{
		m_showSettings = !m_showSettings;
	}

	char fps[32];
	std::snprintf(fps, sizeof(fps), "%.0f FPS", m_application.GetFPS());
	const float textWidth = ImGui::CalcTextSize(fps).x;

	ImGui::SameLine(std::max(ImGui::GetWindowWidth() - textWidth - ImGui::GetStyle().WindowPadding.x * 2.0f, ImGui::GetCursorPosX()));
	ImGui::AlignTextToFramePadding();
	ImGui::TextDisabled("%s", fps);

	ImGui::End();
}

void Editor::DrawHierarchy()
{
	Settings& settings = m_application.GetSettings();
	Scene& scene = m_application.GetScene();
	Selection& selection = m_application.GetSelection();

	const float height = m_layout.bottom - m_layout.top;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, m_layout.top));
	ImGui::SetNextWindowSize(ImVec2(settings.hierarchyWidth, height));
	ImGui::SetNextWindowSizeConstraints(ImVec2(160.0f, height), ImVec2(viewport->Size.x * 0.35f, height));

	if (!ImGui::Begin("Hierarchy", &settings.showHierarchy, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	settings.hierarchyWidth = ImGui::GetWindowWidth();

	if (ImGui::Button("+ Add"))
	{
		ImGui::OpenPopup("AddPopup");
	}

	if (ImGui::BeginPopup("AddPopup"))
	{
		if (ImGui::MenuItem("Model..."))
		{
			m_application.ImportModel();
		}

		ImGui::SeparatorText("Primitives");

		for (PrimitiveType type : { PrimitiveType::Cube, PrimitiveType::Sphere, PrimitiveType::Plane, PrimitiveType::Cylinder })
		{
			if (ImGui::MenuItem(Primitives::GetName(type)))
			{
				m_application.AddPrimitive(type);
			}
		}

		ImGui::SeparatorText("Lights");

		if (ImGui::MenuItem("Directional Light")) m_application.AddLight(LightType::Directional);
		if (ImGui::MenuItem("Point Light")) m_application.AddLight(LightType::Point);
		if (ImGui::MenuItem("Spot Light")) m_application.AddLight(LightType::Spot);

		ImGui::EndPopup();
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputTextWithHint("##Filter", "Search...", m_hierarchyFilter, sizeof(m_hierarchyFilter));

	ImGui::Separator();

	const std::string filter = m_hierarchyFilter;

	auto matches = [&filter](const std::string& name)
	{
		if (filter.empty())
		{
			return true;
		}

		auto it = std::search(name.begin(), name.end(), filter.begin(), filter.end(),
			[](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });

		return it != name.end();
	};

	ImGui::BeginChild("##HierarchyList", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None);

	const ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
		ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

	if (ImGui::Selectable("Environment", selection.type == Selection::Type::None))
	{
		selection.Clear();
	}

	uint32_t pendingDelete = 0;
	bool isDeleteLight = false;

	if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto& entity : scene.GetEntities())
		{
			if (!matches(entity.name))
			{
				continue;
			}

			ImGui::PushID(static_cast<int>(entity.id));

			DrawDot(entity.model ? EntityColor : WarningColor);

			if (!entity.visible)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			}

			const ImGuiTreeNodeFlags flags = baseFlags | (selection.IsEntity(entity.id) ? ImGuiTreeNodeFlags_Selected : 0);
			ImGui::TreeNodeEx("##Entity", flags, "%s%s", entity.name.c_str(), entity.model ? "" : "  (loading)");

			if (!entity.visible)
			{
				ImGui::PopStyleColor();
			}

			if (ImGui::IsItemClicked())
			{
				selection.type = Selection::Type::Entity;
				selection.id = entity.id;
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				m_application.FocusSelection();
			}

			if (ImGui::BeginPopupContextItem())
			{
				selection.type = Selection::Type::Entity;
				selection.id = entity.id;

				if (ImGui::MenuItem("Focus", "F")) m_application.FocusSelection();
				if (ImGui::MenuItem("Duplicate", "Ctrl+D")) m_application.DuplicateSelection();
				if (ImGui::MenuItem(entity.visible ? "Hide" : "Show")) { entity.visible = !entity.visible; scene.SetDirty(true); }
				ImGui::Separator();
				if (ImGui::MenuItem("Delete", "Del")) pendingDelete = entity.id;

				ImGui::EndPopup();
			}

			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto& light : scene.GetLights())
		{
			if (!matches(light.name))
			{
				continue;
			}

			ImGui::PushID(static_cast<int>(light.id));

			DrawDot(ImVec4(light.color.r, light.color.g, light.color.b, 1.0f));

			if (!light.enabled)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			}

			const ImGuiTreeNodeFlags flags = baseFlags | (selection.IsLight(light.id) ? ImGuiTreeNodeFlags_Selected : 0);
			ImGui::TreeNodeEx("##Light", flags, "%s", light.name.c_str());

			if (!light.enabled)
			{
				ImGui::PopStyleColor();
			}

			if (ImGui::IsItemClicked())
			{
				selection.type = Selection::Type::Light;
				selection.id = light.id;
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				m_application.FocusSelection();
			}

			if (ImGui::BeginPopupContextItem())
			{
				selection.type = Selection::Type::Light;
				selection.id = light.id;

				if (ImGui::MenuItem("Focus", "F")) m_application.FocusSelection();
				if (ImGui::MenuItem("Duplicate", "Ctrl+D")) m_application.DuplicateSelection();
				if (ImGui::MenuItem(light.enabled ? "Disable" : "Enable")) { light.enabled = !light.enabled; scene.SetDirty(true); }
				ImGui::Separator();
				if (ImGui::MenuItem("Delete", "Del")) { pendingDelete = light.id; isDeleteLight = true; }

				ImGui::EndPopup();
			}

			ImGui::PopID();
		}
	}

	if (scene.GetEntities().empty() && scene.GetLights().empty())
	{
		ImGui::TextDisabled("The scene is empty.");
	}

	ImGui::EndChild();

	if (pendingDelete != 0)
	{
		selection.type = isDeleteLight ? Selection::Type::Light : Selection::Type::Entity;
		selection.id = pendingDelete;
		m_application.DeleteSelection();
	}

	ImGui::End();
}

void Editor::DrawInspector()
{
	Settings& settings = m_application.GetSettings();
	Scene& scene = m_application.GetScene();
	Selection& selection = m_application.GetSelection();

	const float height = m_layout.bottom - m_layout.top;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - settings.inspectorWidth, m_layout.top));
	ImGui::SetNextWindowSize(ImVec2(settings.inspectorWidth, height));
	ImGui::SetNextWindowSizeConstraints(ImVec2(220.0f, height), ImVec2(viewport->Size.x * 0.4f, height));

	if (!ImGui::Begin("Inspector", &settings.showInspector, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	settings.inspectorWidth = ImGui::GetWindowWidth();

	if (selection.type == Selection::Type::Entity)
	{
		if (Entity* entity = scene.FindEntity(selection.id))
		{
			DrawEntityInspector(*entity);
		}

		else
		{
			selection.Clear();
		}
	}

	else if (selection.type == Selection::Type::Light)
	{
		if (Light* light = scene.FindLight(selection.id))
		{
			DrawLightInspector(*light);
		}

		else
		{
			selection.Clear();
		}
	}

	else
	{
		DrawEnvironmentInspector();
	}

	ImGui::End();
}

void Editor::DrawEntityInspector(Entity& entity)
{
	Scene& scene = m_application.GetScene();
	bool isChanged = false;

	char name[128];
	std::snprintf(name, sizeof(name), "%s", entity.name.c_str());
	PropertyLabel("Name");

	if (ImGui::InputText("##Name", name, sizeof(name)))
	{
		entity.name = name;
		isChanged = true;
	}

	isChanged |= PropertyCheckbox("Visible", entity.visible);
	isChanged |= PropertyCheckbox("Cast Shadows", entity.castShadows);

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		isChanged |= PropertyVector("Position", entity.transform.position, 0.0f);
		isChanged |= PropertyVector("Rotation", entity.transform.rotation, 0.0f, 0.5f);

		glm::vec3 scale = entity.transform.scale;

		if (PropertyVector("Scale", scale, 1.0f, 0.01f))
		{
			if (m_isUniformScale)
			{
				for (int i = 0; i < 3; i++)
				{
					if (scale[i] != entity.transform.scale[i])
					{
						const float ratio = (std::abs(entity.transform.scale[i]) > 1e-6f) ? scale[i] / entity.transform.scale[i] : 1.0f;
						scale = (std::abs(entity.transform.scale[i]) > 1e-6f) ? entity.transform.scale * ratio : glm::vec3(scale[i]);
						break;
					}
				}
			}

			entity.transform.scale = scale;
			isChanged = true;
		}

		PropertyCheckbox("Uniform Scale", m_isUniformScale);

		if (ImGui::Button("Reset Transform", ImVec2(-FLT_MIN, 0.0f)))
		{
			entity.transform = Transform();
			isChanged = true;
		}
	}

	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (!entity.model)
		{
			ImGui::Text("%c Loading %s", Spinner(), entity.source.c_str());
		}

		else
		{
			const glm::vec3 size = entity.GetWorldBounds().GetSize();

			InfoRow("Source", "%s", entity.source.c_str());
			ImGui::SetItemTooltip("%s", entity.source.c_str());
			InfoRow("Meshes", "%zu", entity.model->meshes.size());
			InfoRow("Vertices", "%zu", entity.model->vertexCount);
			InfoRow("Triangles", "%zu", entity.model->triangleCount);
			InfoRow("Size", "%.2f x %.2f x %.2f", size.x, size.y, size.z);
		}

		if (ImGui::Button("Replace Model...", ImVec2(-FLT_MIN, 0.0f)))
		{
			const std::string path = Platform::OpenFileDialog("Replace Model", ModelFilter);

			if (!path.empty())
			{
				const uint32_t id = entity.id;
				entity.source = path;
				entity.model.reset();
				isChanged = true;

				m_application.GetAssets().LoadModelAsync(path, [this, id](const std::shared_ptr<ModelAsset>& model)
				{
					m_application.GetScene().AssignModel(id, model);
				});
			}
		}
	}

	if (entity.model && ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (entity.model->path.rfind("primitive:", 0) != 0)
		{
			ImGui::TextDisabled("Shared by every instance of this model.");
		}

		for (size_t i = 0; i < entity.model->materials.size(); i++)
		{
			Material& material = entity.model->materials[i];

			ImGui::PushID(static_cast<int>(i));

			const ImGuiTreeNodeFlags flags = (entity.model->materials.size() == 1) ? ImGuiTreeNodeFlags_DefaultOpen : 0;

			if (ImGui::TreeNodeEx("##Material", flags | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding, "%s", material.name.c_str()))
			{
				isChanged |= DrawMaterialEditor(material, static_cast<int>(i));
				ImGui::TreePop();
			}

			ImGui::PopID();
		}
	}

	if (isChanged)
	{
		scene.SetDirty(true);
	}
}

bool Editor::DrawMaterialEditor(Material& material, int index)
{
	bool isChanged = false;

	ImGui::PushID(index);

	isChanged |= PropertyColor("Diffuse", material.diffuse);
	isChanged |= PropertyColor("Specular", material.specular);
	isChanged |= PropertySlider("Shininess", material.shininess, 1.0f, 1024.0f, "%.0f", ImGuiSliderFlags_Logarithmic);
	isChanged |= PropertySlider("Opacity", material.opacity, 0.0f, 1.0f);
	isChanged |= PropertyColor("Emissive", material.emissive);
	isChanged |= PropertyFloat("Emission", material.emissiveStrength, 0.05f, 0.0f, 100.0f);
	isChanged |= PropertySlider("Normal Strength", material.normalStrength, 0.0f, 3.0f);
	isChanged |= PropertyCheckbox("Double Sided", material.doubleSided);

	ImGui::SeparatorText("Textures");

	isChanged |= DrawTextureSlot("Diffuse Map", material.diffusePath, true, material, 0);
	isChanged |= DrawTextureSlot("Specular Map", material.specularPath, false, material, 1);
	isChanged |= DrawTextureSlot("Normal Map", material.normalPath, false, material, 2);

	ImGui::PopID();
	return isChanged;
}

bool Editor::DrawTextureSlot(const char* label, std::string& path, bool isSRGB, Material& material, int slot)
{
	std::shared_ptr<Texture>* texture = (slot == 0) ? &material.diffuseMap : (slot == 1) ? &material.specularMap : &material.normalMap;
	bool isChanged = false;

	ImGui::PushID(label);

	const float previewSize = ImGui::GetFrameHeight() * 2.0f;

	if (*texture && (*texture)->IsLoaded())
	{
		ImGui::Image(static_cast<ImTextureID>((*texture)->GetID()), ImVec2(previewSize, previewSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

		if (ImGui::BeginItemTooltip())
		{
			ImGui::Text("%s (%dx%d)", path.c_str(), (*texture)->GetWidth(), (*texture)->GetHeight());
			ImGui::Image(static_cast<ImTextureID>((*texture)->GetID()), ImVec2(256.0f, 256.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::EndTooltip();
		}
	}

	else
	{
		ImGui::Button("None", ImVec2(previewSize, previewSize));
	}

	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::TextUnformatted(label);
	ImGui::TextDisabled("%s", path.empty() ? "No texture" : Platform::GetFileName(path).c_str());

	if (ImGui::SmallButton("Load"))
	{
		const std::string file = Platform::OpenFileDialog("Load Texture", TextureFilter);

		if (!file.empty())
		{
			if (auto loaded = m_application.GetAssets().LoadTexture(file, isSRGB))
			{
				*texture = loaded;
				path = file;
				isChanged = true;
			}
		}
	}

	if (*texture)
	{
		ImGui::SameLine();

		if (ImGui::SmallButton("Clear"))
		{
			texture->reset();
			path.clear();
			isChanged = true;
		}
	}

	ImGui::EndGroup();
	ImGui::PopID();

	return isChanged;
}

void Editor::DrawLightInspector(Light& light)
{
	bool isChanged = false;

	char name[128];
	std::snprintf(name, sizeof(name), "%s", light.name.c_str());
	PropertyLabel("Name");

	if (ImGui::InputText("##Name", name, sizeof(name)))
	{
		light.name = name;
		isChanged = true;
	}

	isChanged |= PropertyCheckbox("Enabled", light.enabled);

	static const char* types[] = { "Directional", "Point", "Spot" };
	int type = static_cast<int>(light.type);

	if (PropertyCombo("Type", type, types, 3))
	{
		light.type = static_cast<LightType>(type);
		isChanged = true;
	}

	if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
	{
		isChanged |= PropertyColor("Color", light.color);
		isChanged |= PropertyFloat("Intensity", light.intensity, 0.05f, 0.0f, 1000.0f);

		if (light.type != LightType::Directional)
		{
			isChanged |= PropertyFloat("Range", light.range, 0.05f, 0.1f, 500.0f);
		}

		if (light.type == LightType::Spot)
		{
			isChanged |= PropertySlider("Inner Angle", light.innerAngle, 1.0f, 89.0f, "%.1f deg");
			isChanged |= PropertySlider("Outer Angle", light.outerAngle, 1.0f, 89.0f, "%.1f deg");
			light.innerAngle = std::min(light.innerAngle, light.outerAngle);
		}

		if (light.type == LightType::Directional)
		{
			isChanged |= PropertyCheckbox("Cast Shadows", light.castShadows);
			ImGui::TextDisabled("The first shadow-casting sun is used for shadows.");
		}
	}

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		isChanged |= PropertyVector("Position", light.position, 0.0f);

		if (light.type == LightType::Directional)
		{
			ImGui::SetItemTooltip("Only used to place the gizmo.");
		}

		if (light.type != LightType::Point)
		{
			isChanged |= PropertySlider("Pitch", light.pitch, -90.0f, 90.0f, "%.1f deg");
			isChanged |= PropertySlider("Yaw", light.yaw, -180.0f, 180.0f, "%.1f deg");
		}
	}

	if (isChanged)
	{
		m_application.GetScene().SetDirty(true);
	}
}

void Editor::DrawEnvironmentInspector()
{
	Scene& scene = m_application.GetScene();
	Environment& environment = scene.GetEnvironment();
	AssetManager& assets = m_application.GetAssets();
	bool isChanged = false;

	ImGui::SeparatorText("Environment");

	isChanged |= PropertyColor("Sky Top", environment.skyTop);
	isChanged |= PropertyColor("Sky Horizon", environment.skyHorizon);
	isChanged |= PropertyColor("Ground", environment.ground);
	isChanged |= PropertySlider("Ambient", environment.ambientIntensity, 0.0f, 2.0f);
	isChanged |= PropertySlider("Sky Intensity", environment.skyIntensity, 0.0f, 5.0f);

	ImGui::Spacing();
	ImGui::TextDisabled("Presets");

	const float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3.0f) / 4.0f;

	if (ImGui::Button("Day", ImVec2(buttonWidth, 0.0f)))
	{
		environment = Environment();
		isChanged = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Sunset", ImVec2(buttonWidth, 0.0f)))
	{
		environment.skyTop = glm::vec3(0.18f, 0.22f, 0.45f);
		environment.skyHorizon = glm::vec3(1.0f, 0.55f, 0.3f);
		environment.ground = glm::vec3(0.2f, 0.13f, 0.1f);
		environment.ambientIntensity = 0.5f;
		environment.skyIntensity = 1.0f;
		isChanged = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Night", ImVec2(buttonWidth, 0.0f)))
	{
		environment.skyTop = glm::vec3(0.01f, 0.015f, 0.04f);
		environment.skyHorizon = glm::vec3(0.04f, 0.06f, 0.12f);
		environment.ground = glm::vec3(0.02f, 0.02f, 0.025f);
		environment.ambientIntensity = 0.3f;
		environment.skyIntensity = 1.0f;
		isChanged = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Studio", ImVec2(buttonWidth, 0.0f)))
	{
		environment.skyTop = glm::vec3(0.32f);
		environment.skyHorizon = glm::vec3(0.22f);
		environment.ground = glm::vec3(0.12f);
		environment.ambientIntensity = 0.8f;
		environment.skyIntensity = 1.0f;
		isChanged = true;
	}

	ImGui::SeparatorText("Scene");

	InfoRow("File", "%s", m_application.GetScenePath().empty() ? "Untitled" : m_application.GetScenePath().c_str());
	InfoRow("Objects", "%zu", scene.GetEntities().size());
	InfoRow("Lights", "%zu", scene.GetLights().size());
	InfoRow("Models", "%zu cached", assets.GetModelCount());
	InfoRow("Textures", "%zu cached", assets.GetTextureCount());

	ImGui::Spacing();
	ImGui::TextDisabled("Select an object or light to edit it.");

	if (isChanged)
	{
		scene.SetDirty(true);
	}
}

void Editor::DrawConsole()
{
	Settings& settings = m_application.GetSettings();

	ImGui::SetNextWindowPos(ImVec2(m_layout.left, m_layout.consoleTop));
	ImGui::SetNextWindowSize(ImVec2(m_layout.right - m_layout.left, settings.consoleHeight));
	ImGui::SetNextWindowSizeConstraints(ImVec2(m_layout.right - m_layout.left, 80.0f),
		ImVec2(m_layout.right - m_layout.left, (m_layout.bottom - m_layout.top) * 0.6f));

	if (!ImGui::Begin("Console", &settings.showConsole, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	settings.consoleHeight = ImGui::GetWindowHeight();

	if (Log::GetVersion() != m_logVersion)
	{
		m_logVersion = Log::GetVersion();
		m_logEntries.clear();
		Log::ForEach([this](const LogEntry& entry) { m_logEntries.push_back(entry); });
	}

	if (ImGui::Button("Clear"))
	{
		Log::Clear();
	}

	ImGui::SameLine();
	ImGui::Checkbox("Info", &m_showInfo);
	ImGui::SameLine();
	ImGui::Checkbox("Warnings", &m_showWarnings);
	ImGui::SameLine();
	ImGui::Checkbox("Errors", &m_showErrors);
	ImGui::SameLine();
	ImGui::Checkbox("Auto-scroll", &m_isAutoScroll);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputTextWithHint("##ConsoleFilter", "Filter...", m_consoleFilter, sizeof(m_consoleFilter));

	ImGui::BeginChild("##ConsoleLog", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);

	std::vector<int> visible;
	visible.reserve(m_logEntries.size());

	for (int i = 0; i < static_cast<int>(m_logEntries.size()); i++)
	{
		const LogEntry& entry = m_logEntries[i];

		const bool isShown = (entry.level == LogLevel::Info && m_showInfo) ||
			(entry.level == LogLevel::Warning && m_showWarnings) ||
			(entry.level == LogLevel::Error && m_showErrors);

		if (isShown && (m_consoleFilter[0] == '\0' || entry.message.find(m_consoleFilter) != std::string::npos))
		{
			visible.push_back(i);
		}
	}

	ImGuiListClipper clipper;
	clipper.Begin(static_cast<int>(visible.size()));

	while (clipper.Step())
	{
		for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
		{
			const LogEntry& entry = m_logEntries[visible[row]];
			const ImVec4 color = (entry.level == LogLevel::Error) ? ErrorColor :
				(entry.level == LogLevel::Warning) ? WarningColor : ImGui::GetStyleColorVec4(ImGuiCol_Text);

			ImGui::TextDisabled("%s", entry.time.c_str());
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::TextUnformatted(entry.message.c_str());
			ImGui::PopStyleColor();
		}
	}

	if (m_isAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f)
	{
		ImGui::SetScrollHereY(1.0f);
	}

	ImGui::EndChild();
	ImGui::End();
}

void Editor::DrawStatusBar()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float height = ImGui::GetFrameHeight();

	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - height));
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, height));

	const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
	ImGui::Begin("##StatusBar", nullptr, flags);
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	Scene& scene = m_application.GetScene();
	const Selection& selection = m_application.GetSelection();
	const std::string& scenePath = m_application.GetScenePath();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("%s%s", scenePath.empty() ? "Untitled" : Platform::GetFileName(scenePath).c_str(), scene.IsDirty() ? " *" : "");

	if (selection.type == Selection::Type::Entity)
	{
		if (const Entity* entity = scene.FindEntity(selection.id))
		{
			ImGui::SameLine();
			ImGui::TextDisabled("|  Selected: %s", entity->name.c_str());
		}
	}

	else if (selection.type == Selection::Type::Light)
	{
		if (const Light* light = scene.FindLight(selection.id))
		{
			ImGui::SameLine();
			ImGui::TextDisabled("|  Selected: %s", light->name.c_str());
		}
	}

	if (m_application.GetAssets().IsLoading())
	{
		ImGui::SameLine();
		ImGui::TextColored(AccentColor, "|  %c Loading %zu asset(s)...", Spinner(), m_application.GetAssets().GetPendingCount());
	}

	const RenderStats& stats = m_application.GetRenderStats();

	char info[128];
	std::snprintf(info, sizeof(info), "%zu objects  |  %zu lights  |  %d draw calls  |  %.1f ms",
		scene.GetEntities().size(), scene.GetLights().size(), stats.drawCalls, m_application.GetFrameTime());

	ImGui::SameLine(std::max(ImGui::GetWindowWidth() - ImGui::CalcTextSize(info).x - 8.0f, ImGui::GetCursorPosX() + 20.0f));
	ImGui::TextDisabled("%s", info);

	ImGui::End();
}

void Editor::DrawViewportOverlay()
{
	Settings& settings = m_application.GetSettings();
	const float padding = 10.0f;

	if (m_application.GetScene().GetEntities().empty())
	{
		const char* hint = "Drop .obj files here or use File > Import Model";
		const ImVec2 size = ImGui::CalcTextSize(hint);

		ImGui::GetBackgroundDrawList()->AddText(ImVec2(m_viewportLeft + (m_viewportWidth - size.x) * 0.5f,
			m_viewportTop + (m_viewportHeight - size.y) * 0.5f), IM_COL32(200, 200, 200, 160), hint);
	}

	if (settings.showStats)
	{
		const RenderStats& stats = m_application.GetRenderStats();
		const auto& history = m_application.GetFrameHistory();

		ImGui::SetNextWindowPos(ImVec2(m_viewportLeft + padding, m_viewportTop + padding));
		ImGui::SetNextWindowBgAlpha(0.6f);

		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

		if (ImGui::Begin("##Stats", nullptr, flags))
		{
			ImGui::Text("%.0f FPS  (%.2f ms)", m_application.GetFPS(), m_application.GetFrameTime());
			ImGui::PlotLines("##FrameTimes", history.data(), static_cast<int>(history.size()), m_application.GetFrameHistoryOffset(),
				nullptr, 0.0f, 33.3f, ImVec2(ImGui::GetFontSize() * 11.0f, ImGui::GetFontSize() * 2.2f));
			ImGui::Text("GPU         %.2f ms", stats.gpuTime);
			ImGui::Text("Draw calls  %d", stats.drawCalls);
			ImGui::Text("Triangles   %d", stats.triangles);
			ImGui::Text("Meshes      %d visible, %d culled", stats.visibleMeshes, stats.culledMeshes);
			ImGui::Text("Lights      %d active", stats.lights);
			ImGui::Text("Shadows     %d casters", stats.shadowCasters);
		}

		ImGui::End();
	}

	if (settings.showAxisGizmo)
	{
		const glm::mat4& view = m_application.GetCamera().GetView();
		const float radius = ImGui::GetFontSize() * 2.2f;
		const ImVec2 center(m_viewportLeft + m_viewportWidth - radius - padding * 2.0f, m_viewportTop + radius + padding * 2.0f);

		struct Axis
		{
			glm::vec3 direction;
			ImU32 color;
			const char* label;
		};

		Axis axes[] =
		{
			{ glm::vec3(view * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)), IM_COL32(230, 70, 70, 255), "X" },
			{ glm::vec3(view * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)), IM_COL32(110, 200, 80, 255), "Y" },
			{ glm::vec3(view * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)), IM_COL32(70, 130, 240, 255), "Z" }
		};

		std::sort(std::begin(axes), std::end(axes), [](const Axis& a, const Axis& b) { return a.direction.z < b.direction.z; });

		ImDrawList* drawList = ImGui::GetBackgroundDrawList();
		drawList->AddCircleFilled(center, radius + 8.0f, IM_COL32(20, 22, 26, 140));

		for (const auto& axis : axes)
		{
			const ImVec2 end(center.x + axis.direction.x * radius, center.y - axis.direction.y * radius);
			drawList->AddLine(center, end, axis.color, 2.0f);
			drawList->AddCircleFilled(end, ImGui::GetFontSize() * 0.45f, axis.color);

			const ImVec2 textSize = ImGui::CalcTextSize(axis.label);
			drawList->AddText(ImVec2(end.x - textSize.x * 0.5f, end.y - textSize.y * 0.5f), IM_COL32(15, 15, 15, 255), axis.label);
		}
	}
}

void Editor::DrawSettingsWindow()
{
	Settings& settings = m_application.GetSettings();
	bool isApplyNeeded = false;

	ImGui::SetNextWindowSizeConstraints(ImVec2(ImGui::GetFontSize() * 30.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (!ImGui::Begin("Settings", &m_showSettings, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	if (ImGui::BeginTabBar("##SettingsTabs"))
	{
		if (ImGui::BeginTabItem("Display"))
		{
			isApplyNeeded |= PropertyCheckbox("Fullscreen", settings.fullscreen);
			isApplyNeeded |= PropertyCheckbox("V-Sync", settings.vsync);

			PropertyLabel("FPS Limit");
			ImGui::BeginDisabled(settings.vsync);
			ImGui::SliderInt("##FpsLimit", &settings.fpsLimit, 0, 360, settings.fpsLimit == 0 ? "Unlimited" : "%d");
			ImGui::EndDisabled();

			PropertyCheckbox("Pause Unfocused", settings.pauseWhenUnfocused);
			ImGui::SetItemTooltip("Lower the frame rate while the window is in the background.");

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Rendering"))
		{
			static const char* msaaNames[] = { "Off", "2x", "4x", "8x" };
			static const int msaaValues[] = { 0, 2, 4, 8 };
			int msaa = static_cast<int>(std::find(std::begin(msaaValues), std::end(msaaValues), settings.msaaSamples) - std::begin(msaaValues));
			msaa = std::min(msaa, 3);

			if (PropertyCombo("MSAA", msaa, msaaNames, 4))
			{
				settings.msaaSamples = msaaValues[msaa];
			}

			PropertyCheckbox("FXAA", settings.fxaa);

			static const char* anisotropyNames[] = { "Off", "2x", "4x", "8x", "16x" };
			static const int anisotropyValues[] = { 1, 2, 4, 8, 16 };
			int anisotropy = static_cast<int>(std::find(std::begin(anisotropyValues), std::end(anisotropyValues), settings.anisotropy) - std::begin(anisotropyValues));
			anisotropy = std::min(anisotropy, 4);

			if (PropertyCombo("Anisotropic", anisotropy, anisotropyNames, 5))
			{
				settings.anisotropy = anisotropyValues[anisotropy];
				isApplyNeeded = true;
			}

			PropertyCheckbox("Frustum Culling", settings.frustumCulling);
			PropertyCheckbox("Wireframe", settings.wireframe);

			static const char* modes[] = { "Lit", "Albedo", "Normals", "Lighting Only" };
			int mode = static_cast<int>(settings.renderMode);

			if (PropertyCombo("Render Mode", mode, modes, 4))
			{
				settings.renderMode = static_cast<RenderMode>(mode);
			}

			ImGui::SeparatorText("Shadows");

			PropertyCheckbox("Enabled", settings.shadows);

			ImGui::BeginDisabled(!settings.shadows);

			static const char* resolutionNames[] = { "512", "1024", "2048", "4096" };
			static const int resolutionValues[] = { 512, 1024, 2048, 4096 };
			int resolution = static_cast<int>(std::find(std::begin(resolutionValues), std::end(resolutionValues), settings.shadowResolution) - std::begin(resolutionValues));
			resolution = std::min(resolution, 3);

			if (PropertyCombo("Resolution", resolution, resolutionNames, 4))
			{
				settings.shadowResolution = resolutionValues[resolution];
			}

			static const char* softnessNames[] = { "Hard", "Soft (3x3)", "Softer (5x5)", "Softest (7x7)" };
			PropertyCombo("Filtering", settings.shadowSoftness, softnessNames, 4);
			PropertySlider("Normal Bias", settings.shadowBias, 0.0f, 5.0f);

			ImGui::EndDisabled();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Post Processing"))
		{
			static const char* toneMappingNames[] = { "None", "Reinhard", "ACES Filmic" };
			int toneMapping = static_cast<int>(settings.toneMapping);

			if (PropertyCombo("Tone Mapping", toneMapping, toneMappingNames, 3))
			{
				settings.toneMapping = static_cast<ToneMapping>(toneMapping);
			}

			PropertySlider("Exposure", settings.exposure, 0.05f, 8.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
			PropertySlider("Gamma", settings.gamma, 1.0f, 3.0f);
			PropertyCheckbox("Vignette", settings.vignette);

			ImGui::SeparatorText("Bloom");

			PropertyCheckbox("Enabled", settings.bloom);
			ImGui::BeginDisabled(!settings.bloom);
			PropertySlider("Threshold", settings.bloomThreshold, 0.0f, 5.0f);
			PropertySlider("Intensity", settings.bloomIntensity, 0.0f, 3.0f);
			PropertySlider("Radius", settings.bloomRadius, 0.5f, 3.0f);
			ImGui::EndDisabled();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Camera"))
		{
			PropertySlider("Field of View", settings.fieldOfView, 20.0f, 120.0f, "%.0f deg");
			PropertySlider("Near Clip", settings.nearClip, 0.01f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
			PropertySlider("Far Clip", settings.farClip, 50.0f, 5000.0f, "%.0f", ImGuiSliderFlags_Logarithmic);
			PropertySlider("Move Speed", settings.cameraSpeed, 0.5f, 50.0f, "%.1f m/s", ImGuiSliderFlags_Logarithmic);
			PropertySlider("Sensitivity", settings.mouseSensitivity, 0.02f, 0.6f);
			PropertyCheckbox("Invert Y", settings.invertY);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Interface"))
		{
			static const char* themeNames[] = { "Dark", "Light", "Classic" };
			int theme = static_cast<int>(settings.theme);

			if (PropertyCombo("Theme", theme, themeNames, 3))
			{
				settings.theme = static_cast<UITheme>(theme);
				isApplyNeeded = true;
			}

			PropertySlider("UI Scale", settings.uiScale, 0.75f, 2.0f, "%.2f");

			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				isApplyNeeded = true;
			}

			ImGui::SeparatorText("Viewport");

			PropertyCheckbox("Statistics", settings.showStats);
			PropertyCheckbox("Axis Gizmo", settings.showAxisGizmo);
			PropertyCheckbox("Grid", settings.showGrid);
			PropertyCheckbox("Light Icons", settings.showLightIcons);
			PropertyCheckbox("Bounding Boxes", settings.showBounds);

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::Separator();

	if (ImGui::Button("Reset to Defaults"))
	{
		ShowConfirm("Reset Settings", "Reset every setting to its default value?", [this]() { m_application.ResetSettings(); });
	}

	ImGui::SameLine();

	if (ImGui::Button("Save"))
	{
		settings.Save("settings.ini");
		Log::Info("Settings saved.");
	}

	ImGui::SameLine();
	ImGui::TextDisabled("Settings are saved automatically on exit.");

	ImGui::End();

	if (isApplyNeeded)
	{
		m_application.ApplySettings();
	}
}

void Editor::DrawControlsWindow()
{
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 26.0f, 0.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

	if (!ImGui::Begin("Controls", &m_showControls, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	static const char* controls[][2] =
	{
		{ "Right Mouse + Move", "Look around" },
		{ "W A S D", "Move camera" },
		{ "Q / E", "Move down / up" },
		{ "Shift", "Move faster" },
		{ "Right Mouse + Wheel", "Change move speed" },
		{ "Alt + Left Mouse", "Orbit" },
		{ "Middle Mouse", "Pan" },
		{ "Mouse Wheel", "Zoom" },
		{ "Left Click", "Select" },
		{ "Double Click (Hierarchy)", "Focus" },
		{ "F", "Focus selection" },
		{ "Ctrl + D", "Duplicate" },
		{ "Delete", "Delete selection" },
		{ "Esc", "Deselect" },
		{ "Ctrl + N / O / S", "New / Open / Save scene" },
		{ "Ctrl + I", "Import model" },
		{ "Ctrl + ,", "Settings" },
		{ "F5", "Reload shaders" },
		{ "F11", "Fullscreen" },
		{ "F12", "Screenshot" },
		{ "Drag & Drop", "Import .obj / open .scene" }
	};

	if (ImGui::BeginTable("##Controls", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
	{
		for (const auto& control : controls)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextColored(AccentColor, "%s", control[0]);
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(control[1]);
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

void Editor::DrawAboutWindow()
{
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (!ImGui::Begin("About", &m_showAbout, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	if (m_largeFont)
	{
		ImGui::PushFont(m_largeFont);
	}

	ImGui::TextColored(AccentColor, "Absolute Engine");

	if (m_largeFont)
	{
		ImGui::PopFont();
	}

	ImGui::TextDisabled("Version 2.0  -  A small, fast OpenGL renderer and scene editor");
	ImGui::Separator();

	InfoRow("OpenGL", "%s", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	InfoRow("GPU", "%s", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
	InfoRow("Vendor", "%s", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));

	SDL_version version;
	SDL_GetVersion(&version);
	InfoRow("SDL", "%d.%d.%d", version.major, version.minor, version.patch);
	InfoRow("Dear ImGui", "%s", ImGui::GetVersion());

	ImGui::Separator();
	ImGui::TextDisabled("Created by Mohammad Reza Yadollahi. Released under the MIT License.");

	ImGui::End();
}

void Editor::DrawConfirmDialog()
{
	if (m_isConfirmPending)
	{
		ImGui::OpenPopup("##Confirm");
		m_isConfirmPending = false;
	}

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("##Confirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::TextColored(AccentColor, "%s", m_confirmTitle.c_str());
		ImGui::Separator();
		ImGui::TextUnformatted(m_confirmMessage.c_str());
		ImGui::Spacing();

		const float buttonWidth = ImGui::GetFontSize() * 6.0f;

		if (ImGui::Button("Yes", ImVec2(buttonWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			ImGui::CloseCurrentPopup();

			if (m_onConfirm)
			{
				auto callback = std::move(m_onConfirm);
				m_onConfirm = nullptr;
				callback();
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			m_onConfirm = nullptr;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}
