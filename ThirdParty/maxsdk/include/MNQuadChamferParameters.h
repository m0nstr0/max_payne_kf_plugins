//**************************************************************************/
// Copyright (c) 2018 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Interface to MNQuadChamferParameters class
// AUTHOR: Tom Hudson
//**************************************************************************/

#pragma once

#include <WTypes.h>

#include "MaxHeap.h"
#include "mnmesh.h"

/*!
This class is used to provide parameters to the Universal Quad Chamfer interface in \ref IMNMeshUtilities16.

The parameter set includes all applicable parameters for Quad Chamfer operations and allows for specifying version
numbers (See \ref QuadChamferVersionValues). In this way the IMNMeshUtilities16 interface can be used for any Quad
Chamfer operations now and into the future -- Various constructors are provided for ease of use, and as parameters are
added in the future, constructors will be added to access that functionality.
*/

class MNQuadChamferParameters : public MaxHeapOperators
{
protected:
	//! The Quad Chamfer version to use (See \ref QuadChamferVersionValues)
	unsigned int m_version;
	//! The chamfer amount for non-weighted chamfers
	float m_amount;
	//! The chamfer amount for crease == 0.0
	float m_minVarAmount;
	//! The chamfer amount for crease == 1.0
	float m_maxVarAmount;
	//! The number of segments in the chamfers
	int m_segments;
	//! The tension of the chamfer (0 = no tension, 0.5 = rounded, 1.0 = linear)
	float m_tension;
	//! The depth of the chamfer (-1 = concave, 0.0 = linear, 1.0 = convex)
	float m_depth;
	//! The amount of the inset, if in use
	float m_insetAmount;
	//! The number of inset segments
	int m_insetSegments;
	//! The inset offset
	float m_insetOffset;
    //! Biasing for transition along unchamfered edge connecting to a chamfered edge
    float m_miterEndBias;
	//! Chamfer options (See \ref QuadChamferOptions)
	DWORD m_optionFlags;
	//! Chamfer result options (See \ref MN_QCHAM_TYPE)
	MN_QCHAM_TYPE m_resultType;
	//! The MN_XXX flag that indicates edges to chamfer (See \ref GeneralMNMeshComponentFlags)
	DWORD m_flag;
public :
	/*! Constructor for operation as original version (3ds Max 2015)
		\param amount The chamfer amount
		\param segments The number of segments in the chamfers
		\param tension The tension of the chamfer (0 = no tension, 0.5 = rounded, 1.0 = linear)
		\param resultType Chamfer result options (See \ref MN_QCHAM_TYPE)
		\param flag The MN_XXX flag that indicates edges to chamfer (See \ref GeneralMNMeshComponentFlags)
		*/
	MNQuadChamferParameters(float amount, int segments, float tension, MN_QCHAM_TYPE resultType, DWORD flag) :
		m_version(QCHAM_VERSION_ELWOOD),
		m_amount(amount),
		m_minVarAmount(0.0f),
		m_maxVarAmount(0.0f),
		m_segments(segments),
		m_tension(tension),
		m_depth(0.0f),
		m_insetAmount(0.0f),
		m_insetSegments(0),
		m_insetOffset(0.0f),
        m_miterEndBias(0.5f),
		m_optionFlags(QCHAM_OPTION_ELWOOD_DEFAULTS),
		m_resultType(resultType),
		m_flag(flag) {}

	/*! Constructor for operation as Phoenix (Max 2016) version.
	(If QCHAM_OPTION_VARIABLE_EDGE_WEIGHTS option is used, the chamfers will vary from 0.0 at Crease == 0.0 to "amount"
	at Crease == 1.0.) 
	\param amount The chamfer amount 
	\param segments The number of segments in the chamfers 
	\param tension The tension of the chamfer (0 = no tension, 0.5 = rounded, 1.0 = linear) 
	\param optionFlags Chamfer options (See \ref QuadChamferOptions) 
	\param resultType Chamfer result options (See \ref MN_QCHAM_TYPE) 
	\param flag The MN_XXX flag that indicates edges to chamfer (See \ref GeneralMNMeshComponentFlags) 
	\param version The Quad Chamfer version to use (See \ref QuadChamferVersionValues)
	*/
	MNQuadChamferParameters(float amount, int segments, float tension, DWORD optionFlags, MN_QCHAM_TYPE resultType, DWORD flag, int version) :
		m_version(version),
		m_amount(amount),
		m_minVarAmount(0.0f),
		m_maxVarAmount(amount),
		m_segments(segments),
		m_tension(tension),
		m_depth(0.0f),
		m_insetAmount(0.0f),
		m_insetSegments(0),
		m_insetOffset(0.0f),
        m_miterEndBias(0.5f),
		m_optionFlags(optionFlags),
		m_resultType(resultType),
		m_flag(flag) {}
    
	/*! Constructor for operation as 3ds Max 2020 or later
		\param amount The chamfer amount
		\param minAmount When QCHAM_OPTION_VARIABLE_EDGE_WEIGHTS option is used, the chamfer amount at Crease == 0.0
		\param maxAmount When QCHAM_OPTION_VARIABLE_EDGE_WEIGHTS option is used, the chamfer amount at Crease == 1.0
		\param segments The number of segments in the chamfers
		\param tension The tension of the chamfer (0 = no tension, 0.5 = rounded, 1.0 = linear)
		\param depth The depth of the chamfer (-1.0 = concave, 0.0 = linear, 1.0 = convex)
		\param insetAmount The inset amount
		\param insetSegments The number of segments for the inset
		\param insetOffset The inset offset amount
		\param miterEndBias The miter end bias amount
		\param optionFlags Chamfer options (See \ref QuadChamferOptions)
		\param resultType Chamfer result options (See \ref MN_QCHAM_TYPE)
		\param flag The MN_XXX flag that indicates edges to chamfer (See \ref GeneralMNMeshComponentFlags)
		\param version The Quad Chamfer version to use (See \ref QuadChamferVersionValues)
		*/
	MNQuadChamferParameters(float amount, float minAmount, float maxAmount, int segments, float tension, float depth, float insetAmount, int insetSegments, float insetOffset, float miterEndBias, DWORD optionFlags, MN_QCHAM_TYPE resultType, DWORD flag, int version) :
		m_version(version),
		m_amount(amount),
		m_minVarAmount(minAmount),
		m_maxVarAmount(maxAmount),
		m_segments(segments),
		m_tension(tension),
		m_depth(depth),
		m_insetAmount(insetAmount),
		m_insetSegments(insetSegments),
		m_insetOffset(insetOffset),
        m_miterEndBias(miterEndBias),
		m_optionFlags(optionFlags),
		m_resultType(resultType),
		m_flag(flag) {}

    //! Copy constructor.
    MNQuadChamferParameters(const MNQuadChamferParameters& from):
        m_version(from.m_version),
        m_amount(from.m_amount),
        m_minVarAmount(from.m_minVarAmount),
        m_maxVarAmount(from.m_maxVarAmount),
        m_segments(from.m_segments),
        m_tension(from.m_tension),
        m_depth(from.m_depth),
        m_insetAmount(from.m_insetAmount),
        m_insetSegments(from.m_insetSegments),
        m_insetOffset(from.m_insetOffset),
        m_miterEndBias(from.m_miterEndBias),
        m_optionFlags(from.m_optionFlags),
        m_resultType(from.m_resultType),
        m_flag(from.m_flag)
    {
    }

	unsigned int GetVersion() const { return m_version; }
	float GetAmount() const { return m_amount; }
	float GetMinVarAmount() const { return m_minVarAmount; }
	float GetMaxVarAmount() const { return m_maxVarAmount; }
	int GetSegments() const { return m_segments; }
	float GetTension() const { return m_tension; }
	float GetDepth() const { return m_depth; }
	float GetInsetAmount() const { return m_insetAmount; }
	int GetInsetSegments() const { return m_insetSegments; }
	float GetInsetOffset() const { return m_insetOffset; }
    float GetMiterEndBias() const { return m_miterEndBias; }
	DWORD GetOptionFlags() const { return m_optionFlags; }
	MN_QCHAM_TYPE GetResultType() const { return m_resultType; }
	DWORD GetFlag() const { return m_flag; }
};

/*! 
This is a simple derived class of MNQuadChamferParameters used to encapsulate the new parameters added to the Universal Quad
Chamfer interface in IMNMeshUtilities17. These parameters represent additional functionality of the underlying quad chamfer
operations introduced in 3ds Max 2020 Update 1, and should eventually be moved into the main MNQuadChamferParameters class
above when SDK considerations allow.
*/
class MNQuadChamferExtendedParameters: public MNQuadChamferParameters
{
public:

    /*! Constructor with each parameter explicitly specified.
      \param amount The chamfer amount
      \param minAmount When QCHAM_OPTION_VARIABLE_EDGE_WEIGHTS option is used, the chamfer amount at Crease == 0.0
      \param maxAmount When QCHAM_OPTION_VARIABLE_EDGE_WEIGHTS option is used, the chamfer amount at Crease == 1.0
      \param segments The number of segments in the chamfers
      \param tension The tension of the chamfer (0 = no tension, 0.5 = rounded, 1.0 = linear)
      \param depth The depth of the chamfer (-1.0 = concave, 0.0 = linear, 1.0 = convex)
      \param insetAmount The inset amount
      \param insetSegments The number of segments for the inset
      \param insetOffset The inset offset amount
      \param miterEndBias The miter end bias amount
      \param optionFlags Chamfer options (See \ref QuadChamferOptions)
      \param resultType Chamfer result options (See \ref MN_QCHAM_TYPE)
      \param flag The MN_XXX flag that indicates edges to chamfer (See \ref GeneralMNMeshComponentFlags)
      \param version The Quad Chamfer version to use (See \ref QuadChamferVersionValues)
      \param radiusBias: biasing factor controlling the radius of chamfered corners
      \param absWeightScale: scaling factor to be applied to weights when chamfer amount is specified via absolute weighting approach
    */
    MNQuadChamferExtendedParameters(float amount, float minAmount, float maxAmount, int segments, float tension, float depth, float insetAmount, int insetSegments, float insetOffset, float miterEndBias, DWORD optionFlags, MN_QCHAM_TYPE resultType, DWORD flag, int version, float radiusBias, float absWeightScale):
		MNQuadChamferParameters(amount, minAmount, maxAmount, segments, tension, depth, insetAmount, insetSegments, insetOffset, miterEndBias, optionFlags, resultType, flag, version),
        m_radiusBias(radiusBias),
        m_absWeightScale(absWeightScale)
    {
    }

    /*! Constructor with base class parameters specified via an MNQuadChamferParameters object.
        \param baseParams: MNQuadChamferParameters object specifying all base-class parameters
        \param radiusBias: biasing factor controlling the radius of chamfered corners
        \param absWeightScale: scaling factor to be applied to weights when chamfer amount is specified via absolute weighting approach
    */
    MNQuadChamferExtendedParameters(const MNQuadChamferParameters& baseParams, float radiusBias, float absWeightScale):
		MNQuadChamferParameters(baseParams),
        m_radiusBias(radiusBias),
        m_absWeightScale(absWeightScale)
    {
    }

    /*! Constructor for 3ds Max 2020 and previous versions.
        \param baseParams: MNQuadChamferParameters object specifying all base-class parameters
    */
    MNQuadChamferExtendedParameters(const MNQuadChamferParameters& baseParams):
		MNQuadChamferParameters(baseParams),
        m_radiusBias(0.0f),
        m_absWeightScale(1.0f)
    {
    }

    //! Returns the radius bias factor.
    float GetRadiusBias() const { return m_radiusBias; }

    // ! Returns the absolute weight scale.
    float GetAbsWeightScale() const { return m_absWeightScale; }

protected:

    //! Biasing factor controlling the radius of chamfered corners
    float m_radiusBias;

    //! Scaling factor to be applied to weights when chamfer amount is specified via absolute weighting approach
    float m_absWeightScale;
};

/*!
This class encapsulates the parameters defining the quad vertex chamfer operation accessed via the interface defined in
\ref IMNMeshUtilities18. Definitions of each parameter are given below.
*/
class MNQuadVertexChamferParameters: public MaxHeapOperators
{

// Public member functions:
public:

    /*! Constructor with each parameter explicitly specified.
    \param flag: the MN_XXX flag that indicates vertices to chamfer (see \ref GeneralMNMeshComponentFlags)
    \param optionFlags: array of bits representing chamfer options; here, only QCHAM_OPTION_LIMIT_EFFECT is checked
    \param resultType: specifies the geometry to be generated
    \param segments: number of line segments to use to connect the chamfered edge ends with the chamfered corner point
    \param amount: chamfer amount by which the edge endpoints at a vertex should be pushed back from the original vertex
    \param depth: depth value, specifying the shape of the chamfered surfaces
    \param absWeightScale: scaling factor to be applied to weights when chamfer amount is specified via absolute weighting
                           approach
    \param angleFactorVertex: specifies the angles between the original edges at a chamfered vertex, and the lines connecting
                              the chamfered edge endpoints and the chamfered corner point for the unit depth case
    */
    MNQuadVertexChamferParameters(DWORD flag, DWORD optionFlags, MN_QCHAM_TYPE resultType, int segments, float amount, float depth, float absWeightScale, float angleFactorVertex):
        m_flag(flag),
        m_optionFlags(optionFlags),
        m_resultType(resultType),
        m_segments(segments),
        m_amount(amount),
        m_depth(depth),
        m_absWeightScale(absWeightScale),
        m_angleFactorVertex(angleFactorVertex)
    {
    }

    // ! Accessor for m_flag.
    DWORD GetFlag() const
    {
        return m_flag;
    }

    // ! Accessor for m_optionFlags.
    DWORD GetOptionFlags() const
    {
        return m_optionFlags;
    }

    // ! Accessor for m_resultType.
    MN_QCHAM_TYPE GetResultType() const
    {
        return m_resultType;
    }

    // ! Accessor for m_segments.
    int GetSegments() const
    {
        return m_segments;
    }

    // ! Accessor for m_amount.
    float GetAmount() const
    {
        return m_amount;
    }

    // ! Accessor for m_depth.
    float GetDepth() const
    {
        return m_depth;
    }

    // ! Accessor for m_absWeightScale.
    float GetAbsWeightScale() const
    {
        return m_absWeightScale;
    }

    // ! Accessor for m_angleFactorVertex.
    float GetAngleFactor() const
    {
        return m_angleFactorVertex;
    }

// Private variables:
private:

	/* ! Specifies the flag indicating a vertex targeted for chamfering
    */
    DWORD m_flag;

    /* ! Array of bits representing chamfer options; here, the bit corresponding to QCHAM_OPTION_LIMIT_EFFECT is checked to
         determine whether limiting has been requested
    */
    DWORD m_optionFlags;

    /* ! Specifies the geometry to be generated; may be only the chamfered surfaces, everything but the chamfered surfaces (an
         "open" chamfer), or both 
    */
    MN_QCHAM_TYPE m_resultType;

    /* ! Number of line segments to use to connect the chamfered end positions of affected original mesh edges with the central
         vertex of the arcs replacing affected original mesh face corners (i.e., with the chamfered corner point position)
    */
    int m_segments;

    /* ! Chamfer amount, interpreted in the context of a vertex chamfer as the amount by which the edge endpoints at a
         chamfered vertex should be pushed back from the original vertex position along the negative sense of the edge vector
    */
    float m_amount;

    /* ! Depth value, specifying the shape of the chamfered surfaces
    */
    float m_depth;

    /* ! Scaling factor to be applied to weights when chamfer amount is specified via absolute weighting approach
    */
    float m_absWeightScale;

    /* ! Angle factor, specifiying for the unit depth case the angles between the original edges at a chamfered vertex, and the
         lines connecting the chamfered edge endpoints and the chamfered corner point; at m_angleFactorVertex = 0.0, the angles
         computed from the zero depth case are used, so that the zero depth corner point is recovered, while at
         m_angleFactorVertex = 1.0, a zero angle is imposed, so that the chamfered corner point coincides with the original
         vertex, with linear interpolation between these conditions for value on the range (0.0, 1.0)
    */
    float m_angleFactorVertex;
};
