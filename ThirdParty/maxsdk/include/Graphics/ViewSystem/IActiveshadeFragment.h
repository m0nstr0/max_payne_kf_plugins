//**************************************************************************/
// Copyright (c) 2020 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "../../Rendering/Renderer.h"
#include "../GraphicsExport.h"

namespace MaxSDK {namespace Graphics {
	
	/** This class is used to manage ActiveShade in a viewport (called ActiveShadeFragment)
		A view fragment is a part of the viewport drawing system. The view fragments are assembled in a network to form a view graph used by the viewport.
		The ActiveShade fragment is part of these view fragments and is always present in a 3D viewport. There is only one instance per 3D viewport and it can be enabled, 
		meaning ActiveShade is running in this viewport, or disabled : meaning ActiveShade is not running in this viewport.
		To get access to a IActiveShadeFragment*, please do :
		#include <Graphics/ViewSystem/IViewportFragmentManager.h>
		MaxSDK::Graphics::IActiveShadeFragment*  GetIViewportFragmentManager()->GetActiveShadeFragmentFromActiveViewport()
		Link your project with ViewSystem.lib.
	 */
	class IActiveShadeFragment		
	{
	public:
		virtual ~IActiveShadeFragment() {};

		/** Is ActiveShade enabled ?
		\return true if ActiveShade is running in this viewport, false if it is not running.
		*/
		virtual bool IsEnabled(void) const = 0;

		/** Enable or disable ActiveShade in this viewport (meaning respectively run or stop ActiveShade in this viewport)
		\param[in] val a boolean which is true to run ActiveShade in this viewport or false to stop it
		*/
		virtual void Enable(bool val) = 0;

		/** Get the ActiveShade fragment resolution multiplier.
		A multiplier of 0.5 will give a half resolution ActiveShade rendering used and upscaled in the viewport. So a better time performance but resulting in a lower image quality
		This multiplier has to be a positive, non-zero float number.
		\return the value of the multiplier.
		*/
		virtual float GetResolutionMultiplier(void) const = 0;

		/** Set the ActiveShade fragment resolution multiplier.
		A multiplier of 0.5 will give a half resolution ActiveShade rendering used and upscaled in the viewport. So a better time performance but resulting in a lower image quality
		This multiplier has to be a positive, non-zero float number.
		\param[in] factor the value of the multiplier, if the multipler is negative or 0, we set this value to 1.0f.
		*/
		virtual void SetResolutionMultiplier(float factor) = 0;

		/**Set the ActiveShade renderer instance to use for running ActiveShade in this viewport. This has to be an ActiveShade compatible renderer (different from Scanline as it is not supported).
		If none is set, or the renderer instance set is not compatible with ActiveShade, we will use the ActiveShade renderer from the render settings.
		\param[in] pRenderer an instance of an ActiveShade compatible renderer.
		*/
        virtual void SetActiveShadeRenderer(Renderer* pRenderer) = 0;

		/**Get the ActiveShade renderer instance to use for running ActiveShade in this viewport. This can be an ActiveShade renderer taken from the render settings or
		an instance you have set using SetActiveShadeRenderer(Renderer* pRenderer).
		\return an ActiveShade renderer pointer used by this viewport if the ActiveShade fragment is enabled.
		*/
        virtual Renderer* GetActiveShadeRenderer(void)const = 0;
	};
}}// namespaces