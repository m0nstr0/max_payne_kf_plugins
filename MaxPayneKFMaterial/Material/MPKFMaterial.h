/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#pragma once

#include "MaxPayneKFMaterial.h"
#include <stdmat.h>
#include <Graphics/ITextureDisplay.h>

#define MPKFMaterial_CLASS_ID					Class_ID(0x74450439, 0x62673b6c)
#define MPKFMaterial_PBLOCK_REF					0
#define MPKFMaterial_SUB_TEXMAPS				5
#define MPFKMaterial_DIFFUSE_TEXMAP				0
#define MPFKMaterial_REFLECTION_TEXMAP			1
#define MPFKMaterial_SPECULAR_TEXMAP			2
#define MPFKMaterial_ALPHA_BLEND_TEXMAP			3
#define MPFKMaterial_MASK_TEXMAP				4
#define MPFKMaterial_DIFFUSE_TEXMAP_REF			1
#define MPFKMaterial_REFLECTION_TEXMAP_REF		2
#define MPFKMaterial_SPECULAR_TEXMAP_REF		3
#define MPFKMaterial_ALPHA_BLEND_TEXMAP_REF		4
#define MPFKMaterial_MASK_TEXMAP_REF			5

class MPKFMaterial : public Mtl, public MaxSDK::Graphics::ITextureDisplay
{
public:
	friend class MPKFMaterialPBAccessor;

	MPKFMaterial();

	MPKFMaterial(BOOL loading);

	BaseInterface* GetInterface(Interface_ID id) override;

	virtual ~MPKFMaterial();

	ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp) override;

	BOOL SetDlgThing(ParamDlg* dlg) override;

	void Update(TimeValue t, Interval& valid) override;

	Interval Validity(TimeValue t) override;

	void Reset() override;

	//From ITextureDisplay
	void SetupTextures(TimeValue t, MaxSDK::Graphics::DisplayTextureHelper& updater) override;

	BOOL SupportTexDisplay() override { return TRUE; }

	void ActivateTexDisplay(BOOL onoff) override;

	void DiscardTexHandles();

	BOOL SupportsMultiMapsInViewport() override { return TRUE; }

	int MapSlotType(int i) override { return MAPSLOT_TEXTURE; }

	// Base
	Class_ID ClassID() override { return MPKFMaterial_CLASS_ID; }

	SClass_ID SuperClassID() override { return MATERIAL_CLASS_ID; }

	void GetClassName(TSTR& s, bool localized = true) const override { UNUSED_PARAM(localized); s = TSTR(_M("MKFMaterial")); }

	RefTargetHandle Clone(RemapDir& remap) override;

	ULONG LocalRequirements(int subMtlNum) override;

	void LocalMappingsRequired(int subMtlNum, BitArray& mapreq, BitArray& bumpreq) override;

	// Refs
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override;

	int NumRefs() override { return 6; }

	RefTargetHandle GetReference(int i) override;

	int NumParamBlocks() override { return 1; }

	IParamBlock2* GetParamBlock(int i) override { return pblock; }

	IParamBlock2* GetParamBlockByID(BlockID id) override { return (pblock->ID() == id) ? pblock : NULL; }

	void DeleteThis() override { delete this; }

	//Color
	Color GetAmbient(int mtlNum = 0, BOOL backFace = FALSE) override;

	Color GetDiffuse(int mtlNum = 0, BOOL backFace = FALSE) override;

	Color GetSpecular(int mtlNum = 0, BOOL backFace = FALSE) override;

	float GetShininess(int mtlNum = 0, BOOL backFace = FALSE) override { return 0.f; }
	
	float GetShinStr(int mtlNum = 0, BOOL backFace = FALSE) override { return 1.f; }

	float GetXParency(int mtlNum = 0, BOOL backFace = FALSE) override;

	void SetAmbient(Color c, TimeValue t) override;

	void SetDiffuse(Color c, TimeValue t) override;

	void SetSpecular(Color c, TimeValue t) override;

	void SetShininess(float v, TimeValue t) override {}

	float WireSize(int mtlNum = 0, BOOL backFace = FALSE) override { return 1.f; }

	// SubTexmaps
	int NumSubTexmaps() override { return MPKFMaterial_SUB_TEXMAPS; }

	Texmap* GetSubTexmap(int i) override;

	void SetSubTexmap(int i, Texmap* m) override;

	TSTR GetSubTexmapSlotName(int i, bool localized) override;

	int SubTexmapOn(int i) override;

	// Sub Anim
	int	NumSubs() override { return 6; }

	Animatable* SubAnim(int i) override;

	TSTR SubAnimName(int i, bool localized) override;

	int	SubNumToRefNum(int subNum) override { return subNum; }

	// SubMaterials
	int NumSubMtls() override { return 0; }

	Mtl* GetSubMtl(int i) override { return nullptr; }

	void SetSubMtl(int i, Mtl* m) override { }

	TSTR GetSubMtlSlotName(int i, bool localized) override { return TSTR(_T("")); }

	// Shade
	float EvalDisplacement(ShadeContext& sc) override { return 0.0f; }

	Interval DisplacementValidity(TimeValue t) override;

	void Shade(ShadeContext& sc) override;

	// Loading/Saving
	IOResult Load(ILoad* iload) override;

	IOResult Save(ISave* isave) override;

protected:
	void SetReference(int i, RefTargetHandle rtarg) override;

private:
	TexHandle* ViewportDiffuseTexHandle;

	TexHandle* ViewportOpacityTexHandle;

	Interval ViewportValidInterval;

	IParamBlock2* pblock;

	Interval ivalid;

	Texmap* SubTexmaps[MPKFMaterial_SUB_TEXMAPS];

	Color AmbientColor;

	Color DiffuseColor;

	Color SpecularColor;

	int VertexAlphaValue;

	float SpecularExponent;

	BOOL HasVertexAlpha;

	BOOL TwoSided;

	BOOL Fogging;

	BOOL InvisibleGeometry;

	int DiffuseColorShadingType;

	int SpecularColorShadigType;

	int AlphaReferenceValue;

	int DiffuseShadingType;

	int ReflectionShadingType;

	int ReflectionLitShadingType;
	
	int MaskShadingType;

	BOOL HasDiffuseTexture;

	BOOL HasAlphaCompare;

	BOOL HasEdgeBlend;

	BOOL HasAlphaBlendTexture;

	BOOL HasReflectionTexture;

	BOOL HasLit;

	BOOL HasSpecularTexture;

	BOOL HasMaskTexture;

	static class MPKFMaterialBasicDlgProc* BasicDlgProc;
};