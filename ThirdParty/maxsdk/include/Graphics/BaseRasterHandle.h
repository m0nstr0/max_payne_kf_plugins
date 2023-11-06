//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "SmartHandle.h"
#include "RenderEnums.h"
#include "..\strclass.h"
#include <wtypes.h>

namespace MaxSDK { namespace Graphics {

/** This is our base graphics raster resource class. 
The graphics raster resource includes TextureHandle and Render Target.
TextureHandle is a texture class for material.
*/
class BaseRasterHandle : public SmartHandle
{
public:
	GraphicsDriverAPI BaseRasterHandle();
	GraphicsDriverAPI BaseRasterHandle(const BaseRasterHandle& from);
	GraphicsDriverAPI BaseRasterHandle& operator = (const BaseRasterHandle& from);
	GraphicsDriverAPI virtual ~BaseRasterHandle();

	GraphicsDriverAPI static Class_ID ClassID();

	/** Initialize the instance of raster handle. A BaseRasterHandle should be 
		initialized before it's used
	\param[in] width the width of target.
	\param[in] height the height of target.
	\param[in] format the format of the target
	\return true if succeed or false otherwise
	*/
	GraphicsDriverAPI virtual bool Initialize(size_t width, size_t height, TargetFormat format);

	/** Returns the width of raster resource.
	\return the raster width.
	*/
	GraphicsDriverAPI size_t GetWidth() const;

	/** Returns the height of raster resource.
	\return the raster height.
	*/
	GraphicsDriverAPI size_t GetHeight() const;

	/** Set the size of the raster handle.
	\param[in] width the width of the raster handle
	\param[in] height the height of the raster handle
	*/
	GraphicsDriverAPI void SetSize(size_t width, size_t height);

	/** Set the format of the raster handle.
	*/
	GraphicsDriverAPI void SetFormat(TargetFormat format);

	/** Get the format of the raster handle.
	\return the format of the raster handle
	*/
	GraphicsDriverAPI TargetFormat GetFormat() const;

	/** Locks the entire raster handle and access the raster data.
	\param[in] accessType data access type
	\param[out] rowPitch byte size of one row
	\param[out] slicePitch byte size of one slice(for texture handle only)
	\return the mapped data of the raster handle
	*/
	GraphicsDriverAPI void* Map(MaxSDK::Graphics::AccessType accessType,
		size_t& rowPitch, size_t& slicePitch);

	/** Unlocks the entire raster handle
	*/
	GraphicsDriverAPI void UnMap();

	/** Copy from another raster handle
	\param[in] sourceRasterHandle the source raster handle
	*/
	GraphicsDriverAPI void CopyFrom(const BaseRasterHandle& sourceRasterHandle);

private:
	virtual bool InitializeTarget();
};

namespace HelperFunctionsBaseRaster
{
	/** Global function to save a raster as a Bitmap (for debugging purpose, this is a slow process)
	\param baseRasterHandle
	\param[in] fullpathFilename the full path filename such as : "d:/MyTexture.png" or "d:\\SubFolder\\MyTexture.exr", we will save it in the file format whose extension is set in this parameter.
	* Make sure you have the rights to save a file in that folder. If the file already exists, we will replace it.
	* To check if the target format used in the BaseRasterHandle can be saved, please use TargetFormatCanBeSaved below.
	* If you use this global function, please link with maxgraphicsObjects.lib since it is defined in that module.
	* \return true if everything went right or false if an error occured in the saving process.
	*/
	MaxGraphicsObjectAPI bool Save(BaseRasterHandle& baseRasterHandle, const MSTR& fullpathFilename);

	/** Global function to know if saving the current target format of the BaseRasterHandle is implemented.
	* If you use this function, please link with MaxGraphicsObjects.lib since it is defined in that module.
	* \param baseRasterHandle
	* \return true if the target format can be saved or false if it is not yet implemented. If it can be saved, you can use the Save function above.
	*/
	MaxGraphicsObjectAPI bool TargetFormatCanBeSaved(const BaseRasterHandle& baseRasterHandle);
}

}}