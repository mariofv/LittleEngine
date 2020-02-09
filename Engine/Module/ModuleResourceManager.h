#ifndef _MODULERESOURCEMANAGER_H_
#define _MODULERESOURCEMANAGER_H_

#include "Module.h"

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

class Texture;
class File;
class Mesh;
class Resource;
class Importer;
class Timer;
#include "ResourceManagement/Importer/TextureImporter.h"
#include "ResourceManagement/Importer/ModelImporter.h"
class ModuleResourceManager : public Module
{
public:

	ModuleResourceManager() = default;

	bool Init() override;

	update_status PreUpdate() override;
	bool CleanUp() override;

	std::pair<bool, std::string> Import(const File& file);
	void RemoveResourceFromCacheIfNeeded(const std::shared_ptr<Resource> & resource);
	std::shared_ptr<Texture> LoadTexture(const std::string& file_path) const;
	std::shared_ptr<Mesh> LoadModel(const std::string& file_path) const;
	/*template<class T>
	std::shared_ptr<T> Load(const std::string& uid)
	{
		return Loader::LoadResource(uid);
	}*/

private:
	std::pair<bool, std::string> InternalImport(const File& file);
	void ImportAllFileHierarchy(const File& file);
	std::shared_ptr<Resource> RetrieveFromCacheIfExist(const std::string& uid) const;
	void StartThread();

public:
	struct ThreadComunication
	{
		std::mutex thread_mutex;
		std::atomic_bool stop_thread = false;
		std::atomic_bool finished_loading = false;
		std::atomic_uint thread_importing_hash = 0;
		std::atomic_uint main_importing_hash = 0;
		std::atomic_uint loaded_items = 0;
		std::atomic_uint total_items = 0;
	} thread_comunication;

	std::unique_ptr<TextureImporter> texture_importer = nullptr;
private:
	const size_t importer_interval_millis = 30000;
	float last_imported_time = 0;
	std::thread importing_thread;
	std::unique_ptr<Timer> thread_timer = std::make_unique<Timer>();
	//Importers

	std::unique_ptr<Importer> default_importer = std::make_unique<Importer>();
	std::unique_ptr<ModelImporter> model_importer = nullptr;
	mutable std::vector<std::shared_ptr<Resource>> resource_cache;
};

#endif // _MODULERESOURCEMANAGER_H_