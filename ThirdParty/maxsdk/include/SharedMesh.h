//
// Copyright 2021 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once

#include "mesh.h"
#include "Graphics/IRefObject.h"

namespace MaxSDK
{
	/**SharedMesh is a class to share a mesh between different classes. 
	* It uses reference counting and gets deleted once its ref count reaches 0.
	* \see Class ARefObject.
	* Usage is :
	* In class A, a member is SharedMeshPtr	mMeshPtr;
	* In one of A's method :
	* mMeshPtr = new SharedMesh(); //ref count is 1;
	* No need to do a mMeshPtr->Release() in A::~A() since we use a Ref<> pointer. 
	* When A is deleted, the ref count of mMeshPtr will decrease automatically.
	* And in another class B : you have another member SharedMeshPtr mAnotherMeshPtr;
	* //Share the mesh between the 2 classes
	* mAnotherMeshPtr = mMeshPtr;//ref count is 2, the SharedMesh will be deleted automatically when both SharedMeshPtr instances are deleted.
	* Use the mesh data :
	* if (nullptr != mAnotherMeshPtr){
		* const int numVertices = mAnotherMeshPtr.GetMesh().numverts;
	* }
	*/
class SharedMesh : public MaxSDK::Graphics::ARefObject{
	public:
	DllExport SharedMesh();
	DllExport virtual ~SharedMesh();
	DllExport const	Mesh& GetMesh()const	{return mMesh;}
	DllExport		Mesh& GetMesh()			{return mMesh;}
	DllExport const	Mesh* GetMeshPtr()const {return &mMesh;}
	DllExport		Mesh* GetMeshPtr()		{return &mMesh;}

	private:	
		/** The shared mesh. */
		Mesh	mMesh;
	};	
	
	/**About SharedMeshPtr defined below : This template class act similar to CComPtr. Using RefPtr<RefObjectType> 
	will be much safer than RefObjectType* since this template class 
	handles the AddRef and Release in its constructor and destructor. 

	Usage Example: 
	class DerivedRefObject : public ARefObject {
	public:
		void foo();
	};

	Use DerivedRefObject without RefPtr: 
	{
		DerivedRefObject* pObject = new DerivedRefObject;
		pObject->AddRef(); // Add reference
		pObject->foo();
		pObject->Release(); // release the object
		pObject = NULL;
	}

	Use DerivedRefObject with RefPtr: 
	{
		RefPtr<DerivedRefObject> pObject = new DerivedRefObject;
		pObject->foo();
		// don't need to worry about deleting this object
	}

	More RefPtr examples: 
	RefPtr<DerivedRefObject> pObject2 = new DerivedRefObject;
	RefPtr<DerivedRefObject> pObject3 = pObject2;
	pObject2 = NULL; // pObject3 is still valid.
	pObject3->foo();
	pObject3 = NULL; // Release the last reference to the object and the 
					 // object is destroyed automatically.
*/
	typedef MaxSDK::Graphics::RefPtr<SharedMesh> SharedMeshPtr;
};//end of namespace Max SDK