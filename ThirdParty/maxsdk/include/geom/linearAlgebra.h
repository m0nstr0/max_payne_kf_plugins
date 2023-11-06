//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Linear algebra functions
//              Fundamental point and geometry distance and intersection queries.
//              Handles integer and float, 2D and 3D point types.
//              Be aware that integer types can easily overflow (e.g. where we call
//              DotProd and CrossProduct2D), if using values on the order of
//              16 bits or above.  We could alleviate this by using 64 bits for
//              intermediate calculations.
// AUTHOR: David MacDonald
//**************************************************************************/

#pragma once

#include <algorithm> // std::max
#include "IPoint2.h"
#include "IPoint3.h"
#include "Point2.h"
#include "Point3.h"

/*! \remarks Traits information for all point classes */

template <class PointType>
class PointTraits {
public:
};

/*! \remarks Traits information IPoint2 */
template <>
class PointTraits< IPoint2 > {
public:
	typedef int     StorageType;
	typedef IPoint2 PointType2D;
};

/*! \remarks Traits information IPoint3 */
template <>
class PointTraits< IPoint3 > {
public:
	typedef int     StorageType;
	typedef IPoint2 PointType2D;
};

/*! \remarks Traits information Point2 */
template <>
class PointTraits< Point2 > {
public:
	typedef float  StorageType;
	typedef Point2 PointType2D;
};

/*! \remarks Traits information Point3 */
template <>
class PointTraits< Point3 > {
public:
	typedef float   StorageType;
	typedef Point2  PointType2D;
};

/*! \remarks Interpolates between two points. */
template <class PointType>
inline PointType Interpolate(
	const PointType  &P0,
	const PointType  &P1,
	const float       alpha)
{
	return P0 * (1.0f - alpha) + P1 * alpha;
}

/*! \remarks Interpolates between two points.  Specialization to handle rounding of integer type */
template <>
inline IPoint2 Interpolate(
	const IPoint2  &P0,
	const IPoint2  &P1,
	const float     alpha)
{
	return IPoint2(int(std::floor(P0.x * (1.0 - alpha) + P1.x * alpha + 0.5)),
	               int(std::floor(P0.y * (1.0 - alpha) + P1.y * alpha + 0.5)));
}

/*! \remarks Interpolates between two points.  Specialization to handle rounding of integer type */
template <>
inline IPoint3 Interpolate(
	const IPoint3  &P0,
	const IPoint3  &P1,
	const float     alpha)
{
	return IPoint3(int(std::floor(P0.x * (1.0 - alpha) + P1.x * alpha + 0.5)),
	               int(std::floor(P0.y * (1.0 - alpha) + P1.y * alpha + 0.5)),
	               int(std::floor(P0.z * (1.0 - alpha) + P1.z * alpha + 0.5)));
}

/*! \remarks Returns the 2D cross product of two vectors */
template <class PointType>
inline typename PointTraits< PointType >::StorageType CrossProduct2D(const PointType &a, const PointType &b)
{
	return a.x * b.y - a.y * b.x;
}

/*! \remarks Returns the signed distance of a point relative to a line.
Positive sign is in the perpendicular direction counterclockwise from the line direction */ 
template <class PointType>
inline float SignedDistanceToLine2D(
	const PointType &P,
	const PointType &P0,
	const PointType &P1)
{
	const Point2 v(P1 - P0);
	const Point2 w( P - P0);
	const double lengthV = v.Length();  // use double precision even for integer points
	if (lengthV == 0.0)
	{
		return w.Length();
	}
	else
	{
		const double vCrossW = CrossProduct2D(v, w);

		// No danger of overflow because vCrossW is at most length(v) * length(w), so we
		// can safely divide by length(v), even if this is near zero.
		return vCrossW / lengthV;
	}
}

/*! \remarks Returns the squared distance between a point and a line segment */
template <class PointType>
inline float SquaredDistanceToSegment(
	const PointType &P,
	const PointType &P0,
	const PointType &P1)
{
	const PointType v = P1 - P0;
	const PointType w =  P - P0;
	const double wDotV = w.DotProd(v);  // use double precision even for integer points
	if (wDotV <= 0.0)
	{
		return w.LengthSquared();  // Nearest point is P0
	}

	const double vDotV = v.DotProd(v);
	if (wDotV >= vDotV)
	{
		return LengthSquared(P - P1); // Nearest point is P1
	}
	else
	{
		const double wDotW = w.DotProd(w);
		// No danger of overflow since vDotv > wDotV and wDotV > 0
		return static_cast< float >(std::max(wDotW - wDotV * wDotV / vDotV, 0.0));
	}
}

/*! \remarks Returns the distance between a point and a line segment */
template <class PointType>
inline float DistanceToSegment(
	const PointType &P,
	const PointType &P0,
	const PointType &P1)
{
	return std::sqrt(SquaredDistanceToSegment(P, P0, P1));
}

/*! \remarks Helper for finding the closest point to a line segment.
Handles all degenerate and near-degenerate cases.  */
template <class PointType>
inline PointType ClosestPointOnSegmentGivenProjection(
	const PointType                                &P0,
	const PointType                                &P1,
	typename PointTraits< PointType >::StorageType  projectionAmount,
	typename PointTraits< PointType >::StorageType  edgeLengthSquared)
{
	if (projectionAmount <= 0)
	{
		return P0;  // segment is zero length or point is to the left of segment
	}
	else if (projectionAmount >= edgeLengthSquared)
	{
		return P1;  // point is to the right of the segment
	}
	else {
		// The segment is non-zero length, and the closest point is strictly between the two end points.
		const double alpha = static_cast< double >(projectionAmount) / static_cast< double >(edgeLengthSquared);
		return Interpolate(P0, P1, alpha);
	}
}

/*! \remarks Uses the 2D portions of the points to determine the closest point on a segment.
The type of the P argument can be different from that of the segment arguments.
For integer point types, the closest point components will be rounded to the nearest integer,
so the return value might not be exactly on the segment. */
template <class PointType, class PointType2>
inline PointType ClosestPointOnSegment2D(
	const PointType2 &P,
	const PointType  &P0,
	const PointType  &P1)
{
	typedef typename PointTraits< PointType >::StorageType StorageType;
	typedef typename PointTraits< PointType >::PointType2D PointType2D;

	// Get the two vectors defining the geometry, extracting the 2D portions from the possibly 3D input types
	const PointType2D edgeDir2D    (P1.x - P0.x, P1.y - P0.y);
	const PointType2D dirToTarget2D( P.x - P0.x, P.y - P0.y);

	// Project the vector onto the edge and get the squared length of the edge.
	const StorageType projectionAmount  = edgeDir2D.DotProd(dirToTarget2D);
	const StorageType edgeLengthSquared = edgeDir2D.DotProd(edgeDir2D);

	return ClosestPointOnSegmentGivenProjection(P0, P1, projectionAmount, edgeLengthSquared);
}

/*! \remarks Returns the closest point on a segment. Handles 2D or 3D points.
For integer point types, the closest point components will be rounded to the nearest integer,
so the return value might not be exactly on the segment. */
template <class PointType>
inline PointType ClosestPointOnSegment(
	const PointType &P,
	const PointType &P0,
	const PointType &P1)
{
	typedef typename PointTraits< PointType >::StorageType StorageType;

	// Get the two vectors defining the geometry
	const PointType edgeDir     = P1 - P0;
	const PointType dirToTarget =  P  - P0;

	// Project the vector onto the edge and get the squared length of the edge.
	const StorageType projectionAmount  = edgeDir.DotProd(dirToTarget);
	const StorageType edgeLengthSquared = edgeDir.DotProd(edgeDir);

	// Now interpolate along the edge based on the projection amount relative to the edge length
	return ClosestPointOnSegmentGivenProjection(P0, P1, projectionAmount, edgeLengthSquared);
}

/*! \remarks Returns true if the segment intersects the circle/sphere.
Point type can be integer or float, 2D or 3D. */
template <class PointType>
inline bool SegmentIntersectsCircleOrSphere(
	const PointType                                &P0,
	const PointType                                &P1,
	const PointType                                &centre,
	typename PointTraits< PointType >::StorageType  radius)
{
	// Find the closest distance from the centre to the segment
	const float squaredDistanceCentreToSegment = SquaredDistanceToSegment(centre, P0, P1);

	// If this distance is less than or equal to the radius, we have an intersection
	return squaredDistanceCentreToSegment <= static_cast< float >(radius * radius);
}

/*! \remarks Returns true if the point is on or inside the triangle.
Point type can be integer or float, 2D or 3D, but only the 2D portions of the points are used
to determine inside status.  Handles all degenerate and near-degenerate cases.
If the PointType is IPoint2 or IPoint3, integer overflow can occur if the values are not
well within 16 bit range.  */
template <class PointType>
inline bool  PointInsideTriangle2D(
	const PointType &P,
	const PointType &V0,
	const PointType &V1,
	const PointType &V2)
{
	typedef typename PointTraits< PointType >::StorageType   StorageType;
	typedef typename PointTraits< PointType >::PointType2D   PointType2D;

	// Because the input types might be 3D, extract the 2D portions
	const PointType2D edge0   (V1.x - V0.x, V1.y - V0.y);
	const PointType2D edge1   (V2.x - V0.x, V2.y - V0.y);
	const PointType2D PMinusV0( P.x - V0.x,  P.y - V0.y);

	// Get the signed area of the triangle
	const StorageType signedArea = CrossProduct2D(edge0, edge1);

	// Get the relative distance of the point from each edge
	const StorageType signedDistanceEdge0 = edge0.x * PMinusV0.y - edge0.y * PMinusV0.x;
	const StorageType signedDistanceEdge1 = PMinusV0.x * edge1.y - PMinusV0.y * edge1.x;
	const StorageType signedDistanceEdge2 = signedArea - signedDistanceEdge0 - signedDistanceEdge1;

	if (signedArea > 0)
	{
		// Triangle is ccw and non-degenerate.  Check on or inside all 3 edges.
		return signedDistanceEdge0 >= 0 && signedDistanceEdge1 >= 0 && signedDistanceEdge2 >= 0;
	}
	else if (signedArea < 0)
	{
		// Triangle is cw and non-degenerate.  Check on or inside all 3 edges.
		return signedDistanceEdge0 <= 0 && signedDistanceEdge1 <= 0 && signedDistanceEdge2 <= 0;
	}
	else {
		// Triangle is degenerate

		if (signedDistanceEdge0 != 0 || signedDistanceEdge1 != 0)
		{
			// The triangle is colinear and the point is not on the infinite line through the edge segments
			return false;
		}

		// Get the length of the 2 edges and the projection of the point onto the 2 edges
		const StorageType edge0LengthSquared = edge0.LengthSquared();
		const StorageType edge1LengthSquared = edge1.LengthSquared();


		if (edge0LengthSquared != 0)
		{
			if (edge1LengthSquared != 0)
			{
				// Both edges are non-zero length, so the point could be on either one of them
				const StorageType PDotEdge0 = PMinusV0.DotProd(edge0);
				const StorageType PDotEdge1 = PMinusV0.DotProd(edge1);
				return 0 <= PDotEdge0 && PDotEdge0 <= edge0LengthSquared ||
				       0 <= PDotEdge1 && PDotEdge1 <= edge1LengthSquared;
			}
			else
			{
				// Edge 1 is zero length, so just check edge 1
				const StorageType PDotEdge0 = PMinusV0.DotProd(edge0);
				return 0 <= PDotEdge0 && PDotEdge0 <= edge0LengthSquared;
			}
		}
		else
		{
			if (edge1LengthSquared != 0)
			{
				// Edge 0 is zero length, so just check edge 2
				const StorageType PDotEdge1 = PMinusV0.DotProd(edge1);
				return 0 <= PDotEdge1 && PDotEdge1 <= edge1LengthSquared;
			}
			else
			{
				// All 3 vertices equal, just check if the point is equal P0
				return PMinusV0.x == 0 && PMinusV0.y == 0;
			}
		}
	}
}
