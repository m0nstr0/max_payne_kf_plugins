//
// Copyright 2020 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "../maxapi.h"
#include "GraphicsEnums.h"
#include "IViewportViewSetting.h"

#define IVIEWPORT_SETTINGS_2_INTERFACE_ID Interface_ID(0x761840b0, 0x437f40a)

namespace MaxSDK { namespace Graphics {

	/** \brief Viewport Setting For The Nitrous Graphics Viewport
		\note To access this interface from a ViewExp object, please call 
		ViewExp::GetInterface(IVIEWPORT_SETTINGS_2_INTERFACE_ID).
	*/
	class IViewportViewSetting2 : public IViewportViewSetting
	{
	public:
		
		/** Enable/Disable the Bloom effect in a viewport, bloom effect is a per viewport setting.
		*/
		virtual void SetBloomEnabled(bool bEnabled) = 0;
		/** Get the enabled state of Bloom effect
			\return true if Bloom is enabled or false otherwise
		*/
		virtual bool GetBloomEnabled() const = 0;

		/** Set Bloom tint color , using a Point3 instead of Color because we want float components for r,g,b, there is no limit to this parameter
		*/
		virtual void SetBloomTintColor(Point3 c) = 0;

		/** Get the Bloom tint color, using a Point3 instead of Color because we want float components for r,g,b
		*/
		virtual Point3 GetBloomTintColor() const = 0;

		/** Set Bloom tint contribution (multiplier applied on the tint color), there is no limit to this parameter, it can also be negative
		*/
		virtual void SetBloomTintContribution(float tintContrib) = 0;

		/** Get the Bloom tint contribution (multiplier applied on the tint color)
		*/
		virtual float GetBloomTintContribution() const = 0;

		/** Set Bloom Threshold, there is no limit to this parameter, values can also be negative
		*/
		virtual void SetBloomThreshold(float threshold) = 0;
		
		/** Get the Bloom Threshold
		*/
		virtual float GetBloomThreshold() const = 0;

		/** Set Bloom ThresholdSmoothing, is the size of the window to smooth the threshold with a smoothstep function with 
		float brightness = smoothstep(Threshold, Threshold + ThresholdSmoothing, pixelLuminance)
		Doing this smoothstep reduces flickering of the Bloom effect
		There is no limit to this parameter
		*/
		virtual void SetBloomThresholdSmoothing(float smoothWidth) = 0;

		/** Get the Bloom ThresholdSmoothing, is the size of the window to smooth the threshold with a smoothstep function with 
		float brightness = smoothstep(Threshold, Threshold + ThresholdSmoothing, pixelLuminance)
		*/
		virtual float GetBloomThresholdSmoothing() const = 0;

		/** Set Bloom Strength (multiplier on the Bloom effect), there is no limit to this parameter, it can also be negative
		*/
		virtual void SetBloomStrength(float strength) = 0;

		/** Get the Bloom Strength (multiplier on the Bloom effect)
		*/
		virtual float GetBloomStrength() const = 0;

		/** Set Bloom Radius, is the number of mipmaps (smaller sub-textures) used to create the blur effect, the more mipmaps there is, the larger and smoother is the effect.
		This parameter should be greater than 1 and for the mipmap \#n it has a size of your viewport size divided by 2 at the power n, so usually 10 is a visual limit as it fills all your viewport
		*/
		virtual void SetBloomRadius(int numMipmaps) = 0;

		/** Get the Bloom Radius, is the number of mipmaps (smaller sub-textures) used to create the blur effect, the more mipmaps there is, the larger and smoother is the effect
		*/
		virtual int GetBloomRadius() const = 0;

		/** Set Bloom quality as a BloomQuality enum
		*/
		virtual void SetBloomQuality(BloomQuality quality) = 0;

		/** Get the Bloom quality as an enum 
		*/
		virtual BloomQuality GetBloomQuality() const = 0;

		/** Enable/Disable the Bloom dirt map.
		*/
		virtual void SetBloomDirtMapEnabled(bool bEnabled) = 0;
		
		/** Get the enabled state of Bloom dirt map
			\return true if Bloom dirt map is enabled or false otherwise
		*/
		virtual bool GetBloomDirtMapEnabled() const = 0;

		/** Set Bloom dirt map as an image filename (is an image blended with the Bloom effect, also called Lens Schmutz)
		*/
		virtual void SetBloomDirtMapImage(const TSTR& imageFilename) = 0;

		/** Get Bloom dirt map as an image filename
		This will return the string for the bitmap that was set from the UI or by Maxscript, 
		which could be a relative or full path filename. We use the asset manager to try to resolve the path if it is relative.
		*/
		virtual const TSTR& GetBloomDirtMapImage() const = 0;

		/** Get Bloom dirt map as an image full path filename
		This will return the full path filename of the bitmap or "" if the bitmap doesn't exist on this machine or hasn't been set
		The result is cached in a local string so the resolution of any relative filename is not done for every call.
		*/
		virtual const TSTR& GetBloomDirtMapImageCachedFullPathFileName() const = 0;

		/** Set Bloom dirt map contribution (multiplier applied on the dirt map)
		There is no limit to this parameter
		*/
		virtual void SetBloomDirtContribution(float dirtContrib) = 0;

		/** Get Bloom dirt map contribution (multiplier applied on the dirt map)
		*/
		virtual float GetBloomDirtContribution() const = 0;

		/** Get if Bloom is applied on the background or not
		*/
		virtual bool GetBloomApplyOnBackground()const = 0;
		/** Set if Bloom is applied on the background or not
		*/
		virtual void SetBloomApplyOnBackground(bool val) = 0;

		/** Get, if instead of the resulting color buffer with bloom applied, we want to display only the bloom pass (w/o the dirt map applied)
		If GetBloomShowFilterPass() is true or GetBloomShowRadiusPassNumber() is not -1 then SetBloomShowBloomPass is ignored, as the 2 previously mentioned parameters will overwrite GetBloomShowBloomPass
		*/
		virtual bool GetBloomShowBloomPass()const = 0;

		/** Set, if instead of the resulting color buffer with bloom applied, we want to display only the bloom pass (w/o the dirt map applied)
		If GetBloomShowFilterPass() is true or GetBloomShowRadiusPassNumber() is not -1 then SetBloomShowBloomPass is ignored, as the 2 previously mentioned parameters will overwrite GetBloomShowBloomPass
		*/
		virtual void SetBloomShowBloomPass(bool val) = 0;

		/** Get, if instead of the resulting color buffer with bloom applied, we want to display only the filtering pass
		If GetBloomShowRadiusPassNumber() is not -1 then SetBloomShowFilterPass is ignored, as the previously mentioned parameter will overwrite SetBloomShowFilterPass
		*/
		virtual bool GetBloomShowFilterPass()const = 0;

		/** Set, if instead of the resulting color buffer with bloom applied, we want to display only the filtering pass
		If GetBloomShowRadiusPassNumber() is not -1 then SetBloomShowFilterPass is ignored, as the previously mentioned parameter will overwrite SetBloomShowFilterPass
		*/
		virtual void SetBloomShowFilterPass(bool val) = 0;

		/** Get, if instead of the resulting color buffer with bloom applied, we want to display only the n-th radius pass (mipmap sub-texture).
		A value of -1  means we show the result and not any of these radius passes, if the value is greater than GetBloomRadius() then this is ignored.
		*/
		virtual int  GetBloomShowRadiusPassNumber()const = 0;

		/** Set, if instead of the resulting color buffer with bloom applied, we want to display only the n-th radius pass (mipmap sub-texture).
		A value of -1  means we show the result and not any of these radius passes, if the value is greater than GetBloomRadius() then this is ignored.
		*/
		virtual void SetBloomShowRadiusPassNumber(int val) = 0;

		/** Get the enabled state of automatic mode in viewport color pipeline
		\return true if automatic mode is enabled or false otherwise
		*/
		virtual bool GetColorPipelineAutomaticModeEnabled() const = 0;

		/** Set the enabled state of automatic mode in viewport color pipeline
		*/
		virtual void SetColorPipelineAutomaticModeEnabled(bool enable) = 0;

		/** Get if viewport color pipeline affects background or not
			\return true if viewport color pipeline affects background or false otherwise
		*/
		virtual bool GetColorPipelineAffectsBackground() const = 0;

		/** Set if viewport color pipeline affects background or not
		*/
		virtual void SetColorPipelineAffectsBackground(bool affect) = 0;

		/** Get scene linear exposure value to be applied in viewport color pipeline
			\return scene linear exposure value to be applied in viewport color pipeline
		*/
		virtual float GetColorPipelineSceneLinearExposure() const = 0;

		/** Set scene linear exposure value to be applied in viewport color pipeline
		*/
		virtual void SetColorPipelineSceneLinearExposure(float exposure) = 0;

		/** Get post display gamma value applied in viewport color pipeline
			\return post display gamma value applied in viewport color pipeline
		*/
		virtual float GetColorPipelinePostDisplayGamma() const = 0;

		/** Set post display gamma value to be applied in viewport color pipeline
		*/
		virtual void SetColorPipelinePostDisplayGamma(float gamma) = 0;

		/** Get color view transform applied in viewport color pipeline
			\return view color view transform applied in viewport color pipeline
		*/
		virtual MSTR GetColorPipelineViewTransform() const = 0;

		/** Set color view transform to be applied in viewport color pipeline
		*/
		virtual void SetColorPipelineViewTransform(const MSTR& viewTransform) = 0;

		/** Get display name applied in viewport color pipeline
			\return display name applied in viewport color pipeline
		*/
		virtual MSTR GetColorPipelineDisplay() const = 0;

		/** Set display name to be applied in viewport color pipeline
		*/
		virtual void SetColorPipelineDisplay(const MSTR& displaySpace) = 0;

		/** Get rendering color space name applied in viewport color pipeline
			\return rendering color space name applied in viewport color pipeline
		*/
		virtual MSTR GetColorPipelineRenderingSpace() const = 0;
	};
} }// namespaces


