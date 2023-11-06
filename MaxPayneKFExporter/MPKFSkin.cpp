#include "MPKFSkin.h"
#include "MPKFMesh.h"

IGameNode* GetRootBone(IGameSkin* GameSkin)
{
	assert(GameSkin);
	
	IGameNode* RootBone{nullptr};
	for (int BoneId = 0; BoneId < GameSkin->GetTotalBoneCount(); ++BoneId) {
		IGameNode* Bone = GameSkin->GetIGameBone(BoneId, true);
		if (Bone == nullptr) {
			SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONE, BoneId)
			return nullptr;
		}
		while (Bone != nullptr) {
			if (Bone->GetNodeParent() == nullptr) {
				if (RootBone != nullptr && RootBone != Bone) {
					SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_MORE_THAN_ONE_ROOT)
					return nullptr;
				}
				RootBone = Bone;
				break;
			}
			Bone = Bone->GetNodeParent();
		}
	}

	return RootBone;
}

bool BuildBonesTree(IGameNode* RootBone, MPKFSkin* Skin)
{
	INode* MaxNode = RootBone->GetMaxNode();
	if (MaxNode->UserPropExists(_T("MP_BONE_HELPER"))) {
		return true;
	}

	IS_ASCII_STRING(MaxNode->GetName())
	
	Skin->Bones.push_back(RootBone);

	for (int NodeId{0}; NodeId < RootBone->GetChildCount(); ++NodeId) {
		if (!BuildBonesTree(RootBone->GetNodeChild(NodeId), Skin)) {
			return false;
		}
	}

	return true;
}

bool ExporterCreateKFSkin(IGameSkin* GameSkin, MPKFMesh* Mesh, MPKFSkin** OutSkin)
{
	assert(GameSkin && Mesh);
	
	if (GameSkin->GetTotalSkinBoneCount() == 0) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONES)
		return false;
	}

	if (static_cast<size_t>(GameSkin->GetNumOfSkinnedVerts()) != Mesh->OriginalMeshNumVertices) {  // NOLINT(clang-diagnostic-sign-compare)
		SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_FREE_VERTEX, Mesh->Node->GetName())
		return false;
	}

	IGameNode* RootBone{GetRootBone(GameSkin)};
	if (!RootBone) {
		return false;
	}

	std::unique_ptr<MPKFSkin> Skin{std::make_unique<MPKFSkin>()};
	if (!BuildBonesTree(RootBone, Skin.get())) {
		return false;
	}
	
	for (const auto& SubMesh : Mesh->SubMeshes) {
		for (const auto& [LocalIndex, OriginalIndex] : SubMesh->LocalToOrigVertexId)
		{
			MPKFWeight Weight;
			switch (GameSkin->GetVertexType(OriginalIndex)) {  // NOLINT(clang-diagnostic-switch-enum)
			case IGameSkin::IGAME_RIGID: {

				size_t BoneIdx{ 0 };
				if (const INode* Node = Skin->GetBoneParentNode(GameSkin->GetBone(OriginalIndex, 0), BoneIdx); Node == nullptr) {
					SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONE, OriginalIndex)
						return false;
				}

				Weight.AddWeight(BoneIdx, 1.f);
				break;
			}
			case IGameSkin::IGAME_RIGID_BLENDED: {
				const int NumberOfBones{ GameSkin->GetNumberOfBones(OriginalIndex) };

				for (int BoneId{ 0 }; BoneId < NumberOfBones; ++BoneId) {
					size_t BoneIdx{ 0 };
					if (const INode* Node = Skin->GetBoneParentNode(GameSkin->GetBone(OriginalIndex, BoneId), BoneIdx); Node == nullptr) {
						SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONE, OriginalIndex);
						return false;
					}

					Weight.AddWeight(BoneIdx, GameSkin->GetWeight(OriginalIndex, BoneId));
				}
				break;
			}
			default:
				SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_WRONG_VERTEX_TYPE);
				return false;
			}

			Skin->Weights.push_back(std::move(Weight.Normalize()));
		}
	}

	*OutSkin = Skin.release();
	
	return true;
}

bool ExporterExportKFSkin(const MPKFMesh* Mesh, const MPKFExporterOptions& ExportOptions, MPMemoryWriter& MemoryWriter)
{
	assert(Mesh);

	if (Mesh->Skin == nullptr) {
		return true;
	}

	std::unique_ptr<MPMemoryChunkWriter> ChunkWriter{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_SKIN_CHUNK_ID, 1) };

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(1);
	WRITE_ASCII_STRING(SkinObjectName, Mesh->Node->GetName(), ChunkWriter);

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(Mesh->Skin->Bones.size());
	for (IGameNode* Bone : Mesh->Skin->Bones) {
		WRITE_ASCII_STRING(SkinBoneName, Bone->GetName(), ChunkWriter);
	}

	ChunkWriter->WriteTag(0x1C);
	int32_t NumVertexOffsets{ 0 };

	for (const auto& Weight : Mesh->Skin->Weights) {
		NumVertexOffsets += static_cast<int32_t>(Weight.Bones.size());
	}

	uint32_t NumVertices{ 0 };
	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		NumVertices += SubMesh->Vertices.size();
	}
	
	*ChunkWriter << static_cast<int32_t>(Mesh->Skin->Weights.size());

	int32_t VertexOffset{ 0 };
	for (const auto& Weight : Mesh->Skin->Weights) {
		*ChunkWriter << static_cast<int32_t>(VertexOffset);
		VertexOffset += static_cast<int32_t>(Weight.Bones.size());
	}

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(Mesh->Skin->Weights.size());
	for (const auto& Weight : Mesh->Skin->Weights) {
		*ChunkWriter << static_cast<int32_t>(Weight.Bones.size());
	}

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(NumVertexOffsets);
	for (const auto& Weight : Mesh->Skin->Weights) {
		for (const auto& BoneId : Weight.Bones) {
			*ChunkWriter << static_cast<int32_t>(BoneId);
		}
	}

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(NumVertexOffsets);
	for (const auto& Weight : Mesh->Skin->Weights) {
		for (const auto& Weight : Weight.Weights) {
			*ChunkWriter << Weight;
		}
	}
	
	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(Mesh->SubMeshes.size());
	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		*ChunkWriter << static_cast<uint32_t>(SubMesh->Vertices.size());
	}

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(Mesh->SubMeshes.size());
	uint32_t StartVertexIndex = 0;
	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		*ChunkWriter << StartVertexIndex;
		StartVertexIndex += static_cast<uint32_t>(SubMesh->Vertices.size());
	}

	MemoryWriter << ChunkWriter.get();
	ChunkWriter.reset();

	return true;
}

bool ExporterExportKFSkins(const MPKFScene* Scene, const MPKFExporterOptions& ExportOptions, MPMemoryWriter& MemoryWriter)
{
	assert(Scene);

	for (const std::unique_ptr<MPKFSceneNode>& SceneNode : Scene->Nodes)
	{
		if (SceneNode->Type != MPKFSCENE_TYPE::kMesh) {
			continue;
		}

		if (const auto Mesh = dynamic_cast<MPKFMesh*>(SceneNode.get()); !ExporterExportKFSkin(Mesh, ExportOptions, MemoryWriter)) {
			return false;
		}
	}

	return true;
}
