//**************************************************************************/
// Copyright (c) 2023 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "..\strclass.h"

namespace MaxSDK::ColorManagement {

/** Enum for color space assignment state.  
 * In a modern color management system, color holding entities (such as bitmaps)
 * need to have valid color spaces assigned to them so that they can be
 * converted to and from the working (rendering) color space. Unlike the
 * floating point gamma value used in the simple de-gamma/re-gamma work flows,
 * the assigned color space may not always be a valid value, therefore it's
 * important to keep track of the assigned color space and validity of it all
 * the time. This enum provides different states the color space assignment may
 * be in.
 *
 * \see	BitmapInfo::ColorSpace, BitmapInfo::ColorSpaceStatus
 *
*/
enum class ColSpaceStatus
{
	NotSet = 0,		//!< No color space is assigned yet.
	Normal,			//!< A valid color space is assigned. Things are healthy.
	ForcedDefault,	//!< System has force-assigned a color space probably because a previously assigned color space became unavailable.
	Invalid,		//!< Assigned color space is invalid, things will not work properly. \see BitmapInfo::ValidateColorSpace
};

/** Enum for indicating the source of the assigned color space.  
 * In a modern color management system, color holding entities (such as bitmaps)
 * need to have valid color spaces assigned to them so that they can be
 * converted to and from the working (rendering) color space. This assignment
 * can be done automatically based on the file name or type or can be done
 * explicitly by the users. 3ds %Max keeps track of the source of the color space
 * assignment so that if for some reason color space becomes unavailable (see
 * ColSpaceStatus), an informed decision can be made to re-assign one of the
 * currently available color spaces. Knowing the source of the color space
 * assignment will also make trouble shooting a color space related problems
 * easier.
 *
 * \see BitmapInfo::ColorSpaceSource, BitmapInfo::SetRequestedColorSpace,
 *      BitmapInfo::GetRequestedColorSpaceSource
 */
enum class ColSpaceSource
{
	NotSet = 0,			//!< No color space is assigned yet.
	InputRules,			//!< Color space assigned automatically based on the file name (see \ref IModeSettings::GetColorSpaceFromImageFilePath)
	User,				//!< User has assigned color space manually.
	SystemHeuristics,	//!< System heuristics are used to assign most suitable color space.
};


/** This class is used internally by the BitmapInfo class. Please see color
 * space related functions of the BitmapInfo class for public access to the data
 * this class holds.  
 *
 * \see BitmapInfo, BitmapInfo::ColorSpace, BitmapInfo::ColorSpaceSource,
 *      BitmapInfo::ColorSpaceStatus, BitmapInfo::ColorSpaceRule,
 *      BitmapInfo::GetRequestedColorSpace, BitmapInfo::SetRequestedColorSpace,
 *      BitmapInfo::ValidateColorSpace, BitmapInfo::ResetColorSpaceInfo
 */
class InputColorSpaceInfo
{
public:
	MSTR colorSpaceAssigned;	// may differ from colorSpaceRequested if it's not available.
	MSTR colorSpaceRequested;
	MSTR ruleName;				// if the color space source is "input rules", this field holds the name of the rule that's used.	

	ColSpaceSource colorSpaceRequestSource	= ColSpaceSource::NotSet;
	ColSpaceSource colorSpaceAssignSource	= ColSpaceSource::NotSet;
	ColSpaceStatus colorSpaceStatus			= ColSpaceStatus::NotSet;

public:
	bool operator ==(const InputColorSpaceInfo &b) const
	{
		bool res =
			colorSpaceRequestSource == b.colorSpaceRequestSource &&
			colorSpaceAssignSource == b.colorSpaceAssignSource &&
			colorSpaceStatus == colorSpaceStatus &&
			colorSpaceAssigned.CaseInsensitiveEqual(b.colorSpaceAssigned) && 
			colorSpaceRequested.CaseInsensitiveEqual(b.colorSpaceRequested) && 
			ruleName.CaseInsensitiveEqual(b.ruleName);
			
		return res;
	}

	void Reset()
	{
		colorSpaceRequested = {};
		colorSpaceRequestSource = ColSpaceSource::NotSet;

		ResetAssigned();
	}

	void ResetAssigned()
	{
		colorSpaceAssigned = {};
		colorSpaceAssignSource = ColSpaceSource::NotSet;
		ruleName = {};
		colorSpaceStatus = ColSpaceStatus::NotSet;
	}

};


} // namespace MaxSDK::ColorManagement
