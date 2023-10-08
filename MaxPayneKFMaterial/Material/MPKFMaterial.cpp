/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#include "MPKFMaterial.h"
#include "MPKFMaterialDesc.h"
#include "MPKFMaterialParams.h"

#define BMIDATA(x)    ((UBYTE *)((BYTE *)(x) + sizeof(BITMAPINFOHEADER)))

MPKFMaterialTexturingDlgProc* MPKFMaterial::TexturingDlgProc;

MPKFMaterial::MPKFMaterial() :
	ViewportDiffuseTexHandle{ nullptr },
	ViewportOpacityTexHandle{ nullptr },
	pblock{ nullptr },
	AmbientColor(0.f, 0.f, 0.f),
	DiffuseColor(1.f, 0.f, 0.f),
	SpecularColor(1.f, 1.f, 1.f),
	VertexAlphaValue{ 100 },
	SpecularExponent{ 8.f },
	IsVertexAlphaOn{ FALSE },
	TwoSided{ FALSE },
	Fogging{ FALSE },
	InvisibleGeometry{ FALSE },
	DiffuseColorShadingType{ MPFKMaterial_DIFFUSE_COLOR_TYPE_GOURAND },
	SpecularColorShadigType{ MPFKMaterial_SPECULAR_COLOR_TYPE_GOURAND },
	AlphaReferenceValue{ 240 },
	DiffuseShadingType{ MPFKMaterial_DIFFUSE_TEXMAP_TYPE_COPY },
	ReflectionShadingType{ MPFKMaterial_REFLECTION_TEXMAP_TYPE_COPY },
	ReflectionLitShadingType {MPFKMaterial_LIT_TYPE_PHONG},
	MaskShadingType{ MPFKMaterial_MASK_TEXMAP_TYPE_UVW1 },
	HasDiffuseTexmap{ FALSE },
	HasAlphaComp{ FALSE },
	IsEdgeBlendOn{ FALSE },
	HasAlphaTexmap{ FALSE },
	HasReflectionTexmap{ FALSE },
	HasReflectionLit{ FALSE },
	HasBumpTexmap{ FALSE },
	HasMaskTexmap{ FALSE },
	CurrentGame{ MPFKMaterial_GAME_IS_MP1 },
	EmbossFactor{ 50.f },
	ModelingUnshded{ FALSE },
	ModelingWireframe{ FALSE },
	HasModelingTransparency{ FALSE },
	ModelingTransparencyValue{ 100 }
{
	for (int i = 0; i < MPKFMaterial_SUB_TEXMAPS; i++)
		SubTexmaps[i] = nullptr;

	Reset();

	ViewportValidInterval.SetEmpty();
}

MPKFMaterial::MPKFMaterial(BOOL loading):
	ViewportDiffuseTexHandle{ nullptr },
	ViewportOpacityTexHandle{ nullptr },
	pblock{ nullptr },
	AmbientColor(0.f, 0.f, 0.f),
	DiffuseColor(1.f, 0.f, 0.f),
	SpecularColor(1.f, 1.f, 1.f),
	VertexAlphaValue{ 100 },
	SpecularExponent{ 8.f },
	IsVertexAlphaOn{ FALSE },
	TwoSided{ FALSE },
	Fogging{ FALSE },
	InvisibleGeometry{ FALSE },
	DiffuseColorShadingType{ MPFKMaterial_DIFFUSE_COLOR_TYPE_GOURAND },
	SpecularColorShadigType{ MPFKMaterial_SPECULAR_COLOR_TYPE_GOURAND },
	AlphaReferenceValue{ 240 },
	DiffuseShadingType{ MPFKMaterial_DIFFUSE_TEXMAP_TYPE_COPY },
	ReflectionShadingType{ MPFKMaterial_REFLECTION_TEXMAP_TYPE_COPY },
	ReflectionLitShadingType{ MPFKMaterial_LIT_TYPE_PHONG },
	MaskShadingType{ MPFKMaterial_MASK_TEXMAP_TYPE_UVW1 },
	HasDiffuseTexmap{ FALSE },
	HasAlphaComp{ FALSE },
	IsEdgeBlendOn{ FALSE },
	HasAlphaTexmap{ FALSE },
	HasReflectionTexmap{ FALSE },
	HasReflectionLit{ FALSE },
	HasBumpTexmap{ FALSE },
	HasMaskTexmap{ FALSE },
	CurrentGame{ MPFKMaterial_GAME_IS_MP1 },
	EmbossFactor{ 50.f },
	ModelingUnshded{ FALSE },
	ModelingWireframe{ FALSE },
	HasModelingTransparency{ FALSE },
	ModelingTransparencyValue{ 100 }
{
	for (int i = 0; i < MPKFMaterial_SUB_TEXMAPS; i++)
		SubTexmaps[i] = nullptr;

	if (!loading)
		Reset();

	ViewportValidInterval.SetEmpty();
}

BaseInterface* MPKFMaterial::GetInterface(Interface_ID id)
{
	if (id == ITEXTURE_DISPLAY_INTERFACE_ID) {
		return static_cast<MaxSDK::Graphics::ITextureDisplay*>(this);
	}

	return Mtl::GetInterface(id);
}

MPKFMaterial::~MPKFMaterial()
{
	DeleteAllRefs();
	DiscardTexHandles();
}

ParamDlg* MPKFMaterial::CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp)
{
	IAutoMParamDlg* mainDlg = GetMPKFMaterialDesc()->CreateParamDlgs(hwMtlEdit, imp, this);

	TexturingDlgProc = new MPKFMaterialTexturingDlgProc(this, imp);

	GetMPKFMaterialParamBlock()->SetUserDlgProc(mpkfmaterial_params_textures_rollout, TexturingDlgProc);

	return mainDlg;
}

BOOL MPKFMaterial::SetDlgThing(ParamDlg* dlg)
{
	return FALSE;
}

void MPKFMaterial::Update(TimeValue t, Interval& valid)
{
	for (int i = 0; i < MPKFMaterial_SUB_TEXMAPS; i++) {
		if (SubTexmaps[i]) {
			SubTexmaps[i]->Update(t, valid);
		}
	}

	if (pblock == nullptr) return;

	if (!ivalid.InInterval(t))
	{
		ivalid.SetInfinite();

		pblock->GetValue(mpkfmaterial_params_mask_texmap_type, t, MaskShadingType, ivalid);
		pblock->GetValue(mpkfmaterial_params_color_ambient, t, AmbientColor, ivalid);
		AmbientColor.ClampMinMax();
		pblock->GetValue(mpkfmaterial_params_color_diffuse, t, DiffuseColor, ivalid);
		DiffuseColor.ClampMinMax();
		pblock->GetValue(mpkfmaterial_params_color_specular, t, SpecularColor, ivalid);
		SpecularColor.ClampMinMax();
		pblock->GetValue(mpkfmaterial_params_vertex_alpha, t, IsVertexAlphaOn, ivalid);
		pblock->GetValue(mpkfmaterial_params_specular_exponent, t, SpecularExponent, ivalid);
		pblock->GetValue(mpkfmaterial_params_reference_value, t, AlphaReferenceValue, ivalid);
		pblock->GetValue(mpkfmaterial_params_two_sided, t, TwoSided, ivalid);
		pblock->GetValue(mpkfmaterial_params_fogging, t, Fogging, ivalid);
		pblock->GetValue(mpkfmaterial_params_invisible_geometry, t, InvisibleGeometry, ivalid);
		pblock->GetValue(mpkfmaterial_params_diffuse_color_type, t, DiffuseColorShadingType, ivalid);
		pblock->GetValue(mpkfmaterial_params_specular_color_type, t, SpecularColorShadigType, ivalid);
		pblock->GetValue(mpkfmaterial_params_vertex_alpha_value, t, VertexAlphaValue, ivalid);
		pblock->GetValue(mpkfmaterial_params_diffuse_texmap_type, t, DiffuseShadingType, ivalid);
		pblock->GetValue(mpkfmaterial_params_reflection_texmap_type, t, ReflectionShadingType, ivalid);
		pblock->GetValue(mpkfmaterial_params_reflection_lit_type, t, ReflectionLitShadingType, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_diffuse, t, HasDiffuseTexmap, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_alpha_compare, t, HasAlphaComp, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_edge_blend, t, IsEdgeBlendOn, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_alpha_blend, t, HasAlphaTexmap, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_reflection, t, HasReflectionTexmap, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_lit, t, HasReflectionLit, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_bump, t, HasBumpTexmap, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_mask, t, HasMaskTexmap, ivalid);
		pblock->GetValue(mpkfmaterial_params_game, t, CurrentGame, ivalid);
		pblock->GetValue(mpkfmaterial_params_emboss_factor, t, EmbossFactor, ivalid);
		pblock->GetValue(mpkfmaterial_params_modeling_unshaded, t, ModelingUnshded, ivalid);
		pblock->GetValue(mpkfmaterial_params_modeling_wireframe, t, ModelingWireframe, ivalid);
		pblock->GetValue(mpkfmaterial_params_modeling_transparency, t, HasModelingTransparency, ivalid);
		pblock->GetValue(mpkfmaterial_params_modeling_transparency_value, t, ModelingTransparencyValue, ivalid);
	}

	valid &= ivalid;
}

Interval MPKFMaterial::Validity(TimeValue t)
{
	Interval valid = FOREVER;

	for (int i = 0; i < MPKFMaterial_SUB_TEXMAPS; i++)
	{
		if (SubTexmaps[i]) {
			valid &= SubTexmaps[i]->Validity(t);
		}
	}

	return valid;
}

void MPKFMaterial::Reset()
{
	ivalid.SetEmpty();

	for (int i = MPKFMaterial_SUB_TEXMAPS - 1; i >= 0; i--)
	{
		if (SubTexmaps[i]) {
			DeleteReference(i);
			SubTexmaps[i] = nullptr;
		}
	}

	DeleteReference(MPKFMaterial_PBLOCK_REF);

	AmbientColor = Color(0.f, 0.f, 0.f);
	DiffuseColor = Color(1.f, 0.f, 0.f);
	SpecularColor = Color(1.f, 1.f, 1.f);
	VertexAlphaValue = 100;
	SpecularExponent = 8.f;
	IsVertexAlphaOn = FALSE;
	TwoSided = FALSE;
	Fogging = FALSE;
	InvisibleGeometry = FALSE;
	DiffuseColorShadingType = MPFKMaterial_DIFFUSE_COLOR_TYPE_GOURAND;
	SpecularColorShadigType = MPFKMaterial_SPECULAR_COLOR_TYPE_GOURAND;
	AlphaReferenceValue = 240;
	DiffuseShadingType = MPFKMaterial_DIFFUSE_TEXMAP_TYPE_COPY;
	ReflectionShadingType = MPFKMaterial_REFLECTION_TEXMAP_TYPE_COPY;
	ReflectionLitShadingType = MPFKMaterial_LIT_TYPE_PHONG;
	MaskShadingType = MPFKMaterial_MASK_TEXMAP_TYPE_UVW1;
	HasDiffuseTexmap = FALSE;
	HasAlphaComp = FALSE;
	IsEdgeBlendOn = FALSE;
	HasAlphaTexmap = FALSE;
	HasReflectionTexmap = FALSE;
	HasReflectionLit = FALSE;
	HasBumpTexmap = FALSE;
	HasMaskTexmap = FALSE;
	CurrentGame = MPFKMaterial_GAME_IS_MP1;
	EmbossFactor = 50.f;
	ModelingUnshded = FALSE;
	ModelingWireframe = FALSE;
	HasModelingTransparency = FALSE;
	ModelingTransparencyValue = 100;

	GetMPKFMaterialDesc()->MakeAutoParamBlocks(this);
}

KFMaterialGameVersion MPKFMaterial::GetGameVersion()
{
	if (CurrentGame == MPFKMaterial_GAME_IS_MP1) {
		return KFMaterialGameVersion::kMaxPayne1;
	}
    return KFMaterialGameVersion::kMaxPayne2;
}

bool MPKFMaterial::IsTwoSided()
{
    return TwoSided == TRUE;
}

bool MPKFMaterial::IsFogging()
{
    return Fogging == TRUE;
}

bool MPKFMaterial::IsInvisibleGeometry()
{
    return InvisibleGeometry == TRUE;
}

bool MPKFMaterial::HasVertexAlpha()
{
    return IsVertexAlphaOn == TRUE;
}

float MPKFMaterial::GetVertexAlphaValue()
{
    return static_cast<float>(VertexAlphaValue) / 100.f;
}

MPColor MPKFMaterial::GetAmbientColor()
{
    return MPColor(AmbientColor.r * 255.f, AmbientColor.g * 255.f, AmbientColor.b * 255.f, 255.f);
}

MPColor MPKFMaterial::GetDiffuseColor()
{
    return MPColor(DiffuseColor.r * 255.f, DiffuseColor.g * 255.f, DiffuseColor.b * 255.f, 255.f);
}

KFDiffuseColorShadingType MPKFMaterial::GetDiffuseColorShadingType()
{
	switch (DiffuseColorShadingType)
	{
	case MPFKMaterial_DIFFUSE_COLOR_TYPE_NONE:
		return KFDiffuseColorShadingType::kNone;
	case MPFKMaterial_DIFFUSE_COLOR_TYPE_COLOR:
		return KFDiffuseColorShadingType::kColor;
	case MPFKMaterial_DIFFUSE_COLOR_TYPE_GOURAND:
		return KFDiffuseColorShadingType::kGourand;
	default:
		return KFDiffuseColorShadingType::kNone;
	}
}

MPColor MPKFMaterial::GetSpecularColor()
{
    return MPColor(SpecularColor.r * 255.f, SpecularColor.g * 255.f, SpecularColor.b * 255.f, 255.f);
}

KFSpecularColorShadingType MPKFMaterial::GetSpecularColorShadingType()
{
	switch (SpecularColorShadigType)
	{
	case MPFKMaterial_SPECULAR_COLOR_TYPE_NONE:
		return KFSpecularColorShadingType::kNone;
	case MPFKMaterial_SPECULAR_COLOR_TYPE_GOURAND:
		return KFSpecularColorShadingType::kGourand;
	default:
		return KFSpecularColorShadingType::kNone;
	}
}

float MPKFMaterial::GetSpecularExponent()
{
    return SpecularExponent;
}

bool MPKFMaterial::HasDiffuseTexture()
{
    return HasDiffuseTexmap;
}

Texmap* MPKFMaterial::GetDiffuseTexture()
{
	return SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP];
}

KFDiffuseTextureShadingType MPKFMaterial::GetDiffuseTextureShadingType()
{
    switch (DiffuseShadingType)
	{
	case MPFKMaterial_DIFFUSE_TEXMAP_TYPE_COPY:
		return KFDiffuseTextureShadingType::kCopy;
	case MPFKMaterial_DIFFUSE_TEXMAP_TYPE_ADDITIVE:
		return KFDiffuseTextureShadingType::kAdditive;
	case MPFKMaterial_DIFFUSE_TEXMAP_TYPE_MULTIPLICATIVE:
		return KFDiffuseTextureShadingType::kMultiplicative;
	default:
		return KFDiffuseTextureShadingType::kCopy;
	}
}

bool MPKFMaterial::HasEdgeBlend()
{
    return IsEdgeBlendOn == TRUE;
}

bool MPKFMaterial::HasAlphaCompare()
{
    return HasAlphaComp;
}

float MPKFMaterial::GetAlphaReferenceValue()
{
    return AlphaReferenceValue;
}

bool MPKFMaterial::HasAlphaTexture()
{
    return HasAlphaTexmap;
}

Texmap* MPKFMaterial::GetAlphaTexture()
{
	return SubTexmaps[MPFKMaterial_ALPHA_BLEND_TEXMAP];
}

bool MPKFMaterial::HasReflectionTexture()
{
    return HasReflectionTexmap;
}

Texmap* MPKFMaterial::GetReflectionTexture()
{
	return SubTexmaps[MPFKMaterial_REFLECTION_TEXMAP];
}

KFReflectionTextureShadingType MPKFMaterial::GetReflectionTextureShadingType()
{
    switch (ReflectionShadingType)
	{
	case MPFKMaterial_REFLECTION_TEXMAP_TYPE_COPY:
		return KFReflectionTextureShadingType::kCopy;
	case MPFKMaterial_REFLECTION_TEXMAP_TYPE_ADDITIVE:
		return KFReflectionTextureShadingType::kAdditive;
	case MPFKMaterial_REFLECTION_TEXMAP_TYPE_MULTIPLICATIVE:
		return KFReflectionTextureShadingType::kMultiplicative;
	default:
		return KFReflectionTextureShadingType::kCopy;
	}
}

bool MPKFMaterial::HasLit()
{
	return HasReflectionLit;
}

KFLitShadingType MPKFMaterial::GetLitShadingType()
{
    switch (ReflectionLitShadingType)
	{
	case MPFKMaterial_LIT_TYPE_PHONG:
		return KFLitShadingType::kPhong;
	case MPFKMaterial_LIT_TYPE_UVW2:
		return KFLitShadingType::kUVW2;
	case MPFKMaterial_LIT_TYPE_ENVIRONMENT:
		return KFLitShadingType::kEnvironment;
	default:
		return KFLitShadingType::kPhong;
	}
}

bool MPKFMaterial::HasBumpTexture()
{
    return HasBumpTexmap;
}

Texmap* MPKFMaterial::GetBumpTexture()
{
	return SubTexmaps[MPFKMaterial_BUMP_TEXMAP];
}

float MPKFMaterial::GetBumpEmbossFactor()
{
    return EmbossFactor;
}

bool MPKFMaterial::HasMaskTexture()
{
    return HasMaskTexmap;
}

Texmap* MPKFMaterial::GetMaskTexture()
{
	return SubTexmaps[MPFKMaterial_MASK_TEXMAP];
}

KFMaskTextureShadingType MPKFMaterial::GetMaskTextureShadingType()
{
    switch (ReflectionLitShadingType)
	{
	case MPFKMaterial_MASK_TEXMAP_TYPE_UVW1:
		return KFMaskTextureShadingType::kUVW1;
	case MPFKMaterial_MASK_TEXMAP_TYPE_UVW2:
		return KFMaskTextureShadingType::kUVW2;
	default:
		return KFMaskTextureShadingType::kUVW1;
	}
}

bool MPKFMaterial::IsAllTexturesCorrect()
{
    return true;
}

void MPKFMaterial::SetupTextures(TimeValue t, MaxSDK::Graphics::DisplayTextureHelper& updater)
{
	MaxSDK::Graphics::ISimpleMaterial* pISimpleMtl = (MaxSDK::Graphics::ISimpleMaterial*)GetProperty(PROPID_SIMPLE_MATERIAL);

	if (!pISimpleMtl) {
		return;
	}

	if (SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP] && HasDiffuseTexmap) {
		updater.UpdateTextureMapInfo(t, MaxSDK::Graphics::ISimpleMaterial::MapUsage::UsageDiffuse, SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]);
	}

	if (!ViewportValidInterval.InInterval(t))
	{
		DiscardTexHandles();
		pISimpleMtl->ClearTextures();
		ViewportValidInterval.SetInfinite();

		BITMAPINFO* bmi = nullptr;
		if (SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP] && HasDiffuseTexmap) {
			Interval validInterval;
			bmi = SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]->GetVPDisplayDIB(t, updater, validInterval, FALSE);
			ViewportValidInterval = ViewportValidInterval & validInterval;
		}

		if (bmi) {
			ViewportDiffuseTexHandle = updater.MakeHandle(bmi);
		}
	}

	pISimpleMtl->ClearTextures();
	if (ViewportDiffuseTexHandle && ModelingUnshded == FALSE && HasDiffuseTexmap)
	{
		pISimpleMtl->SetTexture(ViewportDiffuseTexHandle, MaxSDK::Graphics::ISimpleMaterial::UsageDiffuse);
	}

	Color diffuseColor = DiffuseColor;
	AColor diffuseBorderColor(int(255 * diffuseColor.r + 0.5), int(255 * diffuseColor.g + 0.5), int(255 * diffuseColor.b + 0.5), 0);
	pISimpleMtl->SetBorderColor(MaxSDK::Graphics::ISimpleMaterial::UsageDiffuse, diffuseBorderColor);
	pISimpleMtl->SetAmbientColor(GetAmbient());
	pISimpleMtl->SetDiffuseColor(GetDiffuse());
	pISimpleMtl->SetSpecularColor(SpecularColor);
	pISimpleMtl->SetSpecularPower(SpecularExponent);
}

void MPKFMaterial::ActivateTexDisplay(BOOL onoff)
{
	if (!onoff) {
		DiscardTexHandles();
	}
}

void MPKFMaterial::DiscardTexHandles()
{
	if (ViewportDiffuseTexHandle)	{
		ViewportDiffuseTexHandle->DeleteThis();
		ViewportDiffuseTexHandle = NULL;
	}
	ViewportValidInterval.SetEmpty();
}

RefTargetHandle MPKFMaterial::Clone(RemapDir& remap)
{
	MPKFMaterial* mnew = new MPKFMaterial(FALSE);
	*((MtlBase*)mnew) = *((MtlBase*)this);
	
	mnew->ReplaceReference(MPKFMaterial_PBLOCK_REF, remap.CloneRef(pblock));
	
	mnew->ivalid.SetEmpty();
	mnew->DiscardTexHandles();

	for (int i = 0; i < MPKFMaterial_SUB_TEXMAPS; i++) {
		mnew->SubTexmaps[i] = nullptr;
		mnew->ReplaceReference(i + 1, remap.CloneRef(SubTexmaps[i]));
	}

	mnew->AmbientColor = AmbientColor;
	mnew->DiffuseColor = DiffuseColor;
	mnew->SpecularColor = SpecularColor;
	mnew->VertexAlphaValue = VertexAlphaValue;
	mnew->SpecularExponent = SpecularExponent;
	mnew->IsVertexAlphaOn = IsVertexAlphaOn;
	mnew->TwoSided = TwoSided;
	mnew->Fogging = Fogging;
	mnew->InvisibleGeometry = InvisibleGeometry;
	mnew->DiffuseColorShadingType = DiffuseColorShadingType;
	mnew->SpecularColorShadigType = SpecularColorShadigType;
	mnew->AlphaReferenceValue = AlphaReferenceValue;
	mnew->DiffuseShadingType = DiffuseShadingType;
	mnew->ReflectionShadingType = ReflectionShadingType;
	mnew->ReflectionLitShadingType = ReflectionLitShadingType;
	mnew->MaskShadingType = MaskShadingType;
	mnew->HasDiffuseTexmap = HasDiffuseTexmap;
	mnew->HasAlphaComp = HasAlphaComp;
	mnew->IsEdgeBlendOn = IsEdgeBlendOn;
	mnew->HasAlphaTexmap = HasAlphaTexmap;
	mnew->HasReflectionTexmap = HasReflectionTexmap;
	mnew->HasReflectionLit = HasReflectionLit;
	mnew->HasBumpTexmap = HasBumpTexmap;
	mnew->HasMaskTexmap = HasMaskTexmap;
	mnew->CurrentGame = CurrentGame;
	mnew->EmbossFactor = EmbossFactor;
	mnew->ModelingUnshded = ModelingUnshded;
	mnew->ModelingWireframe = ModelingWireframe;
	mnew->HasModelingTransparency = HasModelingTransparency;
	mnew->ModelingTransparencyValue = ModelingTransparencyValue;

	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

ULONG MPKFMaterial::LocalRequirements(int subMtlNum)
{
	ULONG retval = 0;
	for (int i = 0; i < MPKFMaterial_SUB_TEXMAPS; i++) {
		if (SubTexmaps[i]) {
			retval |= SubTexmaps[i]->LocalRequirements(subMtlNum);
		}
	}

	retval |= MTLREQ_UV;
	retval |= MTLREQ_TRANSP;
	//retval |= MTLREQ_UV2;

	if (TwoSided) {
		retval |= MTLREQ_2SIDE;
	}

	if (ModelingWireframe) {
		retval |= MTLREQ_WIRE;
	}

	//retval |= MTLREQ_AUTOREFLECT; MTLREQ_ADDITIVE_TRANSP;

	return retval;
}

void MPKFMaterial::LocalMappingsRequired(int subMtlNum, BitArray& mapreq, BitArray& bumpreq)
{
	for (int i = 0; i < MPKFMaterial_SUB_TEXMAPS; i++) {
		if (SubTexmaps[i]) {
			SubTexmaps[i]->LocalMappingsRequired(subMtlNum, mapreq, bumpreq);
		}
	}
}

RefResult MPKFMaterial::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
{
	switch (message)
	{
	case REFMSG_CHANGE:
	{
		ivalid.SetEmpty();
		if (hTarget == pblock)
		{
			ParamID changing_param = pblock->LastNotifyParamID();
			GetMPKFMaterialParamBlock()->InvalidateUI(changing_param);
		}
		else {
			DiscardTexHandles();
		}

		break;
	}
	case REFMSG_TARGET_DELETED:
	{
		if (hTarget == pblock)
		{
			pblock = nullptr;
		}
		else
		{
			for (int i = 0; i < MPKFMaterial_SUB_TEXMAPS; i++)
			{
				if (hTarget == SubTexmaps[i])
				{
					SubTexmaps[i] = nullptr;
					break;
				}
			}

			DiscardTexHandles();
		}
		break;
	}
	}

	return REF_SUCCEED;
}

RefTargetHandle MPKFMaterial::GetReference(int i)
{
	switch (i) {
	case 0:		return pblock;
	default:	return SubTexmaps[i - 1];
	}
}

//Color
Color MPKFMaterial::GetAmbient(int mtlNum, BOOL backFace) 
{ 
	Color c;
	pblock->GetValue(mpkfmaterial_params_color_ambient, 0, c);
	c.ClampMinMax();
	return c;
}

Color MPKFMaterial::GetDiffuse(int mtlNum, BOOL backFace) 
{ 
	Color c;
	pblock->GetValue(mpkfmaterial_params_color_diffuse, 0, c);
	c.ClampMinMax();
	return c;
}

Color MPKFMaterial::GetSpecular(int mtlNum, BOOL backFace) 
{ 
	Color c;
	pblock->GetValue(mpkfmaterial_params_color_specular, 0, c);
	c.ClampMinMax();
	return c;
}

float MPKFMaterial::GetXParency(int mtlNum, BOOL backFace) 
{ 
	BOOL HasAlpha;
	BOOL HasAlphaModeling;
	BOOL IsInvisible;
	int RefValue;
	int RefValueModeling;

	pblock->GetValue(mpkfmaterial_params_vertex_alpha_value, 0, RefValue);
	pblock->GetValue(mpkfmaterial_params_vertex_alpha, 0, HasAlpha);
	pblock->GetValue(mpkfmaterial_params_invisible_geometry, 0, IsInvisible);
	pblock->GetValue(mpkfmaterial_params_modeling_transparency, 0, HasAlphaModeling);
	pblock->GetValue(mpkfmaterial_params_modeling_transparency_value, 0, RefValueModeling);

	if (IsInvisible) {
		return 1.0f;
	}

	if (HasAlpha) {
		return 1.0f - (float)RefValue / 100.f;
	}

	if (HasAlphaModeling) {
		return 1.0f - (float)RefValueModeling / 100.f;
	}

	return 0.f;
}

void MPKFMaterial::SetAmbient(Color c, TimeValue t) {}

void MPKFMaterial::SetDiffuse(Color c, TimeValue t) {}

void MPKFMaterial::SetSpecular(Color c, TimeValue t) {}

Texmap* MPKFMaterial::GetSubTexmap(int i) 
{ 
	return (Texmap*)GetReference(i + 1);
}

void MPKFMaterial::SetSubTexmap(int i, Texmap* m)
{
	ReplaceReference(i + 1, m);
	
	switch (i)
	{
	case MPFKMaterial_DIFFUSE_TEXMAP: {
		GetMPKFMaterialParamBlock()->InvalidateUI(mpkfmaterial_params_diffuse);
		ivalid.SetEmpty();
		DiscardTexHandles();
		return;
	}
	case MPFKMaterial_REFLECTION_TEXMAP: {
		GetMPKFMaterialParamBlock()->InvalidateUI(mpkfmaterial_params_alpha_blend);
		ivalid.SetEmpty();
		return;
	}
	case MPFKMaterial_BUMP_TEXMAP: {
		GetMPKFMaterialParamBlock()->InvalidateUI(mpkfmaterial_params_reflection);
		ivalid.SetEmpty();
		return;
	}
	case MPFKMaterial_ALPHA_BLEND_TEXMAP: {
		GetMPKFMaterialParamBlock()->InvalidateUI(mpkfmaterial_params_bump);
		ivalid.SetEmpty();
		return;
	}
	case MPFKMaterial_MASK_TEXMAP: {
		GetMPKFMaterialParamBlock()->InvalidateUI(mpkfmaterial_params_mask);
		ivalid.SetEmpty();
		return;
	}
	}
}

TSTR MPKFMaterial::GetSubTexmapSlotName(int i, bool localized)
{
	switch (i)
	{
	case MPFKMaterial_DIFFUSE_TEXMAP: return _T("Diffuse");
	case MPFKMaterial_REFLECTION_TEXMAP: return _T("Reflection");
	case MPFKMaterial_BUMP_TEXMAP: return _T("Specular");
	case MPFKMaterial_ALPHA_BLEND_TEXMAP: return _T("AlphaBlend");
	case MPFKMaterial_MASK_TEXMAP: return _T("Mask");
	default: return TSTR(_T(""));
	}
}

int MPKFMaterial::SubTexmapOn(int i)
{
	return SubTexmaps[i] != nullptr;
}

Animatable* MPKFMaterial::SubAnim(int i)
{
	switch (i) {
	case 0:		return pblock;
	default:	return SubTexmaps[i - 1];
	}
}

TSTR MPKFMaterial::SubAnimName(int i, bool localized)
{
	switch (i)
	{
	case 0: return _T("Parameters");
	default: return GetSubTexmapTVName(i - 1, localized);
	}
}

Interval MPKFMaterial::DisplacementValidity(TimeValue t) 
{
	Interval iv;
	iv.SetInfinite();
	return iv;
}

void MPKFMaterial::Shade(ShadeContext& sc)
{
	AColor DColor;// = DiffuseColor;
	AColor AmbColor;// = AmbientColor;
	AColor SColor;// = SpecularColor;
	AColor RColor;
	AColor MColor;
	AColor AlpColor;
	AColor TColor;

	if (gbufID) {
		sc.SetGBufferID(gbufID);
	}

	if (sc.mode == SCMODE_SHADOW)
	{
		sc.out.t = AColor(0.f, 0.f, 0.f);
		return;
	}

	Color lightCol;
	Color diffwk(0.0f, 0.0f, 0.0f);
	Color specwk(0.0f, 0.0f, 0.0f);
	Color ambwk(0.0f, 0.0f, 0.0f);
	Point3 N = sc.Normal();
	Point3	R = sc.ReflectVector();
	LightDesc* l;
	for (int i = 0; i < sc.nLights; i++) {
		l = sc.Light(i);
		float NL, diffCoef;
		Point3 L;
		if (!l->Illuminate(sc, N, lightCol, L, NL, diffCoef))
			continue;

		if (l->ambientOnly) {
			// [dl | 10june2002] Addition of ambient color.
			ambwk += lightCol * AColor(0.f, 1.f, 0.f);
			continue;
		}
		// diffuse
		if (l->affectDiffuse)
			diffwk += diffCoef * lightCol;
		// specular
		if (l->affectSpecular) {
			float c = DotProd(L, R);
			if (c > 0.0f) {
				c = (float)pow((double)c, (double)8.0);
				specwk += c * lightCol * NL;   // multiply by NL to SOFTEN 
			}
		}
	}

	if (SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]) { //HasDiffuseTexmap &&
		DColor = SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]->EvalColor(sc);
	}

	sc.out.t = Color(0.0f, 0.0f, 0.0f);
	sc.out.c = (.3f * sc.ambientLight + diffwk) * DColor + specwk * AColor(1.f, 1.f, 1.f) + ambwk;

	//sc.out.c = AColor(1.f, 0.f, 0.f);//DColor;

	sc.out.c = DColor;

	return;

	if (SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]) { //HasDiffuseTexmap &&
		DColor = SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]->EvalColor(sc);
	}

	if (HasBumpTexmap && SubTexmaps[MPFKMaterial_BUMP_TEXMAP]) {
		SColor = SubTexmaps[MPFKMaterial_BUMP_TEXMAP]->EvalColor(sc);
	}

	if (HasAlphaTexmap && SubTexmaps[MPFKMaterial_ALPHA_BLEND_TEXMAP]) {
		AlpColor = SubTexmaps[MPFKMaterial_ALPHA_BLEND_TEXMAP]->EvalColor(sc);
	}

	if (HasMaskTexmap && SubTexmaps[MPFKMaterial_MASK_TEXMAP]) {
		MColor = SubTexmaps[MPFKMaterial_MASK_TEXMAP]->EvalColor(sc);
	}

	if (HasReflectionTexmap && SubTexmaps[MPFKMaterial_REFLECTION_TEXMAP]) {
		RColor = SubTexmaps[MPFKMaterial_REFLECTION_TEXMAP]->EvalColor(sc);
	}

	//BOOL HasAlphaComp;

	if (InvisibleGeometry) {
		sc.out.t = AColor(1.f, 1.f, 1.f, 1.f);
	}
	else if (IsVertexAlphaOn) {
		sc.out.t = AColor(1.f - (float)VertexAlphaValue / 100.f, 1.f - (float)VertexAlphaValue / 100.f, 1.f - (float)VertexAlphaValue / 100.f, 1.f - (float)VertexAlphaValue / 100.f);
	}

	sc.out.c = AColor(1.f, 0.f, 0.f);//DColor;
}

#define MTL_MATERIAL_HDR_CHUNK		0x4000
#define MTL_MATERIAL_PARAM_CHUNK	0x0001

IOResult MPKFMaterial::Load(ILoad* iload)
{
	ULONG Nb;
	IOResult res;

	while (IO_OK == (res = iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
		case MTL_MATERIAL_HDR_CHUNK: {
			res = MtlBase::Load(iload);
			ivalid.SetEmpty();
			break;
		}
		case MTL_MATERIAL_PARAM_CHUNK: {
			res = iload->Read(&ModelingTransparencyValue, sizeof(ModelingTransparencyValue), &Nb);
			res = iload->Read(&HasModelingTransparency, sizeof(HasModelingTransparency), &Nb);
			res = iload->Read(&ModelingWireframe, sizeof(ModelingWireframe), &Nb);
			res = iload->Read(&ModelingUnshded, sizeof(ModelingUnshded), &Nb);
			res = iload->Read(&EmbossFactor, sizeof(EmbossFactor), &Nb);
			res = iload->Read(&CurrentGame, sizeof(CurrentGame), &Nb);
			res = iload->Read(&HasMaskTexmap, sizeof(HasMaskTexmap), &Nb);
			res = iload->Read(&HasBumpTexmap, sizeof(HasBumpTexmap), &Nb);
			res = iload->Read(&HasReflectionLit, sizeof(HasReflectionLit), &Nb);
			res = iload->Read(&HasReflectionTexmap, sizeof(HasReflectionTexmap), &Nb);
			res = iload->Read(&HasAlphaTexmap, sizeof(HasAlphaTexmap), &Nb);
			res = iload->Read(&IsEdgeBlendOn, sizeof(IsEdgeBlendOn), &Nb);
			res = iload->Read(&HasAlphaComp, sizeof(HasAlphaComp), &Nb);
			res = iload->Read(&HasDiffuseTexmap, sizeof(HasDiffuseTexmap), &Nb);
			res = iload->Read(&MaskShadingType, sizeof(MaskShadingType), &Nb);
			res = iload->Read(&ReflectionLitShadingType, sizeof(ReflectionLitShadingType), &Nb);
			res = iload->Read(&ReflectionShadingType, sizeof(ReflectionShadingType), &Nb);
			res = iload->Read(&DiffuseShadingType, sizeof(DiffuseShadingType), &Nb);
			res = iload->Read(&AlphaReferenceValue, sizeof(AlphaReferenceValue), &Nb);
			res = iload->Read(&SpecularColorShadigType, sizeof(SpecularColorShadigType), &Nb);
			res = iload->Read(&DiffuseColorShadingType, sizeof(DiffuseColorShadingType), &Nb);
			res = iload->Read(&InvisibleGeometry, sizeof(InvisibleGeometry), &Nb);
			res = iload->Read(&Fogging, sizeof(Fogging), &Nb);
			res = iload->Read(&TwoSided, sizeof(TwoSided), &Nb);
			res = iload->Read(&IsVertexAlphaOn, sizeof(IsVertexAlphaOn), &Nb);
			res = iload->Read(&SpecularExponent, sizeof(SpecularExponent), &Nb);
			res = iload->Read(&VertexAlphaValue, sizeof(VertexAlphaValue), &Nb);
			res = iload->Read(&SpecularColor.r, sizeof(SpecularColor.r), &Nb);
			res = iload->Read(&SpecularColor.g, sizeof(SpecularColor.g), &Nb);
			res = iload->Read(&SpecularColor.b, sizeof(SpecularColor.b), &Nb);
			res = iload->Read(&DiffuseColor.r, sizeof(DiffuseColor.r), &Nb);
			res = iload->Read(&DiffuseColor.g, sizeof(DiffuseColor.g), &Nb);
			res = iload->Read(&DiffuseColor.b, sizeof(DiffuseColor.b), &Nb);
			res = iload->Read(&AmbientColor.r, sizeof(AmbientColor.r), &Nb);
			res = iload->Read(&AmbientColor.g, sizeof(AmbientColor.g), &Nb);
			res = iload->Read(&AmbientColor.b, sizeof(AmbientColor.b), &Nb);
			break;
		}
		}

		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

IOResult MPKFMaterial::Save(ISave* isave)
{
	ULONG Nb;
	IOResult res;
	isave->BeginChunk(MTL_MATERIAL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(MTL_MATERIAL_PARAM_CHUNK);
	isave->Write(&ModelingTransparencyValue, sizeof(ModelingTransparencyValue), &Nb);
	isave->Write(&HasModelingTransparency, sizeof(HasModelingTransparency), &Nb);
	isave->Write(&ModelingWireframe, sizeof(ModelingWireframe), &Nb);
	isave->Write(&ModelingUnshded, sizeof(ModelingUnshded), &Nb);
	isave->Write(&EmbossFactor, sizeof(EmbossFactor), &Nb);
	isave->Write(&CurrentGame, sizeof(CurrentGame), &Nb);
	isave->Write(&HasMaskTexmap, sizeof(HasMaskTexmap), &Nb);
	isave->Write(&HasBumpTexmap, sizeof(HasBumpTexmap), &Nb);
	isave->Write(&HasReflectionLit, sizeof(HasReflectionLit), &Nb);
	isave->Write(&HasReflectionTexmap, sizeof(HasReflectionTexmap), &Nb);
	isave->Write(&HasAlphaTexmap, sizeof(HasAlphaTexmap), &Nb);
	isave->Write(&IsEdgeBlendOn, sizeof(IsEdgeBlendOn), &Nb);
	isave->Write(&HasAlphaComp, sizeof(HasAlphaComp), &Nb);
	isave->Write(&HasDiffuseTexmap, sizeof(HasDiffuseTexmap), &Nb);
	isave->Write(&MaskShadingType, sizeof(MaskShadingType), &Nb);
	isave->Write(&ReflectionLitShadingType, sizeof(ReflectionLitShadingType), &Nb);
	isave->Write(&ReflectionShadingType, sizeof(ReflectionShadingType), &Nb);
	isave->Write(&DiffuseShadingType, sizeof(DiffuseShadingType), &Nb);
	isave->Write(&AlphaReferenceValue, sizeof(AlphaReferenceValue), &Nb);
	isave->Write(&SpecularColorShadigType, sizeof(SpecularColorShadigType), &Nb);
	isave->Write(&DiffuseColorShadingType, sizeof(DiffuseColorShadingType), &Nb);
	isave->Write(&InvisibleGeometry, sizeof(InvisibleGeometry), &Nb);
	isave->Write(&Fogging, sizeof(Fogging), &Nb);
	isave->Write(&TwoSided, sizeof(TwoSided), &Nb);
	isave->Write(&IsVertexAlphaOn, sizeof(IsVertexAlphaOn), &Nb);
	isave->Write(&SpecularExponent, sizeof(SpecularExponent), &Nb);
	isave->Write(&VertexAlphaValue, sizeof(VertexAlphaValue), &Nb);
	isave->Write(&SpecularColor.r, sizeof(SpecularColor.r), &Nb);
	isave->Write(&SpecularColor.g, sizeof(SpecularColor.g), &Nb);
	isave->Write(&SpecularColor.b, sizeof(SpecularColor.b), &Nb);
	isave->Write(&DiffuseColor.r, sizeof(DiffuseColor.r), &Nb);
	isave->Write(&DiffuseColor.g, sizeof(DiffuseColor.g), &Nb);
	isave->Write(&DiffuseColor.b, sizeof(DiffuseColor.b), &Nb);
	isave->Write(&AmbientColor.r, sizeof(AmbientColor.r), &Nb);
	isave->Write(&AmbientColor.g, sizeof(AmbientColor.g), &Nb);
	isave->Write(&AmbientColor.b, sizeof(AmbientColor.b), &Nb);
	isave->EndChunk();

	return IO_OK;
}

void MPKFMaterial::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i) {
	case 0:		pblock = (IParamBlock2*)rtarg;	break;
	default:	SubTexmaps[i - 1] = (Texmap*)rtarg;	break;
	}
}
