#include "ModuleEditor.h"
#include "Globals.h"
#include "Application.h"
#include "ModuleCamera.h"
#include "ModuleDebug.h"
#include "ModuleDebugDraw.h"
#include "ModuleFileSystem.h"
#include "ModuleProgram.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "Component/ComponentMesh.h"

#include "Config.h"
#include "Hierarchy.h"
#include "OLQuadTree.h"
#include "UI/DebugDraw.h"

#include <FontAwesome5/IconsFontAwesome5.h>

// Called before render is available
bool ModuleEditor::Init()
{
	APP_LOG_SECTION("************ Module Editor Init ************");
	
	
	APP_LOG_SUCCESS("IMGUI editor initialized correctly.");

	return true;
}

// Called before quitting
bool ModuleEditor::CleanUp()
{
	remove(TMP_SCENE_PATH);

	return true;
}

void ModuleEditor::OpenScene(const std::string &path) const
{
	size_t readed_bytes;
	char* scene_file_data = App->filesystem->Load(path.c_str(), readed_bytes);
	std::string serialized_scene_string = scene_file_data;
	free(scene_file_data);

	Config scene_config(serialized_scene_string);
	App->scene->Load(scene_config);
}

void ModuleEditor::SaveScene(const std::string &path) const
{
	Config scene_config;
	App->scene->Save(scene_config);
	std::string serialized_scene_string;
	scene_config.GetSerializedString(serialized_scene_string);

	App->filesystem->Save(path.c_str(), serialized_scene_string.c_str(), serialized_scene_string.size() + 1);
}

void ModuleEditor::RenderDebugDraws()
{
	if (App->debug->show_grid)
	{
		dd::xzSquareGrid(-100.0f, 100.0f, 0.0f, 1.0f, math::float3(0.65f, 0.65f, 0.65f));
	}

	if (App->debug->show_quadtree)
	{
		for (auto& ol_quadtree_node : App->renderer->ol_quadtree.flattened_tree)
		{
			float3 quadtree_node_min = float3(ol_quadtree_node->box.minPoint.x, 0, ol_quadtree_node->box.minPoint.y);
			float3 quadtree_node_max = float3(ol_quadtree_node->box.maxPoint.x, 0, ol_quadtree_node->box.maxPoint.y);
			dd::aabb(quadtree_node_min, quadtree_node_max, float3::one);
		}
	}

	if (App->scene->hierarchy.selected_game_object != nullptr)
	{
		RenderCameraFrustum();
		RenderOutline(); // This function tries to render again the selected game object. It will fail because depth buffer
	}

	if (App->debug->show_bounding_boxes)
	{
		RenderBoundingBoxes();
	}

	if (App->debug->show_global_bounding_boxes)
	{
		RenderGlobalBoundingBoxes();
	}

	App->debug_draw->Render(*App->cameras->scene_camera);
}

void ModuleEditor::RenderCameraFrustum() const
{
	if (!App->debug->show_camera_frustum)
	{
		return;
	}

	Component * selected_camera_component = App->scene->hierarchy.selected_game_object->GetComponent(Component::ComponentType::CAMERA);
	if (selected_camera_component != nullptr) {
		ComponentCamera* selected_camera = static_cast<ComponentCamera*>(selected_camera_component);
		
		dd::frustum(selected_camera->GetInverseClipMatrix(), float3::one);
	}
}

void ModuleEditor::RenderOutline() const
{
	GameObject* selected_game_object = App->scene->hierarchy.selected_game_object;
	Component* selected_object_mesh_component = selected_game_object->GetComponent(Component::ComponentType::MESH);

	if (selected_object_mesh_component != nullptr)
	{
		ComponentMesh* selected_object_mesh = static_cast<ComponentMesh*>(selected_object_mesh_component);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		glStencilMask(0xFF);

		selected_object_mesh->Render();

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		GLuint outline_shader_program = App->program->outline_program;
		glUseProgram(outline_shader_program);

		ComponentTransform object_transform_copy = selected_game_object->transform;
		float3 object_scale = object_transform_copy.GetScale();
		object_transform_copy.SetScale(object_scale*1.01f);
		object_transform_copy.GenerateGlobalModelMatrix();

		glBindBuffer(GL_UNIFORM_BUFFER, App->program->uniform_buffer.ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, App->program->uniform_buffer.MATRICES_UNIFORMS_OFFSET, sizeof(float4x4), object_transform_copy.GetGlobalModelMatrix().Transposed().ptr());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		selected_object_mesh->RenderModel();

		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);

		glUseProgram(0);

	}
}

void ModuleEditor::RenderBoundingBoxes() const
{
	for (auto& mesh : App->renderer->meshes_to_render)
	{
		GameObject* mesh_game_object = mesh->owner;
		if (!mesh_game_object->aabb.IsEmpty())
		{
			dd::aabb(mesh_game_object->aabb.bounding_box.minPoint, mesh_game_object->aabb.bounding_box.maxPoint, float3::one);
		}
	}
}

void ModuleEditor::RenderGlobalBoundingBoxes() const
{
	for (auto& object : App->scene->game_objects_ownership)
	{
		dd::aabb(object->aabb.global_bounding_box.minPoint, object->aabb.global_bounding_box.maxPoint, float3::one);
	}
}

void ModuleEditor::RenderEditorDraws()
{
	ImGuizmo::SetRect(scene_window_content_area_pos.x, scene_window_content_area_pos.y, scene_window_content_area_width, scene_window_content_area_height);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::Enable(true);

	if (App->scene->hierarchy.selected_game_object != nullptr)
	{
		RenderGizmo();
		RenderCameraPreview();
	}

	RenderEditorCameraGizmo();
}

void ModuleEditor::RenderGizmo()
{
	float4x4 model_global_matrix_transposed = App->scene->hierarchy.selected_game_object->transform.GetGlobalModelMatrix().Transposed();

	ImGuizmo::Manipulate(
		App->cameras->scene_camera->GetViewMatrix().Transposed().ptr(),
		App->cameras->scene_camera->GetProjectionMatrix().Transposed().ptr(),
		gizmo_operation,
		ImGuizmo::WORLD,
		model_global_matrix_transposed.ptr()
	);

	gizmo_hovered = ImGuizmo::IsOver();
	if (ImGuizmo::IsUsing())
	{
		App->scene->hierarchy.selected_game_object->transform.SetGlobalModelMatrix(model_global_matrix_transposed.Transposed());
	}
}

void ModuleEditor::RenderEditorCameraGizmo()
{
	float4x4 old_view_matrix = App->cameras->scene_camera->GetViewMatrix().Transposed();
	float4x4 transposed_view_matrix = App->cameras->scene_camera->GetViewMatrix().Transposed();
	bool modified;
	ImGuizmo::ViewManipulate(
		transposed_view_matrix.ptr(),
		1,
		ImVec2(scene_window_content_area_pos.x + scene_window_content_area_width - 100, scene_window_content_area_pos.y),
		ImVec2(100, 100),
		ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 0.f)),
		modified
	);

	if (modified)
	{
		App->cameras->scene_camera->SetViewMatrix(transposed_view_matrix.Transposed());
	}

}

void ModuleEditor::RenderCameraPreview() const
{
	Component * selected_camera_component = App->scene->hierarchy.selected_game_object->GetComponent(Component::ComponentType::CAMERA);
	if (selected_camera_component != nullptr) {
		ComponentCamera* selected_camera = static_cast<ComponentCamera*>(selected_camera_component);

		ImGui::SetCursorPos(ImVec2(scene_window_content_area_width - 200, scene_window_content_area_height - 200));
		ImGui::BeginChildFrame(ImGui::GetID("Camera Preview"), ImVec2(200, 200), ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar())
		{
			ImGui::BeginMenu("Camera Preview", false);
			ImGui::EndMenuBar();
		}

		ImVec2 content_area_max_point = ImGui::GetWindowContentRegionMax();

		float width = content_area_max_point.x - ImGui::GetCursorPos().x;
		float height = content_area_max_point.y - ImGui::GetCursorPos().y;

		App->cameras->main_camera->RecordFrame(width, height);
		ImGui::Image(
			(void *)App->cameras->main_camera->GetLastRecordedFrame(),
			ImVec2(width, height),
			ImVec2(0, 1),
			ImVec2(1, 0)
		);

		ImGui::EndChild();
	}
}

void ModuleEditor::MousePicking(const float2& mouse_position)
{
	if (App->editor->gizmo_hovered)
	{
		return;
	}

	float2 window_center_pos = scene_window_content_area_pos + float2(scene_window_content_area_width, scene_window_content_area_height) / 2;

	float2 window_mouse_position = mouse_position - window_center_pos;
	float2 window_mouse_position_normalized = float2(window_mouse_position.x * 2 / scene_window_content_area_width, -window_mouse_position.y * 2 / scene_window_content_area_height);

	LineSegment ray;
	App->cameras->scene_camera->GetRay(window_mouse_position_normalized, ray);
	GameObject* intersected = App->renderer->GetRaycastIntertectedObject(ray);
	App->scene->hierarchy.selected_game_object = intersected;
}

void ModuleEditor::ShowSceneTab()
{
	if (ImGui::BeginTabItem(ICON_FA_TH " Scene"))
	{
		ImVec2 scene_window_pos_ImVec2 = ImGui::GetWindowPos();
		float2 scene_window_pos = float2(scene_window_pos_ImVec2.x, scene_window_pos_ImVec2.y);

		ImVec2 scene_window_content_area_max_point_ImVec2 = ImGui::GetWindowContentRegionMax();
		scene_window_content_area_max_point_ImVec2 = ImVec2(
			scene_window_content_area_max_point_ImVec2.x + scene_window_pos_ImVec2.x,
			scene_window_content_area_max_point_ImVec2.y + scene_window_pos_ImVec2.y
		); // Pass from window space to screen space
		float2 scene_window_content_area_max_point = float2(scene_window_content_area_max_point_ImVec2.x, scene_window_content_area_max_point_ImVec2.y);

		ImVec2 scene_window_content_area_pos_ImVec2 = ImGui::GetCursorScreenPos();
		scene_window_content_area_pos = float2(scene_window_content_area_pos_ImVec2.x, scene_window_content_area_pos_ImVec2.y);

		scene_window_content_area_width = scene_window_content_area_max_point.x - scene_window_content_area_pos.x;
		scene_window_content_area_height = scene_window_content_area_max_point.y - scene_window_content_area_pos.y;

		App->cameras->scene_camera->RecordFrame(scene_window_content_area_width, scene_window_content_area_height);
		App->cameras->scene_camera->RecordDebugDraws(scene_window_content_area_width, scene_window_content_area_height);
		
		ImGui::GetWindowDrawList()->AddImage(
			(void *)App->cameras->scene_camera->GetLastRecordedFrame(),
			scene_window_content_area_pos_ImVec2,
			scene_window_content_area_max_point_ImVec2,
			ImVec2(0, 1),
			ImVec2(1, 0)
		);

		AABB2D content_area = AABB2D(scene_window_content_area_pos, scene_window_content_area_max_point);
		float2 mouse_pos_f2 = float2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
		AABB2D mouse_pos = AABB2D(mouse_pos_f2, mouse_pos_f2);
		scene_window_is_hovered = content_area.Contains(mouse_pos); // TODO: This seems to be inneficient, check with partner

		App->editor->RenderEditorDraws(); // This should be render after rendering framebuffer texture.

		if (App->cameras->IsMovementEnabled() && scene_window_is_hovered) // CHANGES CURSOR IF SCENE CAMERA MOVEMENT IS ENABLED
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
		}
		ImGui::EndTabItem();
	}
}

void ModuleEditor::ShowGameTab()
{
	if (ImGui::BeginTabItem(ICON_FA_GHOST " Game"))
	{
		ImVec2 game_window_pos_ImVec2 = ImGui::GetWindowPos();
		float2 game_window_pos = float2(game_window_pos_ImVec2.x, game_window_pos_ImVec2.y);

		ImVec2 game_window_content_area_max_point_ImVec2 = ImGui::GetWindowContentRegionMax();
		game_window_content_area_max_point_ImVec2 = ImVec2(
			game_window_content_area_max_point_ImVec2.x + game_window_pos_ImVec2.x,
			game_window_content_area_max_point_ImVec2.y + game_window_pos_ImVec2.y
		); // Pass from window space to screen space
		float2 game_window_content_area_max_point = float2(game_window_content_area_max_point_ImVec2.x, game_window_content_area_max_point_ImVec2.y);

		ImVec2 game_window_content_area_pos_ImVec2 = ImGui::GetCursorScreenPos();
		float2 game_window_content_area_pos = float2(game_window_content_area_pos_ImVec2.x, game_window_content_area_pos_ImVec2.y);

		float game_window_content_area_width = game_window_content_area_max_point.x - game_window_content_area_pos.x;
		float game_window_content_area_height = game_window_content_area_max_point.y - game_window_content_area_pos.y;

		App->cameras->main_camera->RecordFrame(game_window_content_area_width, game_window_content_area_height);

		ImGui::GetWindowDrawList()->AddImage(
			(void *)App->cameras->main_camera->GetLastRecordedFrame(),
			game_window_content_area_pos_ImVec2,
			game_window_content_area_max_point_ImVec2,
			ImVec2(0, 1),
			ImVec2(1, 0)
		);
		ImGui::EndTabItem();
	}
}

void ModuleEditor::ShowGizmoControls()
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