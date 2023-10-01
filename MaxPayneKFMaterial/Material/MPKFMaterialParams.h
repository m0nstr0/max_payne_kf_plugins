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
	mpkfmaterial_params_has_specular,
	mpkfmaterial_params_specular,
	mpkfmaterial_params_has_mask,
	mpkfmaterial_params_mask,
	mpkfmaterial_params_diffuse_texmap_type,
	mpkfmaterial_params_reflection_texmap_type,
	mpkfmaterial_params_reflection_lit_type,
	mpkfmaterial_params_mask_texmap_type,
	mpkfmaterial_params_reference_value,
};

ParamBlockDesc2* GetMPKFMaterialParamBlock();

class MPKFMaterialTexturingDlgProc : public ParamMap2UserDlgProc
{
public:
	MPKFMaterialTexturingDlgProc(MPKFMaterial* InMaterial, IMtlParams* InIp);

	virtual ~MPKFMaterialTexturingDlgProc();

	void UpdateMtl();

	void EnableDiffuseTexMap(BOOL Enable);

	virtual INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

	void DeleteThis() override { delete this; }

	//virtual void SetThing(ReferenceTarget* m) { }

	//virtual void Update(TimeValue t) { }

	//virtual void SetParamBlock(IParamBlock2* pb) { }
private:
	HWND Handle;
	class MPKFMaterial* Material;
	IMtlParams* Ip;
	//IParamBlock2* Pblock;
};
