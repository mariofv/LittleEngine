#pragma once


#include "ResourceManagement/Resources/Skeleton.h"
#include "ResourceManagement/Importer/Importer.h"

#include <assimp/mesh.h>
#include <string>

struct aiScene;
struct aiNode;

class SkeletonImporter : public Importer
{
public:
	SkeletonImporter() = default;
	~SkeletonImporter() = default;

	ImportResult ImportSkeleton(const aiScene* scene, const aiMesh* mesh, const std::string& imported_file, float unit_scale_factor) const;
	static math::float4x4 GetTransform(const aiMatrix4x4& current_transform);
private:
	void ImportChildBone(const aiMesh* mesh, const aiNode * previus_node, uint32_t previous_joint_index, const aiMatrix4x4& parent_transformation, aiMatrix4x4& accumulated_local_transformation,Skeleton & skeleton) const;
	aiBone * GetNodeBone(const aiMesh* mesh, const std::string & bone_name) const;
	bool SaveBinary(const Skeleton & skeleton, const std::string& exported_file, const std::string& imported_file) const;
};

