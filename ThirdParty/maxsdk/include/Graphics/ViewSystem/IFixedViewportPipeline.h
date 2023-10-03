// Copyright 2014 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "../FragmentGraph/Fragment.h"

namespace MaxSDK { namespace Graphics {

/** This class provide some facility to add some extended behavior to the fixed 
	render pipeline. For e.g., 3rd party developer may specify their own background 
	fragment, shading fragment for the viewport. 
	Also developer could add post-shading fragments and overlay fragments. Post-shading
	fragments(usually some screen space effects, for e.g., bloom effect or glow effect)
	are directly evaluated right after the evaluation of shaded fragments.
	Overlay fragments which are evaluated at last could be some viewport UI overlay or 
	any other elements to be rendered topmost.
 */
class IFixedViewportPipeline : public MaxHeapOperators
{
public:
	/** Get the count of post shading fragments added.
	*/
	virtual size_t GetNumPostShadingFragments() const = 0;
	/** Get the post shading fragments with index 'i'
	\param[in] i the index of the post shading fragment
	\return the post shading fragment required.
	*/
	virtual FragmentPtr GetPostShadingFragment(size_t i) const = 0;
	/** Insert a new post shading fragments before the post-shading fragment with 
		index 'i'
	\param[in] i the index of the new post shading fragment
	\param[in] pFragment the new post shading fragment
	\return true if succeed or false otherwise.
	*/
	virtual bool InsertPostShadingFragment(size_t i, FragmentPtr pFragment) = 0;
	/** Delete the post-shading fragment with index 'i'
	\param[in] i the index of the post shading fragment to be deleted
	\return true if succeed or false otherwise.
	*/
	virtual bool DeletePostShadingFragment(size_t i) = 0;
	/** Delete a specified post-shading fragment
	\param[in] pFragment the post shading fragment to be deleted
	\return true if succeed or false otherwise.
	*/
	virtual bool DeletePostShadingFragment(FragmentPtr pFragment) = 0;

	/** Get the count of overlay fragments added.
	*/
	virtual size_t GetNumOverlayFragments() const = 0;
	/** Get the overlay fragments with index 'i'
	\param[in] i the index of the overlay fragment
	\return the overlay fragment required.
	*/
	virtual FragmentPtr GetOverlayFragment(size_t i) const = 0;
	/** Insert a new overlay fragments before the post-shading fragment with 
		index 'i'
	\param[in] i the index of the new overlay fragment
	\param[in] pFragment the new overlay fragment
	\return true if succeed or false otherwise.
	*/
	virtual bool InsertOverlayFragment(size_t i, FragmentPtr pFragment) = 0;
	/** Delete the post-shading fragment with index 'i'
	\param[in] i the index of the overlay fragment to be deleted
	\return true if succeed or false otherwise.
	*/
	virtual bool DeleteOverlayFragment(size_t i) = 0;
	/** Delete a specified post-shading fragment
	\param[in] pFragment the overlay fragment to be deleted
	\return true if succeed or false otherwise.
	*/
	virtual bool DeleteOverlayFragment(FragmentPtr pFragment) = 0;

	/** Get the customized background fragment
	*/
	virtual FragmentPtr GetBackgroundFragment() const = 0;
	/** Set a customized background fragment that will replace the default background
		fragment
	\param[in] pFragment the customized background fragment
	\return true if succeed or false otherwise.
	*/
	virtual bool SetBackgroundFragment(FragmentPtr pFragment) = 0;
	/** Restore the default background fragment
	*/
	virtual void RestoreDefaultBackgroundFragment() = 0;

	/** This has been deprecated as of 3ds Max 2024, Please use GetViewportShadingFragment()
	*/
	MAX_DEPRECATED virtual FragmentPtr GetShadingFragment() const = 0;

	/** This has been deprecated as of 3ds Max 2024, Please use SetViewportShadingFragment()
	Set a customized shading fragment that will replace the default illuminance/shading	fragment.
	\param[in] pFragment the customized shading fragment
	\return true if succeed or false otherwise.
	*/
	MAX_DEPRECATED virtual bool SetShadingFragment(FragmentPtr pFragment) = 0;

	/** Get the customized viewport shading fragment
	* \param[out] bUseSameFragmentForWireframe is set by the function to true if the same fragment can also handle wireframe display.
	*/
	virtual FragmentPtr GetViewportShadingFragment(bool& bUseSameFragmentForWireframe) const = 0;

	/** Set a viewport shading fragment that will replace the default illuminance/shading fragment from Nitrous
	* This fragment gets blended with the others fragments automatically. The color target should contain an alpha layer, which can be opaque, 
	* but we use alpha blending so it should always contain an alpha layer.
	* Your fragment must have 2 inputs which are in that order : the color target and the depth target, it must have 2 outputs in that order the color and the depth target.
	* All inputs and output should be a TargetHandle::ClassID(). The color and depth input targets are empty, set at the good format for the viewport and need to be filled by your fragment.
	\param[in] pFragment the customized shading fragment
	\param[in] bUseSameFragmentForWireframe is true if the same fragment can also handle wireframe display.
	\return true if succeed or false otherwise.
	*/
	virtual bool SetViewportShadingFragment(FragmentPtr pFragment, bool bUseSameFragmentForWireframe) = 0;

	/** Restore the default shading fragment, if you use a ShadingFragment or a Viewport Shading fragment, we will remove it to restore what was originally in the viewport.
	*/
	virtual void RestoreDefaultShadingFragment() = 0;
};

} }// namespaces
