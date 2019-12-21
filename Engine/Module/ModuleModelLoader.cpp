#include "ModuleModelLoader.h"
#include "Globals.h"
#include "Application.h"
#include "ModuleCamera.h"
#include "ModuleTexture.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "Component/ComponentCamera.h"
#include "Component/ComponentMaterial.h"
#include "Component/ComponentMesh.h"
#include "Importer/MeshImporter.h"
#include "Importer/MaterialImporter.h"


#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/mesh.h>

// Called before render is available
bool ModuleModelLoader::Init()
{
	APP_LOG_SECTION("************ Module ModelLoader Init ************");
	LoadModel(HOUSE_MODEL_PATH);
	return true;
}
GameObject* ModuleModelLoader::LoadModel(const char *new_model_file_path)
{
	std::string model_output;

	App->mesh_importer->Import(new_model_file_path, model_output);

	ModuleFileSystem::File file = ModuleFileSystem::File(new_model_file_path);
	GameObject *model_root_node = App->scene->CreateGameObject();
	model_root_node->name = std::string("RootNode");

	model_output = model_output.substr(0, model_output.size());
	std::vector<std::shared_ptr<ModuleFileSystem::File>> files_in_output_path;
	App->filesystem->GetAllFilesInPath(model_output, files_in_output_path);
	for (auto file : files_in_output_path )
	{
		LoadNode(model_root_node, file);
	}

	return model_root_node;
}

void ModuleModelLoader::LoadNode(GameObject *parent_node, const std::shared_ptr<ModuleFileSystem::File> model_base_path)
{
	GameObject *node_game_object = App->scene->CreateChildGameObject(parent_node);

	std::shared_ptr<Mesh> mesh_for_component = App->mesh_importer->Load(model_base_path->file_path.c_str());

	ComponentMesh *mesh_component = (ComponentMesh*)node_game_object->CreateComponent(Component::ComponentType::MESH);
	mesh_component->SetMesh(mesh_for_component);
	node_game_object->name = model_base_path->filename_no_extension;
	node_game_object->Update();

	if (mesh_for_component->meshes_textures_path.size() > 0)
	{
		ComponentMaterial *componentMaterial = (ComponentMaterial*)node_game_object->CreateComponent(Component::ComponentType::MATERIAL);
		for (auto texture_path : mesh_for_component->meshes_textures_path)
		{
			std::shared_ptr<Texture> material_texture = App->material_importer->Load(texture_path.c_str());
			componentMaterial->texture = material_texture;
		}
	}
}


