// ===========================================================================
// Copyright 2020 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================

//**************************************************************************/
// DESCRIPTION: Objects that represent the stack data channels for MNMesh.
//		Part of mnmath.dll.
// AUTHOR: Autodesk Inc.
//**************************************************************************/

#pragma once

#include "export.h"
#include "MaxHeap.h"
#include "geom/bitarray.h"
#include "tab.h"

class MNMesh;
class MNVert;
class MNFace;
class MNEdge;
class MNMap;
class PerData;

namespace MaxSDK {

// private implementation for MN channels to hide tbb from public include.
class MNChannelDataPrivate;

//!< Base class for MNMesh data channel objects.
class MNChannelData : public MaxHeapOperators {
public:
	DllExport MNChannelData();
	DllExport virtual ~MNChannelData();
	MNChannelData(MNChannelData&) = delete;
	MNChannelData(MNChannelData&&) = delete;
	MNChannelData& operator=(const MNChannelData&) = delete;
	MNChannelData& operator=(MNChannelData&&) = delete;
	
	/*! Grab the lock for this channel object. To be followed by a call to \ref ReleaseLock().
		Used when modifying this object when there is a chance other threads are also trying to modify it.
	\see MNMesh::ScopedLockChannels
	*/
	void AcquireLock() const;
	/*! Release the lock for this channel object. To be preceded with a call to \ref AcquireLock().
		Used when modifying this object when there is a chance other threads are also trying to modify it.
	\see MNMesh::ScopedLockChannels
	*/
	void ReleaseLock() const;

	//!< Return a complete copy of this channel object.
	virtual MNChannelData* Clone(const MNMesh* /*parent*/) const = 0;

	//!< Updates the non-owning members of MNMesh that this channel object owns.
	virtual void SyncParentInterface(MNMesh* /*parent*/) const = 0;

private:
	MNChannelDataPrivate* mPrivate; //owns a mutex that we want ot hide from this header
};

/*! Topology channel object for MNMesh, stored in the MNChannelContainer.
	Prior to 3ds Max 2024, the members of this object belonged to MNMesh.
\see TOPO_CHANNEL
\see MaxSDK::MNChannelContainer
*/
class MNTopoChannelData : public MNChannelData
{
public:
	DllExport MNTopoChannelData();
	DllExport virtual ~MNTopoChannelData();
	DllExport virtual MNTopoChannelData* Clone(const MNMesh* parent) const override;

	/** \brief Updates the non-owning members of MNMesh that this channel object owns.
		If you fetch and modify this object, SyncParentInterface() must be called to keep the owning MNMesh in sync.
		See \ref MNMesh::GetChannelDataContainer() for usage example.
		This gets called automatically called by \ref MaxSDK::MNChannelContainer when channels are manipulated.
		See \ref MNMesh::numf, \ref MNMesh::f, \ref MNMesh::nume, \ref MNMesh::e
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport virtual void SyncParentInterface(MNMesh* mnmesh) const override;

	/** \brief Called to clear the public BlockWrite members of MNMesh that this channel object owns.
		See \ref MNMesh::numf, \ref MNMesh::f, \ref MNMesh::nume, \ref MNMesh::e
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport static void ClearParentInterface(MNMesh* mnmesh);

	int numf; //!< Number of faces used.
	int nf_alloc; //!< Number of faces allocated.
	MNFace* f; //!< The face data array, size is \ref MNMesh::numf.
	int nume;  //!< Number of edges used.
	int ne_alloc; //!< Number of edges allocated.
	MNEdge* e; //!< The edge data array, size is \ref MNMesh::nume.
	Tab<int>* vedg; //!< Which edges use each vertex.
	Tab<int>* vfac; //!< which faces touch each vertex.
	/*! This bit array indicates if a particular \ref MNTopoChannelData::ed vertex data channel is supported.
		If the bit is set the channel is supported. Do not manipulate this directly. */
	BitArray edSupport;
	PerData* ed; //!< The per edge data array, size \ref MNMesh::EDNumSupported()
	bool fillInMeshFlag; //!< Owns this flag \ref MN_MESH_FILLED_IN. Whether or not the \ref MNTopoChannelData::e structure has been constructed.
	bool vertsOrderedFlag; //!< Owns this flag \ref MN_MESH_VERTS_ORDERED. Whether the \ref MNTopoChannelData::vfac and \ref MNTopoChannelData::vedg have been sorted and are in a counter clockwise order.
	bool noBadVertsFlag; //!< Owns this flag \ref MN_MESH_NO_BAD_VERTS. This indicates that the mnmesh has had its vertices checked and "bad" ones eliminated by EliminateBadVerts.
};

/*! Geometry channel object for MNMesh, stored in the MNChannelContainer.
	Prior to 3ds Max 2024, the members of this object belonged to MNMesh.
\see GEOM_CHANNEL
\see MaxSDK::MNChannelContainer
*/
class MNGeomChannelData : public MNChannelData
{
public:
	DllExport MNGeomChannelData();
	DllExport virtual ~MNGeomChannelData();
	DllExport virtual class MNGeomChannelData* Clone(const MNMesh* parent) const override;

	/** \brief Updates the non-owning members of MNMesh that this channel object owns.
		If you fetch and modify this object, SyncParentInterface() must be called to keep the owning MNMesh in sync.
		See \ref MNMesh::GetChannelDataContainer() for usage example.
		This gets called automatically called by \ref MaxSDK::MNChannelContainer when channels are manipulated.
		See \ref MNMesh::v, \ref MNMesh::numv
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport virtual void SyncParentInterface(MNMesh* mnmesh) const override;

	/** \brief Called to clear the public BlockWrite members of MNMesh that this channel object owns.
		See \ref MNMesh::v, \ref MNMesh::numv
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport static void ClearParentInterface(MNMesh* mnmesh);

	int numv; //!< Number of verts used.
	int nv_alloc; //!< Number of verts allocated.
	MNVert* v; //!< The verts data array, size is \ref MNMesh::numv.
	/*! This bit array indicates if a particular \ref MNGeomChannelData::vd vertex data channel is supported.
		If the bit is set the channel is supported. Do not manipulate this directly. */
	BitArray vdSupport;
	PerData* vd; //!< The per edge data array, size \ref MNMesh::EDNumSupported().
};

/*! Vertex Color channel object for MNMesh, stored in the MNChannelContainer.
	This represents the map channel stored at index 0.
\see VERTCOLOR_CHANNEL
\see MaxSDK::MNChannelContainer
*/
class MNVertexColorChannelData : public MNChannelData
{
public:
	DllExport MNVertexColorChannelData();
	DllExport virtual ~MNVertexColorChannelData();
	DllExport virtual MNVertexColorChannelData* Clone(const MNMesh* parent) const override;
	DllExport virtual void SyncParentInterface(MNMesh* /*mnmesh*/) const override {}
	DllExport static void ClearParentInterface(MNMesh* /*mnmesh*/) {}

	//Not a value to avoid a circular dependency during compile.
	MNMap* colorMap;
};

/*! Texture map channel object for MNMesh, stored in the MNChannelContainer.
	This represents the map channels in the 1-99 indices.
\see TEXMAP_CHANNEL
\see MaxSDK::MNChannelContainer
*/
class MNTextureMapChannelData : public MNChannelData
{
public:
	DllExport MNTextureMapChannelData();
	DllExport virtual ~MNTextureMapChannelData();
	DllExport virtual MNTextureMapChannelData* Clone(const MNMesh* parent) const override;
	DllExport virtual void SyncParentInterface(MNMesh* /*mnmesh*/) const override {}
	DllExport static void ClearParentInterface(MNMesh* /*mnmesh*/) {}

	int numm; //!< number of texture maps. \ref MNMesh::MNum().
	MNMap* m; //!< texture maps array, size \ref numm.
};
} //namespace
