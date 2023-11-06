#pragma once

#include <vector>
#include "MaxPayneKFExporter.h"

#define MPKFExporter_CLASS_ID       Class_ID(0x7f7d370d, 0x6e0c0771)

class MPKFScene;

class MPKFExporter final : public SceneExport
{
public:
    MPKFExporter();

    ~MPKFExporter() override = default;

	int	ExtCount() override { return 1; }

    const MCHAR* Ext(int N) override { return _T("KF2"); }

	const MCHAR* LongDesc() override { return _T("Max Payne KF2 File"); }

	const MCHAR* ShortDesc() override { return _T("Max Payne KF2 File"); }
	
	const MCHAR* AuthorName() override { return _T("Max Payne KF2 File"); }

	const MCHAR* CopyrightMessage() override { return _T("Bolotaev Sergey Borisovich"); }

	const MCHAR* OtherMessage1() override { return _T("Max Payne KF2 File"); }

	const MCHAR* OtherMessage2() override { return _T(""); }

	unsigned int Version() override { return 100; }

	void ShowAbout(HWND HWnd) override {}

	BOOL SupportsOptions(int Ext, DWORD Options) override { return TRUE; }

	int	DoExport(const MCHAR *Name, ExpInterface *Ei, Interface *I, BOOL SuppressPrompts=FALSE, DWORD Options=0) override;
	
	MPKFExporterOptions* GetExportOptions() { return &ExportOptions; }
	
private:
	bool DoIterateSubNodes(IGameNode* Node, MPKFScene* Scene);

	bool DoExportNode(IGameNode* Node, MPKFScene* Scene) const;
	
	bool PrepareKFMeshExportAnimationContext(class IGameNode* Node, class IGameMesh* Mesh, class IGameObject* GameObject, class MPKFGlobalExporterContext* GlobalContext, class MPKFMeshExporterContext* Context);
	
	bool DoExportKFAnimation(class IGameNode* Node, class MPKFMeshExporterContext* Context, class MPKFGlobalExporterContext* GlobalContext);
	
	MPKFExporterOptions ExportOptions;

	IGameScene* GameScene;
};