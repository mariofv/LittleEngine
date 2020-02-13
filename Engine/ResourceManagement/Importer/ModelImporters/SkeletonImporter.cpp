#include "SkeletonImporter.h"
#include <assimp/scene.h>

bool SkeletonImporter::ImportSkeleton(const aiScene* scene, const aiMesh* mesh, const aiMatrix4x4& mesh_transformation, const std::string& output_file) const
{

	aiString boneName = mesh->mBones[0]->mName;
	aiNode * bone = scene->mRootNode->FindNode(boneName);
	Skeleton skeleton("", "");
	ImportBone(mesh, bone);


	return true;
}

void SkeletonImporter::ImportBone(const aiMesh* mesh, const aiNode * previus_node,  Skeleton::Joint * previous_joint, const aiMatrix4x4& parent_transformation, const Skeleton & skeleton) const
{
	aiMatrix4x4& current_transformation = parent_transformation * previus_node->mTransformation;
	for (size_t i = 0; i < previus_node->mNumChildren; i++)
	{
		aiString bone_name = previus_node->mChildren[i]->mName;
		aiBone * node_bone = GetNodeBone(mesh, bone_name);
		if (node_bone != nullptr) {
		
			Skeleton::Joint bone{ GetTranform(current_transformation), previous_joint};
		}

	}
}
aiBone* SkeletonImporter::GetNodeBone(const aiMesh* mesh,  const aiString & boneName) const
{
	for (size_t i = 0; i < mesh->mNumBones; i++)
	{
		if (mesh->mBones[i]->mName == boneName)
		{
			return mesh->mBones[i];
		}
	}
	return nullptr;
}


ComponentTransform SkeletonImporter::GetTranform(const aiMatrix4x4 & current_transform) const
{
	aiVector3t<float> pScaling, pPosition;
	aiQuaterniont<float> pRotation;
	aiMatrix4x4 node_transformation = current_transform;
	node_transformation.Decompose(pScaling, pRotation, pPosition);
	ComponentTransform transform(nullptr);
	math::float3 scale(pScaling.x, pScaling.y,pScaling.z);
	math::Quat rotation(pRotation.x, pRotation.y, pRotation.z, pRotation.w);
	math::float3 translation(pPosition.x, pPosition.y, pPosition.z);
	transform.SetScale(scale);
	transform.Rotate(rotation);
	transform.SetTranslation(translation);
	return transform;
}