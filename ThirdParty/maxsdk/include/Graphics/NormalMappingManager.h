//**************************************************************************/
// Copyright (c) 2012 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//

#pragma once

#include "../ifnpub.h"

class ISave;
class ILoad;

// The interface ID for the INormalMappingManager class
#define NORMALMAPPINGMGR_INTERFACE Interface_ID(0x35612d76, 0x154f79ea)

namespace MaxSDK { namespace Graphics {

// Non-technical normal bump terms to align with UI
enum NormalBumpMode {
	//! 3ds Max Mode
	//! In this mode, the vertex tangent and binormal(bitangent) are perpendicular to the face normal.
	//! The face normal is always perpendicular to the triangle face.
	//! During pixel shading, the tangent and binormal interpolated from triangle vertices are used in the normal bump calculation.
	NB_3dsMax = 0,
	//! Maya Mode
	//! In this mode, the vertex tangent and binormal(bitangent) are perpendicular to the vertex normal.
	//! Vertex normals take their smoothing group into account.
	//! During pixel shading, the tangent and binormal interpolated from triangle vertices are orthogonalized against the normal 
	//! before the normal bump calculation.
	NB_Maya,
	//! DirectX Mode
	//! In this mode, the vertex tangent and binormal(bitangent) are perpendicular to the vertex normal.
	//! During pixel shading, the tangent and binormal interpolated from triangle vertices are used in the normal bump calculation.
	NB_DirectX,
	//! MikkT Mode - by Morten S. Mikkelsen
	//! In this mode, the same vertex tangent and binormal(bitangent) are generated even if there is a change in
	//! index list(s), ordering of faces/vertices of a face, and/or the removal of degenerate primitives.
	//! During pixel shading, the tangent interpolated from vertices is used in the normal bump calculation, however,
	//! the binormal(bitangent) could be either interpolated from vertices or re-caluclated against the pixel normal and tangent, depends on
	//! CalculateBitangentPerPixel flag is enabled or not.
	NB_MikkT
};

// Technical normal bump terms. See following mapping between NormalBumpMode and TangentBasisMode:
// kPerpendicularToFaceNormal <=> NB_3dsMax.
// kPerpendicularToVertexNormal + OrthogonalizePerPixel(true) <=> NB_Maya.
// kPerpendicularToVertexNormal + OrthogonalizePerPixel(false) <=> NB_DirectX.
// kMikkT <=> NB_MikkT.
enum TangentBasisMode {
	kPerpendicularToFaceNormal = 0, //!< Tangent basis mode that requires the tangent and binormal are perpendicular to face normal
	kPerpendicularToVertexNormal,   //!< Tangent basis mode that requires the tangent and binormal are perpendicular to vertex normal
	kMikkT                          //!< Tangent basis mode that's aligned with MikkT
};

/*! \brief This interface manages the normal mapping modes that can affect how 3ds Max calculates/interprets the tangent/binormal(bitangent) in 
	vertex/pixel shading level during baking/rendering. This interface is implemented by the system. 
	Plug-ins can get/set the following values on the NormalMappingManager. 
	\li NormalBumpMode, the normal bump mode which can be one of the following: NB_3dsMax, NB_Maya, NB_DirectX, NB_MikkT. The default value is NB_3dsMax. 
	See description for the definition of NormalBumpMode.
	\li TangentBasisMode, the tangent basis mode which can be one of the following: kPerpendicularToFaceNormal, kPerpendicularToVertexNormal, kMikkT. 
	The default value is kPerpendicularToFaceNormal. See description for the definition of TangentBasisMode.
	\li OrthogonalizePerPixel, the flag indicating if Orthogonalize Per Pixel is enabled or disabled.
	\li CalculateBitangentPerPixel, the flag indicating if Calculate Bitangent Per Pixel is enabled or disabled.

	Usage:
	Set 3ds Max to use the mode compatible with Maya to calculate/interpret tangent/binormal(bitangent)
	\code
	INormalMappingManager* nMMgr = static_cast<INormalMappingManager*>(GetCOREInterface(NORMALMAPPINGMGR_INTERFACE));
	nMMgr->SetNormalBumpMode(NB_Maya);
	\endcode
	Set 3ds Max to use the mode compatible with MikkT to calculate/interpret tangent/binormal(bitangent), and enable calculate bitangent per pixel flag.
	\code
	INormalMappingManager* nMMgr = static_cast<INormalMappingManager*>(GetCOREInterface(NORMALMAPPINGMGR_INTERFACE));
	nMMgr->SetNormalBumpMode(NB_MikkT);
	nMMgr->SetCalculateBitangentPerPixel(true);
	\endcode
*/
class INormalMappingManager : public FPStaticInterface
{
public:
	/** Get the normal bump mode used by 3dsmax system
	\returns the normal bump mode
	*/
	virtual NormalBumpMode GetNormalBumpMode() const = 0;

	/** Set the normal bump mode used by 3dsmax system
	\param[in] mode The normal bump mode
	*/
	virtual void SetNormalBumpMode(const NormalBumpMode mode) = 0;

	/** Get the tangent basis mode used by 3dsmax system
	\returns the normal bump mode
	*/
	virtual TangentBasisMode GetTangentBasisMode() const = 0;

	/** Set the tangent basis mode used by 3dsmax system
	\param[in] mode The tangent basis mode
	*/
	virtual void SetTangentBasisMode(const TangentBasisMode mode) = 0;

	/** Get the parameter name of Orthogonalize Per Pixel flag
	\returns the name of Orthogonalize Per Pixel flag
	*/
	virtual const TCHAR* OrthogonalizePerPixelParamName() const = 0;

	/** Get the flag indicating if Orthogonalize Per Pixel is enabled or disabled
	\returns True if Orthogonalize Per Pixel is enabled, False otherwise.
	*/
	virtual bool GetOrthogonalizePerPixel() const = 0;

	/** Set the flag indicating if Orthogonalize Per Pixel is enabled or disabled
	\param[in] on The flag indicating if Orthogonalize Per Pixel is enabled or disabled
	*/
	virtual void SetOrthogonalizePerPixel(bool on) = 0;

	/** Get the parameter name of Calculate Bitangent Per Pixel flag
	\returns the name of Calculate Bitangent Per Pixel flag
	*/
	virtual const TCHAR* CalculateBitangentPerPixelParamName() const = 0;

	/** Get the flag indicating if Calculate Bitangent Per Pixel is enabled or disabled
	\returns True if Calculate Bitangent Per Pixel is enabled, False otherwise.
	*/
	virtual bool GetCalculateBitangentPerPixel() const = 0;

	/** Set the flag indicating if Calculate Bitangent Per Pixel is enabled or disabled
	\param[in] on The flag indicating if Calculate Bitangent Per Pixel is enabled or disabled
	*/
	virtual void SetCalculateBitangentPerPixel(bool on) = 0;

	/** FileIO(Internal Use Only)
	*/
	virtual IOResult Save(ISave* iSave) = 0;
	/** FileIO(Internal Use Only)
	*/
	virtual IOResult Load(ILoad* iLoad) = 0;

protected:
	virtual ~INormalMappingManager() {}

};

} } // namespace
