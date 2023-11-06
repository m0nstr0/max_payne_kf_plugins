#include "MPKFScene.h"

bool MPKFScene::FindNodeInScene(IGameNode* Node)
{
	for (const std::unique_ptr<MPKFSceneNode>& SceneNode : Nodes) {
		if (SceneNode->Node == Node) {
			return true;
		}
	}

	return false;
}

GMatrix MPKFScene::GetParentWorldTransform(IGameNode* Node)
{
	IGameNode* ParentNode = Node->GetNodeParent();
	while(ParentNode && !FindNodeInScene(ParentNode)) {
		ParentNode = Node->GetNodeParent();
	}

	return ParentNode ? ParentNode->GetWorldTM() : GMatrix();
}

void MPKFScene::FixTransforms(bool RetainHierarchies)
{
	for (std::unique_ptr<MPKFSceneNode>& SceneNode : Nodes) {
		if (!RetainHierarchies) {
			SceneNode->Transform = SceneNode->Node->GetWorldTM();
			continue;
		}

		GMatrix ParentTransform = GetParentWorldTransform(SceneNode->Node);

		SceneNode->Transform = SceneNode->Node->GetWorldTM() * ParentTransform.Inverse();
	}
}
