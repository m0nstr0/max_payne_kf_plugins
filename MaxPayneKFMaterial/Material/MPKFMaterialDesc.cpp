/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#include "MPKFMaterialDesc.h"
#include "MPKFMaterial.h"
#include <IMaterialBrowserEntryInfo.h>
#include <IMtlRender_Compatibility.h>

class MPKFMaterialClassDesc : public ClassDesc2, public IMtlRender_Compatibility_MtlBase
{
public:
	MPKFMaterialClassDesc() {
		IMtlRender_Compatibility_MtlBase::Init(*this);
	}

	int IsPublic() override { return TRUE; }
	void* Create(BOOL loading = FALSE) override { return new MPKFMaterial(loading); }
	const TCHAR* ClassName() override { return _T("Max Payne KF Material"); }
	const TCHAR* NonLocalizedClassName() override { return _T("Max Payne KF Material"); }
	SClass_ID SuperClassID() override { return MATERIAL_CLASS_ID; }
	Class_ID ClassID() override { return MPKFMaterial_CLASS_ID; }
	const TCHAR* Category() override { return _T(""); }
	const TCHAR* InternalName() override { return _T("MPKFMaterial"); }
	HINSTANCE     HInstance() override { return hInstance; }

	
	bool IsCompatibleWithRenderer(ClassDesc& rendererClassDesc) override { return true; }
};


ClassDesc2* GetMPKFMaterialDesc()
{
	static MPKFMaterialClassDesc KFMaterialClassDesc;
	return &KFMaterialClassDesc;
}