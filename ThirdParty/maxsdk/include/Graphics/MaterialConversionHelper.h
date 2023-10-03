//
// Copyright 2020 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "GraphicsExport.h"
#include "BaseMaterialHandle.h"
#include "../Materials/Mtl.h"
#include "../maxTypes.h"

namespace MaxSDK { namespace Graphics {

/** 
* MaterialConversionHelper is a class to convert from a 3ds Max material (Mtl class) into a Viewport material (BaseMaterialHandle class)
* This viewport material can be assigned with :
\code
MaxSDK::Graphics::RenderItemHandle hInstanceItem;
BaseMaterialHandle physMatHandle;
hInstanceItem.SetCustomMaterial(physMatHandle);
\endcode
or if you are a method such as IObjectDisplay2::UpdatePerNodeItems(const UpdateDisplayContext& updateDisplayContext, UpdateNodeContext& nodeContext, IRenderItemContainer& targetRenderItemContainer)
you can do :
\code
//This is the material that will be used with the solid representation of the mesh 
//as opposed to the wireframe material which is what is used in wireframe.
nodeContext.GetRenderNode().SetSolidMaterial(physMatHandle);
\endcode
* 
* Examples of usage :
* \code
*	//Create a the Nitrous equivalent of a physical material with a bitmap texture as its base color.
*	MSTR texturePath;
*	if (GetSpecDir(APP_MAP_DIR, _T("Maps"), texturePath)){
*		texturePath += TSTR(_T("/uv-grid.png")); //Use UV grid to get something
*	}
*	//Create a BitmapTex which holds the bitmap
*	BitmapTex *pBitmapTex = NewDefaultBitmapTex();
*	pBitmapTex->SetMapName(texturePath);
*	pBitmapTex->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);//Activate its display in the viewport
*	pBitmapTex->ActivateTexDisplay(TRUE); //activate it
*	pBitmapTex->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
*
*	Interval valid;
*	valid.SetInfinite();
*	
*	//Create a physical material and get its Nitrous material equivalent
*	BaseMaterialHandle physMatHandle ;
*	Mtl* pPhysicalMaterial = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, PHYSICALMATERIAL_CLASS_ID);
*	pPhysicalMaterial->SetName(_T("PhysMat1"));
	const int mapSlot = GetPrimaryMapSlot(pPhysicalMaterial);
	pPhysicalMaterial->SetSubTexmap(mapSlot, pBitmapTex);
*	pPhysicalMaterial->Update(t, valid);
*	//debug if needed, put the created material to compact material editor slot #0
*	//GetCOREInterface()->PutMtlToMtlEditor(pPhysicalMaterial, 0);
*	//Convert to Nitrous the physical material
*	physMatHandle = MaxSDK::Graphics::MaterialConversionHelper::ConvertMaxToNitrousMaterial(*pPhysicalMaterial, t);
*
*	//Another example :
*	//Create a physical material from a preset
*	BaseMaterialHandle physicalMaterialHandleFromGoldPreset;
*	const MSTR presetName (_T("Gold Polished")); //another example is const MSTR presetName (_T("Aluminium Matte"));
*	MaxSDK::Graphics::MaterialConversionHelper::GetNitrousMaterialFromPhysicalMaterialPreset(physicalMaterialHandleFromGoldPreset, presetName, t);
* \endcode
*	You can also check the code sample from the instancing API in <b>maxsdk\\howto\\objects\\InstancedObject\\Instancing.cpp in the function Instancing::CreateAndConvertAStandardMaterial </b>.
* 
* <b>Link with MaxGraphicsObjects.lib to use this API</b>.
*/
namespace MaterialConversionHelper
{
	/** Convert a 3ds Max Mtl into a viewport BaseMaterialHandle
	\param[in] mtl : the 3ds Max material.
	\param[in] t : the time at which you want the conversion to happen (could be different from the current time).
	\param[in] bRealisticMode : set it to true if the 3ds Max material is using Bump/Normal mapping or needs to be at the maximum quality available in the viewport.
								Be aware that this quality will stay the same even when switching to standard or high quality viewports.
	\return a BaseMaterialHandle
	*/
	MaxGraphicsObjectAPI  BaseMaterialHandle ConvertMaxToNitrousMaterial(Mtl& mtl, TimeValue t, bool bRealisticMode);

	/** Create a viewport BaseMaterialHandle from a Physical material preset
	\param[out] outBaseMaterialHandle : a BaseMaterialHandle which can be used with RenderItemHandle::SetCustomMaterial()
	\param[in] presetName : the name used in the preset, you can find the preset names from en-US\Plugcfg\PhysicalMaterialTemplates.ini 
		But <b>when there is a composite name like Polished Gold" it's usually the reverse order like "Gold Polished", in the .ini, 
		the actual name is what is after PhysicalTemplate_ActiveMaterial in the .ini.</b>
	\param[in] t : the time at which you want to conversion to happen (could be different from the current time).
	\param[in] bRealisticMode : set it to true if the 3ds Max material is using Bump/Normal mapping or needs to be at the maximum quality available in the viewport.
								Be aware that this quality will stay the same even when switching to standard or high quality viewports.
	\return true if the preset name was found and the BaseMaterialHandle correctly filled.
	*/
    MaxGraphicsObjectAPI bool GetNitrousMaterialFromPhysicalMaterialPreset(	BaseMaterialHandle& outBaseMaterialHandle, const MSTR& presetName, TimeValue t, bool bRealisticMode);
}

} } // namespace
