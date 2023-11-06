#include "MPKFExporterDesc.h"
#include "MPKFExporter.h"

class MPKFExporterClassDesc : public ClassDesc2
{
public:
	int IsPublic() override { return TRUE; }
	void* Create(BOOL loading = FALSE) override { return new MPKFExporter(); }
	const TCHAR* ClassName() override { return _T("Max Payne KF2 File"); }
	const TCHAR* NonLocalizedClassName() override { return _T("Max Payne KF2 File"); }
	SClass_ID SuperClassID() override { return SCENE_EXPORT_CLASS_ID; }
	Class_ID ClassID() override { return MPKFExporter_CLASS_ID; }
	const TCHAR* Category() override { return _T(""); }
	const TCHAR* InternalName() override { return _T("MPKFExporter"); }
	HINSTANCE     HInstance() override { return hInstance; }
};

ClassDesc2* GetMPKFExporterDesc()
{
	static MPKFExporterClassDesc KFExporterClassDesc;
	return &KFExporterClassDesc;
}