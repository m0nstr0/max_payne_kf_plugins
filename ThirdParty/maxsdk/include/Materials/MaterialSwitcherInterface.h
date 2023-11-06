#pragma once

/**********************************************************************
 *<
   FILE: MaterialSwitcherInterface.h

   DESCRIPTION:  Material Switcher Interface File

   CREATED BY: Mathew Kaustinen

   HISTORY: Created 4 August 2022

 *>   Copyright (c) 2022 Autodesk, Inc.
 **********************************************************************/

#pragma once

#include "..\max.h"
#include "..\imtl.h"

namespace MaxSDK {

// ClassID for Material Switcher Class
#define MATERIAL_SWITCHER_CLASS_ID Class_ID(0x4ecd74a6, 0x0)

// Interface for Material Switcher
#define MTL_SWITCHER_ACCESS_INTERFACE Interface_ID(0x4ecd74a6, 0x2)

// Interface for 3rd party Renderers
class MtlSwitcherInterface : public BaseInterface
{
public:
	// Return the active material
	virtual Mtl* GetActiveMtl()
	{
		return nullptr;
	}

	virtual Mtl* GetActiveMtl(TimeValue /*t*/, Interval& ivalid)
	{
		ivalid = FOREVER;
		return nullptr;
	}
};

} // namespace MaxSDK
