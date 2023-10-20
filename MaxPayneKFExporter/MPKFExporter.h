#pragma once

#include "MaxPayneKFExporter.h"

#define MPKFExporter_CLASS_ID       Class_ID(0x7f7d370d, 0x6e0c0771)

struct MPKFExporterOptions
{
	int Game{ 0 };
	bool ExportGeometry{ false };
	bool ExportSkinning{ false };
	bool ExportMaterials{ false };
	bool ExportCameras{ false };
	bool ExportLights{ false };
	bool ExportAnimations{ false };
	bool ExportHelpers{ false };
	bool ExportEnvironmets{ false };
	bool ExportSkeleton{ false };
	bool CopyTexturesToExportPath{ false };
	bool Scale{ false };
	float ScaleValue{ 0.01f };
	bool RetainHierarchies{ false };
	bool UsePivotAsMeshCenter{ false };
	bool SaveReferencesOnlyOnce{ false };
	bool RemoveFloatingVertices{ false };
	bool RemoveHiddenObjects{ false };
	bool UseGlobalAnimationRange{ false };
	bool LoopWhenFinished{ false };
	bool MaintainMatrixScaling{ false };
	bool LoopInterpolation{ false };
	bool DecEndFrameByOne{ false };
	int SampleRate{ 30 };
	float MinDeltaPosition{ 0.01f };
	float MinDeltaRotation{ 0.01f };
	int StartFrame{ 0 };
	int EndFrame{ 100 };
	int LoopToFrame{ 0 };
	int FrameToFrameRotationInterpolation{ 0 };
	TSTR SkinningPostfix{};
	TSTR SkeletonPostfix{};
	TSTR Paths{};

	void ResetToDefault()
	{
		Game = 0;
		ExportGeometry = false;
		ExportSkinning = false;
		ExportMaterials = false;
		ExportCameras = false;
		ExportLights = false;
		ExportAnimations = false;
		ExportHelpers = false;
		ExportEnvironmets = false;
		ExportSkeleton = false;
		CopyTexturesToExportPath = false;
		Scale = false;
		ScaleValue = 0.01f;
		RetainHierarchies = false;
		UsePivotAsMeshCenter = false;
		SaveReferencesOnlyOnce = false;
		RemoveFloatingVertices = false;
		RemoveHiddenObjects = false;
		UseGlobalAnimationRange = false;
		LoopWhenFinished = false;
		MaintainMatrixScaling = false;
		LoopInterpolation = false;
		DecEndFrameByOne = false;
		SampleRate = 30;
		MinDeltaPosition = 0.01f;
		MinDeltaRotation = 0.01f;
		StartFrame = 0;
		EndFrame = 100;
		LoopToFrame = 0;
		FrameToFrameRotationInterpolation = 0;
		Paths = _T("");
		SkinningPostfix = _T("");
		SkeletonPostfix = _T("");
	}
};

class MPKFExporter: public SceneExport
{
public:
    MPKFExporter();

    virtual ~MPKFExporter();

	int	ExtCount() override { return 1; }

    const MCHAR* Ext(int n) override { return _T("KF2"); }

	const MCHAR* LongDesc() override { return _T("Max Payne KF2 File"); }

	const MCHAR* ShortDesc() override { return _T("Max Payne KF2 File"); }
	
	const MCHAR* AuthorName() override { return _T("Max Payne KF2 File"); }

	const MCHAR* CopyrightMessage() override { return _T("Bolotaev Sergey Borisovich"); }

	const MCHAR* OtherMessage1() override { return _T("Max Payne KF2 File"); }

	const MCHAR* OtherMessage2() override { return _T(""); }

	unsigned int Version() override { return 100; }

	void ShowAbout(HWND hWnd) override {}

	BOOL SupportsOptions(int ext, DWORD options) { return TRUE; }

	int	DoExport(const MCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0) override;

	class IGameNode* GetParentNode(class IGameNode* Node);

	bool PrepareNodesList(class IGameNode* Node, class MPKFGlobalExporterContext* GlobalContext);

	MPKFExporterOptions* GetExportOptions() { return &ExportOptions; }

	HWND GetMaxHWND() const { return MaxHWND; }

private:
	bool DoExportSubNodes(class MPKFGlobalExporterContext* GlobalContext);

	bool DoExportNode(class IGameNode* Node, class MPKFGlobalExporterContext* GlobalContext);

	bool DoExportKFNode(class IGameNode* Node, class MPMemoryChunkWriter* ChunkWriter);

	bool DoExportKFGeometry(class MPKFMeshExporterContext* Context, class MPMemoryChunkWriter* ChunkWriter);

	bool DoExportKFPolygons(class MPKFMeshExporterContext* Context, class MPMemoryChunkWriter* ChunkWriter);

	bool DoExportKFSkinning(class MPKFMeshExporterContext* Context, class MPMemoryChunkWriter* ChunkWriter);

	bool DoExportKFPolygonMaterials(class IGameNode* Node, class MPKFMeshExporterContext* Context, class MPMemoryChunkWriter* ChunkWriter);

	bool DoExportKFUVWMapping(class MPKFMeshExporterContext* Context, class MPMemoryChunkWriter* ChunkWriter);

	bool BuildSkinBoneTree(class IGameNode* Node, class MPKFMeshExporterContext* Context);

	bool PrepareKFMeshExportSkinContext2(class IGameNode* Node, class IGameMesh* Mesh, class IGameObject* GameObject, class MPKFGlobalExporterContext* GlobalContext, class MPKFMeshExporterContext* Context);

	bool PrepareKFMeshExportSkinContext(class IGameNode* Node, class IGameMesh* Mesh, class IGameObject* GameObject, class MPKFGlobalExporterContext* GlobalContext, class MPKFMeshExporterContext* Context);

	bool PrepareKFMeshExportContext(class IGameNode* Node, class IGameMesh* Mesh, class IGameObject* GameObject, class MPKFGlobalExporterContext* GlobalContext, class MPKFMeshExporterContext* Context);

	bool DoExportKFMesh(class IGameNode* Node, class MPKFMeshExporterContext* Context, class MPMemoryChunkWriter* ChunkWriter);

	bool DoExportKFMaterials(class MPKFGlobalExporterContext* GlobalContext, const TSTR& CopyDirTo);

	bool DoExportKFTexture(Texmap* Texture, const TSTR& CopyDirTo, class MPMemoryChunkWriter* ChunkWriter);

	bool DoExportKFMaterial(class IGameMaterial* mat, const TSTR& CopyDirTo, class MPMemoryChunkWriter* ChunkWriter);

	MPKFExporterOptions ExportOptions;

	HWND MaxHWND;

	class IGameScene* IGameExporter;
};