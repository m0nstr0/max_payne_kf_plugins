#pragma once

#include <numeric>

#include "MaxPayneKFExporter.h"
#include "MPKFMesh.h"

#define MPKFEXPORTER_ERROR_MP2_SKIN_BONES_NUM				_T("Max Payne 2: Object \"{}\" has more that 4 bones per vertex")
#define MPKFEXPORTER_ERROR_MP1_SKIN_BONES_NUM				_T("Max Payne 1: Object \"{}\" has more that 3 bones per vertex")
#define MPKFEXPORTER_ERROR_SKIN_FREE_VERTEX					_T("There are some vertices have zero weight sum \"{}\"")
#define MPKFEXPORTER_ERROR_SKIN_NULL_BONE					_T("Bone with index \"{}\" not found")
#define MPKFEXPORTER_ERROR_SKIN_NULL_BONES					_T("There are no bones")
#define MPKFEXPORTER_ERROR_SKIN_MORE_THAN_ONE_ROOT			_T("There are more than one root bone")
#define MPKFEXPORTER_ERROR_SKIN_NOT_FOUND					_T("Skin or Physique modifier not found")
#define MPKFEXPORTER_ERROR_SKIN_WRONG_VERTEX_TYPE			_T("Unknown vertex type. Only RIGID and RIGI_BLENDED are supported")

class MPKFMesh;

struct MPKFWeight
{
    std::vector<float> Weights;
    std::vector<size_t> Bones;

    MPKFWeight() = default;

    MPKFWeight(const MPKFWeight& Other) = default;

    ~MPKFWeight() = default;
    
    MPKFWeight(MPKFWeight&& Other) noexcept
    {
        Weights = std::move(Other.Weights);
        Bones = std::move(Other.Bones);
    }
    
    void AddWeight(size_t BoneIdx, float Weight)
    {
        if (const auto Result = std::ranges::find(Bones, BoneIdx); Result == std::end(Bones)) {
            Bones.emplace_back(BoneIdx);
            Weights.emplace_back(Weight);
        } else {
            Weights[std::distance(std::begin(Bones), Result)] += Weight;
        }
    }

    MPKFWeight& Normalize()
    {
        const float Sum = std::accumulate(std::begin(Weights), std::end(Weights), 0.f);
        
        for (float& Weight: Weights) {
            Weight /= Sum;
        }

        return *this;
    }
};

struct MPKFSkin
{
    std::vector<IGameNode*> Bones;
    std::vector<MPKFWeight> Weights;

    INode* GetBoneParentNode(INode* Node, size_t& BoneIdx) const
    {
        INode* BoneNode{Node};

        bool Found{false};
        while (BoneNode != nullptr && !Found) {
            for (size_t BoneNameIdx{0}; BoneNameIdx < Bones.size(); ++BoneNameIdx) {
                if (_tcscmp(Bones[BoneNameIdx]->GetName(), BoneNode->GetName()) == 0) {
                    Found = true;
                    BoneIdx = BoneNameIdx;
                    break;
                }
            }

            if (!Found) {
                BoneNode = BoneNode->GetParentNode();
            }
        }

        return BoneNode;
    }
};

bool ExporterCreateKFSkin(IGameSkin* GameSkin, MPKFMesh* Mesh, MPKFSkin** OutSkin);

bool ExporterExportKFSkins(const MPKFScene* Scene, const MPKFExporterOptions& ExportOptions, MPMemoryWriter& MemoryWriter);