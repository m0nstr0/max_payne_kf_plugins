//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: OpenVDB Interface
// AUTHOR: Tom Hudson
// HISTORY: created December 10, 2020
//***************************************************************************/

#pragma once

#include "max.h"
#include <vector>

#define OpenVDBInterfaceID Interface_ID(0x198b0c3e, 0x6a1b6f15)

// Default OpenVDB HalfWidth
#define DEFAULT_VDB_HALFWIDTH 3.0f

typedef int VDBHandle;

class MaxOpenVDBInterface : public FPInterfaceDesc
{
public:
	enum FilterType {
		kFilterTypeGaussian = 0,
		kFilterTypeMedian,
		kFilterTypeMean,
		// Must be last 
		kTotalFilterTypes
	};
	enum BooleanOperationType {
		kBooleanTypeUnion = 0,
		kBooleanTypeAMinusB,
		kBooleanTypeIntersection,
		// Must be last 
		kTotalBooleanOperationTypes
	};
	enum VDBGridType {
		kGridTypeUnknown = 0,
		kGridTypeLevelSet = 1,
		kGridTypeFogVolume = 2,
		kGridTypeStaggered = 3,
		// Must be last 
		kTotalVDBGridTypes
	};
	MaxOpenVDBInterface() : FPInterfaceDesc(OpenVDBInterfaceID, _T("OpenVDBInterface"), 0, NULL, FP_CORE, p_end) {}
    virtual ~MaxOpenVDBInterface() {}

	// Volume functions

	/** Prepare an OpenVDB volume and return a handle to it
	\par Parameters:
	\param handle (out) Unique handle that will refer to this volume. Use DisposeVolume to dispose of the volume when you are finished with it.
	NOTE: All volumes are automatically disposed of on Max reset.
	\param voxelSize (in) The voxel size that this volume uses
	\param halfWidth (in) The half-width for the level set
	\return true if successful, false if not
	*/
	virtual bool PrepareVolume(VDBHandle& handle, float voxelSize = 1.0f, float halfWidth = DEFAULT_VDB_HALFWIDTH) = 0;
	/** Copy one OpenVDB volume to another
	\par Parameters:
	\param fromHandle The handle of the VDB volume to clone
	\param cloneHandle (out) Unique handle that will refer to the volume's clone
	\return true if successful, false if not
	*/
	virtual bool CloneVolume(VDBHandle fromHandle, VDBHandle& cloneHandle) = 0;
	/** Retrieve information from an OpenVDB .vdb file
	\par Parameters:
	\param filename (in) The .vdb file name
	\param numberOfGrids Returns the number of grids in the .vdb file
	\param gridTypes Returns a table with a grid type per grid in the file \see VDBGridType
	\return true if successful, false if not
	*/
	virtual bool VolumeFileInfo(const TSTR& filename, int& numberOfGrids, IntTab& gridTypes) = 0;
	/** Loads a volume grid from a .vdb file
	\par Parameters:
	\param handle (in) Handle of the VDB volume that will receive the loaded grid
	\param filename (in) The .vdb file name
	\param index (in) The index of the grid to load into the volume
	\return true if successful, false if not
	*/
	virtual bool LoadVolume(VDBHandle handle, const TSTR& filename, int index = 0) = 0;
	/** Save an OpenVDB volume to a .vdb file
	\par Parameters:
	\param handle (in) Handle of the VDB volume that contains the grid to save
	\param filename (in) The .vdb file name
	\param gridName (in) The name to assign the volume grid in the .vdb file
	\return true if successful, false if not
	*/
	virtual bool SaveVolume(VDBHandle handle, const TSTR& filename, const TSTR& gridName) = 0;
	/** Delete an OpenVDB volume from memory
	\par Parameters:
	\param handle Handle of the VDB volume to delete
	\return true if successful, false if not
	*/
	virtual bool DisposeVolume(VDBHandle handle) = 0;
	/** Perform a Boolean operation on OpenVDB volumes
	\par Parameters:
	\param handleA Handle of the VDB volume that contains operand A and will receive the result
	\param handleB Handle of the VDB volume that contains operand B; empty after the operation.
	\param operation The Boolean operation type. \see BooleanOperationType
	\return true if successful, false if not
	*/
	virtual bool VolumeBoolean(VDBHandle handleA, VDBHandle handleB, BooleanOperationType operation) = 0;
	/** Dilate a volume by a given amount, with optional masking
	\par Parameters:
	\param gridHandle Handle of the VDB volume that will be dilated
	\param amount Amount to dilate the volume
	\param maskHandle (Optional) Handle to a volume containing masking data (0-1); should match gridHandle
	   volume topology.
	\return true if successful, false if not
	*/
	virtual bool DilateVolume(VDBHandle gridHandle, float amount, VDBHandle maskHandle = -1) = 0;
	/** Erode a volume by a given amount, with optional masking
	\param gridHandle Handle of the VDB volume that will be eroded
	\param amount Amount to erode the volume
	\param maskHandle (Optional) Handle to a volume containing masking data (0-1); should match gridHandle
	   volume topology.
	\return true if successful, false if not
	*/
	virtual bool ErodeVolume(VDBHandle gridHandle, float amount, VDBHandle maskHandle = -1) = 0;
	/** Perform filtering (smoothing) on a volume
	\par Parameters:
	\param gridHandle Handle of the VDB volume that will be filtered
	\param type The filter type to use. \see FilterType
	\param width The width to use in filtering
	\param maskHandle (Optional) Handle to a volume containing masking data (0-1); should match gridHandle
	   volume topology.
	\return true if successful, false if not
	*/
	virtual bool FilterVolume(VDBHandle gridHandle, FilterType type, int width, VDBHandle maskHandle = -1) = 0;
	/** Clears a volume to empty
	\par Parameters:
	\param handle Handle of the VDB volume that will be cleared
	\return true if successful, false if not
	*/
	virtual bool ClearVolume(VDBHandle handle) = 0;
	/** Convert a volume to a mesh
	\par Parameters:
	\param handle Handle of the VDB volume that will be meshed
	\param meshOut (out) The Mesh that will receive the meshed result
	\param tm (in) The transform of the volume
	\param adaptivity The amount of adaptivity to apply during meshing (0-1)
	\param relaxDisoriented When true, relaxes disoriented triangles during adaptive meshing
	\return true if successful, false if not
	*/
	virtual bool VolumeToMesh(VDBHandle handle, ::Mesh& meshOut, const Matrix3& tm = Matrix3(), float adaptivity = 0.0f, bool relaxDisoriented = false) = 0;
	/** Create a mask volume to match a given volume
	\par Parameters:
	\param gridHandle Handle of the VDB volume that the mask will be created for
	\param voxelSize The voxel size to use in the meshing operation
	\param tm (in) The transform of the volume
	\param threshold The threshold value for the map
	\param map (in) The texmap that will be the source of the mask data
	\param t The time to use for the Texmap sampling
	\param maskHandle The Mesh that will receive the resulting mask data
	\return true if successful, false if not
	*/
	virtual bool CreateMask(VDBHandle gridHandle, float voxelSize, const Matrix3& tm, float threshold, Texmap* map, TimeValue t, VDBHandle maskHandle) = 0;

	// Point-to-volume functions:

	/** Creates an SDF volume containing points of a fixed size
	\par Parameters:
	\param points (in) A table of Point3 pointers, one per point
	\param radius The radius to use for all points
	\param voxelSize The voxelsize to use for the volume
	\param handle The handle of the volume that will receive the resulting data
	\return true if successful, false if not
	*/
	virtual bool PointsSRToVolume(const Tab<Point3*>& points, float radius, float voxelSize, VDBHandle handle) = 0;
	/** Creates an SDF volume containing points of a fixed size
	\par Parameters:
	\param points (in) A table of Point3s, one per point
	\param radius The radius to use for all points
	\param voxelSize The voxelsize to use for the volume
	\param handle The handle of the volume that will receive the resulting data
	\return true if successful, false if not
	*/
	virtual bool PointsSRToVolume(const Tab<Point3>& points, float radius, float voxelSize, VDBHandle handle) = 0;
	/** Creates an SDF volume containing points of varying sizes
	\par Parameters:
	\param points (in) A table of Point3 pointers, one per point
	\param radii (in) Table of radii, one per point
	\param voxelSize The voxelsize to use for the volume
	\param handle The handle of the volume that will receive the resulting data
	\return true if successful, false if not
	*/
	virtual bool PointsMRToVolume(const Tab<Point3*>& points, const Tab<float>& radii, float voxelSize, VDBHandle handle) = 0;
	/** Creates an SDF volume containing points of varying sizes
	\par Parameters:
	\param points (in) A table of Point3s, one per point
	\param radii (in) Table of radii, one per point
	\param voxelSize The voxelsize to use for the volume
	\param handle The handle of the volume that will receive the resulting data
	\return true if successful, false if not
	*/
	virtual bool PointsMRToVolume(const Tab<Point3>& points, const Tab<float>& radii, float voxelSize, VDBHandle handle) = 0;

	// Mesh-to-volume functions:

	/** Converts a mesh to a volume
	\par Parameters:
	\param meshIn (in) A mesh to convert to a volume
	\param voxelSize The voxelsize to use for the volume
	\param handle The handle of the volume that will receive the resulting data
	\param tm (in) Transform to apply to the mesh
	\return true if successful, false if not
	*/
	virtual bool MeshToVolume(const ::Mesh& meshIn, float voxelSize, int handle, const Matrix3& tm = Matrix3()) = 0;

	// Point-to-volume-to-mesh functions:

	/** Creates an SDF volume containing points of a fixed size and outputs the result to a mesh
	\par Parameters:
	\param points (in) A table of Point3 pointers, one per point
	\param radius The radius to use for all points
	\param voxelSize The voxelsize to use for the volume
	\param meshOut (out) The mesh that will receive the resulting data
	\param adaptivity The amount of adaptivity to apply during meshing (0-1)
	\param relaxDisoriented When true, relaxes disoriented triangles during adaptive meshing
	\return true if successful, false if not
	*/
	virtual bool PointsSRToMesh(const Tab<Point3*>& points, float radius, float voxelSize, ::Mesh& meshOut, float adaptivity = 0.0f, bool relaxDisoriented = false) = 0;
	/** Creates an SDF volume containing points of a fixed size and outputs the result to a mesh
	\par Parameters:
	\param points (in) A table of Point3s, one per point
	\param radius The radius to use for all points
	\param voxelSize The voxelsize to use for the volume
	\param meshOut (out) The mesh that will receive the resulting data
	\param adaptivity The amount of adaptivity to apply during meshing (0-1)
	\param relaxDisoriented When true, relaxes disoriented triangles during adaptive meshing
	\return true if successful, false if not
	*/
	virtual bool PointsSRToMesh(const Tab<Point3>& points, float radius, float voxelSize, ::Mesh& meshOut, float adaptivity = 0.0f, bool relaxDisoriented = false) = 0;
	/** Creates an SDF volume containing points of varying sizes and outputs the result to a mesh
	\par Parameters:
	\param points (in) A table of Point3 pointers, one per point
	\param radii (in) Table of radii, one per point
	\param voxelSize The voxelsize to use for the volume
	\param meshOut (out) The mesh that will receive the resulting data
	\param adaptivity The amount of adaptivity to apply during meshing (0-1)
	\param relaxDisoriented When true, relaxes disoriented triangles during adaptive meshing
	\return true if successful, false if not
	*/
	virtual bool PointsMRToMesh(const Tab<Point3*>& points, const Tab<float>& radii, float voxelSize, ::Mesh& meshOut, float adaptivity = 0.0f, bool relaxDisoriented = false) = 0;
	/** Creates an SDF volume containing points of varying sizes and outputs the result to a mesh
	\par Parameters:
	\param points (in) A table of Point3s, one per point
	\param radii (in) Table of radii, one per point
	\param voxelSize The voxelsize to use for the volume
	\param meshOut (out) The mesh that will receive the resulting data
	\param adaptivity The amount of adaptivity to apply during meshing (0-1)
	\param relaxDisoriented When true, relaxes disoriented triangles during adaptive meshing
	\return true if successful, false if not
	*/
	virtual bool PointsMRToMesh(const Tab<Point3>& points, const Tab<float>& radii, float voxelSize, ::Mesh& meshOut, float adaptivity = 0.0f, bool relaxDisoriented = false) = 0;

	// Mesh-to-volume-to-mesh functions:

	/** Converts a mesh to a volume then back to a mesh (round trip)
	\par Parameters:
	\param meshIn (in) The mesh to convert
	\param voxelSize The voxelsize to use for the volume
	\param meshOut (out) The mesh that will receive the resulting data (may be the same as the input mesh)
	\param adaptivity The amount of adaptivity to apply during meshing (0-1)
	\param relaxDisoriented When true, relaxes disoriented triangles during adaptive meshing
	\return true if successful, false if not
	*/
	virtual bool MeshToVolumeToMesh(const ::Mesh& meshIn, float voxelSize, ::Mesh& meshOut, float adaptivity = 0.0f, bool relaxDisoriented = false) = 0;

	/** Test whether a given point in space is inside a level set
	\par Parameters:
	\param gridHandle Handle of the VDB volume to test
	\param point (in) The point to test
	\param insideVolumeOut (out) Returns true if the point is inside the level set; false if not
	\return true if successful, false if not
	*/
	virtual bool TestPointInsideLevelSet(VDBHandle gridHandle, const Point3& point, bool& insideVolumeOut) = 0;

	/** Test a set of points in space to see which are inside a level set
	\par Parameters:
	\param gridHandle Handle of the VDB volume to test
	\param points (in) A table of the points to test
	\param insideVolumeOut (out) Returns a BitArray where the corresponding bit for each supplied point is TRUE if the point is inside the level set; FALSE if not
	\return true if successful, false if not
	*/
	virtual bool TestPointsInsideLevelSet(VDBHandle gridHandle, const Tab<Point3>& points, BitArray& insideVolumeOut) = 0;

	/** Test a set of points in space to see which are inside a level set
	\par Parameters:
	\param gridHandle Handle of the VDB volume to test
	\param points (in) A table of pointers to the points to test
	\param insideVolumeOut (out) Returns a BitArray where the corresponding bit for each supplied point is TRUE if the point is inside the level set; FALSE if not
	\return true if successful, false if not
	*/
	virtual bool TestPointsInsideLevelSet(VDBHandle gridHandle, const Tab<Point3*>& points, BitArray& insideVolumeOut) = 0;
};
