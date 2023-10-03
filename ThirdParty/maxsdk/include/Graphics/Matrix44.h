//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "DataBridgeExport.h"
#include "Types.h"
#include "../MaxHeap.h"
#include "GraphicsExport.h"
#include <memory.h> // for memcpy()
#include "../geom/point4.h"

class Point3;
class Matrix3;

namespace MaxSDK { namespace Graphics {

class Matrix44 : public MaxHeapOperators
{
public:
	inline Matrix44();
	inline Matrix44(const Matrix44& from);
	inline Matrix44(float mat[4][4]);
	inline Matrix44& operator = (const Matrix44& from);
	inline bool operator==(const Matrix44& M) const;
	inline bool operator!=(const Matrix44& M) const;
	inline ~Matrix44();

	DataBridgeAPI Matrix44& MakeTranspose();
	DataBridgeAPI Matrix44& MakeIdentity();
	DataBridgeAPI float MakeInverse();
	DataBridgeAPI Matrix44& MakeMultiply(const Matrix44& m2);

	DataBridgeAPI Matrix44& MakeTranslation(
		float xOffset, 
		float yOffset, 
		float zOffset);
	DataBridgeAPI Matrix44& MakeTranslation(
		const Point3& offset);
	DataBridgeAPI Matrix44& MakeScale(
		float xScale, 
		float yScale, 
		float zScale);
	DataBridgeAPI Matrix44& MakeScale(
		const Point3& scale);
	DataBridgeAPI Matrix44& MakeRotation(
		const Point3& axis, 
		float rotationRadius);

	DataBridgeAPI Matrix44& MakeLookAt(
		const Point3& eyePos, 
		const Point3& direction, 
		const Point3& approxUp);

	DataBridgeAPI Matrix44& MakeOrthoProjection(
		float w, 
		float h, 
		float znear, 
		float zfar, 
		bool lefthand = true);
	DataBridgeAPI Matrix44& MakeOrthoProjectionLH(
		float w, 
		float h, 
		float znear, 
		float zfar);
	DataBridgeAPI Matrix44& MakeOrthoProjectionRH(
		float w, 
		float h, 
		float znear, 
		float zfar);

	DataBridgeAPI Matrix44& MakePerspectiveProjection(
		float fov, 
		float aspectRatio, 
		float znear, 
		float zfar, 
		bool lefthand = true);
	DataBridgeAPI Matrix44& MakePerspectiveProjectionLH(
		float fov, 
		float aspectRatio, 
		float znear, 
		float zfar);
	DataBridgeAPI Matrix44& MakePerspectiveProjectionRH(
		float fov, 
		float aspectRatio, 
		float znear, 
		float zfar);

	DataBridgeAPI static Matrix44& Multiply(
		Matrix44& mOut, 
		const Matrix44& m1, 
		const Matrix44& m2);

	// warning C4201: nonstandard extension used : nameless struct/union
#pragma warning(disable:4201)
	union {
		struct {
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];
	};
#pragma warning(default:4201)
};

inline Matrix44::Matrix44()
{
	// do nothing.
}

inline Matrix44::Matrix44(float mat[4][4])
{
	memcpy_s(m, sizeof(m), mat, sizeof(m));
}

inline Matrix44::Matrix44(const Matrix44& from)
{
	memcpy_s(m, sizeof(m), from.m, sizeof(m));
}

inline Matrix44& Matrix44::operator = (const Matrix44& from)
{
	if (&from == this)
	{
		return *this;
	}

	memcpy_s(m, sizeof(m), from.m, sizeof(m));

	return *this;
}

//Point4 multiplied by a Matrix44
inline Point4 operator * (const Matrix44& M, const Point4& p)
{
	Point4 result;
	result[0] = M.m[0][0]*p[0] + M.m[1][0]*p[1] + M.m[2][0]*p[2] + M.m[3][0]*p[3];
	result[1] = M.m[0][1]*p[0] + M.m[1][1]*p[1] + M.m[2][1]*p[2] + M.m[3][1]*p[3];
	result[2] = M.m[0][2]*p[0] + M.m[1][2]*p[1] + M.m[2][2]*p[2] + M.m[3][2]*p[3];
	result[3] = M.m[0][3]*p[0] + M.m[1][3]*p[1] + M.m[2][3]*p[2] + M.m[3][3]*p[3];
	return result;
}

//Point3 multiplied by a Matrix44, assuming the Point3 is a Point4 with its w component being 1.0
inline Point4 operator * (const Matrix44& M, const Point3& p)
{
	return M * Point4(p, 1.f);
}

inline bool Matrix44::operator==(const Matrix44& M) const
{
	int i, j;
	for (i = 0; i < 4; ++i)
		for (j = 0; j < 4; ++j)
			if (m[i][j] != M.m[i][j])
				return false;
	return true;
}


bool Matrix44::operator!=(const Matrix44& M) const
{
	return (operator==(M) == false);
}

inline Matrix44::~Matrix44()
{
	// do nothing.
}

/** Convert Matrix3 to Matrix44
\param outMatrix the Matrix44 output
\param inMatrix the source Matrix3
*/
GraphicsDriverAPI void MaxWorldMatrixToMatrix44(
	Matrix44& outMatrix, 
	const Matrix3& inMatrix);

/** Convert Matrix44 to Matrix3
\param outMatrix the Matrix3 output
\param inMatrix the source Matrix44
*/
GraphicsDriverAPI void Matrix44ToMaxWorldMatrix(
	Matrix3& outMatrix,
	const Matrix44& inMatrix);

} } // namespaces
