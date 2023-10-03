//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "../FragmentGraph/Fragment.h"
#include "../../maxapi.h"
#include "../../ifnpub.h"

#define VIEWPORT_FRAGMENT_MANAGER_INTERFACE_ID Interface_ID(0x73415573, 0x52c05573)

namespace MaxSDK { namespace Graphics {

class IActiveShadeFragment;

/** IViewportFragmentManager is used to manage the view fragment graph for viewports.
* The developer can get access to this CORE interface by using
\code
IViewportFragmentManager *pIViewportFragmentManagerManager = GetIViewportFragmentManager();
\endcode
*/
class IViewportFragmentManager : public FPStaticInterface
{
public: 
	/** Get the ActiveShade fragment from the active viewport.
	 \return a IActiveShadeFragment pointer or nullptr is something failed for some reason, such as you are querying this ActiveShadeFragment on a viewport that is not a 3D viewport.
	*/
	virtual IActiveShadeFragment* GetActiveShadeFragmentFromActiveViewport() = 0;

	/** Get the ActiveShade fragment from a given viewport ViewExp.
	 \return a IActiveShadeFragment pointer or nullptr is something failed for some reason, such as you are querying this ActiveShadeFragment on a viewport that is not a 3D viewport.
	*/
	virtual IActiveShadeFragment* GetActiveShadeFragment(ViewExp& viewExp) = 0;

	/** Get the default custom shading fragment class ID.
	* \param[out] bUseSameFragmentForWireframeViews is true if the default fragment also handles wireframe views or false if it does not.
	 \return a classID which is not Class_ID(0,0) if the default custom shading fragment has been set.
	*/
	virtual const Class_ID& GetDefaultCustomShadingFragmentClassID(bool& bUseSameFragmentForWireframeViews)const = 0;

	/** Set a default custom shading fragment class ID that will replace the default illuminance/shading fragment in all viewports.
	* This fragment gets blended with the others fragments automatically. The color target should contain an alpha layer, which can be opaque, 
	* but we use alpha blending so it should always contain an alpha layer.
	* Your fragment must have 2 inputs which are in that order : the color target and the depth target, it must have 2 outputs in that order the color and the depth target.
	* All inputs and output should be a TargetHandle::ClassID(). The color and depth input targets are empty, set at the good format for the viewport and need to be filled by your fragment.
	* If you need call this via Maxscript, please use NitrousGraphicsManager.SetDefaultViewShadeFragment classIdPartA classIdPartB bUseSameFragmentForWireframeViews
	\param[in] classID the customized shading fragment class ID
	\param[in] bUseSameFragmentForWireframeViews should be true if your fragment also handles wireframe views or false if it does not and we will use the default wireframe fragment in that case.
	\return true if succeed or false otherwise.
	*/
	virtual bool SetDefaultCustomShadingFragmentClassID(const Class_ID& classID, bool bUseSameFragmentForWireframeViews) = 0;

	/** Remove any default custom shading fragment class ID previsouly set.
	* If you need call this via Maxscript, please use NitrousGraphicsManager.ResetDefaultViewShadeFragment()
	*/
	virtual void RemoveDefaultCustomShadingFragmentClassID() = 0;
};

/** Helper function to retrieve a pointer on the IViewportFragmentManager interface
* \return a pointer on IViewportFragmentManager, the pointer cannot be null.
*/
inline IViewportFragmentManager* GetIViewportFragmentManager()
{
	return static_cast<IViewportFragmentManager*>(GetCOREInterface(VIEWPORT_FRAGMENT_MANAGER_INTERFACE_ID));
}

}} // namespace MaxSDK::Graphics
