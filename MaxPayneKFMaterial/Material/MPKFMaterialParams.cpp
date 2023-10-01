/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#include "MPKFMaterialParams.h"
#include "MPKFMaterial.h"
#include "MPKFMaterialDesc.h"

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
	mpkfmaterial_params_color_ambient, _T("ambinet_color"), TYPE_RGBA, 0, IDS_KF_MATERIAL_COLOR_AMBIENT,
	p_default, Color(0.f, 0.f, 0.0f),
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_COLORSWATCH, IDC_KF_MATERIAL_COLOR_AMBIENT,
	p_end,

	mpkfmaterial_params_color_diffuse, _T("diffuse_color"), TYPE_RGBA, 0, IDS_KF_MATERIAL_COLOR_DIFFUSE,
	p_default, Color(0.f, 0.f, 0.0f),
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_COLORSWATCH, IDC_KF_MATERIAL_COLOR_DIFFUSE,
	p_end,

	mpkfmaterial_params_color_specular, _T("specular_color"), TYPE_RGBA, 0, IDS_KF_MATERIAL_COLOR_SPECULAR,
	p_default, Color(0.f, 0.f, 0.0f),
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_COLORSWATCH, IDC_KF_MATERIAL_COLOR_SPECULAR,
	p_end,

	mpkfmaterial_params_vertex_alpha_value, _T("vertex_alpha_value"), TYPE_INT, 0, IDS_KF_MATERIAL_VERTEX_ALPHA_SPIN,
	p_default, 100,
	p_enabled, FALSE,
	p_range, 0, 100,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SPINNER, EDITTYPE_INT, IDC_KF_MATERIAL_VERTEX_ALPHA_SPIN_VIEW_CONTROL, IDC_KF_MATERIAL_VERTEX_ALPHA_SPIN_CONTROL, 1,
	p_end,
	
	mpkfmaterial_params_specular_exponent, _T("specular_exponent"), TYPE_FLOAT, 0, IDS_KF_MATERIAL_SPECULAR_EXPONENT_SPIN,
	p_default, 8.f,
	p_range, 0.f, 100.f,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_KF_MATERIAL_SPECULAR_EXPONENT_SPIN_VIEW_CONTROL, IDC_KF_MATERIAL_SPECULAR_EXPONENT_SPIN_CONTROL, 1.f,
	p_end,

	mpkfmaterial_params_vertex_alpha, _T("vertex_alpha"), TYPE_BOOL, 0, IDS_KF_MATERIAL_VERTEX_ALPHA,
	p_default, FALSE,
	p_enable_ctrls, 1, mpkfmaterial_params_vertex_alpha_value,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_VERTEX_ALPHA,
	p_end,

	mpkfmaterial_params_two_sided, _T("two_sided"), TYPE_BOOL, 0, IDS_KF_MATERIAL_TWO_SIDED,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_TWO_SIDED,
	p_end,

	mpkfmaterial_params_fogging, _T("fogging"), TYPE_BOOL, 0, IDS_KF_MATERIAL_FOGGING,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_FOGGING,
	p_end,

	mpkfmaterial_params_invisible_geometry, _T("invisible_geometry"), TYPE_BOOL, 0, IDS_KF_MATERIAL_INVISIBLE_GEOMETRY,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_INVISIBLE_GEOMETRY,
	p_end,

	mpkfmaterial_params_diffuse_color_type, _T("diffuse_color_type"), TYPE_INT, 0, IDS_KF_MATERIAL_DIFFUSE_COLOR_TYPE,
	p_default, 2,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_INT_COMBOBOX, IDC_KF_MATERIAL_DIFFUSE_COLOR_TYPE, 3, IDS_KF_MATERIAL_DIFFUSE_COLOR_TYPE_NONE, IDS_KF_MATERIAL_DIFFUSE_COLOR_TYPE_COLOR, IDS_KF_MATERIAL_DIFFUSE_COLOR_TYPE_GOURAND,
	p_vals, 0, 1, 2,
	p_end,

	mpkfmaterial_params_specular_color_type, _T("specular_color_type"), TYPE_INT, 0, IDS_KF_MATERIAL_SPECULAR_COLOR_TYPE,
	p_default, 0,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_INT_COMBOBOX, IDC_KF_MATERIAL_SPECULAR_COLOR_TYPE, 2, IDS_KF_MATERIAL_SPECULAR_COLOR_TYPE_NONE, IDS_KF_MATERIAL_SPECULAR_COLOR_TYPE_GOURAND,
	p_vals, 0, 1,
	p_end,

	//mpkfmaterial_params_textures_rollout
	mpkfmaterial_params_reference_value, _T("reference_value"), TYPE_INT, 0, IDS_KF_MATERIAL_REFERENCE_VALUE_SPIN,
	p_enabled, FALSE,
	p_default, 240,
	p_range, 0, 255,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_SPINNER, EDITTYPE_INT, IDC_KF_MATERIAL_REFERENCE_VALUE_SPIN_VIEW_CONTROL, IDC_KF_MATERIAL_REFERENCE_VALUE_SPIN_CONTROL, 1,
	p_end,

	mpkfmaterial_params_diffuse, _T("diffuse_texmap"), TYPE_TEXMAP, P_SHORT_LABELS, IDS_KF_MATERIAL_DIFFUSE_TEXMAP,
	//p_enabled, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_TEXMAPBUTTON, IDC_KF_MATERIAL_DIFFUSE_TEXMAP,
	p_end,

	mpkfmaterial_params_diffuse_texmap_type, _T("diffuse_texmap_type"), TYPE_INT, 0, IDS_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE,
	p_enabled, FALSE,
	p_default, 0,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_INT_COMBOBOX, IDC_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE, 3, IDS_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE_COPY, IDS_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE_ADDITIVE, IDS_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE_MULTIPLICATIVE,
	p_vals, 0, 1, 2,
	p_end,

	mpkfmaterial_params_reflection, _T("reflection_texmap"), TYPE_TEXMAP, P_SHORT_LABELS, IDS_KF_MATERIAL_REFLECTION_TEXMAP,
	p_enabled, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_TEXMAPBUTTON, IDC_KF_MATERIAL_REFLECTION_TEXMAP,
	p_end,

	mpkfmaterial_params_reflection_texmap_type, _T("reflection_texmap_type"), TYPE_INT, 0, IDS_KF_MATERIAL_REFLECTION_TEXMAP_TYPE,
	p_enabled, FALSE,
	p_default, 0,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_INT_COMBOBOX, IDC_KF_MATERIAL_REFLECTION_TEXMAP_TYPE, 3, IDS_KF_MATERIAL_REFLECTION_TEXMAP_TYPE_COPY, IDS_KF_MATERIAL_REFLECTION_TEXMAP_TYPE_ADDITIVE, IDS_KF_MATERIAL_REFLECTION_TEXMAP_TYPE_MULTIPLICATIVE,
	p_vals, 0, 1, 2,
	p_end,

	mpkfmaterial_params_reflection_lit_type, _T("reflection_lit_type"), TYPE_INT, 0, IDS_KF_MATERIAL_REFLECTION_LIT_TYPE,
	p_default, 0,
	p_enabled, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_INT_COMBOBOX, IDC_KF_MATERIAL_REFLECTION_LIT_TYPE, 2, IDS_KF_MATERIAL_REFLECTION_LIT_TYPE_PHONG, IDS_KF_MATERIAL_REFLECTION_LIT_TYPE_UVW2,
	p_vals, 0, 1,
	p_end,

	mpkfmaterial_params_mask, _T("mask_texmap"), TYPE_TEXMAP, P_SHORT_LABELS, IDS_KF_MATERIAL_MASK_TEXMAP,
	p_enabled, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_TEXMAPBUTTON, IDC_KF_MATERIAL_MASK_TEXMAP,
	p_end,

	mpkfmaterial_params_specular, _T("specular_texmap"), TYPE_TEXMAP, P_SHORT_LABELS, IDS_KF_MATERIAL_SPECULAR_TEXMAP,
	p_enabled, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_TEXMAPBUTTON, IDC_KF_MATERIAL_SPECULAR_TEXMAP,
	p_end,

	mpkfmaterial_params_alpha_blend, _T("alpha_blend_texmap"), TYPE_TEXMAP, P_SHORT_LABELS, IDS_KF_MATERIAL_ALPHA_BLEND_TEXMAP,
	p_enabled, FALSE,
	p_ui, mpkfmaterial_params_textures_rollout, TYPE_TEXMAPBUTTON, IDC_KF_MATERIAL_ALPHA_BLEND_TEXMAP,
	p_end,

	mpkfmaterial_params_has_diffuse, _T("has_diffuse"), TYPE_BOOL, 0, IDS_KF_MATERIAL_DIFFUSE_CHECKBOX,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_DIFFUSE_CHECKBOX,
	p_enable_ctrls, 3, mpkfmaterial_params_diffuse, mpkfmaterial_params_diffuse_texmap_type, mpkfmaterial_params_has_alpha_compare,
	p_end,

	mpkfmaterial_params_has_alpha_compare, _T("has_alpha_compare"), TYPE_BOOL, 0, IDS_KF_MATERIAL_ALPHA_COMPARE_CHECKBOX,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_ALPHA_COMPARE_CHECKBOX,
	p_enabled, FALSE,
	p_enable_ctrls, 1, mpkfmaterial_params_has_edge_blend,
	p_end,

	mpkfmaterial_params_has_edge_blend, _T("has_edge_blend"), TYPE_BOOL, 0, IDS_KF_MATERIAL_EDGE_BLEND_CHECKBOX,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_EDGE_BLEND_CHECKBOX,
	p_enabled, FALSE,
	p_enable_ctrls, 1, mpkfmaterial_params_reference_value,
	p_end,

	mpkfmaterial_params_has_alpha_blend, _T("has_alpha_blend"), TYPE_BOOL, 0, IDS_KF_MATERIAL_ALPHA_BLEND_CHECKBOX,
	p_default, FALSE,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_ALPHA_BLEND_CHECKBOX,
	p_enabled, FALSE,
	p_enable_ctrls, 1, mpkfmaterial_params_alpha_blend,
	p_end,

	mpkfmaterial_params_has_reflection, _T("has_reflection"), TYPE_BOOL, 0, IDS_KF_MATERIAL_REFLECTION_CHECKBOX,
	p_default, FALSE,
	p_enable_ctrls, 3, mpkfmaterial_params_has_lit, mpkfmaterial_params_reflection_texmap_type, mpkfmaterial_params_reflection,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_REFLECTION_CHECKBOX,
	p_end,

	mpkfmaterial_params_has_lit, _T("has_lit"), TYPE_BOOL, 0, IDS_KF_MATERIAL_LIT_CHECKBOX,
	p_default, FALSE,
	p_enable_ctrls, 1, mpkfmaterial_params_reflection_lit_type,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_LIT_CHECKBOX,
	p_end,

	mpkfmaterial_params_has_specular, _T("has_specular"), TYPE_BOOL, 0, IDS_KF_MATERIAL_SPECULAR_CHECKBOX,
	p_default, FALSE,
	p_enable_ctrls, 1, mpkfmaterial_params_specular,
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_SPECULAR_CHECKBOX,
	p_end,

	mpkfmaterial_params_has_mask, _T("has_mask"), TYPE_BOOL, 0, IDS_KF_MATERIAL_MASK_CHECKBOX,
	p_default, FALSE,
	p_enable_ctrls, 1, mpkfmaterial_params_mask, //TODO: 2 mask_texmap_type
	p_ui, mpkfmaterial_params_basic_rollout, TYPE_SINGLECHECKBOX, IDC_KF_MATERIAL_MASK_CHECKBOX,
	p_end,

	//mpkfmaterial_params_modeling_rollout

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
	//Material-> = nullptr;
	DLSetWindowLongPtr(Handle, NULL);
	Handle = nullptr;
}

void MPKFMaterialTexturingDlgProc::UpdateMtl()
{
	if (Material->TestMtlFlag(MTL_TEX_DISPLAY_ENABLED) && Ip) {
		Ip->MtlChanged();
	}
}

void MPKFMaterialTexturingDlgProc::EnableDiffuseTexMap(BOOL Enable)
{
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_DIFFUSE_TEXMAP), Enable);
	EnableWindow(GetDlgItem(Handle, IDC_KF_MATERIAL_DIFFUSE_TEXMAP_TYPE), Enable);
}

INT_PTR MPKFMaterialTexturingDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int ID = LOWORD(wParam);
	int Code = HIWORD(wParam);

	switch (msg) {
		case WM_INITDIALOG: {
			Handle = map->GetHWnd();
			return TRUE;
		}
		case WM_COMMAND: {
			switch (ID) {
			case IDC_KF_MATERIAL_DIFFUSE_CHECKBOX: {
				EnableDiffuseTexMap(TRUE);
				return TRUE;
			}
			//case IDC_KF_TEXTURE_MIPMAPS_MANUAL: {
			//	EnableManualMimmapsSpinner(TRUE);
			//	return TRUE;
			//}
			//case IDC_KF_TEXTURE_SELECT_BUTTON: {
			//	mmpkftexture_param_blk.InvalidateUI(pb_mpkftexture_basic_params_bitmap);
			//	if (HIWORD(wParam) == BN_CLICKED) {
			//		if ((map != NULL) && (map->GetParamBlock() != NULL)) {
			//			ReferenceMaker* pbOwner = map->GetParamBlock()->GetOwner();
			//			if ((pbOwner != NULL) && (pbOwner->ClassID() == MPKFTexture_CLASS_ID)) {
			///				MPKFTexture* OwnerTexture = static_cast<MPKFTexture*>(pbOwner);
			//				OwnerTexture->ReloadActiveBitmap(true);
			//				UpdateMtl();
			//				InvalidateRect(Handle, nullptr, 0);
			//			}
			//		}
			//		return TRUE;
			//	}
				break;
			}
			case IDC_KF_TEXTURE_TEXTURE_RELOAD_BUTTON: {
			//	Texture->ReloadActiveBitmap(true);
				//BroadcastNotification(NOTIFY_BITMAP_CHANGED, (void*)theBMTex->m_bi.Name());
				//theBMTex->NotifyChanged();
			//	UpdateMtl();
			//	InvalidateRect(Handle, nullptr, 0);
			//	return TRUE;
			}
			}
		}

	return FALSE;
}
