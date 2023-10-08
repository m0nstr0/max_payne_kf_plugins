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
	float MinDeltaPosition{ 0.f };
	float MinDeltaRotation{ 0.f };
	int StartFrame{ 0 };
	int EndFrame{ 100 };
	int LoopToFrame{ 0 };
	int FrameToFrameRotationInterpolation{ 0 };
	TSTR Paths{};
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

	int	DoExport(const MCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0) override;

	MPKFExporterOptions* GetExportOptions() { return &ExportOptions; }

	bool DoExportMesh(class IGameMesh* Mesh);

	bool DoExportMaterials(class MPMemoryWriter& MemoryWriter, const TSTR& CopyDirTo);

	bool DoExportTexture(Texmap* Texture, const TSTR& CopyDirTo, class MPMemoryChunkWriter* ChunkWriter);

	bool DoExportMaterial(class IGameMaterial* mat, const TSTR& CopyDirTo, class MPMemoryChunkWriter* ChunkWriter);

	HWND GetMaxHWND() const { return MaxHWND; }
private:
	MPKFExporterOptions ExportOptions;
	HWND MaxHWND;
	class IGameScene* pIgame;
};