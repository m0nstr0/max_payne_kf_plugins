/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#include "MPKFMaterialParams.h"
#include "MPKFMaterial.h"
#include "MPKFMaterialDesc.h"

class MPKFMaterialPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);

	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval& valid) {}
};

void MPKFMaterialPBAccessor::Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
{
	MPKFMaterial* Material = (MPKFMaterial*)owner;
	IParamMap2* pmap = Material->pblock->GetMap();

	switch (id) {
	case mpkfmaterial_params_modeling_transparency_value:
		Material->ModelingTransparencyValue = val.i;
		break;
	case mpkfmaterial_params_modeling_unshaded:
		Material->ModelingUnshded = val.i;
		break;
	case mpkfmaterial_params_modeling_wireframe:
		Material->ModelingWireframe = val.i;
		break;
	case mpkfmaterial_params_modeling_transparency:
		Material->HasModelingTransparency = val.i;
		break;
	case mpkfmaterial_params_mask_texmap_type:
		Material->MaskShadingType = val.i;
		break;
	case mpkfmaterial_params_color_ambient:
		Material->AmbientColor = Color(val.p->x, val.p->y, val.p->z);
		Material->AmbientColor.ClampMinMax();
		break;
	case mpkfmaterial_params_color_diffuse:
		Material->DiffuseColor = Color(val.p->x, val.p->y, val.p->z);
		Material->DiffuseColor.ClampMinMax();
		break;
	case mpkfmaterial_params_color_specular:
		Material->SpecularColor = Color(val.p->x, val.p->y, val.p->z);
		Material->SpecularColor.ClampMinMax();
		break;
	case mpkfmaterial_params_vertex_alpha:
		Material->HasVertexAlpha = val.i;
		break;
	case mpkfmaterial_params_specular_exponent:
		Material->SpecularExponent = val.f;
		break;
	case mpkfmaterial_params_reference_value:
		Material->AlphaReferenceValue = val.i;
		break;
	case mpkfmaterial_params_two_sided:
		Material->TwoSided = val.i;
		break;
	case mpkfmaterial_params_fogging:
		Material->Fogging = val.i;
		break;
	case mpkfmaterial_params_invisible_geometry:
		Material->InvisibleGeometry = val.i;
		break;
	case mpkfmaterial_params_diffuse_color_type:
		Material->DiffuseColorShadingType = val.i;
		break;
	case mpkfmaterial_params_specular_color_type:
		Material->SpecularColorShadigType = val.i;
		break;
	case mpkfmaterial_params_vertex_alpha_value:
		Material->VertexAlphaValue = val.i;
		break;
	case mpkfmaterial_params_diffuse_texmap_type:
		Material->DiffuseShadingType = val.i;
		break;
	case mpkfmaterial_params_reflection_texmap_type:
		Material->ReflectionShadingType = val.i;
		break;
	case mpkfmaterial_params_reflection_lit_type:
		Material->ReflectionLitShadingType = val.i;
		break;
	case mpkfmaterial_params_has_diffuse:
		Material->HasDiffuseTexture = val.i;
		break;
	case mpkfmaterial_params_has_alpha_compare:
		Material->HasAlphaCompare = val.i;
		break;
	case mpkfmaterial_params_has_edge_blend:
		Material->HasEdgeBlend = val.i;
		break;
	case mpkfmaterial_params_has_alpha_blend:
		Material->HasAlphaBlendTexture = val.i;
		break;
	case mpkfmaterial_params_has_reflection:
		Material->HasReflectionTexture = val.i;
		break;
	case mpkfmaterial_params_has_lit:
		Material->HasLit = val.i;
		break;
	case mpkfmaterial_params_has_bump:
		Material->HasBumpTexture = val.i;
		break;
	case mpkfmaterial_params_has_mask:
		Material->HasMaskTexture = val.i;
		break;
	case mpkfmaterial_params_game:
		Material->CurrentGame = val.i;
	case mpkfmaterial_params_emboss_factor:
		Material->EmbossFactor = val.f;
		break;
	case mpkfmaterial_params_diffuse:
		Material->DiscardTexHandles();
		break;
	}
}

static MPKFMaterialPBAccessor gMPKFMaterialAccessor;

static ParamBlockDesc2 mmpkftexture_param_blk(mpkfmaterial_params, _T("params"), 0, GetMPKFMaterialDesc(),
	P_AUTO_CONSTRUCT | P_MULTIMAP | P_AUTO_UI, MPKFMaterial_PBLOCK_REF,
	//rollout
	4,
	//DIALOG_ID, Title, flag_mask, rollup_flag, dlg_proc 
	mpkfmaterial_params_about_rollout,
	IDD_KF_MATERIAL_ABOUT, IDS_KF_MATERIAL_ABOUT, 0, 0, NULL,

	mpkfmaterial_params_basic_rollout,
	IDD_KF_MATERIAL_BASIC_PROPERTIES, IDS_KF_MATERIAL_BASIC_PROPERTIES, 0, 0, NULL,

	mpkfmaterial_params_textures_rollout,
	IDD_KF_MATERIAL_TEXTURES_PROPERTIES, IDS_KF_MATERIAL_TEXTURES_PROPERTIES, 0, 0, NULL,

	mpkfmaterial_params_modeling_rollout,
	IDD_KF_MATERIAL_MODELING_PROPERTIES, IDS_KF_MATERIAL_MODELING_PROPERTIES, 0, 0, NULL,

	//mpkfmaterial_params_basic_rollout
	mpkfmaterial_params_color_ambient, _T("ambinet_color"), TYPE_RGBA, P_ANIMATABLE, IDS_KF_MATERIAL_COLOR_AMBIENT,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, Color(0.f, 0.f, 0.0f),
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_COLORSWATCH, IDC_KF_MATERIAL_COLOR_AMBIENT,
	p_end,

	mpkfmaterial_params_color_diffuse, _T("diffuse_color"), TYPE_RGBA, P_ANIMATABLE, IDS_KF_MATERIAL_COLOR_DIFFUSE,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, Color(1.f, 0.f, 0.0f),
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_COLORSWATCH, IDC_KF_MATERIAL_COLOR_DIFFUSE,
	p_end,

	mpkfmaterial_params_color_specular, _T("specular_color"), TYPE_RGBA, P_ANIMATABLE, IDS_KF_MATERIAL_COLOR_SPECULAR,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, Color(1.f, 1.f, 1.0f),
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_COLORSWATCH, IDC_KF_MATERIAL_COLOR_SPECULAR,
	p_end,

	mpkfmaterial_params_vertex_alpha_value, _T("vertex_alpha_value"), TYPE_INT, P_ANIMATABLE, IDS_KF_MATERIAL_VERTEX_ALPHA_SPIN,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, 100,
	p_range, 0, 100,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SPINNER, EDITTYPE_INT, IDC_KF_MATERIAL_VERTEX_ALPHA_SPIN_VIEW_CONTROL, IDC_KF_MATERIAL_VERTEX_ALPHA_SPIN_CONTROL, 1,
	p_end,
	
	mpkfmaterial_params_specular_exponent, _T("specular_exponent"), TYPE_FLOAT, P_ANIMATABLE, IDS_KF_MATERIAL_SPECULAR_EXPONENT_SPIN,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, 8.f,
	p_range, 0.f, 100.f,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_KF_MATERIAL_SPECULAR_EXPONENT_SPIN_VIEW_CONTROL, IDC_KF_MATERIAL_SPECULAR_EXPONENT_SPIN_CONTROL, 1.f,
	p_end,

	mpkfmaterial_params_vertex_alpha, _T("vertex_alpha"), TYPE_BOOL, 0, IDS_KF_MATERIAL_VERTEX_ALPHA,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_enable_ctrls, 1, mpkfmaterial_params_vertex_alpha_value,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_VERTEX_ALPHA,
	p_end,

	mpkfmaterial_params_two_sided, _T("two_sided"), TYPE_BOOL, 0, IDS_KF_MATERIAL_TWO_SIDED,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_TWO_SIDED,
	p_end,

	mpkfmaterial_params_fogging, _T("fogging"), TYPE_BOOL, 0, IDS_KF_MATERIAL_FOGGING,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_FOGGING,
	p_end,

	mpkfmaterial_params_invisible_geometry, _T("invisible_geometry"), TYPE_BOOL, 0, IDS_KF_MATERIAL_INVISIBLE_GEOMETRY,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_INVISIBLE_GEOMETRY,
	p_end,

	mpkfmaterial_params_diffuse_color_type, _T("diffuse_color_type"), TYPE_INT, 0, IDS_KF_MATERIAL_DIFFUSE_COLOR_TYPE,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, 2,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_INT_COMBOBOX, IDC_KF_MATERIAL_DIFFUSE_COLOR_TYPE, 3, IDS_KF_MATERIAL_DIFFUSE_COLOR_TYPE_NONE, IDS_KF_MATERIAL_DIFFUSE_COLOR_TYPE_COLOR, IDS_KF_MATERIAL_DIFFUSE_COLOR_TYPE_GOURAND,
	p_vals, 0, 1, 2,
	p_end,

	mpkfmaterial_params_specular_color_type, _T("specular_color_type"), TYPE_INT, 0, IDS_KF_MATERIAL_SPECULAR_COLOR_TYPE,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, 1,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_INT_COMBOBOX, IDC_KF_MATERIAL_SPECULAR_COLOR_TYPE, 2, IDS_KF_MATERIAL_SPECULAR_COLOR_TYPE_NONE, IDS_KF_MATERIAL_SPECULAR_COLOR_TYPE_GOURAND,
	p_vals, 0, 1,
	p_end,

	//mpkfmaterial_params_textures_rollout
	mpkfmaterial_params_reference_value, _T("reference_value"), TYPE_INT, 0, IDS_KF_MATERIAL_REFERENCE_VALUE_SPIN,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, 240,
	p_range, 0, 255,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_SPINNER, EDITTYPE_INT, IDC_KF_MATERIAL_REFERENCE_VALUE_SPIN_VIEW_CONTROL, IDC_KF_MATERIAL_REFERENCE_VALUE_SPIN_CONTROL, 1,
	p_end,

	mpkfmaterial_params_diffuse, _T("diffuse_texmap"), TYPE_TEXMAP, P_OWNERS_REF | P_SHORT_LABELS, IDS_KF_MATERIAL_DIFFUSE_TEXMAP,
	p_refno, MPFKMaterial_DIFFUSE_TEXMAP_REF,
	p_subtexno, MPFKMaterial_DIFFUSE_TEXMAP,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_TEXMAPBUTTON, IDC_KF_MATERIAL_DIFFUSE_TEXMAP,
	p_end,

	mpkfmaterial_params_diffuse_texmap_type, _T("diffuse_texmap_type"), TYPE_INT, 0, IDS_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, 0,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_INT_COMBOBOX, IDC_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE, 3, IDS_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE_COPY, IDS_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE_ADDITIVE, IDS_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE_MULTIPLICATIVE,
	p_vals, 0, 1, 2,
	p_end,

	mpkfmaterial_params_reflection, _T("reflection_texmap"), TYPE_TEXMAP, P_OWNERS_REF | P_SHORT_LABELS, IDS_KF_MATERIAL_REFLECTION_TEXMAP,
	p_refno, MPFKMaterial_REFLECTION_TEXMAP_REF,
	p_subtexno, MPFKMaterial_REFLECTION_TEXMAP,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_TEXMAPBUTTON, IDC_KF_MATERIAL_REFLECTION_TEXMAP,
	p_end,

	mpkfmaterial_params_reflection_texmap_type, _T("reflection_texmap_type"), TYPE_INT, 0, IDS_KF_MATERIAL_REFLECTION_TEXMAP_TYPE,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, 0,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_INT_COMBOBOX, IDC_KF_MATERIAL_REFLECTION_TEXMAP_TYPE, 3, IDS_KF_MATERIAL_REFLECTION_TEXMAP_TYPE_COPY, IDS_KF_MATERIAL_REFLECTION_TEXMAP_TYPE_ADDITIVE, IDS_KF_MATERIAL_REFLECTION_TEXMAP_TYPE_MULTIPLICATIVE,
	p_vals, 0, 1, 2,
	p_end,

	mpkfmaterial_params_reflection_lit_type, _T("reflection_lit_type"), TYPE_INT, 0, IDS_KF_MATERIAL_REFLECTION_LIT_TYPE,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, 0,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_INT_COMBOBOX, IDC_KF_MATERIAL_REFLECTION_LIT_TYPE, 3, IDS_KF_MATERIAL_REFLECTION_LIT_TYPE_PHONG, IDS_KF_MATERIAL_REFLECTION_LIT_TYPE_UVW2, IDS_KF_MATERIAL_REFLECTION_LIT_TYPE_ENVIRONMENT,
	p_vals, 0, 1, 2,
	p_end,

	mpkfmaterial_params_mask_texmap_type, _T("mask_texmap_type"), TYPE_INT, 0, IDS_KF_MATERIAL_MASK_TYPE,
	p_accessor, & gMPKFMaterialAccessor,
	p_default, 0,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_INT_COMBOBOX, IDC_KF_MATERIAL_MASK_TYPE, 2, IDS_KF_MATERIAL_MASK_TEXMAP_TYPE_UVW1, IDS_KF_MATERIAL_MASK_TEXMAP_TYPE_UVW2,
	p_vals, 0, 1,
	p_end,

	mpkfmaterial_params_mask, _T("mask_texmap"), TYPE_TEXMAP, P_OWNERS_REF | P_SHORT_LABELS, IDS_KF_MATERIAL_MASK_TEXMAP,
	p_refno, MPFKMaterial_MASK_TEXMAP_REF,
	p_subtexno, MPFKMaterial_MASK_TEXMAP,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_TEXMAPBUTTON, IDC_KF_MATERIAL_MASK_TEXMAP,
	p_end,

	mpkfmaterial_params_bump, _T("bump_texmap"), TYPE_TEXMAP, P_OWNERS_REF | P_SHORT_LABELS, IDS_KF_MATERIAL_BUMP_TEXMAP,
	p_refno, MPFKMaterial_BUMP_TEXMAP_REF,
	p_subtexno, MPFKMaterial_BUMP_TEXMAP,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_TEXMAPBUTTON, IDC_KF_MATERIAL_BUMP_TEXMAP,
	p_end,

	mpkfmaterial_params_emboss_factor, _T("emboss_factor"), TYPE_FLOAT, 0, IDS_KF_MATERIAL_EMBOSS_FACTOR_SPIN,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, 50.f,
	p_range, 0.f, 1000.f,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_KF_MATERIAL_EMBOSS_FACTOR_SPIN_VIEW_CONTROL, IDC_KF_MATERIAL_EMBOSS_FACTOR_SPIN_CONTROL, 1.f,
	p_end,

	mpkfmaterial_params_alpha_blend, _T("alpha_blend_texmap"), TYPE_TEXMAP, P_OWNERS_REF | P_SHORT_LABELS, IDS_KF_MATERIAL_ALPHA_BLEND_TEXMAP,
	p_refno, MPFKMaterial_ALPHA_BLEND_TEXMAP_REF,
	p_subtexno, MPFKMaterial_ALPHA_BLEND_TEXMAP,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_TEXMAPBUTTON, IDC_KF_MATERIAL_ALPHA_BLEND_TEXMAP,
	p_end,

	mpkfmaterial_params_has_diffuse, _T("has_diffuse"), TYPE_BOOL, 0, IDS_KF_MATERIAL_DIFFUSE_CHECKBOX,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_DIFFUSE_CHECKBOX,
	p_end,

	mpkfmaterial_params_has_alpha_compare, _T("has_alpha_compare"), TYPE_BOOL, 0, IDS_KF_MATERIAL_ALPHA_COMPARE_CHECKBOX,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_ALPHA_COMPARE_CHECKBOX,
	p_end,

	mpkfmaterial_params_has_edge_blend, _T("has_edge_blend"), TYPE_BOOL, 0, IDS_KF_MATERIAL_EDGE_BLEND_CHECKBOX,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_EDGE_BLEND_CHECKBOX,
	p_end,

	mpkfmaterial_params_has_alpha_blend, _T("has_alpha_blend"), TYPE_BOOL, 0, IDS_KF_MATERIAL_ALPHA_BLEND_CHECKBOX,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_ALPHA_BLEND_CHECKBOX,
	p_end,

	mpkfmaterial_params_has_reflection, _T("has_reflection"), TYPE_BOOL, 0, IDS_KF_MATERIAL_REFLECTION_CHECKBOX,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_REFLECTION_CHECKBOX,
	p_end,

	mpkfmaterial_params_has_lit, _T("has_lit"), TYPE_BOOL, 0, IDS_KF_MATERIAL_LIT_CHECKBOX,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_LIT_CHECKBOX,
	p_end,

	mpkfmaterial_params_has_bump, _T("has_bump"), TYPE_BOOL, 0, IDS_KF_MATERIAL_BUMP_CHECKBOX,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_BUMP_CHECKBOX,
	p_end,

	mpkfmaterial_params_has_mask, _T("has_mask"), TYPE_BOOL, 0, IDS_KF_MATERIAL_MASK_CHECKBOX,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_MASK_CHECKBOX,
	p_end,

	mpkfmaterial_params_game, _T("game"), TYPE_INT, 0, IDS_KF_MATERIAL_GAME,
	p_default, 0,
	p_accessor, &gMPKFMaterialAccessor,
	p_range, 0, 1,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_RADIO, 2, IDC_KF_MATERIAL_GAME_MP1, IDC_KF_MATERIAL_GAME_MP2,
	p_vals, 0, 1,
	p_end,

	//mpkfmaterial_params_modeling_rollout
	mpkfmaterial_params_modeling_transparency_value, _T("modeling_transparency_value"), TYPE_INT, 0, IDS_KF_MATERIAL_MODELING_TRANSPARENCY_SPIN,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, 100,
	p_range, 0, 100,
	p_ui, mpkfmaterial_params_modeling_rollout, TYPE_SPINNER, EDITTYPE_INT, IDC_KF_MATERIAL_MODELING_TRANSPARENCY_SPIN_VIEW_CONTROL, IDC_KF_MATERIAL_MODELING_TRANSPARENCY_SPIN_CONTROL, 1,
	p_enabled, FALSE,
	p_end,

	mpkfmaterial_params_modeling_unshaded, _T("modeling_unshaded"), TYPE_BOOL, 0, IDS_KF_MATERIAL_MODELING_UNSHADED,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_modeling_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_MODELING_UNSHADED,
	p_end,

	mpkfmaterial_params_modeling_wireframe, _T("modeling_wireframe"), TYPE_BOOL, 0, IDS_KF_MATERIAL_MODELING_WIREFRAME,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_modeling_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_MODELING_WIREFRAME,
	p_end,

	mpkfmaterial_params_modeling_transparency, _T("modeling_transparency"), TYPE_BOOL, 0, IDS_KF_MATERIAL_MODELING_TRANSPARENCY_SPIN,
	p_accessor, &gMPKFMaterialAccessor,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_modeling_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_MODELING_TRANSPARENCY,
	p_enable_ctrls, 1, mpkfmaterial_params_modeling_transparency_value,
	p_end,

	p_end
);

ParamBlockDesc2* GetMPKFMaterialParamBlock()
{
	return &mmpkftexture_param_blk;
}

MPKFMaterialTexturingDlgProc::MPKFMaterialTexturingDlgProc(MPKFMaterial* InMaterial, IMtlParams* InIp) :
	Handle{ nullptr },
	Material{ InMaterial }, 
	Ip{ InIp }
{

}

MPKFMaterialTexturingDlgProc::~MPKFMaterialTexturingDlgProc()
{
	DLSetWindowLongPtr(Handle, NULL);
	Handle = nullptr;
	ReleaseISpinner(EmbossFactorSpin);
	ReleaseISpinner(RefvalueSpin);
	ReleaseICustButton(MaskTexmap);
	ReleaseICustButton(BumpTexmap);
	ReleaseICustButton(ReflectionTexmap);
	ReleaseICustButton(DiffuseTexmap);
	ReleaseICustButton(AlphaTexmap);
}

void MPKFMaterialTexturingDlgProc::UpdateMtl()
{
	if (Material->TestMtlFlag(MTL_TEX_DISPLAY_ENABLED) && Ip) {
		Ip->MtlChanged();
	}
}

void MPKFMaterialTexturingDlgProc::UpdateControls()
{
	CurrentGame = IsDlgButtonChecked(Handle, IDC_KF_MATERIAL_GAME_MP1) == BST_CHECKED ? IDC_KF_MATERIAL_GAME_MP1 : IDC_KF_MATERIAL_GAME_MP2;

	BOOL IsDiffuseChecked = IsDlgButtonChecked(Handle, IDC_KF_MATERIAL_DIFFUSE_CHECKBOX) == BST_CHECKED ? TRUE : FALSE;
	BOOL IsReflectionChecked = IsDlgButtonChecked(Handle, IDC_KF_MATERIAL_REFLECTION_CHECKBOX) == BST_CHECKED ? TRUE : FALSE;
	BOOL IsAlphaChecked = IsDlgButtonChecked(Handle, IDC_KF_MATERIAL_ALPHA_BLEND_CHECKBOX) == BST_CHECKED ? TRUE : FALSE;
	BOOL IsBumpChecked = IsDlgButtonChecked(Handle, IDC_KF_MATERIAL_BUMP_CHECKBOX) == BST_CHECKED ? TRUE : FALSE;
	BOOL IsLitCheked = IsDlgButtonChecked(Handle, IDC_KF_MATERIAL_LIT_CHECKBOX) == BST_CHECKED ? TRUE : FALSE;
	BOOL IsAlphaCompareChecked = IsDlgButtonChecked(Handle, IDC_KF_MATERIAL_ALPHA_COMPARE_CHECKBOX) == BST_CHECKED ? TRUE : FALSE;
	BOOL IsEdgeBlendChecked = IsDlgButtonChecked(Handle, IDC_KF_MATERIAL_EDGE_BLEND_CHECKBOX) == BST_CHECKED ? TRUE : FALSE;
	BOOL IsMaskChecked = IsDlgButtonChecked(Handle, IDC_KF_MATERIAL_MASK_CHECKBOX) == BST_CHECKED ? TRUE : FALSE;

	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE), FALSE);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_REFLECTION_LIT_TYPE), FALSE);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_REFLECTION_TEXMAP_TYPE), FALSE);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_MASK_TYPE), FALSE);

	EmbossFactorSpin->Enable(FALSE);
	RefvalueSpin->Enable(FALSE);
	MaskTexmap->Enable(FALSE);
	BumpTexmap->Enable(FALSE);
	ReflectionTexmap->Enable(FALSE);
	DiffuseTexmap->Enable(FALSE);
	AlphaTexmap->Enable(FALSE);

	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_DIFFUSE_CHECKBOX), FALSE);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_REFLECTION_CHECKBOX), FALSE);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_ALPHA_BLEND_CHECKBOX), FALSE);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_BUMP_CHECKBOX), FALSE);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_LIT_CHECKBOX), FALSE);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_ALPHA_COMPARE_CHECKBOX), FALSE);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_EDGE_BLEND_CHECKBOX), FALSE);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_MASK_CHECKBOX), FALSE);

	if (IsDiffuseChecked) {
		DiffuseTexmap->Enable(TRUE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE), TRUE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_ALPHA_COMPARE_CHECKBOX), TRUE);

		if (IsAlphaCompareChecked) {
			EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_EDGE_BLEND_CHECKBOX), TRUE);
		}

		if (IsEdgeBlendChecked) {
			RefvalueSpin->Enable(TRUE);
		}
	}

	if (IsAlphaChecked) {
		AlphaTexmap->Enable(TRUE);
	}

	if (IsReflectionChecked) {
		ReflectionTexmap->Enable(TRUE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_LIT_CHECKBOX), TRUE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_REFLECTION_TEXMAP_TYPE), TRUE);

		if (IsLitCheked) {
			EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_REFLECTION_LIT_TYPE), TRUE);
		}
	}

	if (IsBumpChecked) {
		BumpTexmap->Enable(TRUE);
		EmbossFactorSpin->Enable(TRUE);
	}

	if (IsMaskChecked) {
		MaskTexmap->Enable(TRUE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_MASK_TYPE), TRUE);
	}

	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_DIFFUSE_CHECKBOX), TRUE);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_REFLECTION_CHECKBOX), TRUE);

	if (CurrentGame == IDC_KF_MATERIAL_GAME_MP1) {
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_ALPHA_COMPARE_CHECKBOX), FALSE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_EDGE_BLEND_CHECKBOX), FALSE);
		RefvalueSpin->Enable(FALSE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_BUMP_CHECKBOX), TRUE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_ALPHA_BLEND_CHECKBOX), TRUE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_MASK_CHECKBOX), TRUE);
	}

	if (CurrentGame == IDC_KF_MATERIAL_GAME_MP2) {
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_BUMP_CHECKBOX), FALSE);
		EmbossFactorSpin->Enable(FALSE);
		BumpTexmap->Enable(FALSE);
		MaskTexmap->Enable(FALSE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_MASK_TYPE), FALSE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_MASK_CHECKBOX), FALSE);
		EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_ALPHA_BLEND_CHECKBOX), FALSE);
		AlphaTexmap->Enable(FALSE);
	}

	UpdateMtl();
}

INT_PTR MPKFMaterialTexturingDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int ID = LOWORD(wParam);
	int Code = HIWORD(wParam);

	switch (msg) {
	case WM_INITDIALOG: {
		Handle = map->GetHWnd();
		EmbossFactorSpin = GetISpinner(GetDlgItem(Handle, IDC_KF_MATERIAL_EMBOSS_FACTOR_SPIN_CONTROL));
		RefvalueSpin = GetISpinner(GetDlgItem(Handle, IDC_KF_MATERIAL_REFERENCE_VALUE_SPIN_CONTROL));
		MaskTexmap = GetICustButton(GetDlgItem(Handle, IDC_KF_MATERIAL_MASK_TEXMAP));
		BumpTexmap = GetICustButton(GetDlgItem(Handle, IDC_KF_MATERIAL_BUMP_TEXMAP));
		ReflectionTexmap = GetICustButton(GetDlgItem(Handle, IDC_KF_MATERIAL_REFLECTION_TEXMAP));
		DiffuseTexmap = GetICustButton(GetDlgItem(Handle, IDC_KF_MATERIAL_DIFFUSE_TEXMAP));
		AlphaTexmap = GetICustButton(GetDlgItem(Handle, IDC_KF_MATERIAL_ALPHA_BLEND_TEXMAP));
		UpdateControls();
		return TRUE;
	}
	case WM_COMMAND: {
		switch (ID) {
		case IDC_KF_MATERIAL_GAME_MP1:
		case IDC_KF_MATERIAL_GAME_MP2:
		case IDC_KF_MATERIAL_DIFFUSE_CHECKBOX:
		case IDC_KF_MATERIAL_REFLECTION_CHECKBOX:
		case IDC_KF_MATERIAL_ALPHA_BLEND_CHECKBOX:
		case IDC_KF_MATERIAL_BUMP_CHECKBOX:
		case IDC_KF_MATERIAL_LIT_CHECKBOX:
		case IDC_KF_MATERIAL_ALPHA_COMPARE_CHECKBOX:
		case IDC_KF_MATERIAL_EDGE_BLEND_CHECKBOX:
		case IDC_KF_MATERIAL_MASK_CHECKBOX:
			UpdateControls();
			return TRUE;
		}
		break;
	}
	}

	return FALSE;
}