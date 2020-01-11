#ifndef _ModuleFileSystem_H
#define _ModuleFileSystem_H

#include <Module/Module.h>
#include <string>
#include <vector>
#include <memory>

#include <physfs/physfs.h>
#include "Filesystem/File.h"
class ModuleFileSystem : public Module
{

public:

	bool Init() override;
	bool CleanUp() override;
	ModuleFileSystem() = default;
	~ModuleFileSystem();

	char* Load( const char* file_name, size_t & size) const;
	unsigned int Save(const char* file_name, const void* buffer, unsigned int size, bool append = false) const;

	bool Remove(const File & file);
	bool Exists(const char* file) const;
	std::string MakeDirectory(const std::string & new_directory_full_path);
	bool Copy(const char* source, const char* destination);
	std::shared_ptr<File> GetFileHierarchyFromPath(const std::string & path) const;
	
	FileType GetFileType(const char *file_path, const PHYSFS_FileType & file_type = PHYSFS_FileType::PHYSFS_FILETYPE_OTHER) const;
	void GetAllFilesInPath(const std::string & path, std::vector<std::shared_ptr<File>> & files, bool directories_only = false) const;

	void RefreshFilesHierarchy();
private:
	char *save_path = NULL;
	bool IsValidFileName(const char * file_name) const;
	std::string GetFileExtension(const char *file_path) const;

public:

	std::shared_ptr<File> root_file;
};


#endif // !_ModuleFileSystem_H
