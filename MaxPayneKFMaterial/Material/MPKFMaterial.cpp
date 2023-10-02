#include "MPKFMaterial.h"
#include "MPKFMaterialDesc.h"
#include "MPKFMaterialParams.h"
//#include <ihardwarematerial.h>

#define BMIDATA(x)    ((UBYTE *)((BYTE *)(x) + sizeof(BITMAPINFOHEADER)))

MPKFMaterialBasicDlgProc* MPKFMaterial::BasicDlgProc;

MPKFMaterial::MPKFMaterial():
	ViewportDiffuseTexHandle{ nullptr },
	ViewportOpacityTexHandle{ nullptr },
	pblock{ nullptr }
{
	for (int i = 0; i < MPKFMaterial_SUB_TEXMAPS; i++)
		SubTexmaps[i] = nullptr;

	Reset();
}

MPKFMaterial::MPKFMaterial(BOOL loading):
	ViewportDiffuseTexHandle{ nullptr },
	ViewportOpacityTexHandle{ nullptr },
	pblock { nullptr }
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

	BasicDlgProc = new MPKFMaterialBasicDlgProc(this, imp);

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
		pblock->GetValue(mpkfmaterial_params_vertex_alpha, t, HasVertexAlpha, ivalid);
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
		pblock->GetValue(mpkfmaterial_params_has_diffuse, t, HasDiffuseTexture, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_alpha_compare, t, HasAlphaCompare, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_edge_blend, t, HasEdgeBlend, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_alpha_blend, t, HasAlphaBlendTexture, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_reflection, t, HasReflectionTexture, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_lit, t, HasLit, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_specular, t, HasSpecularTexture, ivalid);
		pblock->GetValue(mpkfmaterial_params_has_mask, t, HasMaskTexture, ivalid);
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

	GetMPKFMaterialDesc()->MakeAutoParamBlocks(this);
}

void MPKFMaterial::SetupTextures(TimeValue t, MaxSDK::Graphics::DisplayTextureHelper& updater)
{
	MaxSDK::Graphics::ISimpleMaterial* pISimpleMtl = (MaxSDK::Graphics::ISimpleMaterial*)GetProperty(PROPID_SIMPLE_MATERIAL);

	if (!pISimpleMtl) {
		return;
	}

	if (SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]) {
		updater.UpdateTextureMapInfo(t, MaxSDK::Graphics::ISimpleMaterial::MapUsage::UsageDiffuse, SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]);
	}

	// if the texHandler is valid, use the cached data and return.
	if (!ViewportValidInterval.InInterval(t))
	{
		DiscardTexHandles();
		pISimpleMtl->ClearTextures();
		ViewportValidInterval.SetInfinite();

		BITMAPINFO* bmi = nullptr;
		if (SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]) {
			Interval validInterval;
			bmi = SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]->GetVPDisplayDIB(t, updater, validInterval, FALSE);
			ViewportValidInterval = ViewportValidInterval & validInterval;
		}

		if (bmi) {
			ViewportDiffuseTexHandle = updater.MakeHandle(bmi);
		}
	}

	pISimpleMtl->ClearTextures();
	if (ViewportDiffuseTexHandle)
	{
		pISimpleMtl->SetTexture(ViewportDiffuseTexHandle, MaxSDK::Graphics::ISimpleMaterial::UsageDiffuse);
	}

	Color diffuseColor = DiffuseColor;// GetDiffuse();
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
	// First clone the parameter block
	mnew->ReplaceReference(MPKFMaterial_PBLOCK_REF, remap.CloneRef(pblock));

	// Next clone the sub-materials
	mnew->ivalid.SetEmpty();
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

	if (TwoSided) {
		retval |= MTLREQ_2SIDE;
	}

	//retval |= MTLREQ_2SIDE; //MTLREQ_WIRE
	//retval |= MTLREQ_TRANSP; //MTLREQ_2SIDE; MTLREQ_TRANSP; MTLREQ_UV;  MTLREQ_AUTOREFLECT; MTLREQ_ADDITIVE_TRANSP; MTLREQ_UV2; 
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
	BOOL IsInvisible;
	int RefValue;

	pblock->GetValue(mpkfmaterial_params_vertex_alpha_value, 0, RefValue);
	pblock->GetValue(mpkfmaterial_params_vertex_alpha, 0, HasAlpha);
	pblock->GetValue(mpkfmaterial_params_invisible_geometry, 0, IsInvisible);

	if (IsInvisible) {
		return 1.0f;
	}

	if (HasAlpha) {
		return 1.0f - (float)RefValue / 100.f;
	}

	return 0.f;
}

void MPKFMaterial::SetAmbient(Color c, TimeValue t) 
{
	//pblock->GetValue(mpkfmaterial_params_color_ambient, t, c);
	//AmbientColor = c;
}

void MPKFMaterial::SetDiffuse(Color c, TimeValue t) 
{
	//pblock->GetValue(mpkfmaterial_params_color_diffuse, t, c);
	//GetMPKFMaterialParamBlock()->InvalidateUI(mpkfmaterial_params_color_diffuse);
	//DiffuseColor = c;
}

void MPKFMaterial::SetSpecular(Color c, TimeValue t) 
{
	//pblock->GetValue(mpkfmaterial_params_color_specular, t, c);
	//SpecularColor = c;
}

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
		return;
	}
	case MPFKMaterial_REFLECTION_TEXMAP: {
		GetMPKFMaterialParamBlock()->InvalidateUI(mpkfmaterial_params_alpha_blend);
		ivalid.SetEmpty();
		return;
	}
	case MPFKMaterial_SPECULAR_TEXMAP: {
		GetMPKFMaterialParamBlock()->InvalidateUI(mpkfmaterial_params_reflection);
		ivalid.SetEmpty();
		return;
	}
	case MPFKMaterial_ALPHA_BLEND_TEXMAP: {
		GetMPKFMaterialParamBlock()->InvalidateUI(mpkfmaterial_params_specular);
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
	case MPFKMaterial_SPECULAR_TEXMAP: return _T("Specular");
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

	if (SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]) { //HasDiffuseTexture &&
		DColor = SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]->EvalColor(sc);
	}

	sc.out.t = Color(0.0f, 0.0f, 0.0f);
	sc.out.c = (.3f * sc.ambientLight + diffwk) * DColor + specwk * AColor(1.f, 1.f, 1.f) + ambwk;

	//sc.out.c = AColor(1.f, 0.f, 0.f);//DColor;

	sc.out.c = DColor;

	return;

	if (SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]) { //HasDiffuseTexture &&
		DColor = SubTexmaps[MPFKMaterial_DIFFUSE_TEXMAP]->EvalColor(sc);
	}

	if (HasSpecularTexture && SubTexmaps[MPFKMaterial_SPECULAR_TEXMAP]) {
		SColor = SubTexmaps[MPFKMaterial_SPECULAR_TEXMAP]->EvalColor(sc);
	}

	if (HasAlphaBlendTexture && SubTexmaps[MPFKMaterial_ALPHA_BLEND_TEXMAP]) {
		AlpColor = SubTexmaps[MPFKMaterial_ALPHA_BLEND_TEXMAP]->EvalColor(sc);
	}

	if (HasMaskTexture && SubTexmaps[MPFKMaterial_MASK_TEXMAP]) {
		MColor = SubTexmaps[MPFKMaterial_MASK_TEXMAP]->EvalColor(sc);
	}

	if (HasReflectionTexture && SubTexmaps[MPFKMaterial_REFLECTION_TEXMAP]) {
		RColor = SubTexmaps[MPFKMaterial_REFLECTION_TEXMAP]->EvalColor(sc);
	}

	//BOOL HasAlphaCompare;

	if (InvisibleGeometry) {
		sc.out.t = AColor(1.f, 1.f, 1.f, 1.f);
	}
	else if (HasVertexAlpha) {
		sc.out.t = AColor(1.f - (float)VertexAlphaValue / 100.f, 1.f - (float)VertexAlphaValue / 100.f, 1.f - (float)VertexAlphaValue / 100.f, 1.f - (float)VertexAlphaValue / 100.f);
	}

	sc.out.c = AColor(1.f, 0.f, 0.f);//DColor;
}

IOResult MPKFMaterial::Load(ILoad* iload)
{
	return IOResult();
}

IOResult MPKFMaterial::Save(ISave* isave)
{
	return IOResult();
}

void MPKFMaterial::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i) {
	case 0:		pblock = (IParamBlock2*)rtarg;	break;
	default:	SubTexmaps[i - 1] = (Texmap*)rtarg;	break;
	}
}
