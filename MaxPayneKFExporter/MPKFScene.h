#pragma once

#include "MaxPayneKFExporter.h"
#include <memory>
#include <vector>

enum class MPKFSCENE_TYPE
{
    kMesh
};

struct MPKFSceneNode
{
    IGameNode* Node;

    MPKFSCENE_TYPE Type;

    GMatrix Transform;

    MPKFSceneNode(): Node{nullptr}, Type{MPKFSCENE_TYPE::kMesh} {}

    MPKFSceneNode(IGameNode* InNode, MPKFSCENE_TYPE InType): Node{InNode}, Type{InType} {}

    virtual ~MPKFSceneNode() = default;
};

struct MPKFScene
{
    std::vector<std::unique_ptr<MPKFSceneNode>> Nodes;

    void AddNode(MPKFSceneNode* InNode)
    {
        std::unique_ptr<MPKFSceneNode> Node{InNode};
        Nodes.push_back(std::move(Node));
    }

    bool FindNodeInScene(IGameNode* Node);

    GMatrix GetParentWorldTransform(IGameNode* Node);

    void FixTransforms(bool RetainHierarchies);
};