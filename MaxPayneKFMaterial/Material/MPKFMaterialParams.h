/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#include "MaxPayneKFMaterial.h"

enum { mpkfmaterial_params };

enum {
	mpkfmaterial_params_about_rollout,
	mpkfmaterial_params_basic_rollout,
	mpkfmaterial_params_textures_rollout,
	mpkfmaterial_params_modeling_rollout
};

enum {
	mpkfmaterial_params_color_ambient,
	mpkfmaterial_params_color_diffuse,
	mpkfmaterial_params_color_specular,
	mpkfmaterial_params_vertex_alpha,
	mpkfmaterial_params_two_sided,
	mpkfmaterial_params_fogging,
	mpkfmaterial_params_invisible_geometry,
	mpkfmaterial_params_vertex_alpha_value,
	mpkfmaterial_params_specular_exponent,
	mpkfmaterial_params_diffuse_color_type,
	mpkfmaterial_params_specular_color_type,
	mpkfmaterial_params_has_diffuse,
	mpkfmaterial_params_diffuse,
	mpkfmaterial_params_has_alpha_compare,
	mpkfmaterial_params_has_edge_blend,
	mpkfmaterial_params_has_alpha_blend,
	mpkfmaterial_params_alpha_blend,
	mpkfmaterial_params_has_reflection,
	mpkfmaterial_params_reflection,
	mpkfmaterial_params_has_lit,
	mpkfmaterial_params_has_bump,
	mpkfmaterial_params_bump,
	mpkfmaterial_params_has_mask,
	mpkfmaterial_params_mask,
	mpkfmaterial_params_diffuse_texmap_type,
	mpkfmaterial_params_reflection_texmap_type,
	mpkfmaterial_params_reflection_lit_type,
	mpkfmaterial_params_mask_texmap_type,
	mpkfmaterial_params_reference_value,
	mpkfmaterial_params_emboss_factor,
	mpkfmaterial_params_game,
	mpkfmaterial_params_modeling_unshaded,
	mpkfmaterial_params_modeling_wireframe,
	mpkfmaterial_params_modeling_transparency,
	mpkfmaterial_params_modeling_transparency_value,
};

ParamBlockDesc2* GetMPKFMaterialParamBlock();

class MPKFMaterialTexturingDlgProc : public ParamMap2UserDlgProc
{
public:
	MPKFMaterialTexturingDlgProc(class MPKFMaterial* InMaterial, IMtlParams* InIp);

	virtual ~MPKFMaterialTexturingDlgProc();

	void UpdateMtl();

	void UpdateControls();

	virtual INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

	void DeleteThis() override { delete this; }

	//virtual void SetThing(ReferenceTarget* m) { }

	//virtual void Update(TimeValue t) { }

	//virtual void SetParamBlock(IParamBlock2* pb) { }
private:
	HWND Handle;
	MPKFMaterial* Material;
	IMtlParams* Ip;
	ISpinnerControl* EmbossFactorSpin;
	ISpinnerControl* RefvalueSpin;
	ICustButton* MaskTexmap;
	ICustButton* BumpTexmap;
	ICustButton* ReflectionTexmap;
	ICustButton* DiffuseTexmap;
	ICustButton* AlphaTexmap;
	int CurrentGame;
};
