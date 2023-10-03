//**************************************************************************/
// Copyright (c) 2014 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

// 2014/4/8, Kai Fu Created.
#pragma once

#include "box3.h"
#include "GeomExport.h"

#include <memory>

// This class serves as the accelerating data structure for searching the near Point3
// within the specified radius.
//
namespace detail {
struct Point3OctreeImpl;
}

class Point3Octree
{
public:
	struct OctreeNode
	{
		OctreeNode();
		~OctreeNode();

		OctreeNode* mpChildren[8];
		size_t mPointIndexOffset;
		size_t mPointCnt;
		bool mHasChildFlag;
	};

	// The the caller/owner of Point3Octree instance should guarantee the validity of Point3 array
	// of pPts,
	GEOMEXPORT Point3Octree(Point3* pPts, int ptCnt, int maxAllowedDepth = 16);
	GEOMEXPORT ~Point3Octree();

	GEOMEXPORT Point3Octree(const Point3Octree&) = delete;
	GEOMEXPORT Point3Octree(Point3Octree&&);
	GEOMEXPORT Point3Octree& operator=(const Point3Octree&) = delete;
	GEOMEXPORT Point3Octree& operator=(Point3Octree&&);

	//GEOMEXPORT void LookUp(const Point3& center, float radius, std::vector<size_t>& result) const;
	GEOMEXPORT void LookUp(const Point3& center, float radius, std::unique_ptr<size_t[]>& result, size_t& num_results) const;

private:
	void SplitNode(OctreeNode& node, const Box3& bbox, int depth);
	void SplitIntoSubNodes(OctreeNode pNodes[8], size_t* start, size_t cnt, const Point3& center,
 			int axis, int nodeIdx);

	const std::unique_ptr<detail::Point3OctreeImpl> mImpl;
};
