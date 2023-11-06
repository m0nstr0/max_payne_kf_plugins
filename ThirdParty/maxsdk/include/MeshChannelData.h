// ===========================================================================
// Copyright 2020 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================

//**************************************************************************/
// DESCRIPTION: Objects that represent the stack data channels for Mesh.
//		Part of mnmath.dll.
// AUTHOR: Autodesk Inc.
//**************************************************************************/

#pragma once

#include "export.h"
#include "MaxHeap.h"
#include "geom/bitarray.h"
#include "geom/point3.h"
#include "containers/Array.h"

class Mesh;
class Face;
class PerData;
class MeshMap;
class VEdge;

namespace MaxSDK {

// private implementation for mesh channels to avoid std in public includes
class MeshChannelDataPrivate;

//!< Base class for Mesh data channel objects.
class MeshChannelData : public MaxHeapOperators {
public:
	DllExport MeshChannelData();
	DllExport virtual ~MeshChannelData();
	MeshChannelData(MeshChannelData&) = delete;
	MeshChannelData(MeshChannelData&&) = delete;
	MeshChannelData& operator=(const MeshChannelData&) = delete;
	MeshChannelData& operator=(MeshChannelData&&) = delete;

	/*! Grab the lock for this channel object. To be followed by a call to \ref ReleaseLock().
		Used when modifying this object when there is a chance other threads are also trying to modify it.
	\see Mesh::ScopedLockChannels
	*/
	DllExport void AcquireLock() const;
	/*! Release the lock for this channel object. To be preceded with a call to \ref AcquireLock().
		Used when modifying this object when there is a chance other threads are also trying to modify it.
	\see Mesh::ScopedLockChannels
	*/
	DllExport void ReleaseLock() const;

	//!< Return a complete copy of this channel object.
	virtual MeshChannelData* Clone(const Mesh* /*parent*/) const = 0;

	//!< Updates the non-owning members of Mesh that this channel object owns.
	virtual void SyncParentInterface(Mesh* /*parent*/) const = 0;

private:
	MeshChannelDataPrivate* mPrivate; //owns a mutex that we want ot hide from this header
};

/*! Topology channel object for Mesh, stored in the MeshChannelContainer.
	Prior to 3ds Max 2024, the members of this object belonged to Mesh.
\see TOPO_CHANNEL
\see MaxSDK::MeshChannelContainer
*/
class MeshTopoChannelData : public MeshChannelData
{
public:
	DllExport MeshTopoChannelData();
	DllExport virtual ~MeshTopoChannelData();
	DllExport virtual MeshTopoChannelData* Clone(const Mesh* parent) const override;
	/** \brief Updates the non-owning members of Mesh that this channel object owns.
		If you fetch and modify this object, SyncParentInterface() must be called to keep the owning Mesh in sync.
		See \ref Mesh::GetChannelDataContainer() for usage example.
		This gets called automatically called by \ref MaxSDK::MeshChannelContainer when channels are manipulated.
		See \ref Mesh::numFaces, \ref Mesh::faces
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport virtual void SyncParentInterface(Mesh* mesh) const override;
	/** \brief Called to clear the public BlockWrite members of Mesh that this channel object owns.
		See \ref Mesh::numFaces, \ref Mesh::faces
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport static void ClearParentInterface(Mesh* mesh);

	//! \brief The number of faces in this Mesh.
	int numFaces;
	//! \brief The mesh faces.
	Face* faces;
	//! \brief Indicates the selected faces.
	/*! There is one bit for each face. Bits that are 1 indicate the face is selected.
		Prior to 3ds Max 2024 was part of SELECT_CHANNEL.
	*/
	BitArray faceSel;
	//! \brief Indicates the selected edges.
	/*! There is one bit for each edge of each face. Bits
		that are 1 indicate the edge is selected. The edge is identified by
		3*faceIndex + edgeIndex. Prior to 3ds Max 2024 was part of SELECT_CHANNEL.
	*/
	BitArray edgeSel;
};

/*! Geometry channel object for Mesh, stored in the MeshChannelContainer.
	Prior to 3ds Max 2024, the members of this object belonged to Mesh.
\see GEOM_CHANNEL
\see MaxSDK::MeshChannelContainer
*/
class MeshGeomChannelData : public MeshChannelData
{
public:
	DllExport MeshGeomChannelData();
	DllExport virtual ~MeshGeomChannelData();
	DllExport virtual class MeshGeomChannelData* Clone(const Mesh* parent) const override;
	/** \brief Updates the non-owning members of Mesh that this channel object owns.
		If you fetch and modify this object, SyncParentInterface() must be called to keep the owning Mesh in sync.
		See \ref Mesh::GetChannelDataContainer() for usage example.
		This gets called automatically called by \ref MaxSDK::MeshChannelContainer when channels are manipulated.
		See \ref Mesh::numVerts, \ref Mesh::verts, \ref Mesh::vFlags
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport virtual void SyncParentInterface(Mesh* mesh) const override;
	/** \brief Called to clear the public BlockWrite members of Mesh that this channel object owns.
		See \ref Mesh::numVerts, \ref Mesh::verts, \ref Mesh::vFlags
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport static void ClearParentInterface(Mesh* mesh);

	//! \brief The number of verts in this Mesh.
	int numVerts;
	//! \brief The mesh verts.
	Point3* verts;
	//! \brief Holds flags specific to the corresponding vertex in the verts array.
	long* vFlags;
	/*! This bit array indicates if a particular \ref MeshGeomChannelData::vData vertex data channel is supported.
		If the bit is set the channel is supported. Do not manipulate this directly.
	*/
	BitArray vdSupport;
	/*! The array of PerData objects which maintain and provide access to the floating
		point vertex data. There is one of these for each supported channel. The first
		two PerData objects in this array are used internally by 3ds Max.
	*/
	PerData* vData;
	//! \brief Hidden flags for vertices. Prior to 3ds Max 2024 was part of SELECT_CHANNEL.
	BitArray vertHide;
	//! \brief Indicates the selected vertices. There is one bit for each vertex.
	/*! Bits that are 1 indicate the vertex is selected. Prior to 3ds Max 2024 was part of SELECT_CHANNEL. */
	BitArray vertSel;
};

/*! Edge visibility channel object for Mesh, stored in the MeshChannelContainer.
	This is for internal use only for display purposes, not a stack channel.
\see EDGEVISIBLITY_CHANNEL
\see MaxSDK::MeshChannelContainer
*/
class MeshEdgeVisibilityChannelData : public MeshChannelData
{
public:
	DllExport MeshEdgeVisibilityChannelData();
	DllExport virtual ~MeshEdgeVisibilityChannelData();
	DllExport virtual MeshEdgeVisibilityChannelData* Clone(const Mesh* parent) const override;
	DllExport virtual void SyncParentInterface(Mesh* mesh) const override { UNUSED_PARAM(mesh); }
	DllExport static void ClearParentInterface(Mesh* mesh) { UNUSED_PARAM(mesh); }

	int numVisEdges;
	int edgeListHasAll;
	VEdge* visEdge;
};

/*! Vertex color channel object for Mesh, stored in the MeshChannelContainer.
	This represents the map channel stored at index 0.
\see VERTCOLOR_CHANNEL
\see MaxSDK::MeshChannelContainer
*/
class MeshVertexColorChannelData : public MeshChannelData
{
public:
	DllExport MeshVertexColorChannelData();
	DllExport virtual ~MeshVertexColorChannelData();
	DllExport virtual MeshVertexColorChannelData* Clone(const Mesh* parent) const override;
	/** \brief Updates the non-owning members of Mesh that this channel object owns.
		If you fetch and modify this object, SyncParentInterface() must be called to keep the owning Mesh in sync.
		See \ref Mesh::GetChannelDataContainer() for usage example.
		This gets called automatically called by \ref MaxSDK::MeshChannelContainer when channels are manipulated.
		See \ref Mesh::vcFace, \ref Mesh::vertCol, \ref Mesh::numCVerts
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport virtual void SyncParentInterface(Mesh* mesh) const override;
	/** \brief Called to clear the public BlockWrite members of Mesh that this channel object owns.
		See \ref Mesh::vcFace, \ref Mesh::vertCol, \ref Mesh::numCVerts
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport static void ClearParentInterface(Mesh* mesh);
	
	//! \brief The Mesh color map channel stored at index 0. Prior to 3ds Max 2024, this was the public Mesh member 'maps'.
	MeshMap* colorMap;
};

/*! Texture map channel object for Mesh, stored in the MeshChannelContainer.
	This represents the map channels in the 1-99 indices.
\see TEXMAP_CHANNEL
\see MaxSDK::MeshChannelContainer
*/
class MeshTextureMapChannelData : public MeshChannelData
{
public:
	DllExport MeshTextureMapChannelData();
	DllExport virtual ~MeshTextureMapChannelData();
	DllExport virtual MeshTextureMapChannelData* Clone(const Mesh* parent) const override;
	/** \brief Updates the non-owning members of Mesh that this channel object owns.
		If you fetch and modify this object, SyncParentInterface() must be called to keep the owning Mesh in sync.
		See \ref Mesh::GetChannelDataContainer() for usage example.
		This gets called automatically called by \ref MaxSDK::MeshChannelContainer when channels are manipulated.
		See \ref Mesh::tvFace, \ref Mesh::tVerts, \ref Mesh::numTVerts.
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport virtual void SyncParentInterface(Mesh* mesh) const override;
	/** \brief Called to clear the public BlockWrite members of Mesh that this channel object owns.
		See \ref Mesh::tvFace, \ref Mesh::tVerts, \ref Mesh::numTVerts.
	\see MaxSDK::BlockWrite_Value
	\see MaxSDK::BlockWrite_Ptr
	*/
	DllExport static void ClearParentInterface(Mesh* mesh);

	//! \brief The Mesh map channels in indices 1-99. Moved here from Mesh::maps
	MaxSDK::Array<MeshMap> maps;
};

/*! Material channel object for Mesh, stored in the MeshChannelContainer.
\see MTL_CHANNEL
\see MaxSDK::MeshChannelContainer
*/
class MeshMaterialChannelData : public MeshChannelData
{
public:
	DllExport MeshMaterialChannelData() :MeshChannelData() { mtlIndex = 0; }
	DllExport virtual ~MeshMaterialChannelData() = default;
	DllExport virtual MeshMaterialChannelData* Clone(const Mesh* /*parent*/) const override
	{
		MeshMaterialChannelData* copy = new MeshMaterialChannelData();
		copy->mtlIndex = mtlIndex;
		return copy;
	}
	DllExport virtual void SyncParentInterface(Mesh* /*mesh*/) const override {}
	DllExport static void ClearParentInterface(Mesh* /*mesh*/) {}

	//! \brief Mesh's MtlID, moved here from \ref Mesh.
	//! \ref Mesh::GetMtlIndex() const, \ref Mesh::SetMtlIndex(MtlID mtlIndex)
	unsigned short mtlIndex;
};
} //namespace