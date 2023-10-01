#include "MPKFMaterial.h"
#include "MPKFMaterialDesc.h"
#include "MPKFMaterialParams.h"

MPKFMaterial::MPKFMaterial():
	pblock{ nullptr }
{
	for (int i = 0; i < MPKFMaterial_SUB_TEXMAPS; i++)
		SubTexmaps[i] = nullptr;

	Reset();
}

MPKFMaterial::MPKFMaterial(BOOL loading):
	pblock{ nullptr }
{
	for (int i = 0; i < MPKFMaterial_SUB_TEXMAPS; i++)
		SubTexmaps[i] = nullptr;

	if (!loading)
		Reset();

	//	SetMtlFlag(MTL_HW_MAT_ENABLED | MTL_HW_TEX_ENABLED);
}

MPKFMaterial::~MPKFMaterial()
{
	DeleteAllRefs();
}

ParamDlg* MPKFMaterial::CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp)
{
	static IAutoMParamDlg* mainDlg = GetMPKFMaterialDesc()->CreateParamDlgs(hwMtlEdit, imp, this);
	return mainDlg;
}

BOOL MPKFMaterial::SetDlgThing(ParamDlg* dlg)
{
	return FALSE;
}

void MPKFMaterial::Update(TimeValue t, Interval& valid)
{
}

Interval MPKFMaterial::Validity(TimeValue t)
{
	Interval valid = FOREVER;

	//for (int i = 0; i < NUM_SUBMATERIALS; i++)
	//{
	//	if (submtl[i])
	//		valid &= submtl[i]->Validity(t);
	//}

	//float u;
	//pblock->GetValue(pb_spin, t, u, valid);
	return valid;
}

void MPKFMaterial::Reset()
{
	ivalid.SetEmpty();
	// Always have to iterate backwards when deleting references.
	for (int i = MPKFMaterial_SUB_TEXMAPS - 1; i >= 0; i--)
	{
		if (SubTexmaps[i])
		{
			DeleteReference(i);
			SubTexmaps[i] = nullptr;
		}
		//mapOn[i] = FALSE;
	}

	DeleteReference(MPKFMaterial_PBLOCK_REF);

	GetMPKFMaterialDesc()->MakeAutoParamBlocks(this);
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
		break;
	}
	case REFMSG_TARGET_DELETED:
	{
		if (hTarget == pblock)
		{
			pblock = nullptr;
		}
		//else
		//{
		//	for (int i = 0; i < NUM_SUBMATERIALS; i++)
		//	{
		//		if (hTarget == submtl[i])
		//		{
		//			submtl[i] = nullptr;
		//			break;
		//		}
		//	}
		//}
		break;
	}
	}

	return REF_SUCCEED;
}

RefTargetHandle MPKFMaterial::GetReference(int i)
{
	//if ((i >= 0) && (i < NUM_SUBMATERIALS))
	//	return submtl[i];
	//else 
	if (i == MPKFMaterial_PBLOCK_REF)
		return pblock;
	else
		return nullptr;
}

TSTR MPKFMaterial::GetSubTexmapSlotName(int i, bool localized)
{
	return TSTR(_T(""));
}

TSTR MPKFMaterial::GetSubTexmapTVName(int i, bool localized)
{
	return TSTR(_T(""));
}

void MPKFMaterial::Shade(ShadeContext& sc)
{
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
	//if ((i >= 0) && (i < NUM_SUBMATERIALS))
	//	submtl[i] = (Mtl*)rtarg;
	//else 
	if (i == MPKFMaterial_PBLOCK_REF)
	{
		pblock = (IParamBlock2*)rtarg;
	}
}
