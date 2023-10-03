//*********************************************************************/
// Copyright (c) 1998-2020 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//*********************************************************************/

#pragma once
#include "baseinterface.h"

namespace MaxSDK {

/** \brief Interface ID for the IMaterialBrowserEntryInstanceCallback
 * Interface 
 * If a plug-in implements the IMaterialBrowserEntryInstanceCallback
 * interface, it should return an instance of the class in response to \c
 * GetInterface(MATERIAL_BROWSER_ENTRY_INSTANCE_CALLBACK_INTERFACE_ID),
 * called by the system on the individual texture/material reference.
 * 
 * \see IMaterialBrowserEntryInstanceCallback 
 */
#define MATERIAL_BROWSER_ENTRY_INSTANCE_CALLBACK_INTERFACE_ID Interface_ID(0x2ce87882, 0x3c3f15d6)

/** The IMaterialBrowserEntryInstanceCallback allows you to register a callback
 * that gets executed when a user creates a material or texture from the
 * material browser in the Schematic or Compact Material Editor.
 * 
 * This callback is used, for example, in the Bitmap Texture (see the samples)
 * to launch a file chooser to allow the user to choose a file directly.
 * 
 * To make use of this functionality, the overwritten \c GetInterface(InterfaceID)
 * function of the material/texture has to return an instance of this interface
 * when asked for the MATERIAL_BROWSER_ENTRY_INSTANCE_CALLBACK_INTERFACE_ID,
 * from which OnInteractiveCreation() is being called by the system.
 * 
 * \par Sample Code:
 *      This code is from <b>/maxsdk/samples/materials/bmtex.cpp</b>. It is used
 *      to immediately launch a file chooser, when the user creates a Bitmap
 *      texture.\n\n
 * 
 * \snippet samples\materials\bmtex.cpp BMTex.GetInterface Example
 * 
 * \snippet samples\materials\bmtex.cpp BMTex.OnInteractiveCreation Example
 * 
 * \see The BMTex sample,
 *      MATERIAL_BROWSER_ENTRY_INSTANCE_CALLBACK_INTERFACE_ID 
 */
class IMaterialBrowserEntryInstanceCallback : public BaseInterface
{
public:

	/** This function gets called from the material browser when a user creates
	 * a texture/material interactively. */
	virtual void OnInteractiveCreation() = 0;

	inline Interface_ID GetID() override { return MATERIAL_BROWSER_ENTRY_INSTANCE_CALLBACK_INTERFACE_ID; }
};

} // namespace MaxSDK