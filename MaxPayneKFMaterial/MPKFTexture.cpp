/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#include "MPKFTexture.h"
#include "MPKFTextureDesc.h"

enum { mpkftexture_params };

enum {
	mpkftexture_params_about_rollout,
	mpkftexture_params_basic_rollout,
	mpkftexture_params_animation_rollout
};

enum {
	pb_mpkftexture_basic_params_bitmap,
	pb_mpkftexture_basic_params_bitmap_file,
	pb_mpkftexture_basic_params_reload_texuture_button,
	pb_mpkftexture_basic_params_mipmaps_num,
	pb_mpkftexture_basic_params_filtering,
	pb_mpkftexture_basic_params_mipmaps,
	pb_mpkftexture_animation_params_start_frame,
	pb_mpkftexture_animation_params_fps,
	pb_mpkftexture_animation_params_automatic_start,
	pb_mpkftexture_animation_params_random_start,
	pb_mpkftexture_animation_params_end_condition,
};

class MPKFTexturePBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);

	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval& valid) {}
};

void MPKFTexturePBAccessor::Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
{
	MPKFTexture* Texture = (MPKFTexture*)owner;
	IParamMap2* pmap = Texture->pblock->GetMap();
	TSTR p, f, e, name;

	switch (id)
	{
		case pb_mpkftexture_basic_params_bitmap:
		{
			break;
		}
		case pb_mpkftexture_basic_params_mipmaps_num: {
			Texture->MipmapsNum = val.i;
			break;
		}
		case pb_mpkftexture_basic_params_filtering: {
			Texture->Filtering = val.i;
			break;
		}
		case pb_mpkftexture_basic_params_mipmaps: {
			Texture->MimapsMode = val.i;
			break;
		}
		case pb_mpkftexture_animation_params_start_frame: {
			Texture->StartFrame = val.i;
			break;
		}
		case pb_mpkftexture_animation_params_fps:
		{
			Texture->PlaybackFPS = val.i;
			break;
		}
		case pb_mpkftexture_animation_params_automatic_start:
		{
			Texture->IsAutomaticStart = val.i;
			break;
		}
		case pb_mpkftexture_animation_params_random_start:
		{
			Texture->IsRandomFrameStart = val.i;
			break;
		}
		case pb_mpkftexture_animation_params_end_condition:
		{
			Texture->AnimationEndCondition = val.i;
			break;
		}
	}
}

static MPKFTexturePBAccessor gMPKFTextureAccessor;


static ParamBlockDesc2 mmpkftexture_param_blk(mpkftexture_params, _T("params"), 0, GetMPKFTextureDesc(),
	P_AUTO_CONSTRUCT | P_MULTIMAP | P_AUTO_UI, MPKFTexture_PBLOCK_REF,
	//rollout
	//DIALOG_ID, Title, flag_mask, rollup_flag, dlg_proc 
	3,
	mpkftexture_params_about_rollout,
	IDD_KF_TEXTURE_ABOUT, IDS_KF_TEXTURE_ABOUT, 0, 0, NULL,
	//Basic params
	mpkftexture_params_basic_rollout,
	IDD_KF_TEXTURE_BASIC_PROPERTIES, IDS_KF_TEXTURE_BASIC_PARAMS, 0, 0, NULL,
	//Animation params
	mpkftexture_params_animation_rollout,
	IDD_KF_TEXTURE_ANIMATION_PROPERTIES, IDS_KF_TEXTURE_ANIMATION_PARAMS, 0, 0, NULL,

	//Basic params
	pb_mpkftexture_basic_params_bitmap, _T("bitmap"), TYPE_BITMAP, P_SHORT_LABELS, IDS_KF_TEXTURE_BITMAP,
	p_accessor, &gMPKFTextureAccessor,
	p_ui, mpkftexture_params_basic_rollout, TYPE_BITMAPBUTTON, IDC_KF_TEXTURE_SELECT_BUTTON,
	p_end,

	pb_mpkftexture_basic_params_bitmap_file, _T("bitmap_filename"), TYPE_FILENAME, 0, IDS_KF_TEXTURE_BITMAP,
	p_accessor, &gMPKFTextureAccessor,
	p_assetTypeID, MaxSDK::AssetManagement::AssetType::kBitmapAsset,
	p_end,

	pb_mpkftexture_basic_params_filtering, _T("filtering"), TYPE_INT, 0, IDS_KF_TEXTURE_FILTERING,
	p_default, 0,
	p_accessor, & gMPKFTextureAccessor,
	p_ui, mpkftexture_params_basic_rollout, TYPE_RADIO, 5, IDC_KF_TEXTURE_FILTERING_NONE, IDC_KF_TEXTURE_FILTERING_BILINEAR, IDC_KF_TEXTURE_FILTERING_TRILINEAR, IDC_KF_TEXTURE_FILTERING_ANISOTROPIC, IDC_KF_TEXTURE_FILTERING_AUTO,
	p_vals, 0, 1, 2, 3, 4,
	p_end,

	pb_mpkftexture_basic_params_mipmaps, _T("mipmaps"), TYPE_INT, 0, IDS_KF_TEXTURE_MIPMAPS,
	p_default, 0,
	p_accessor, & gMPKFTextureAccessor,
	p_ui, mpkftexture_params_basic_rollout, TYPE_RADIO, 2, IDC_KF_TEXTURE_MIPMAPS_AUTO, IDC_KF_TEXTURE_MIPMAPS_MANUAL,
	p_vals, 0, 1,
	p_end,

	pb_mpkftexture_basic_params_mipmaps_num, _T("mipmaps_num"), TYPE_INT, 0, IDS_KF_TEXTURE_MIPMAPS_MANUAL_SPIN,
	p_enabled, FALSE,
	p_accessor, & gMPKFTextureAccessor,
	p_default, 1,
	p_range, 0, 100,
	p_ui, mpkftexture_params_basic_rollout, TYPE_SPINNER, EDITTYPE_INT, IDC_KF_TEXTURE_MIPMAPS_MANUAL_SPIN_VIEW_CONTROL, IDC_KF_TEXTURE_MIPMAPS_MANUAL_SPIN_CONTROL, 1,
	p_end,

	//Animation params
	pb_mpkftexture_animation_params_start_frame, _T("start_frame"), TYPE_INT, 0, IDS_KF_TEXTURE_ANIMATION_START_FRAME_SPIN,
	p_default, 0,
	p_accessor, & gMPKFTextureAccessor,
	p_range, 0, 10000,
	p_ui, mpkftexture_params_animation_rollout, TYPE_SPINNER, EDITTYPE_INT, IDC_KF_TEXTURE_ANIMATION_START_FRAME_SPIN_VIEW_CONTROL, IDC_KF_TEXTURE_ANIMATION_START_FRAME_SPIN_CONTROL, 1,
	p_end,

	pb_mpkftexture_animation_params_fps, _T("fps"), TYPE_INT, 0, IDS_KF_TEXTURE_ANIMATION_FPS_SPIN,
	p_default, 1,
	p_accessor, & gMPKFTextureAccessor,
	p_range, 1, 10000,
	p_ui, mpkftexture_params_animation_rollout, TYPE_SPINNER, EDITTYPE_INT, IDC_KF_TEXTURE_ANIMATION_FPS_SPIN_VIEW_CONTROL, IDC_KF_TEXTURE_ANIMATION_FPS_SPIN_CONTROL, 1,
	p_end,

	pb_mpkftexture_animation_params_end_condition, _T("end_condition"), TYPE_INT, 0, IDS_KF_TEXTURE_ANIMATION_END_CONDITION,
	p_default, 0,
	p_accessor, &gMPKFTextureAccessor,
	p_ui, mpkftexture_params_animation_rollout, TYPE_RADIO, 3, IDC_KF_TEXTURE_ANIMATION_END_CONDITION_LOOP, IDC_KF_TEXTURE_ANIMATION_END_CONDITION_PING_PONG, IDC_KF_TEXTURE_ANIMATION_END_CONDITION_HOLD,
	p_vals, 0, 1, 2,
	p_end,

	pb_mpkftexture_animation_params_automatic_start, _T("automatic_start"), TYPE_BOOL, 0, IDS_KF_TEXTURE_ANIMATION_AUTOMATIC_START,
	p_ui, mpkftexture_params_animation_rollout, TYPE_SINGLECHECKBOX, IDC_KF_TEXTURE_ANIMATION_AUTOMATIC_START_CHECK_BOX,
	p_default, FALSE,
	p_accessor, &gMPKFTextureAccessor,
	p_end,

	pb_mpkftexture_animation_params_random_start, _T("random_start"), TYPE_BOOL, 0, IDS_KF_TEXTURE_ANIMATION_RANDOM_START,
	p_ui, mpkftexture_params_animation_rollout, TYPE_SINGLECHECKBOX, IDC_KF_TEXTURE_ANIMATION_RANDOM_FRAME_CHECK_BOX,
	p_default, FALSE,
	p_accessor, &gMPKFTextureAccessor,
	p_end,
	
	p_end
);

class MPKFTextureDlgProc : public ParamMap2UserDlgProc 
{
public:
	MPKFTextureDlgProc() : Texture{ nullptr } {}

	MPKFTextureDlgProc(MPKFTexture* InTexture, IMtlParams* InIp, IParamBlock2* InPblock) : Texture{ InTexture }, Ip{ InIp }, Pblock{ InPblock }  {}

	virtual ~MPKFTextureDlgProc();

	virtual INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

	virtual void DeleteThis() override;

	void EnableManualMimmapsSpinner(BOOL Enable = TRUE);

	virtual void SetThing(ReferenceTarget* m) { }

	virtual void Update(TimeValue t) { }

	virtual void SetParamBlock(IParamBlock2* pb) { }

	void UpdateMtl();
private:
	HWND Handle;
	MPKFTexture* Texture;
	IMtlParams* Ip;
	IParamBlock2* Pblock;
};

void MPKFTextureDlgProc::EnableManualMimmapsSpinner(BOOL Enable)
{
	ISpinnerControl* Spinner = GetISpinner(GetDlgItem(Handle, IDC_KF_TEXTURE_MIPMAPS_MANUAL_SPIN_VIEW_CONTROL));

	if (!Spinner) {
		return;
	}

	if (Enable == TRUE) {
		Spinner->Enable();
	} else {
		Spinner->Disable();
	}

	ReleaseISpinner(Spinner);
}

void MPKFTextureDlgProc::UpdateMtl()
{
	if (Texture->TestMtlFlag(MTL_TEX_DISPLAY_ENABLED) && Ip) {
		Ip->MtlChanged();
	}
}

MPKFTextureDlgProc::~MPKFTextureDlgProc()
{
	Texture->TextureDlgProc = nullptr;
	DLSetWindowLongPtr(Handle, NULL);
	Handle = nullptr;
}

INT_PTR MPKFTextureDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
				case IDC_KF_TEXTURE_MIPMAPS_AUTO: {
					EnableManualMimmapsSpinner(FALSE);
					return TRUE;
				}
				case IDC_KF_TEXTURE_MIPMAPS_MANUAL: {
					EnableManualMimmapsSpinner(TRUE);
					return TRUE;
				}
				case IDC_KF_TEXTURE_SELECT_BUTTON: {
					mmpkftexture_param_blk.InvalidateUI(pb_mpkftexture_basic_params_bitmap);
					if (HIWORD(wParam) == BN_CLICKED) {
						if ((map != NULL) && (map->GetParamBlock() != NULL)) {
							ReferenceMaker* pbOwner = map->GetParamBlock()->GetOwner();
							if ((pbOwner != NULL) && (pbOwner->ClassID() == MPKFTexture_CLASS_ID)) {
								MPKFTexture* OwnerTexture = static_cast<MPKFTexture*>(pbOwner);
								OwnerTexture->ReloadActiveBitmap(true);
								UpdateMtl();
								InvalidateRect(Handle, nullptr, 0);
							}
						}
						return TRUE;
					}
					break;
				}
				case IDC_KF_TEXTURE_TEXTURE_RELOAD_BUTTON: {
					Texture->ReloadActiveBitmap(true);
					//BroadcastNotification(NOTIFY_BITMAP_CHANGED, (void*)theBMTex->m_bi.Name());
					//theBMTex->NotifyChanged();
					UpdateMtl();
					InvalidateRect(Handle, nullptr, 0);
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

void MPKFTextureDlgProc::DeleteThis() 
{
	delete this; 
}

MPKFTextureDlgProc* MPKFTexture::TextureDlgProc;

//--- MPKFTexture -------------------------------------------------------
MPKFTexture::MPKFTexture() : 
	pblock(nullptr), 
	Sampler(this),
	ActiveBitmap{ nullptr },
	ActiveBitmapInfo{ nullptr }
{
	GetMPKFTextureDesc()->MakeAutoParamBlocks(this);
	Reset();
}

MPKFTexture::~MPKFTexture()
{
	//if (TextureDlgProc) {
	//	TextureDlgProc->DeleteThis();
	//	TextureDlgProc = nullptr;
	//}
}

void MPKFTexture::Init() 
{

}

void MPKFTexture::InitSampler()
{
}

void MPKFTexture::FreeActiveBitmap()
{
	if (ActiveBitmap != nullptr) {
		ActiveBitmap->DeleteThis();
		ActiveBitmap = nullptr;
	}
}

void MPKFTexture::ShowBitmapProxyPrecacheDialog() {

	const TCHAR* bmName = ActiveBitmapInfo.Name();
	if ((bmName != NULL) && (bmName[0] != _T('\0'))) {
		IBitmapProxyManager* proxyManager = GetBitmapProxyManager();
		if ((proxyManager != NULL) && proxyManager->GetGlobalProxyEnable()) {
			Tab<const TCHAR*> filenames;
			filenames.Append(1, &bmName);
			proxyManager->ShowPrecacheDialog(&filenames);
		}
	}
}

void MPKFTexture::ReloadActiveBitmap(bool isUIAction)
{
	if (ActiveBitmap != nullptr)
	{
		FreeActiveBitmap();
		LoadActiveBitmap(TRUE);
	}
	else
	{
		LoadActiveBitmap();
	}

	IParamMap2* pmap = pblock->GetMap();
	if (pmap != NULL) {
		pmap->SetText(pb_mpkftexture_basic_params_bitmap, (MCHAR*)ActiveBitmapInfo.Name());
	}

	if (isUIAction)	{
		ShowBitmapProxyPrecacheDialog();
	}
}

BMMRES MPKFTexture::LoadActiveBitmap(BOOL quiet)
{
	BOOL Silent = FALSE;
	BMMRES Status = BMMRES_SUCCESS;

	PBBitmap* BitmapPB = nullptr;
	pblock->GetValue(pb_mpkftexture_basic_params_bitmap, 0, BitmapPB);

	if (BitmapPB != nullptr)
	{
		const TCHAR* OldName = ActiveBitmapInfo.Name();
		const TCHAR* NewName = BitmapPB->bi.Name();
		if (_tcscmp(OldName, NewName))
			FreeActiveBitmap();
		ActiveBitmapInfo.SetName(NewName);
	}
	else 
	{
		ActiveBitmapInfo.SetName(_T(""));
	}

	if (ActiveBitmapInfo.Name()[0] == 0) {
		if (ActiveBitmapInfo.Device()[0] == 0) {
			return BMMRES_NODRIVER;
		}
	}

	if (ActiveBitmap == nullptr)
	{
		//BMMSilentModeGuard silentModeGuard(quiet);
		BOOL PrevSilentMode = TheManager->SilentMode();
		TheManager->SetSilentMode(quiet);
		Bitmap* NewBitmap = TheManager->Load(&BitmapPB->bi, &Status);
		TheManager->SetSilentMode(PrevSilentMode);
		ActiveBitmap = NewBitmap;
	}

	return BMMRES_SUCCESS;
}

// From MtlBase
void MPKFTexture::Reset()
{
	GetMPKFTextureDesc()->Reset(this, TRUE);
	ivalid.SetEmpty();
	Init();
}

void MPKFTexture::Update(TimeValue t, Interval& valid)
{
	if (pblock == nullptr) return;

	if (!ivalid.InInterval(t)) 
	{
		ivalid.SetInfinite();

		PBBitmap* pb;
		pblock->GetValue(pb_mpkftexture_basic_params_bitmap, t, pb, ivalid);

		ReloadActiveBitmap(false);

		pblock->GetValue(pb_mpkftexture_basic_params_filtering, t, Filtering, ivalid);

		pblock->GetValue(pb_mpkftexture_basic_params_mipmaps, t, MimapsMode, ivalid);

		pblock->GetValue(pb_mpkftexture_basic_params_mipmaps_num, t, MipmapsNum, ivalid);

		pblock->GetValue(pb_mpkftexture_animation_params_start_frame, t, StartFrame, ivalid);

		pblock->GetValue(pb_mpkftexture_animation_params_fps, t, PlaybackFPS, ivalid);

		pblock->GetValue(pb_mpkftexture_animation_params_end_condition, t, AnimationEndCondition, ivalid);

		pblock->GetValue(pb_mpkftexture_animation_params_automatic_start, t, IsAutomaticStart, ivalid);

		pblock->GetValue(pb_mpkftexture_animation_params_random_start, t, IsRandomFrameStart, ivalid);
	}

	valid &= ivalid;
}

Interval MPKFTexture::Validity(TimeValue t)
{
	Interval v = FOREVER;
	Update(t, v);
	return ivalid;
}

Bitmap* MPKFTexture::GetBitmap()
{
	return ActiveBitmap;
}

void MPKFTexture::NotifyChanged()
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

ParamDlg* MPKFTexture::CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp)
{
	IAutoMParamDlg* mainDlg = GetMPKFTextureDesc()->CreateParamDlgs(hwMtlEdit, imp, this);
	TextureDlgProc = new MPKFTextureDlgProc(this, imp, pblock);
	mmpkftexture_param_blk.SetUserDlgProc(mpkftexture_params_basic_rollout, TextureDlgProc);
	return mainDlg;
}

BOOL MPKFTexture::SetDlgThing(ParamDlg* dlg)
{
	return FALSE;
}

void MPKFTexture::SetSubTexmap(int i, Texmap* m)
{
}

TSTR MPKFTexture::GetSubTexmapSlotName(int /*i*/, bool /*localized*/)
{
	return TSTR(_T(""));
}

// From ReferenceMaker
RefTargetHandle MPKFTexture::GetReference(int i)
{
	switch (i)
	{
	case 0:
		return pblock;
	default:
		return nullptr;
	}
}

void MPKFTexture::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case 0:
		pblock = (IParamBlock2*)rtarg;
		break;
	}
}

RefResult MPKFTexture::NotifyRefChanged(const Interval& /*changeInt*/, RefTargetHandle hTarget, PartID& /*partID*/, RefMessage message, BOOL /*propagate*/)
{
	switch (message)
	{
	case REFMSG_TARGET_DELETED:
	{
		if (hTarget == pblock)
		{
			pblock = nullptr;
		}
	}
	}
	return (REF_SUCCEED);
}

// From ReferenceTarget
RefTargetHandle MPKFTexture::Clone(RemapDir& remap)
{
	MPKFTexture* mnew = new MPKFTexture();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	// TODO: Add other cloning stuff
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

Animatable* MPKFTexture::SubAnim(int i) {
	switch (i)
	{
	case 0:
		return pblock;
	default:
		return nullptr;
	}
}

TSTR MPKFTexture::SubAnimName(int i, bool localized)
{
	// TODO: Return the sub-anim names
	switch (i)
	{
	case 0:
		return _T("Coordinates");
	case 1:
		return _T("Parameters");
	default:
		return GetSubTexmapTVName(i - 1, localized);
	}
}

#define MTL_HDR_CHUNK			0x4000
#define MTL_PARAM_CHUNK			0x0001
#define MTL_BITMAP_CHUNK		0x0002

IOResult MPKFTexture::Save(ISave* isave)
{
	ULONG Nb;
	IOResult res;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(MTL_PARAM_CHUNK);
	isave->Write(&Filtering, sizeof(Filtering), &Nb);
	isave->Write(&MimapsMode, sizeof(MimapsMode), &Nb);
	isave->Write(&MipmapsNum, sizeof(MipmapsNum), &Nb);
	isave->Write(&StartFrame, sizeof(StartFrame), &Nb);
	isave->Write(&PlaybackFPS, sizeof(PlaybackFPS), &Nb);
	isave->Write(&AnimationEndCondition, sizeof(AnimationEndCondition), &Nb);
	isave->Write(&IsAutomaticStart, sizeof(IsAutomaticStart), &Nb);
	isave->Write(&IsRandomFrameStart, sizeof(IsRandomFrameStart), &Nb);
	isave->EndChunk();

	isave->BeginChunk(MTL_BITMAP_CHUNK);
	ActiveBitmapInfo.Save(isave);
	isave->EndChunk();

	return IO_OK;
}

IOResult MPKFTexture::Load(ILoad* iload)
{
	ULONG Nb;
	IOResult res;
	TSTR oldBMName;
	BOOL IsMtlBase = FALSE;

	while (IO_OK == (res = iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case MTL_HDR_CHUNK: {
				IsMtlBase = TRUE;
				res = MtlBase::Load(iload);
				break;
			}
			case MTL_PARAM_CHUNK: {
				IsMtlBase = FALSE;
				res = iload->Read(&Filtering, sizeof(Filtering), &Nb);
				res = iload->Read(&MimapsMode, sizeof(MimapsMode), &Nb);
				res = iload->Read(&MipmapsNum, sizeof(MipmapsNum), &Nb);
				res = iload->Read(&StartFrame, sizeof(StartFrame), &Nb);
				res = iload->Read(&PlaybackFPS, sizeof(PlaybackFPS), &Nb);
				res = iload->Read(&AnimationEndCondition, sizeof(AnimationEndCondition), &Nb);
				res = iload->Read(&IsAutomaticStart, sizeof(IsAutomaticStart), &Nb);
				res = iload->Read(&IsRandomFrameStart, sizeof(IsRandomFrameStart), &Nb);
				break;
			}
			case MTL_BITMAP_CHUNK: {
				IsMtlBase = FALSE;
				res = ActiveBitmapInfo.Load(iload);
				break;
			}
		}

		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}
		
	return IO_OK;
}

AColor MPKFTexture::EvalColor(ShadeContext& sc)
{
	AColor c(0.0f, 0.0f, 0.0f, 0.0f);

	if (!sc.doMaps) {
		return c;
	}

	if (sc.GetCache(this, c)) {
		return c;
	}

	if (gbufID) {
		sc.SetGBufferID(gbufID);
	}

	Point3 UVW = sc.UVW();
	BMM_Color_fl cl;

	int x, y;
	float u = UVW.x;
	float v = UVW.y;

	float fu = frac(u);
	float fv = 1.0f - frac(v);

	if (fu >= 1.0f)
		fu = 0.0f;
	if (fv >= 1.0f)
		fv = 0.0f;

	if (Bitmap* bm = GetBitmap(); bm != nullptr) {
		x = (int)(fu * bm->Width());
		y = (int)(fv * bm->Height());
		bm->GetLinearPixels(x, y, 1, &cl);
		c.r = cl.r;
		c.g = cl.g;
		c.b = cl.b;
		c.a = cl.a;
	}
	
	sc.PutCache(this, c);
	return c;
}

float MPKFTexture::EvalMono(ShadeContext& sc)
{
	return Intens(EvalColor(sc));
}

Point3 MPKFTexture::EvalNormalPerturb(ShadeContext& /*sc*/)
{
	return Point3(0, 0, 0);
}

ULONG MPKFTexture::LocalRequirements(int subMtlNum)
{
	return 0;
}

void MPKFTexture::ActivateTexDisplay(BOOL /*onoff*/)
{
}

DWORD_PTR MPKFTexture::GetActiveTexHandle(TimeValue /*t*/, TexHandleMaker& /*maker*/)
{
	return 0;
}

int MPKFTextureBitmapNotify::Changed(ULONG flags)
{
	//-- File has been reloaded and it must be reflected as such
	if (flags == BMNOTIFY_FLAG_STORAGE_CHANGE)
	{
		//Texture->NotifyChanged();
		//Texture->DiscardTexHandle();
		//-- File has been changed and must be reloaded
		// GG: 02/10/02
	}
	else if (flags == BMNOTIFY_FLAG_FILE_CHANGE)
	{
		Texture->ReloadActiveBitmap(false);
	}

	return 1;
}

