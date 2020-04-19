#ifndef _SCENE_H_
#define _SCENE_H_

#include "Resource.h"
#include "Helper/Config.h"
#include "ResourceManagement/Manager/SceneManager.h"
#include <array>
#include <vector>

class Metafile;

class Scene : public Resource
{

public:
	Scene();
	Scene(uint32_t uuid, const Config& config);
	~Scene() = default;

	void Save(GameObject* gameobject_to_save) const;
	void Load();

	const std::string GetSerializedConfig() const;
	void SaveSerializedConfig(const std::string& assets_file_path);
	std::string GetName() const;
	std::string GetExportedFile() const;

 
private:
	void SavePrefab(Config& config, GameObject* gameobject_to_save) const;
	void SavePrefabUUIDS(std::vector<Config>& config, GameObject* gameobject_to_save) const;
	bool SaveModifiedPrefabComponents(Config& config, GameObject* gameobject_to_save) const;
	GameObject * LoadPrefab(const Config& config) const;
	void LoadPrefabModifiedComponents(const Config& config) const;
	bool ComputeNameScene(const std::string& assets_path);

private:
	mutable Config scene_config;
	std::string exported_file_path;
	std::string name;

};


namespace ResourceManagement
{
	template<>
	static std::shared_ptr<Scene> Load(uint32_t uuid, const FileData& resource_data)
	{
		return SceneManager::Load(uuid, resource_data);
	}
}

#endif //_SCENE_H_
