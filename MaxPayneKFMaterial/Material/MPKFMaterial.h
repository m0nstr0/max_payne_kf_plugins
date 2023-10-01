/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#pragma once

#include "MaxPayneKFMaterial.h"

#define MPKFMaterial_CLASS_ID		Class_ID(0x74450439, 0x62673b6c)
#define MPKFMaterial_PBLOCK_REF		0
#define MPKFMaterial_SUB_TEXMAPS	1

class MPKFMaterial : public Mtl
{
public:
	MPKFMaterial();

	MPKFMaterial(BOOL loading);

	virtual ~MPKFMaterial();

	ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp) override;

	BOOL SetDlgThing(ParamDlg* dlg) override;

	void Update(TimeValue t, Interval& valid) override;

	Interval Validity(TimeValue t) override;

	void Reset() override;

	// Base
	Class_ID ClassID() override { return MPKFMaterial_CLASS_ID; }

	SClass_ID SuperClassID() override { return MATERIAL_CLASS_ID; }

	virtual void GetClassName(TSTR& s, bool localized = true) const override { UNUSED_PARAM(localized); s = TSTR(_M("MKFMaterial")); }

	RefTargetHandle Clone(RemapDir& remap) override;

	// Refs
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override;

	int NumRefs() override { return 1; }

	RefTargetHandle GetReference(int i) override;

	int NumParamBlocks() override { return 1; }

	IParamBlock2* GetParamBlock(int i) override { return pblock; }

	IParamBlock2* GetParamBlockByID(BlockID id) override { return (pblock->ID() == id) ? pblock : NULL; }

	void DeleteThis() override { delete this; }

	//Color
	Color GetAmbient(int mtlNum = 0, BOOL backFace = FALSE) override { return AColor(0.f, 0.f, 0.f, 0.f); }

	Color GetDiffuse(int mtlNum = 0, BOOL backFace = FALSE) override { return AColor(0.f, 0.f, 0.f, 0.f); }

	Color GetSpecular(int mtlNum = 0, BOOL backFace = FALSE) override { return AColor(0.f, 0.f, 0.f, 0.f); }

	float GetShininess(int mtlNum = 0, BOOL backFace = FALSE) override { return 0.f; }
	
	float GetShinStr(int mtlNum = 0, BOOL backFace = FALSE) override { return 0.f; }

	float GetXParency(int mtlNum = 0, BOOL backFace = FALSE) override { return 0.f; }

	void SetAmbient(Color c, TimeValue t) override {}

	void SetDiffuse(Color c, TimeValue t) override {}

	void SetSpecular(Color c, TimeValue t) override {}

	void SetShininess(float v, TimeValue t) override {}

	float WireSize(int mtlNum = 0, BOOL backFace = FALSE) override { return 1.f; }

	// SubTexmaps
	int NumSubTexmaps() override { return MPKFMaterial_SUB_TEXMAPS; }

	Texmap* GetSubTexmap(int i) override { return SubTexmaps[i]; }

	void SetSubTexmap(int i, Texmap* m) override { SubTexmaps[i] = m; }

	TSTR GetSubTexmapSlotName(int i, bool localized) override;

	virtual TSTR GetSubTexmapTVName(int i, bool localized);

	// SubMaterials
	int NumSubMtls() override { return 0; }

	Mtl* GetSubMtl(int i) override { return nullptr; }

	void SetSubMtl(int i, Mtl* m) override { }

	TSTR GetSubMtlSlotName(int i, bool localized) override { return TSTR(_T("Diffuse")); }

	//TSTR GetSubMtlTVName(int i, bool localized = true) override {}

	//void CopySubMtl(HWND hwnd, int ifrom, int ito) override {}

	// Shade
	float EvalDisplacement(ShadeContext& sc) override { return 0.0f; }

	Interval DisplacementValidity(TimeValue t) override { return FOREVER; }

	void Shade(ShadeContext& sc) override;

	// Loading/Saving
	IOResult Load(ILoad* iload) override;

	IOResult Save(ISave* isave) override;

protected:
	void SetReference(int i, RefTargetHandle rtarg) override;

private:
	IParamBlock2* pblock;

	Interval ivalid;

	Texmap* SubTexmaps[MPKFMaterial_SUB_TEXMAPS];
};