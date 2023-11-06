//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//
// Arnold-specific interface definition for objects that contain OpenVDB
// volume grids. This is an extension of the MaxVolumeInterfaceObject that
// contains additional Arnold-specific parameters.
//
//**************************************************************************/

#pragma once

#include "MaxVolumeInterface.h"

/**
Interface ID 
*/
#define ArnoldMaxVolumeInterfaceID Interface_ID(0x313d1d8d, 0x2fe627f2)

/**
Generic interface class used to access Max volumes for rendering, etc.
*/
class ArnoldMaxVolumeInterfaceObject : public MaxVolumeInterfaceObject
{
public:
	ArnoldMaxVolumeInterfaceObject() = default;
	virtual ~ArnoldMaxVolumeInterfaceObject() = default;

	// The following methods retrieve the listed Arnold-specific parameters

	/** Query for volume step size at the given time.
	\par Parameters:
	TimeValue t - The time to use for the data
	\return  the step size for the volume. */
	virtual float GetStepSize(TimeValue t) = 0;

	/** Query for volume step scale at the given time.
	\par Parameters:
	TimeValue t - The time to use for the data
	\return  the step scale for the volume. */
	virtual float GetStepScale(TimeValue t) = 0;

	/** Query for volume bounds padding amount at the given time.
	\par Parameters:
	TimeValue t - The time to use for the data
	\return  the padding amount for the volume bounds. */
	virtual float GetPadding(TimeValue t) = 0;

	/** Query for velocity value at the given time.
	\par Parameters:
	TimeValue t - The time to use for the data
	\return  the velocity for the volume. */
	virtual float GetVelOut(TimeValue t) = 0;

	/** Query for velocity scale at the given time.
	\par Parameters:
	TimeValue t - The time to use for the data
	\return  the velocity scale for the volume. */
	virtual float GetVelScale(TimeValue t) = 0;
};
