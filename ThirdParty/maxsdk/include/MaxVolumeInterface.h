//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//
// Generic interface definition for objects that contain OpenVDB volume grids.
// Allows such objects to make these grids available to renderers or other
// consumers, via either in-memory data transfers (std::ostringstream) or
// .vdb files.
//
//**************************************************************************/

#pragma once

#include "max.h"

/**
Interface ID
*/
#define MaxVolumeInterfaceID Interface_ID(0xc1320f3, 0x77c8709e)

/**
Generic interface class used to access Max volumes for rendering, etc.
*/
class MaxVolumeInterfaceObject : public BaseInterface
{
public:
	MaxVolumeInterfaceObject() = default;
	virtual ~MaxVolumeInterfaceObject() = default;

	/** Query for primary volume type.
	Some volumes may be primarily level sets; others may be primarily fog volumes. This
	method lets the consumer query for this and, for example, only work with objects which are
	fog volumes.
	\return  true if the primary grid type is a fog volume. */
	virtual bool IsPrimaryGridFogVolume() = 0;

	// Grid access

	/** Fills in the array with the names of the fog volume grids (if any).
	\par Parameters:
	MaxSDK::Array<MSTR>& fogVolumeGridNames (out) - Array is filled in with the names of all fog volume grids
	*/
	virtual void GetFogVolumeGridNames(MaxSDK::Array<MSTR>& fogVolumeGridNames) = 0;

	/** Fills in the array with the names of the level set grids (if any).
	\par Parameters:
	MaxSDK::Array<MSTR>& levelSetGridNames (out) - Array is filled in with the names of all level set grids
	*/
	virtual void GetLevelSetGridNames(MaxSDK::Array<MSTR>& levelSetGridNames) = 0;

	/** Fills in the array with the names of the velocity grids (if any).
	\par Parameters:
	MaxSDK::Array<MSTR>& velocityGridNames (out) - Array is filled in with the names of all grids that represent velocity
	*/
	virtual void GetVelocityGridNames(MaxSDK::Array<MSTR>& velocityGridNames) = 0;

	/** Query the object to see if it has an in-memory OpenVDB grid cache that is capable of streaming
	\return  true if the object has in-memory grids it can stream. */
	virtual bool CanStreamCache() const { return false; }

	/** Ask the volume object to stream all of its grids to a cache.
	\par Parameters:
	std::ostringstream& streamOut (out) - std::stream that will receive the grid data
	TimeValue t - The time to use for the data
	Interval& valid (in/out) - The validity interval of the data (updated for the current time)
	\return  true if the object successfully streamed all grids. */
	virtual bool StreamGrids(std::ostringstream& streamOut, TimeValue t, Interval& valid) { UNUSED_PARAM(streamOut); UNUSED_PARAM(t); UNUSED_PARAM(valid); return false; }

	/** Ask the volume object to stream a specific set of its grids to a cache
	\par Parameters :
	MaxSDK::Array<MSTR>& channelNames (in) - The names of the grids to stream
	std::ostringstream& streamOut(out) - std::stream that will receive the grid data
	TimeValue t - The time to use for the data
	Interval& valid(in/out) - The validity interval of the data (updated for the current time)
	\return  true if the object successfully streamed all grids. */
	virtual bool StreamGrids(const MaxSDK::Array<MSTR>& channelNames, std::ostringstream& streamOut, TimeValue t, Interval& valid) 
	{
		UNUSED_PARAM(channelNames); UNUSED_PARAM(streamOut); UNUSED_PARAM(t); UNUSED_PARAM(valid);
		return false;
	}

	/** Query the object to see if it has its own cache file.
	\return  true if the object has OpenVDB grids on disk in .vdb files. */
	virtual bool HasDiskCache() const { return false; }

	/** If the object has its own cache file, this retrieves its name for a given time along with validity.
	\par Parameters :
	TimeValue t - The time to use for the data
	Interval& valid(in/out) - The validity interval of the data (updated for the current time)
	\return  the name of the OpenVDB .vdb cache file. */
	virtual MSTR GetDiskCache(TimeValue t, Interval& valid) { UNUSED_PARAM(t); UNUSED_PARAM(valid); return _T(""); }

	/** If the object doesn't have its own cache file, this will build one with all grids for us, placing it in the specified filename
	\par Parameters :
	MSTR& filename (in) - The name of the file that should receive the OpenVDB grid data
	TimeValue t - The time to use for the data
	Interval& valid(in/out) - The validity interval of the data (updated for the current time)
	\return  true if the file was saved, false otherwise. */
	virtual bool BuildDiskCache(const MSTR& filename, TimeValue t, Interval& valid) 
	{ 
		UNUSED_PARAM(filename); UNUSED_PARAM(t); UNUSED_PARAM(valid); 
		return false; 
	}

	/** If the object doesn't have its own cache file, this will build one with all specified grids for us, placing it in the specified filename.
	\par Parameters :
	MaxSDK::Array<MSTR>& channelNames (in) - The names of the grids to save
	MSTR& filename (in) - The name of the file that should receive the OpenVDB grid data
	TimeValue t - The time to use for the data
	Interval& valid(in/out) - The validity interval of the data (updated for the current time)
	\return  true if the file was saved, false otherwise. */
	virtual bool BuildDiskCache(const MaxSDK::Array<MSTR>& channelNames, const MSTR& filename, TimeValue t, Interval& valid) 
	{ 
		UNUSED_PARAM(channelNames); UNUSED_PARAM(filename); UNUSED_PARAM(t); UNUSED_PARAM(valid);
		return false; 
	}

	/** Get the shader assigned to this volume (if any).
	\par Parameters :
	TimeValue t - The time to use for the shader
	\return  The material pointer to the shader if the object has one; nullptr otherwise. */
	virtual Mtl* GetVolumeShader(TimeValue t) { UNUSED_PARAM(t); return nullptr; }

	/** Get the bounding box for the volume at the specified time.
	\par Parameters :
	TimeValue t - The time to use
	\return  The bounding box of the object. */
	virtual Box3 GetVolumeBBox(TimeValue t) = 0;

	/** Get the validity interval for the volume at the specified time.
	\par Parameters :
	TimeValue t - The time to use
	\return  The validity interval for the object. */
	virtual Interval VolumeObjectValidity(TimeValue t) { UNUSED_PARAM(t); return FOREVER; }
};

