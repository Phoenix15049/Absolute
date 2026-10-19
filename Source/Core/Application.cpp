#include <algorithm>
#include <cmath>
#include <vector>
#include <SDL.h>
#include <SDL_image.h>
#include "Core/Application.h"
#include "Core/Input.h"
#include "Core/Log.h"
#include "Core/Platform.h"
#include "Editor/Editor.h"

namespace
{
	const char* SettingsFile = "settings.ini";
	const char* SceneFilter = "Absolute Scene (*.scene)\0*.scene\0All Files (*.*)\0*.*\0";
	const char* ModelFilter = "Wavefront OBJ (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";

	void AimLight(Light& light, const glm::vec3& target)
	{
		const glm::vec3 direction = glm::normalize(target - light.position);
		light.pitch = glm::degrees(std::asin(std::clamp(direction.y, -1.0f, 1.0f)));
		light.yaw = glm::degrees(std::atan2(direction.x, -direction.z));
	}
}

Application::Application()
{
	m_isViewportHovered = false;
	m_isRunning = false;
	m_isQuitConfirmed = false;
	m_frameTime = 0.0f;
	m_fps = 0.0f;
	m_frameHistory.fill(0.0f);
	m_frameHistoryOffset = 0;
	m_mouseDownX = 0;
	m_mouseDownY = 0;
	m_captureFrames = 0;
}

Application::~Application()
{
	Shutdown();
}

bool Application::Initialize()
{
	m_settings.Load(SettingsFile);

	if (!m_window.Create("Absolute Engine", m_settings.windowWidth, m_settings.windowHeight, m_settings.maximized))
	{
		return false;
	}

	Platform::SetMainWindow(m_window.GetHandle());

	if (!m_renderer.Initialize())
	{
		Log::Error("Renderer initialization failed.");
		return false;
	}

	m_editor = std::make_unique<Editor>(*this);

	if (!m_editor->Initialize(m_window))
	{
		Log::Error("Editor initialization failed.");
		return false;
	}

	ApplySettings();
	CreateDefaultScene();

	m_isRunning = true;
	return true;
}

void Application::Run()
{
	Uint64 lastTime = SDL_GetPerformanceCounter();
	const double frequency = static_cast<double>(SDL_GetPerformanceFrequency());

	while (m_isRunning)
	{
		const Uint64 frameStart = SDL_GetPerformanceCounter();
		const float deltaTime = std::min(static_cast<float>((frameStart - lastTime) / frequency), 0.25f);
		lastTime = frameStart;

		m_frameTime = deltaTime * 1000.0f;
		m_frameHistory[m_frameHistoryOffset] = m_frameTime;
		m_frameHistoryOffset = (m_frameHistoryOffset + 1) % FrameHistorySize;

		float totalTime = 0.0f;
		int frameCount = 0;

		for (float frameTime : m_frameHistory)
		{
			if (frameTime > 0.0f)
			{
				totalTime += frameTime;
				frameCount++;
			}
		}

		m_fps = (totalTime > 0.0f) ? 1000.0f * frameCount / totalTime : 0.0f;

		Input::Instance()->Update([this](const SDL_Event& event) { m_editor->ProcessEvent(event); });

		if (Input::Instance()->IsQuitRequested())
		{
			Input::Instance()->ClearQuitRequest();
			RequestQuit();
		}

		HandleDroppedFiles();
		m_assets.Update();

		if (m_window.IsMinimized())
		{
			SDL_Delay(50);
			continue;
		}

		m_editor->BeginFrame();
		m_editor->Draw();

		if (!m_editor->WantsTextInput() && !m_camera.IsNavigating())
		{
			HandleShortcuts();
		}

		m_camera.SetAspectRatio(static_cast<float>(m_viewport.width) / static_cast<float>(std::max(m_viewport.height, 1)));
		m_camera.Update(deltaTime, m_settings, m_isViewportHovered, !m_editor->WantsKeyboard());

		const bool isLooking = m_camera.IsNavigating() && Input::Instance()->IsMouseHeld(SDL_BUTTON_RIGHT);

		if ((SDL_GetRelativeMouseMode() == SDL_TRUE) != isLooking)
		{
			SDL_SetRelativeMouseMode(isLooking ? SDL_TRUE : SDL_FALSE);
		}

		HandlePicking();

		int width = 0;
		int height = 0;
		m_window.GetDrawableSize(width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		m_renderer.Render(m_scene, m_camera, m_settings, m_viewport, m_selection);

		m_editor->EndFrame();

		if (!m_capturePath.empty() && !m_assets.IsLoading() && --m_captureFrames <= 0)
		{
			SaveWindowCapture();
		}

		m_window.SwapBuffers();

		UpdateTitle();

		if (!m_settings.vsync && m_settings.fpsLimit > 0)
		{
			const double target = 1.0 / m_settings.fpsLimit;

			while ((SDL_GetPerformanceCounter() - frameStart) / frequency < target)
			{
				const double remaining = target - (SDL_GetPerformanceCounter() - frameStart) / frequency;

				if (remaining > 0.002)
				{
					SDL_Delay(1);
				}
			}
		}

		if (m_settings.pauseWhenUnfocused && !m_window.IsFocused())
		{
			SDL_Delay(30);
		}
	}
}

void Application::Shutdown()
{
	if (!m_editor)
	{
		return;
	}

	m_settings.maximized = m_window.IsMaximized();

	if (!m_settings.maximized && !m_settings.fullscreen)
	{
		m_window.GetWindowSize(m_settings.windowWidth, m_settings.windowHeight);
	}

	m_settings.Save(SettingsFile);

	m_assets.Shutdown();
	m_scene.Clear();
	m_editor->Shutdown();
	m_editor.reset();
	m_renderer.Shutdown();
	m_window.Destroy();
}

void Application::NewScene()
{
	m_scene.Clear();
	m_selection.Clear();
	m_scenePath.clear();
	m_camera.Reset();
	m_assets.CollectGarbage();

	Light& sun = m_scene.CreateLight("Sun", LightType::Directional);
	sun.position = glm::vec3(0.0f, 5.0f, 0.0f);
	sun.intensity = 2.2f;
	sun.color = glm::vec3(1.0f, 0.96f, 0.9f);

	m_scene.SetDirty(false);
	Log::Info("New scene created.");
}

void Application::CreateDefaultScene()
{
	NewScene();

	Entity& ground = m_scene.CreateEntity("Ground");
	ground.source = "primitive:Plane";
	ground.model = m_assets.CreatePrimitive(PrimitiveType::Plane);
	ground.transform.scale = glm::vec3(3.0f, 1.0f, 3.0f);
	ground.castShadows = false;

	Entity& crate = m_scene.CreateEntity("Crate");
	crate.source = "primitive:Cube";
	crate.model = m_assets.CreatePrimitive(PrimitiveType::Cube);
	crate.model->materials[0].diffuse = glm::vec3(1.0f);
	crate.model->materials[0].diffusePath = "Textures/Crate_1.png";
	crate.model->materials[0].diffuseMap = m_assets.LoadTexture(crate.model->materials[0].diffusePath, true);
	crate.transform.position = glm::vec3(-2.6f, 0.6f, 0.8f);
	crate.transform.rotation = glm::vec3(0.0f, 25.0f, 0.0f);
	crate.transform.scale = glm::vec3(1.2f);

	Entity& sphere = m_scene.CreateEntity("Sphere");
	sphere.source = "primitive:Sphere";
	sphere.model = m_assets.CreatePrimitive(PrimitiveType::Sphere);
	sphere.model->materials[0].diffuse = glm::vec3(0.75f, 0.12f, 0.1f);
	sphere.model->materials[0].specular = glm::vec3(0.9f);
	sphere.model->materials[0].shininess = 180.0f;
	sphere.transform.position = glm::vec3(2.6f, 0.75f, 0.6f);
	sphere.transform.scale = glm::vec3(1.5f);

	Entity& orb = m_scene.CreateEntity("Glowing Orb");
	orb.source = "primitive:Sphere";
	orb.model = m_assets.CreatePrimitive(PrimitiveType::Sphere);
	orb.model->materials[0].diffuse = glm::vec3(0.05f);
	orb.model->materials[0].emissive = glm::vec3(0.3f, 0.6f, 1.0f);
	orb.model->materials[0].emissiveStrength = 6.0f;
	orb.transform.position = glm::vec3(-1.6f, 1.9f, 2.2f);
	orb.transform.scale = glm::vec3(0.3f);
	orb.castShadows = false;

	if (Platform::FileExists("Models/Armchair.obj"))
	{
		Entity& dragon = m_scene.CreateEntity("Dragon");
		dragon.source = "Models/Armchair.obj";
		const uint32_t id = dragon.id;

		m_assets.LoadModelAsync(dragon.source, [this, id](const std::shared_ptr<ModelAsset>& model)
		{
			Entity* entity = m_scene.FindEntity(id);

			if (!entity || !model)
			{
				return;
			}

			const float scale = 2.4f / std::max(model->bounds.GetSize().y, 0.001f);
			entity->model = model;
			entity->transform.scale = glm::vec3(scale);
			entity->transform.position = glm::vec3(0.0f, -model->bounds.min.y * scale, -0.6f);
			entity->transform.rotation = glm::vec3(0.0f, 20.0f, 0.0f);
		});
	}

	Light* sun = &m_scene.GetLights().front();
	sun->pitch = -42.0f;
	sun->yaw = 35.0f;

	Light& fill = m_scene.CreateLight("Blue Point Light", LightType::Point);
	fill.position = glm::vec3(-1.6f, 1.9f, 2.2f);
	fill.color = glm::vec3(0.3f, 0.6f, 1.0f);
	fill.intensity = 3.5f;
	fill.range = 5.0f;

	Light& spot = m_scene.CreateLight("Spot Light", LightType::Spot);
	spot.position = glm::vec3(3.5f, 4.5f, 3.0f);
	spot.color = glm::vec3(1.0f, 0.75f, 0.45f);
	spot.intensity = 25.0f;
	spot.range = 14.0f;
	spot.innerAngle = 18.0f;
	spot.outerAngle = 28.0f;
	AimLight(spot, glm::vec3(2.6f, 0.5f, 0.6f));

	m_camera.Reset();
	m_scene.SetDirty(false);
}

void Application::OpenScene()
{
	const std::string path = Platform::OpenFileDialog("Open Scene", SceneFilter);

	if (!path.empty())
	{
		OpenScene(path);
	}
}

void Application::OpenScene(const std::string& path)
{
	m_selection.Clear();

	if (m_scene.Load(path, m_assets))
	{
		m_scenePath = path;
		m_camera.Reset();
		m_assets.CollectGarbage();
	}
}

void Application::SaveScene(bool saveAs)
{
	std::string path = m_scenePath;

	if (saveAs || path.empty())
	{
		Platform::CreateDirectories("Scenes");
		path = Platform::SaveFileDialog("Save Scene", SceneFilter, "scene");
	}

	if (path.empty())
	{
		return;
	}

	if (m_scene.Save(path))
	{
		m_scenePath = path;
		m_scene.SetDirty(false);
		Log::Info("Scene saved to " + path);
	}
}

void Application::ImportModel()
{
	const std::string path = Platform::OpenFileDialog("Import Model", ModelFilter);

	if (!path.empty())
	{
		ImportModel(path);
	}
}

void Application::ImportModel(const std::string& path)
{
	if (Platform::GetExtension(path) != ".obj")
	{
		Log::Warning("Unsupported model format: " + path + " (only .obj is supported)");
		return;
	}

	const std::string relativePath = Platform::MakeRelative(path);

	Entity& entity = m_scene.CreateEntity(Platform::GetFileStem(relativePath));
	entity.source = relativePath;
	entity.transform.position = GetSpawnPosition();

	const uint32_t id = entity.id;
	m_selection.type = Selection::Type::Entity;
	m_selection.id = id;

	m_assets.LoadModelAsync(relativePath, [this, id](const std::shared_ptr<ModelAsset>& model)
	{
		Entity* target = m_scene.FindEntity(id);

		if (!target)
		{
			return;
		}

		if (!model)
		{
			m_scene.RemoveEntity(id);
			return;
		}

		target->model = model;

		const float extent = std::max({ model->bounds.GetSize().x, model->bounds.GetSize().y, model->bounds.GetSize().z });

		if (extent > 50.0f || extent < 0.05f)
		{
			const float scale = 2.0f / std::max(extent, 0.0001f);
			target->transform.scale = glm::vec3(scale);
			Log::Info(target->name + " was rescaled by " + std::to_string(scale) + " to fit the scene.");
		}

		target->transform.position.y -= model->bounds.min.y * target->transform.scale.y;

		if (m_selection.IsEntity(id))
		{
			m_camera.Focus(target->GetWorldBounds());
		}
	});
}

void Application::AddPrimitive(PrimitiveType type)
{
	Entity& entity = m_scene.CreateEntity(Primitives::GetName(type));
	entity.source = std::string("primitive:") + Primitives::GetName(type);
	entity.model = m_assets.CreatePrimitive(type);
	entity.transform.position = GetSpawnPosition();

	if (type != PrimitiveType::Plane)
	{
		entity.transform.position.y += 0.5f;
	}

	m_selection.type = Selection::Type::Entity;
	m_selection.id = entity.id;
}

void Application::AddLight(LightType type)
{
	static const char* names[] = { "Directional Light", "Point Light", "Spot Light" };

	Light& light = m_scene.CreateLight(names[static_cast<int>(type)], type);
	light.position = GetSpawnPosition() + glm::vec3(0.0f, 2.0f, 0.0f);

	if (type == LightType::Spot)
	{
		light.intensity = 15.0f;
		light.pitch = -90.0f;
	}

	else if (type == LightType::Point)
	{
		light.intensity = 4.0f;
	}

	m_selection.type = Selection::Type::Light;
	m_selection.id = light.id;
}

void Application::DuplicateSelection()
{
	if (const Entity* entity = m_scene.FindEntity(m_selection.id); entity && m_selection.type == Selection::Type::Entity)
	{
		const Entity copy = *entity;
		Entity& created = m_scene.CreateEntity(copy.name + " Copy");
		const uint32_t id = created.id;

		created = copy;
		created.id = id;
		created.name = copy.name + " Copy";
		created.transform.position.x += 1.0f;

		PrimitiveType primitive = PrimitiveType::Cube;

		if (copy.model && Primitives::FromSource(copy.source, primitive))
		{
			const auto materials = copy.model->materials;
			created.model = m_assets.CreatePrimitive(primitive);

			for (size_t i = 0; i < materials.size() && i < created.model->materials.size(); i++)
			{
				created.model->materials[i] = materials[i];
			}
		}

		m_selection.id = id;
	}

	else if (const Light* light = m_scene.FindLight(m_selection.id); light && m_selection.type == Selection::Type::Light)
	{
		const Light copy = *light;
		Light& created = m_scene.CreateLight(copy.name + " Copy", copy.type);
		const uint32_t id = created.id;

		created = copy;
		created.id = id;
		created.name = copy.name + " Copy";
		created.position.x += 1.0f;

		m_selection.id = id;
	}
}

void Application::DeleteSelection()
{
	if (m_selection.type == Selection::Type::Entity)
	{
		m_scene.RemoveEntity(m_selection.id);
	}

	else if (m_selection.type == Selection::Type::Light)
	{
		m_scene.RemoveLight(m_selection.id);
	}

	m_selection.Clear();
}

void Application::FocusSelection()
{
	if (m_selection.type == Selection::Type::Entity)
	{
		if (const Entity* entity = m_scene.FindEntity(m_selection.id))
		{
			m_camera.Focus(entity->GetWorldBounds());
		}
	}

	else if (m_selection.type == Selection::Type::Light)
	{
		if (const Light* light = m_scene.FindLight(m_selection.id))
		{
			BoundingBox bounds;
			bounds.Expand(light->position - glm::vec3(1.0f));
			bounds.Expand(light->position + glm::vec3(1.0f));
			m_camera.Focus(bounds);
		}
	}

	else
	{
		m_camera.Focus(m_scene.GetBounds());
	}
}

void Application::CaptureWindowAndQuit(const std::string& filename)
{
	m_capturePath = filename;
	m_captureFrames = 30;
}

void Application::SaveWindowCapture()
{
	int width = 0;
	int height = 0;
	m_window.GetDrawableSize(width, height);

	const size_t rowSize = static_cast<size_t>(width) * 4;
	std::vector<unsigned char> pixels(rowSize * height);
	std::vector<unsigned char> flipped(rowSize * height);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glReadBuffer(GL_BACK);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	for (int row = 0; row < height; row++)
	{
		std::copy_n(&pixels[(height - 1 - row) * rowSize], rowSize, &flipped[row * rowSize]);
	}

	for (size_t i = 3; i < flipped.size(); i += 4)
	{
		flipped[i] = 255;
	}

	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(flipped.data(), width, height, 32,
		static_cast<int>(rowSize), SDL_PIXELFORMAT_RGBA32);

	if (surface && IMG_SavePNG(surface, m_capturePath.c_str()) == 0)
	{
		Log::Info("Window captured to " + m_capturePath);
	}

	SDL_FreeSurface(surface);
	m_capturePath.clear();
	Quit();
}

void Application::TakeScreenshot()
{
	Platform::CreateDirectories("Screenshots");
	m_renderer.RequestScreenshot("Screenshots/Absolute_" + Platform::GetTimestamp() + ".png");
}

void Application::ReloadShaders()
{
	m_renderer.ReloadShaders();
}

void Application::ApplySettings()
{
	m_window.SetVSync(m_settings.vsync);
	m_window.SetFullscreen(m_settings.fullscreen);

	Texture::SetAnisotropy(static_cast<float>(m_settings.anisotropy));
	m_assets.ApplyAnisotropy();

	m_editor->ApplyStyle();
}

void Application::ResetSettings()
{
	const Settings defaults;
	m_settings = defaults;
	ApplySettings();
	Log::Info("Settings reset to defaults.");
}

void Application::RequestQuit()
{
	if (m_scene.IsDirty())
	{
		m_editor->ShowConfirm("Unsaved Changes", "The scene has unsaved changes. Quit anyway?", [this]() { Quit(); });
	}

	else
	{
		Quit();
	}
}

void Application::Quit()
{
	m_isRunning = false;
}

Scene& Application::GetScene()
{
	return m_scene;
}

Settings& Application::GetSettings()
{
	return m_settings;
}

Camera& Application::GetCamera()
{
	return m_camera;
}

Selection& Application::GetSelection()
{
	return m_selection;
}

AssetManager& Application::GetAssets()
{
	return m_assets;
}

const RenderStats& Application::GetRenderStats() const
{
	return m_renderer.GetStats();
}

const std::string& Application::GetScenePath() const
{
	return m_scenePath;
}

float Application::GetFrameTime() const
{
	return m_frameTime;
}

float Application::GetFPS() const
{
	return m_fps;
}

const std::array<float, Application::FrameHistorySize>& Application::GetFrameHistory() const
{
	return m_frameHistory;
}

int Application::GetFrameHistoryOffset() const
{
	return m_frameHistoryOffset;
}

void Application::SetViewport(const Viewport& viewport, bool isHovered)
{
	m_viewport = viewport;
	m_isViewportHovered = isHovered;
}

void Application::HandleShortcuts()
{
	const Input* input = Input::Instance();
	const bool ctrl = input->IsCtrlHeld();
	const bool shift = input->IsShiftHeld();

	if (ctrl && input->IsKeyPressed(SDL_SCANCODE_N))
	{
		m_editor->ShowConfirm("New Scene", "Discard the current scene and start a new one?", [this]() { NewScene(); }, m_scene.IsDirty());
	}

	else if (ctrl && input->IsKeyPressed(SDL_SCANCODE_O))
	{
		OpenScene();
	}

	else if (ctrl && input->IsKeyPressed(SDL_SCANCODE_S))
	{
		SaveScene(shift);
	}

	else if (ctrl && input->IsKeyPressed(SDL_SCANCODE_I))
	{
		ImportModel();
	}

	else if (ctrl && input->IsKeyPressed(SDL_SCANCODE_D))
	{
		DuplicateSelection();
	}

	else if (input->IsKeyPressed(SDL_SCANCODE_DELETE))
	{
		DeleteSelection();
	}

	else if (!ctrl && input->IsKeyPressed(SDL_SCANCODE_F))
	{
		FocusSelection();
	}

	else if (input->IsKeyPressed(SDL_SCANCODE_F12))
	{
		TakeScreenshot();
	}

	else if (input->IsKeyPressed(SDL_SCANCODE_F5))
	{
		ReloadShaders();
	}

	else if (input->IsKeyPressed(SDL_SCANCODE_F11))
	{
		m_settings.fullscreen = !m_settings.fullscreen;
		ApplySettings();
	}

	else if (input->IsKeyPressed(SDL_SCANCODE_ESCAPE))
	{
		m_selection.Clear();
	}
}

void Application::HandlePicking()
{
	const Input* input = Input::Instance();

	if (input->IsAltHeld())
	{
		return;
	}

	if (m_isViewportHovered && input->IsMousePressed(SDL_BUTTON_LEFT))
	{
		m_mouseDownX = input->GetMousePositionX();
		m_mouseDownY = input->GetMousePositionY();
	}

	if (!m_isViewportHovered || !input->IsMouseReleased(SDL_BUTTON_LEFT))
	{
		return;
	}

	const int mouseX = input->GetMousePositionX();
	const int mouseY = input->GetMousePositionY();

	if (std::abs(mouseX - m_mouseDownX) > 4 || std::abs(mouseY - m_mouseDownY) > 4)
	{
		return;
	}

	int windowWidth = 0;
	int windowHeight = 0;
	int drawableWidth = 0;
	int drawableHeight = 0;
	m_window.GetWindowSize(windowWidth, windowHeight);
	m_window.GetDrawableSize(drawableWidth, drawableHeight);

	const float scale = static_cast<float>(drawableWidth) / static_cast<float>(std::max(windowWidth, 1));
	const float localX = mouseX * scale - m_viewport.x;
	const float localY = mouseY * scale - (drawableHeight - m_viewport.y - m_viewport.height);

	const Ray ray = m_camera.ScreenPointToRay(localX, localY, static_cast<float>(m_viewport.width), static_cast<float>(m_viewport.height));

	float closest = FLT_MAX;
	Selection picked;

	for (const auto& light : m_scene.GetLights())
	{
		const float radius = std::max(glm::length(light.position - m_camera.GetPosition()) * 0.02f, 0.1f);
		float distance = 0.0f;

		if (Math::IntersectRaySphere(ray, light.position, radius, distance) && distance < closest)
		{
			closest = distance;
			picked.type = Selection::Type::Light;
			picked.id = light.id;
		}
	}

	for (const auto& entity : m_scene.GetEntities())
	{
		if (!entity.visible || !entity.model)
		{
			continue;
		}

		float distance = 0.0f;

		if (!Math::IntersectRayBox(ray, entity.GetWorldBounds(), distance) || distance > closest)
		{
			continue;
		}

		const glm::mat4 inverse = glm::inverse(entity.transform.GetMatrix());
		const Ray localRay{ glm::vec3(inverse * glm::vec4(ray.origin, 1.0f)), glm::vec3(inverse * glm::vec4(ray.direction, 0.0f)) };

		for (const auto& subMesh : entity.model->meshes)
		{
			if (!Math::IntersectRayBox(localRay, subMesh.bounds, distance) || distance > closest)
			{
				continue;
			}

			for (size_t i = 0; i + 2 < subMesh.indices.size(); i += 3)
			{
				if (Math::IntersectRayTriangle(localRay, subMesh.positions[subMesh.indices[i]],
					subMesh.positions[subMesh.indices[i + 1]], subMesh.positions[subMesh.indices[i + 2]], distance) && distance < closest)
				{
					closest = distance;
					picked.type = Selection::Type::Entity;
					picked.id = entity.id;
				}
			}
		}
	}

	m_selection = picked;
}

void Application::HandleDroppedFiles()
{
	for (const auto& file : Input::Instance()->GetDroppedFiles())
	{
		const std::string extension = Platform::GetExtension(file);

		if (extension == ".obj")
		{
			ImportModel(file);
		}

		else if (extension == ".scene")
		{
			OpenScene(Platform::MakeRelative(file));
		}

		else
		{
			Log::Warning("Unsupported file: " + file);
		}
	}
}

void Application::UpdateTitle()
{
	std::string title = "Absolute Engine - " + (m_scenePath.empty() ? std::string("Untitled") : Platform::GetFileName(m_scenePath));

	if (m_scene.IsDirty())
	{
		title += " *";
	}

	if (title != m_title)
	{
		m_title = title;
		m_window.SetTitle(title);
	}
}

glm::vec3 Application::GetSpawnPosition() const
{
	const glm::vec3 origin = m_camera.GetPosition();
	const glm::vec3 forward = m_camera.GetForward();

	if (forward.y < -0.05f)
	{
		const float distance = -origin.y / forward.y;

		if (distance > 0.0f && distance < 30.0f)
		{
			return origin + forward * distance;
		}
	}

	glm::vec3 position = origin + forward * 5.0f;
	position.y = 0.0f;
	return position;
}
