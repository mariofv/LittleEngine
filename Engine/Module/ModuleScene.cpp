#include "ModuleScene.h"
#include "Application.h"
#include "ModuleCamera.h"
#include "ModuleEditor.h"
#include "ModuleModelLoader.h"
#include "ModuleRender.h"
#include "Component/ComponentCamera.h"
#include "Config.h"

#include <imgui.h>
#include <FontAwesome5/IconsFontAwesome5.h>
#include <algorithm>
#include <stack>
#include "Brofiler/Brofiler.h"

bool ModuleScene::Init()
{
	root = new GameObject(0);
	App->editor->OpenScene(DEFAULT_SCENE_PATH);
	App->renderer->GenerateQuadTree(); // TODO: Move this to load scene and save scene
	return true;
}

update_status ModuleScene::Update()
{
	BROFILER_CATEGORY("Scene Update", Profiler::Color::Crimson);
	root->Update();
	return update_status::UPDATE_CONTINUE;
}


bool ModuleScene::CleanUp()
{
	DeleteCurrentScene();
	return true;
}

GameObject* ModuleScene::CreateGameObject()
{
	std::string created_game_object_name = hierarchy.GetNextGameObjectName();
	std::unique_ptr<GameObject> created_game_object = std::make_unique<GameObject>(created_game_object_name);
	created_game_object->SetParent(root);

	GameObject * created_game_object_ptr = created_game_object.get();
	game_objects_ownership.emplace_back(std::move(created_game_object));
	return created_game_object_ptr;
}

GameObject* ModuleScene::CreateChildGameObject(GameObject *parent)
{
	GameObject * created_game_object_ptr = CreateGameObject();
	parent->AddChild(created_game_object_ptr);

	return created_game_object_ptr;
}

void ModuleScene::RemoveGameObject(GameObject * game_object_to_remove)
{
	auto it = std::find_if(game_objects_ownership.begin(), game_objects_ownership.end(), [game_object_to_remove](auto const & game_object) 
	{
		return game_object_to_remove == game_object.get();
	});
	if (it != game_objects_ownership.end())
	{
		std::vector<GameObject*> children_to_remove;
		game_object_to_remove->Delete(children_to_remove);
		game_objects_ownership.erase(std::remove_if(begin(game_objects_ownership), end(game_objects_ownership), [children_to_remove](auto const &  game_object)
		{
			return std::find(begin(children_to_remove), end(children_to_remove), game_object.get()) != end(children_to_remove);
		}
		), end(game_objects_ownership));
	}
}

GameObject* ModuleScene::GetRoot() const
{
	return root;
}

GameObject* ModuleScene::GetGameObject(uint64_t UUID) const
{
	if (UUID == 0)
	{
		return root;
	}

	for (auto& game_object : game_objects_ownership)
	{
		if (game_object->UUID == UUID) 
		{
			return game_object.get();
		}
	}

	return nullptr;
}

void ModuleScene::DeleteCurrentScene()
{
	for (auto& root_child : root->children)
	{
		RemoveGameObject(root_child);
	}
	delete root;
	game_objects_ownership.clear();
	hierarchy.selected_game_object = nullptr;
}

void ModuleScene::Save(Config& serialized_scene) const
{
	std::vector<Config> game_objects_config(game_objects_ownership.size());
	std::stack<GameObject*> pending_objects;
	unsigned int current_index = 0;

	for (auto& child_game_object : root->children)
	{
		pending_objects.push(child_game_object);
	}	
	
	while (!pending_objects.empty())
	{
		GameObject* current_game_object = pending_objects.top();
		pending_objects.pop();

		current_game_object->Save(game_objects_config[current_index]);
		++current_index;

		for (auto& child_game_object : current_game_object->children)
		{
			pending_objects.push(child_game_object);
		}
	}
	assert(current_index == game_objects_ownership.size());

	serialized_scene.AddChildrenConfig(game_objects_config, "GameObjects");
}

void ModuleScene::Load(const Config& serialized_scene)
{
	DeleteCurrentScene();
	root = new GameObject(0);

	std::vector<Config> game_objects_config;
	serialized_scene.GetChildrenConfig("GameObjects", game_objects_config);
	for (unsigned int i = 0; i < game_objects_config.size(); ++i)
	{
		GameObject* created_game_object = CreateGameObject();
		created_game_object->Load(game_objects_config[i]);
	}
	App->renderer->GenerateQuadTree();
}

void ModuleScene::MousePicking(const float2& mouse_position)
{
	if (gizmo_hovered)
	{
		return;
	}

	float2 window_center_pos = imgui_window_content_pos + float2(imgui_window_content_width, imgui_window_content_height) / 2;

	float2 window_mouse_position = mouse_position - window_center_pos;
	float2 window_mouse_position_normalized = float2(window_mouse_position.x * 2 / imgui_window_content_width, - window_mouse_position.y * 2 / imgui_window_content_height);

	LineSegment ray;
	App->cameras->scene_camera->GetRay(window_mouse_position_normalized, ray);
	GameObject* intersected = App->renderer->GetRaycastIntertectedObject(ray);
	hierarchy.selected_game_object = intersected;
}

void ModuleScene::ShowFrameBufferTab(ComponentCamera & camera_frame_buffer_to_show, const char * title)
{
	if (ImGui::BeginTabItem(title))
	{
		scene_window_is_hovered = ImGui::IsWindowHovered(); // TODO: This should be something like ImGui::IsTabHovered (such function doesn't exist though)

		ImVec2 imgui_window_pos_ImVec2 = ImGui::GetWindowPos();
		float2 imgui_window_pos = float2(imgui_window_pos_ImVec2.x, imgui_window_pos_ImVec2.y);

		ImVec2 max_point_content_area_ImVec2 = ImGui::GetWindowContentRegionMax();
		max_point_content_area_ImVec2 = ImVec2(
			max_point_content_area_ImVec2.x + imgui_window_pos_ImVec2.x,
			max_point_content_area_ImVec2.y + imgui_window_pos_ImVec2.y
		); // Pass from window space to screen space
		float2 max_point_content_area = float2(max_point_content_area_ImVec2.x, max_point_content_area_ImVec2.y);

		ImVec2 window_content_pos_ImVec2 = ImGui::GetCursorScreenPos();
		imgui_window_content_pos = float2(window_content_pos_ImVec2.x, window_content_pos_ImVec2.y);

		imgui_window_content_width = max_point_content_area.x - imgui_window_content_pos.x;
		imgui_window_content_height = max_point_content_area.y - imgui_window_content_pos.y;

		camera_frame_buffer_to_show.RecordFrame(imgui_window_content_width, imgui_window_content_height);

		ImGui::GetWindowDrawList()->AddImage(
			(void *)camera_frame_buffer_to_show.GetLastRecordedFrame(),
			window_content_pos_ImVec2,
			max_point_content_area_ImVec2,
			ImVec2(0, 1),
			ImVec2(1, 0)
		);
		
		ImGuizmo::SetRect(imgui_window_content_pos.x, imgui_window_content_pos.y, imgui_window_content_height, imgui_window_content_height);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::Enable(true);

		DrawEditorCameraGizmo();
		
		if (hierarchy.selected_game_object != nullptr)
		{
			DrawGizmo(camera_frame_buffer_to_show, *hierarchy.selected_game_object);
		}
		
		if (App->cameras->IsMovementEnabled() && scene_window_is_hovered) // CHANGES CURSOR IF SCENE CAMERA MOVEMENT IS ENABLED
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
		}
		ImGui::EndTabItem();
	}
}

void ModuleScene::DrawEditorCameraGizmo()
{
	float4x4 old_view_matrix = App->cameras->scene_camera->GetViewMatrix().Transposed();
	float4x4 transposed_view_matrix = App->cameras->scene_camera->GetViewMatrix().Transposed();
	bool modified;
	ImGuizmo::ViewManipulate(
		transposed_view_matrix.ptr(), 
		1, 
		ImVec2(imgui_window_content_pos.x + imgui_window_content_width - 100, imgui_window_content_pos.y),
		ImVec2(100, 100), 
		ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 0.f)),
		modified
	);

	if (modified)
	{
		App->cameras->scene_camera->SetViewMatrix(transposed_view_matrix.Transposed());
	}
	
}

void ModuleScene::DrawGizmo(const ComponentCamera& camera, GameObject& game_object)
{
	float4x4 model_global_matrix_transposed = game_object.transform.GetGlobalModelMatrix().Transposed();

	ImGuizmo::Manipulate(
		camera.GetViewMatrix().Transposed().ptr(),
		camera.GetProjectionMatrix().Transposed().ptr(),
		gizmo_operation,
		ImGuizmo::WORLD,
		model_global_matrix_transposed.ptr()
	);

	gizmo_hovered = ImGuizmo::IsOver();
	if (ImGuizmo::IsUsing())
	{
		game_object.transform.SetGlobalModelMatrix(model_global_matrix_transposed.Transposed());
	}
}

void ModuleScene::ShowGizmoControls()
{
	if (ImGui::Button(ICON_FA_ARROWS_ALT))
	{
		gizmo_operation = ImGuizmo::TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_SYNC_ALT))
	{
		gizmo_operation = ImGuizmo::ROTATE;
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_EXPAND_ARROWS_ALT))
	{
		gizmo_operation = ImGuizmo::SCALE;
	}
}