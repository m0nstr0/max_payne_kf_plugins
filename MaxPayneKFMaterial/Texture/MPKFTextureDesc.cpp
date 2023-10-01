/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#include "MPKFTextureDesc.h"
#include "MPKFTexture.h"

class MPKFTextureClassDesc : public ClassDesc2
{
public:
	int IsPublic() override { return TRUE; }
	void* Create(BOOL /*loading = FALSE*/) override { return new MPKFTexture(); }
	const TCHAR* ClassName() override { return _T("Max Payne KF Texture"); }
	const TCHAR* NonLocalizedClassName() override { return _T("Max Payne KF Texture"); }
	SClass_ID SuperClassID() override { return TEXMAP_CLASS_ID; }
	Class_ID ClassID() override { return MPKFTexture_CLASS_ID; }
	const TCHAR* Category() override { return _T(""); }
	const TCHAR* InternalName() override { return _T("MPKFTexture"); }
	HINSTANCE     HInstance() override { return hInstance; }
};


ClassDesc2* GetMPKFTextureDesc()
{
	static MPKFTextureClassDesc KFTextureClassDesc;
	return &KFTextureClassDesc;
}