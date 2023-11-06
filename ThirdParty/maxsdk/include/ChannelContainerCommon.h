// ===========================================================================
// Copyright 2020 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================

//**************************************************************************/
// DESCRIPTION: A collection of classes that help wrap data where access is
//		read only.
// AUTHOR: Autodesk Inc.
//**************************************************************************/

#pragma once

#include "MaxHeap.h"

namespace MaxSDK {

/*! Holds a non-const pointer to data but restricts the ability to change the pointer itself. Writing to it should not compile.
	This is used by \ref Mesh and \ref MNMesh to maintain public members but allow channel objects to own and maintain the data pointer.
\tparam T - The pointer type.
\tparam ChannelType - the class type that owns and manages this pointer.
\see Mesh::MeshChannelContainer
\see NNMesh::MNChannelContainer
*/
template<typename T, typename ChannelType >
class BlockWrite_Ptr : public MaxHeapOperators
{
	friend ChannelType; /*Grant write access to the channel who owns the pointer value.*/
public:
	BlockWrite_Ptr() : mPtr(nullptr) {}
	virtual ~BlockWrite_Ptr() = default;
	BlockWrite_Ptr(BlockWrite_Ptr&) = delete;
	BlockWrite_Ptr(BlockWrite_Ptr&&) = delete;
	BlockWrite_Ptr& operator=(const BlockWrite_Ptr&) = delete;
	BlockWrite_Ptr& operator=(BlockWrite_Ptr&&) = delete;
	BlockWrite_Ptr& operator+=(BlockWrite_Ptr&&) = delete;
	BlockWrite_Ptr& operator-=(BlockWrite_Ptr&&) = delete;
	BlockWrite_Ptr& operator++() = delete;
	BlockWrite_Ptr& operator--() = delete;
	operator T* () const { return mPtr; } //!< Auto converts to pointer type in most cases. May need to type cast.
	T* operator->() const { return mPtr; }
	T& operator*() const { return *mPtr; }
	T& operator[](size_t index) const { return mPtr[index]; }
protected:
	//! \brief Pointer to data that is owned elsewhere.
	T* mPtr;
};

/*! Holds a non-const copy of a value but restricts the ability to write to it. Writing to it should not compile.
	This is used by \ref Mesh and \ref MNMesh to maintain public members but allow channel objects to own and maintain the data.
\tparam T - The value type.
\tparam ChannelType - the class type that owns and manages this value.
\see Mesh::MeshChannelContainer
\see NNMesh::MNChannelContainer
*/
template<typename T, typename ChannelType >
class BlockWrite_Value : public MaxHeapOperators
{
	friend ChannelType; /*Grant write access to the channel who owns the pointer value.*/
public:
	BlockWrite_Value() = default;
	virtual ~BlockWrite_Value() = default;
	BlockWrite_Value(BlockWrite_Value&) = delete;
	BlockWrite_Value(BlockWrite_Value&&) = delete;
	BlockWrite_Value& operator=(const BlockWrite_Value&) = delete;
	BlockWrite_Value& operator=(BlockWrite_Value&&) = delete;
	BlockWrite_Value& operator+=(BlockWrite_Value&&) = delete;
	BlockWrite_Value& operator-=(BlockWrite_Value&&) = delete;
	BlockWrite_Value& operator++() = delete;
	BlockWrite_Value& operator--() = delete;
	T const* operator&() const { return &mValue; }
	operator T() const { return mValue; } //!< Auto converts to value type in most cases. May need to type cast.
protected:
	//! \brief This is a copy of the value.
	T mValue;
};
} //namespace
