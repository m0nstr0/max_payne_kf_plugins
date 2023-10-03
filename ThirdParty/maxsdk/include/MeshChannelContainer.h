// ===========================================================================
// Copyright 2020 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================

//**************************************************************************/
// DESCRIPTION: Container for the stack data channels for Mesh.
//		Part of mesh.dll.
// AUTHOR: Autodesk Inc.
//**************************************************************************/

#pragma once

#include "export.h"
#include "MaxHeap.h"
#include "channels.h"
#include <memory>

class Mesh;  //forward declaration to avoid circular dependencies

namespace MaxSDK {

class MeshTopoChannelData;
class MeshGeomChannelData;
class MeshEdgeVisibilityChannelData;
class MeshVertexColorChannelData;
class MeshTextureMapChannelData;
class MeshMaterialChannelData;

/*! Container for managing the \ref MaxSDK::MeshChannelData objects.
	The data in these channels objects are what gets manipulated and copied in the modifier stack.
*/
class MeshChannelContainer : public MaxHeapOperators {
public:
	DllExport MeshChannelContainer(Mesh* parent)
		:m_parent(parent)
	{ }
	DllExport ~MeshChannelContainer() = default;
	MeshChannelContainer(MeshChannelContainer&) = delete;
	MeshChannelContainer(MeshChannelContainer&&) = delete;
	MeshChannelContainer& operator=(const MeshChannelContainer&) = delete;
	MeshChannelContainer& operator=(MeshChannelContainer&&) = delete;

	/** Take and share channel objects with another MNMesh and store them in this container.
	\param fromOb - Specifies the source Mesh to copy channel objects from.
	\param channels - Specifies the channels to reference from fromOb. See \ref ObjectChannels.
	\see MaxSDK::MeshChannelData
	*/
	DllExport void ShallowCopyChannelData(MeshChannelContainer const* fromOb, ChannelMask channels);
	/** Makes a complete copy of some channel objects and keeps the copy. This can be used to break the
		sharing of channel objects with other meshes crated by \ref ShallowCopyChannelData.
		Syncs the BlockWrite members with the owning Mesh using \ref MeshChannelData::SyncParentInterface
	\param channels - Specifies which channels to make complete copies of. See \ref ObjectChannels.
	\see MaxSDK::MeshChannelData
	*/
	DllExport void NewAndCopyChannelData(ChannelMask channels);
	/** Drops the specified channel objects, calls ClearParentInterface() on those objects beforehand.
	\param channels - Specifies channel objects to free. See \ref ObjectChannels.
	\see MaxSDK::MeshChannelData
	*/
	DllExport void FreeChannelData(ChannelMask channels);

	/** Based on a channel mask, request the locks for those channels.
		This should be used avoid race conditions when modifying objects in parallel when some of them share channels.
	\see Mesh::ScopedLockChannels
	*/
	DllExport void AcquireChannelLocks(ChannelMask channels) const;
	/** Based on a channel mask, release the locks for those channels.
	\see Mesh::ScopedLockChannels
	*/
	DllExport void ReleaseChannelLocks(ChannelMask channels) const;

	// MACRO for defining getters for the data channels
#define MESH_DECLARE_CHANNEL_DATA_METHODS(ChannelData_Type)\
	/*! Return the ChannelData_Type channel object, create it if not allocated.
	\see MaxSDK::ChannelData_Type
	 */ \
	DllExport ChannelData_Type* GetOrCreate##ChannelData_Type(); \
	/*! Return the ChannelData_Type channel object, null if not allocated.
	\see MaxSDK::ChannelData_Type
	*/ \
	inline ChannelData_Type* Get##ChannelData_Type() const { \
		return m##ChannelData_Type.get(); \
	}

	MESH_DECLARE_CHANNEL_DATA_METHODS(MeshTopoChannelData)
	MESH_DECLARE_CHANNEL_DATA_METHODS(MeshGeomChannelData)
	MESH_DECLARE_CHANNEL_DATA_METHODS(MeshEdgeVisibilityChannelData)
	MESH_DECLARE_CHANNEL_DATA_METHODS(MeshVertexColorChannelData)
	MESH_DECLARE_CHANNEL_DATA_METHODS(MeshTextureMapChannelData)
	MESH_DECLARE_CHANNEL_DATA_METHODS(MeshMaterialChannelData)

private:

	std::shared_ptr<MeshTopoChannelData> mMeshTopoChannelData;
	std::shared_ptr<MeshGeomChannelData> mMeshGeomChannelData;
	std::shared_ptr<MeshEdgeVisibilityChannelData> mMeshEdgeVisibilityChannelData;
	std::shared_ptr<MeshVertexColorChannelData> mMeshVertexColorChannelData;
	std::shared_ptr<MeshTextureMapChannelData> mMeshTextureMapChannelData;
	std::shared_ptr<MeshMaterialChannelData> mMeshMaterialChannelData;

	Mesh* m_parent = nullptr; // Pointer to owning Mesh, should never be null
};
} //namespace