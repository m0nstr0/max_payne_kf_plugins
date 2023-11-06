//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// FILE:        dummy.h
// DESCRIPTION: 
// AUTHOR:      Dan Silva
// HISTORY:     
//**************************************************************************/

#pragma once
#include "coreexp.h"
#include "maxtypes.h"
#include "object.h"
#include "SharedMesh.h"
#include "Graphics/Utilities/MeshEdgeRenderItem.h"

extern CoreExport Class_ID dummyClassID;

/*! \sa  Class HelperObject, Class Animatable, Class ReferenceMaker, Class ReferenceTarget, Class BaseObject, Class Object, Class GeomObject, Class Mesh.\n\n
\par Description:
This class represents a dummy helper object. There are several methods plug-ins
may call to get and set the size, and set the color of the dummy object. These
are used when a plug-in needs to create a dummy object in the scene. This class
provides implementations of all the required methods of Animatable,
ReferenceMaker, ReferenceTarget, BaseObject, Object, and GeomObject. All
methods of this class are implemented by the system.  */
#pragma warning(push)
#pragma warning(disable:4239 4100)
class DummyObject : public HelperObject {
	friend class LuminaireObject;
	friend class LuminaireObjectCreateCallBack;
	friend class DummyObjectCreateCallBack;
	friend INT_PTR CALLBACK DummyParamDialogProc( HWND hDlg, UINT message, 
		WPARAM wParam, LPARAM lParam );

	friend class CharacterObject;
	
		// Mesh cache
		MaxSDK::SharedMeshPtr mpSharedMesh;
		long dumFlags;	
		Point3 color;
		Box3 box;
		Interval valid;
		void BuildMesh();
		void UpdateMesh();
		
	//  inherited virtual methods for Reference-management
		RefResult NotifyRefChanged( const Interval& changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message, BOOL propagate );

	public:
		/*! \remarks Constructor. The size of the object is set to a minimal
		value, and the color of the object is initialized to <b>(0.6f, 0.8f,
		1.0f);</b> */
		CoreExport DummyObject();

		CoreExport ~DummyObject();

		/*! \remarks Returns the size of the dummy object box representation.
		*/
		CoreExport Box3 GetBox() const;
		/*! \remarks Sets the size of the dummy object box representation. */
		CoreExport void SetBox(const Box3& b);
		/*! \remarks Sets the display color of the dummy object in its normal
		state (not selected or frozen). */
		CoreExport void SetColor(Point3 c);
		/*! \remarks Enables the dummy object so it may be displayed, hit
		tested, snapped, etc. */
		CoreExport void EnableDisplay();
		/*! \remarks Prevents the dummy object from being displayed, hit
		tested, snapped, etc. */
		CoreExport void DisableDisplay();
		CoreExport void SetValidity(Interval v);

		//  inherited virtual methods:
		// From BaseObject
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		CoreExport void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CoreExport CreateMouseCallBack* GetCreateMouseCallBack();
		CoreExport RefTargetHandle Clone(RemapDir& remap);
		Interval ObjectValidity(TimeValue t) { return valid; }

		// From Object
		CoreExport ObjectState Eval(TimeValue time);
		void InitNodeName(MSTR& s);
		int DoOwnSelectHilite() {return 1; }
		int IsRenderable(){ return 0; }
		const MCHAR* GetObjectName(bool localized = true) const override;

		//  From Object
		CoreExport void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& bbox );
		CoreExport void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& bbox );
		CoreExport void GetDeformBBox(TimeValue t, Box3& bbox, Matrix3 *tm, BOOL useSel=FALSE );


		// IO
		CoreExport IOResult Save(ISave *isave);
		CoreExport IOResult Load(ILoad *iload);

		// From ReferenceMaker
		CoreExport void RescaleWorldUnits(float f);

		// Animatable methods
		CoreExport void DeleteThis();
		Class_ID ClassID() { return dummyClassID; }  
		void GetClassName(MSTR& s, bool localized = true) const override;
		int IsKeyable(){ return 1;}
		LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
	            WPARAM wParam,   LPARAM lParam ){return(0);}
		
		CoreExport virtual bool PrepareDisplay(
			const MaxSDK::Graphics::UpdateDisplayContext& prepareDisplayContext);

		CoreExport virtual bool UpdatePerNodeItems(
			const MaxSDK::Graphics::UpdateDisplayContext& updateDisplayContext,
			MaxSDK::Graphics::UpdateNodeContext& nodeContext,
			MaxSDK::Graphics::IRenderItemContainer& targetRenderItemContainer);

		CoreExport virtual unsigned long GetObjectDisplayRequirement() const;
	};
#pragma warning(pop)
CoreExport ClassDesc* GetDummyObjDescriptor();
