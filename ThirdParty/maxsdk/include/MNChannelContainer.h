// ===========================================================================
// Copyright 2020 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================

//**************************************************************************/
// DESCRIPTION: Container for the stack data channels for MNMesh.
//		Part of mnmath.dll.
// AUTHOR: Autodesk Inc.
//**************************************************************************/

#pragma once

#include "export.h"
#include "MaxHeap.h"
#include "channels.h"
#include <memory>

class MNMesh; //forward declaration to avoid circular dependencies

namespace MaxSDK {

class MNTopoChannelData;
class MNGeomChannelData;
class MNVertexColorChannelData;
class MNTextureMapChannelData;

/*! Container for managing the \ref MaxSDK::MNChannelData objects.
	The data in these channels objects are what gets manipulated and copied in the modifier stack.
*/
class MNChannelContainer : public MaxHeapOperators {
public:
	DllExport MNChannelContainer(MNMesh* parent)
		:m_parent(parent)
	{ }
	DllExport ~MNChannelContainer() = default;
	MNChannelContainer(MNChannelContainer&) = delete;
	MNChannelContainer(MNChannelContainer&&) = delete;
	MNChannelContainer& operator=(const MNChannelContainer&) = delete;
	MNChannelContainer& operator=(MNChannelContainer&&) = delete;

	/** Take and share channel objects with another MNMesh and store them in this container.
	\param fromOb - Specifies the source MNMesh to copy channel objects from.
	\param channels - Specifies the channels to reference from fromOb. See \ref ObjectChannels.
	\see MaxSDK::MNChannelData
	*/
	DllExport void ShallowCopyChannelData(MNChannelContainer const* fromOb, ChannelMask channels);
	/** Makes a complete copy of some channel objects and keeps the copy. This can be used to break the
		sharing of channel objects with other meshes crated by \ref ShallowCopyChannelData.
		Syncs the BlockWrite members with the owning MNMesh using \ref MNChannelData::SyncParentInterface
	\param channels - Specifies which channels to make complete copies of. See \ref ObjectChannels.
	\see MaxSDK::MNChannelData
	*/
	DllExport void NewAndCopyChannelData(ChannelMask channels);
	/** Drops the specified channel objects, calls ClearParentInterface() on those objects beforehand.
	\param channels - Specifies channel objects to free. See \ref ObjectChannels.
	\see MaxSDK::MNChannelData
	*/
	DllExport void FreeChannelData(ChannelMask channels);
	
	/** Based on a channel mask, request the locks for those channels.
		This should be used avoid race conditions when modifying objects in parallel when some of them share channels.
	\see MNMesh::ScopedLockChannels
	*/
	DllExport void AcquireChannelLocks(ChannelMask channels) const;
	/** Based on a channel mask, release the locks for those channels.
	\see MNMesh::ScopedLockChannels
	*/
	DllExport void ReleaseChannelLocks(ChannelMask channels) const;

	 // MACRO for defining getters and setters for the data channels
#define MN_DECLARE_CHANNEL_DATA_METHODS(ChannelData_Type) \
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

	MN_DECLARE_CHANNEL_DATA_METHODS(MNTopoChannelData)
	MN_DECLARE_CHANNEL_DATA_METHODS(MNGeomChannelData)
	MN_DECLARE_CHANNEL_DATA_METHODS(MNVertexColorChannelData)
	MN_DECLARE_CHANNEL_DATA_METHODS(MNTextureMapChannelData)

private:

	std::shared_ptr<MNTopoChannelData> mMNTopoChannelData;
	std::shared_ptr<MNGeomChannelData> mMNGeomChannelData;
	std::shared_ptr<MNVertexColorChannelData> mMNVertexColorChannelData;
	std::shared_ptr<MNTextureMapChannelData> mMNTextureMapChannelData;

	MNMesh* m_parent = nullptr; // Pointer to owning MNMesh, should never be null
};
} //namespace
