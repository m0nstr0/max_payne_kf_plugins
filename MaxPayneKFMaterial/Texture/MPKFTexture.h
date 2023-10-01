/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#pragma once

#include "MaxPayneKFMaterial.h"
#include "MPKFTextureSampler.h"

#include <texutil.h>
#include <bitmap.h>
#include <stdmat.h>
#include <IMaterialBrowserEntryInstanceCallback.h>

#define MPKFTexture_CLASS_ID	Class_ID(0x35bb1f36, 0x2a4a7430)
#define MPKFTexture_PBLOCK_REF	0

class MPKFTexture;

class MPKFTextureBitmapNotify : public BitmapNotify
{
public:
	MPKFTexture* Texture;
	void SetTex(MPKFTexture* InTexture) { Texture = InTexture; }
	int Changed(ULONG flags) override;
};

class MPKFTexture : public Texmap
{
public:
	friend class MPKFTexturePBAccessor;
	friend class MPKFTextureDlgProc;

	MPKFTexture();

	virtual ~MPKFTexture();

	void Init();

	void InitSampler();

	void ShowBitmapProxyPrecacheDialog();

	void FreeActiveBitmap();

	void ReloadActiveBitmap(bool isUIAction);

	BMMRES LoadActiveBitmap(BOOL quiet = true);

	void NotifyChanged();

	Class_ID ClassID() override { return MPKFTexture_CLASS_ID; }

	SClass_ID SuperClassID() override { return TEXMAP_CLASS_ID; }

	void GetClassName(TSTR& s, bool localized) const override { s = _T("MPKFTexture"); }

	// From MtlBase
	ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp) override;

	BOOL SetDlgThing(ParamDlg* dlg) override;

	void Update(TimeValue t, Interval& valid) override;

	void Reset() override;

	Interval Validity(TimeValue t) override;

	Bitmap* GetBitmap();

	ULONG LocalRequirements(int subMtlNum) override;

	int NumSubTexmaps() override { return 0; }

	Texmap* GetSubTexmap(int i) override { return nullptr; }

	void SetSubTexmap(int i, Texmap* m) override;

	TSTR GetSubTexmapSlotName(int i, bool localized) override;

	// From Texmap
	RGBA EvalColor(ShadeContext& sc) override;

	float EvalMono(ShadeContext& sc) override;

	Point3 EvalNormalPerturb(ShadeContext& sc) override;

	// TODO: Returns TRUE if this texture can be used in the interactive renderer
	BOOL SupportTexDisplay() override { return FALSE; }

	void ActivateTexDisplay(BOOL onoff) override;

	DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) override;

	// TODO: Return UV transformation matrix for use in the viewports
	void GetUVTransform(Matrix3& uvtrans) override { }

	// TODO: Return the tiling state of the texture for use in the viewports
	int GetTextureTiling() override { return U_WRAP | V_WRAP; }

	int GetUVWSource() override { return UVWSRC_EXPLICIT; }

	UVGen* GetTheUVGen() override { return nullptr; }

	// TODO: Return anim index to reference index
	int SubNumToRefNum(int subNum) override { return subNum; }

	// Loading/Saving
	IOResult Load(ILoad* iload) override;

	IOResult Save(ISave* isave) override;

	RefTargetHandle Clone(RemapDir& remap) override;

	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override;

	int NumSubs() override { return 1; }
	
	Animatable* SubAnim(int i) override;
	
	TSTR SubAnimName(int i, bool localized) override;

	// TODO: Maintain the number or references here
	int NumRefs() override { return 1; }

	RefTargetHandle GetReference(int i) override;

	int NumParamBlocks() override { return 1; } // return number of ParamBlocks in this instance

	IParamBlock2* GetParamBlock(int /*i*/) override { return pblock; } // return i'th ParamBlock

	IParamBlock2* GetParamBlockByID(BlockID id) override { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

	void DeleteThis() override { delete this; }

protected:
	void SetReference(int i, RefTargetHandle rtarg) override;

private:
	static MPKFTextureDlgProc* TextureDlgProc;

	BitmapInfo ActiveBitmapInfo;

	Bitmap* ActiveBitmap;

	MPKFTextureBitmapNotify BitmapNotify;

	MPKFTextureSampler Sampler;

	int Filtering;

	int MimapsMode;

	int MipmapsNum;

	int StartFrame;

	int PlaybackFPS;

	int AnimationEndCondition;

	BOOL IsAutomaticStart;

	BOOL IsRandomFrameStart;

	//UVGen* uvGen; // ref 0

	IParamBlock2* pblock; // ref 1

	//Texmap* subtex[MPKFTexture_NSUBTEX]; // Other refs

	Interval ivalid;
};