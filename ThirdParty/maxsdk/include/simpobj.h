/**********************************************************************
 *<
	FILE: SimpObj.h

	DESCRIPTION:  A base class for procedural objects that fit into
	              a standard form.

	CREATED BY: Rolf Berteig

	HISTORY: created 10/10/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "object.h"
#include "particle.h"
#include "Graphics/IObjectDisplay2.h"
#include "iparamb2.h"
/*! \sa  Class GeomObject, Class Mesh.\n\n
\par Description:
This is a base class for creating procedural objects. This class implements
many of the methods required to create a procedural object. The only limitation
for a procedural object using <b>SimpleObjectBase</b> as a base class is that it
must represent itself with a mesh.
\par Data Members:
Note: Methods of the base class refer to these data members. For example the
base class implementations of the bounding box methods use the <b>mesh</b> data
member. Therefore the plug-in derived from SimpleObjectBase must use these same
data members.\n\n
<b>Mesh mesh;</b>\n\n
The mesh object that is built by <b>BuildMesh()</b>.\n\n
<b>Interval ivalid;</b>\n\n
The validity interval for the <b>mesh</b>. This interval is used to determine
how <b>BuildMesh()</b> is called. If this interval is not set
<b>BuildMesh()</b> will be called over and over as the system won't know when
the mesh is valid or not. Make sure you set this interval to accurately reflect
the validity interval for the mesh.\n\n
<b>BOOL suspendSnap;</b>\n\n
If TRUE, this causes no snapping to occur. This is commonly used to prevent an
object from snapping to itself when it is creating. For example, in the mouse
proc used to create an object, the following code is often used when snapping
mouse points:\n\n
<b>ob-\>suspendSnap = TRUE;</b>\n\n
<b>p0 = vpt-\>SnapPoint(m,m,nullptr,SNAP_IN_PLANE);</b>\n\n
This disables snapping temporarily to keep the object from snapping to
itself.\n\n
Procedural Object plug-ins which subclass off SimpleObjectBase must implement these
methods. The default implementations are noted.  */
#pragma warning(push)
#pragma warning(disable:4100)
class SimpleObjectBase : public GeomObject
{
	public:
		Mesh mesh;
		Interval ivalid;

		BOOL suspendSnap;	// If TRUE, no snapping will occur
		
		static SimpleObjectBase* editOb;

		CoreExport SimpleObjectBase();
		CoreExport ~SimpleObjectBase();
		
		CoreExport void UpdateMesh(TimeValue t);				
		CoreExport void GetBBox(TimeValue t, Matrix3& tm, Box3& box );		
		void MeshInvalid() {ivalid.SetEmpty();}

		// From BaseObject
		CoreExport void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		CoreExport void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		CoreExport void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);		
		CoreExport unsigned long GetObjectDisplayRequirement() const;
		CoreExport bool PrepareDisplay(const MaxSDK::Graphics::UpdateDisplayContext& prepareDisplayContext);

		CoreExport bool UpdatePerNodeItems(
			const MaxSDK::Graphics::UpdateDisplayContext& updateDisplayContext,
			MaxSDK::Graphics::UpdateNodeContext& nodeContext,
			MaxSDK::Graphics::IRenderItemContainer& targetRenderItemContainer);
		// From Object
		CoreExport ObjectState Eval(TimeValue time);
		void InitNodeName(MSTR& s) {s = GetObjectName(true);}
		CoreExport Interval ObjectValidity(TimeValue t);
		CoreExport int CanConvertToType(Class_ID obtype);
		CoreExport Object* ConvertToType(TimeValue t, Class_ID obtype);		
		CoreExport BOOL PolygonCount(TimeValue t, int& numFaces, int& numVerts);

		// From GeomObject
		CoreExport int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);		
		CoreExport void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		CoreExport void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
		CoreExport Mesh* GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete);

		// Animatable methods
		CoreExport void FreeCaches();
		void GetClassName(MSTR& s, bool localized = true) const override {s = GetObjectName(localized);}

		// from InterfaceServer
		CoreExport virtual BaseInterface* GetInterface(Interface_ID iid);
		CoreExport void* GetInterface(ULONG id);

		// From ref
		CoreExport RefResult NotifyRefChanged(const Interval& changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message, BOOL propagate);
		
		// --- These must be implemented by the derived class ----------------
		/*! \remarks This method is called to build the mesh representation of the object
		using its parameter settings at the time passed. The plug-in should use
		the data member <b>mesh</b> to store the built mesh.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to build the mesh. */
		virtual void BuildMesh(TimeValue t)=0;

		/*! \remarks This method returns a BOOL to indicate if it is okay to draw the object
		at the time passed. Normally it is always OK to draw the object, so the
		default implementation returns TRUE. However for certain objects it
		might be a degenerate case to draw the object at a certain time
		(perhaps the size went to zero for example), so these objects could
		return FALSE.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which the object would be displayed.
		\par Default Implementation:
		<b>{ return TRUE; }</b>
		\return  TRUE if the object may be displayed; otherwise FALSE. */
		virtual BOOL OKtoDisplay(TimeValue t) {return TRUE;}

		/*! \remarks This is called if the user interface parameters needs to be updated
		because the user moved to a new time. The UI controls must display
		values for the current time.
		\par Example:
		If the plug-in uses a parameter map for handling its UI, it may call a
		method of the parameter map to handle this:
		\code
		pmapParam->Invalidate();
		\endcode
		If the plug-in uses ParamBlock2, it may call a method on the ParamBlockDesc2 to
		handle this:
		\code
		box_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
		\endcode
		If the plug-in does not use parameter maps, it should call the
		SetValue() method on each of its controls that display a value,
		for example the spinner controls. This will cause to the control to
		update the value displayed. The code below shows how this may be done
		for a spinner control. Note that ip and pblock are assumed to be
		initialized interface and parameter block pointers:
		\code
		void foo(IObjParam* ip, IParamBlock* pblock)
		{
			float newval;
			Interval valid = FOREVER;
			TimeValue t = ip->GetTime();
			// Get the value from the parameter block at the current time.
			pblock->GetValue( PB_ANGLE, t, newval, valid );
			// Set the value. Note that the notify argument is passed as FALSE.
			// This ensures no messages are sent when the value	changes.
			angleSpin->SetValue( newval, FALSE );
		}
		\endcode
		*/
		virtual void InvalidateUI() {}
	};

class IParamBlock;
/*! \sa  Class SimpleObjectBase, Class IParamBlock.\n\n
\par Description:
This class adds parameter block 1 system support to the <b>SimpleObjectBase</b> class. 
It has a public data member <b>IParamBlock *pblock</b> instead of the <b>IParamBlock2 *pblock2</b> 
provided by <b>SimpleObject2</b>. It also provides implementations of
<b>ReferenceMaker::GetReference()</b> and <b>SetReference()</b> which get and
set the <b>pblock</b> pointer. 
It is highly recommended that plugin classes not derive from SimpleObject, rather
that they derive from SimpleObject2. The SimpleObject class will be deprecated
in a future version of 3ds Max.
\par Data Members:
<b>IParamBlock* pblock;</b>\n\n
Points to the ParamBlock instance used by this class. This is the only
reference maintained by the class. */
class SimpleObject : public SimpleObjectBase
{
protected:
	IParamBlock* pblock;
public:
	SimpleObject() { pblock = nullptr; }

	// Animatable methods
	int NumSubs() override { return 1; }
	Animatable* SubAnim(int i) override { DbgAssert(i == 0); return (i == 0) ? (Animatable*)pblock : nullptr; }
	CoreExport MSTR SubAnimName(int i, bool localized = true) override;

	// From ref
	int NumRefs() override { return 1; }
	RefTargetHandle GetReference(int i) override { DbgAssert(i == 0); return (i == 0) ? (RefTargetHandle)pblock : nullptr; }
protected:
	virtual void SetReference(int i, RefTargetHandle rtarg) override
	{
		DbgAssert(i == 0);
		if (i == 0)
			pblock = (IParamBlock*)rtarg; 
	}
public:
	CoreExport RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget,
		PartID& partID, RefMessage message, BOOL propagate) override;

	// From BaseObject
	CoreExport IParamArray *GetParamBlock() override;
	CoreExport int GetParamBlockIndex(int id) override;

	// --- These must be implemented by the derived class ----------------
	/*! \remarks This method returns the parameter dimension of the parameter whose
	index is passed.
	\par Parameters:
	<b>int pbIndex</b>\n\n
	The index of the parameter to return the dimension of.
	\return  Pointer to a ParamDimension.
	\par Example:
	<b>return stdNormalizedDim;</b>
	\par Default Implementation:
	The default implementation returns <b>defaultDim</b>.\n\n
	\sa  ParamDimension */
	virtual	ParamDimension* GetParameterDim(int pbIndex) { return defaultDim; }

	//! \note This method has been deprecated in terms of implementation as of 3ds Max 2022. Plugin developers should implement GetParameterName(int pbIndex, bool localized) instead.
	//!       This method can no longer be overriden and calls to it are now forwarded to the function that replaced it with a "bool localized" value of true.
	//!       This is done so that plugin developers who do not localize their plugins don't have to update all the places where they call this method.
	//!       Plugin developers who do localize their plugins should analyze the places where they call this method to decide what value to pass it for the "bool localized" parameter.
	//! \sa SimpleObject::GetParameterName(int pbIndex, bool localized)
#pragma warning(push)
#pragma warning(disable: 4481)
	MAX_DEPRECATED virtual MSTR GetParameterName(int pbIndex) MAX_SEALED
	{
		return GetParameterName(pbIndex, true);
	}
#pragma warning(pop)

	/*! \remarks This method returns the name of the parameter whose index is passed.
	\par Parameters:
	<b>int pbIndex</b>\n\n
	The index of the parameter to return the name of.
	<b>bool localized</b>\n\n
	If true, then the parameter name returned should be localized in the language 3ds Max is currently using. Otherwise it should be the parameter name in English.
	If a plugin does not provide localized string resources, it can disregard this parameter and always return the parameter name in English.
	\return  The name of the parameter.
	\par Default Implementation:
	The default implementation returns <b>MSTR(_M("Parameter"))</b> */
	virtual MSTR GetParameterName(int pbIndex, bool localized) { return MSTR(_M("Parameter")); }
};

/*! \sa  Class SimpleObjectBase, Class IParamBlock2.\n\n
\par Description:
This class adds parameter block 2 system support to the <b>SimpleObjectBase</b> class.
It has a public data member <b>IParamBlock2 *pblock2</b> instead of the <b>IParamBlock *pblock</b> 
provided by <b>SimpleObject</b>. It also provides implementations of
<b>ReferenceMaker::GetReference()</b> and <b>SetReference()</b> which get and
set the <b>pblock2</b> pointer. 
\par Data Members:
<b>IParamBlock2* pblock2;</b>\n\n
Points to the ParamBlock2 instance used by this class. This is the only
reference maintained by the class. */
class SimpleObject2 : public SimpleObjectBase
{
protected:
	IParamBlock2* pblock2;
public:
	SimpleObject2() { pblock2 = nullptr; }

	// Animatable methods
	int NumSubs() override { return 1; }
	Animatable* SubAnim(int i) override { DbgAssert(i == 0); return (i == 0) ? (Animatable*)pblock2 : nullptr; }
	CoreExport MSTR SubAnimName(int i, bool localized = true) override;
	int	NumParamBlocks() override { return 1; }  // return number of ParamBlocks in this instance
	IParamBlock2* GetParamBlock(int i) override { DbgAssert(i == 0); return (i == 0) ? pblock2 : nullptr; } // return i'th ParamBlock
	IParamBlock2* GetParamBlockByID(BlockID id) override { return (pblock2 && pblock2->ID() == id) ? pblock2 : nullptr; } // return id'd ParamBlock

	// From ref
	int NumRefs() override { return 1; }
	RefTargetHandle GetReference(int i) override { DbgAssert(i == 0); return (i == 0) ? (RefTargetHandle)pblock2 : nullptr;}
protected:
	virtual void SetReference(int i, RefTargetHandle rtarg) override
	{
		DbgAssert(i == 0);
		if (i == 0)
			pblock2 = (IParamBlock2*)rtarg; 
	}
};

/*! \sa  Class WSMObject, Class IParamBlock, Class Mesh,  Class Interval.\n\n
\par Description:
This is the base class for creating space warp objects. This class implements
many of the methods required to create a space warp object. The only limitation
for a space warp object using SimpleWSMObject as a base class is that it must
represent itself with a mesh.
\par Data Members:
Note: Methods of the base class refer to these data members. For example the
base class implementations of the bounding box methods use the <b>mesh</b> data
member. Therefore the plug-in derived from SimpleWSMObject must use these same
data members. These are listed below:\n\n
<b>IParamBlock *pblock;</b>\n\n
Pointer to a parameter block. Clients of SimpleWSMObject should use this
pointer when the pblock reference is created.\n\n
<b>Mesh mesh;</b>\n\n
The mesh object that is built by <b>BuildMesh()</b>.\n\n
<b>Interval ivalid;</b>\n\n
The validity interval of the mesh. <br>  Space warp object plug-ins which
subclass off SimpleWSMObject must implement these methods. The default
implementations are noted. */
class SimpleWSMObject : public WSMObject {
	public:
		IParamBlock *pblock;
		Mesh mesh;
		Interval ivalid;
		
		static SimpleWSMObject *editOb;

		CoreExport SimpleWSMObject();
		CoreExport ~SimpleWSMObject();
		
		CoreExport void UpdateMesh(TimeValue t);				
		CoreExport void GetBBox(TimeValue t, Matrix3& tm, Box3& box );		
		void MeshInvalid() {ivalid.SetEmpty();}
		
		// From BaseObject
		CoreExport void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		CoreExport void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		CoreExport void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);		
		CoreExport IParamArray *GetParamBlock();
		CoreExport int GetParamBlockIndex(int id);

		// From Object
		int DoOwnSelectHilite() {return TRUE;}
		CoreExport ObjectState Eval(TimeValue time);
		void InitNodeName(MSTR& s) {s = GetObjectName(true);}
		CoreExport Interval ObjectValidity(TimeValue t);
		CoreExport void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		CoreExport void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
		int IsRenderable() {return FALSE;}

		// Animatable methods
		CoreExport void FreeCaches();
		void GetClassName(MSTR& s, bool localized = true) const override {s = GetObjectName(localized);}
		int NumSubs() { return 1; }  
		Animatable* SubAnim(int i) { return (Animatable*)pblock; }
		CoreExport MSTR SubAnimName(int i, bool localized = true) override;

		// From ref
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return (RefTargetHandle)pblock;}
protected:
		virtual void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}		
public:
		CoreExport RefResult NotifyRefChanged(const Interval& changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message, BOOL propagate);
		

		// --- These must be implemented by the derived class ----------------
		/*! \remarks This method is called to build the mesh representation of the object
		using its parameter settings at the time passed.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to build the mesh. */
		virtual void BuildMesh(TimeValue t)=0;
		/*! \remarks This method returns a BOOL to indicate if it is okay to draw the object
		at the time passed. Normally it is always OK to draw the object, so the
		default implementation returns TRUE. However for certain objects it
		might be a degenerate case to draw the object at a certain time
		(perhaps the size went to zero for example), so these objects could
		return FALSE.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which the object would be displayed.
		\par Default Implementation:
		<b>{ return TRUE; }</b>
		\return  TRUE if the object may be displayed; otherwise FALSE. */
		virtual BOOL OKtoDisplay(TimeValue t) {return TRUE;}
		/*! \remarks This is called if the user interface parameters needs to be updated
		because the user moved to a new time. The UI controls must display
		values for the current time.
		\par Example:
		If the plug-in uses a parameter map for handling its UI, it may call a
		method of the parameter map to handle this:
		<b>pmapParam-\>Invalidate();</b>\n\n
		If the plug-in does not use parameter maps, it should call the
		SetValue() method on each of its controls that display a value, for
		example the spinner controls. This will cause to the control to update
		the value displayed. The code below shows how this may be done for a
		spinner control. Note that ip and pblock are assumed to be initialized
		interface and parameter block pointers\n\n
		<b>(IObjParam *ip, IParamBlock *pblock).</b>\n\n
		<b>float newval;</b>\n\n
		<b>Interval valid=FOREVER;</b>\n\n
		<b>TimeValue t=ip-\>GetTime();</b>\n\n
		<b>// Get the value from the parameter block at the current
		time.</b>\n\n
		<b>pblock-\>GetValue( PB_ANGLE, t, newval, valid );</b>\n\n
		<b>// Set the value. Note that the notify argument is passed as
		FALSE.</b>\n\n
		<b>// This ensures no messages are sent when the value changes.</b>\n\n
		<b>angleSpin-\>SetValue( newval, FALSE );</b> */
		virtual void InvalidateUI() {}
		/*! \remarks This method returns the parameter dimension of the parameter whose
		index is passed.
		\par Parameters:
		<b>int pbIndex</b>\n\n
		The index of the parameter to return the dimension of.
		\return  Pointer to a ParamDimension.
		\par Example:
		<b>return stdNormalizedDim;</b>
		\par Default Implementation:
		The default implementation returns <b>defaultDim</b>.\n\n
		\sa  ParamDimension */
		virtual	ParamDimension *GetParameterDim(int pbIndex) {return defaultDim;}
		//! \note This method has been deprecated in terms of implementation as of 3ds Max 2022. Plugin developers should implement GetParameterName(int pbIndex, bool localized) instead.
		//!       This method can no longer be overriden and calls to it are now forwarded to the function that replaced it with a "bool localized" value of true.
		//!       This is done so that plugin developers who do not localize their plugins don't have to update all the places where they call this method.
		//!       Plugin developers who do localize their plugins should analyze the places where they call this method to decide what value to pass it for the "bool localized" parameter.
		//! \sa SimpleWSMObject::GetParameterName(int pbIndex, bool localized)
#pragma warning(push)
#pragma warning(disable: 4481)
		MAX_DEPRECATED virtual MSTR GetParameterName(int pbIndex) MAX_SEALED
		{
			return GetParameterName(pbIndex, true);
		}
#pragma warning(pop)
		/*! \remarks This method returns the name of the parameter whose index is passed.
		\par Parameters:
		<b>int pbIndex</b>\n\n
		The index of the parameter to return the name of.
		<b>bool localized</b>\n\n
		If true, then the parameter name returned should be localized in the language 3ds Max is currently using. Otherwise it should be the parameter name in English.
		If a plugin does not provide localized string resources, it can disregard this parameter and always return the parameter name in English.
		\return  The name of the parameter.
		\par Default Implementation:
		The default implementation returns <b>MSTR(_M("Parameter"))</b> */
		virtual MSTR GetParameterName(int pbIndex, bool localized) {return MSTR(_M("Parameter"));}
	};

class SimpleWSMObject2 : public SimpleWSMObject
{
public:
	IParamBlock2* pblock2;
	SimpleWSMObject2() { pblock2 = nullptr; }
	// From ref, upcast
	RefTargetHandle GetReference(int i) {return (RefTargetHandle)pblock;}
protected:
	virtual void SetReference(int i, RefTargetHandle rtarg)
	{
		pblock2 = (IParamBlock2*)rtarg; 
		SimpleWSMObject::SetReference(i, rtarg);
	}
};

class SimpleModBase;
class IParamMap;

/*! \sa  Class SimpleWSMObject, Class SimpleModBase, Class Deformer, Class IParamMap.\n\n
\par Description:
This class is used to allow any Object Space Modifier derived from
<b>SimpleModBase</b> to easily be turned into a World Space Modifier (Space
Warp).\n\n
This is very simple to do because a modifier version already contains just
about everything that needs to be done. This is because the modifier works the
same -- it is just in world space instead of object space.\n\n
All a developer needs to do to turn their <b>SimpleModBase</b> modifier into the
WSM version is implement a class derived from this one and call the
<b>SimpleOSMTOWSMObject</b> constructor from their constructor. See the sample
code below (for the full sample code using this class see
<b>/MAXSDK/SAMPLES/MODIFIERS/BEND.CPP</b>).\n\n
\code
class BendWSM : public SimpleOSMToWSMObject
{
	BendWSM() {}
	BendWSM(BendMod *m) : SimpleOSMToWSMObject(m) {}
	void DeleteThis() { delete this; }
	SClass_ID SuperClassID() {return WSM_OBJECT_CLASS_ID;}
	Class_ID ClassID() {return BENDWSM_CLASSID;}
	const MCHAR *GetObjectName(bool localized = true) const override {return localized ? GetString(IDS_RB_BEND2) : _T("Bend");}
};
\endcode
These new modifier-based space warps are accessed in the drop-down category
list of the Space Warps branch of the Create command panel. Choose
Modifier-Based from the list to display buttons for each of the new space
warps.
\par Data Members :
<b>SimpleModBase *mod; </b>\n\n
Points to the SimpleModBase instance this is based on.\n\n
<b>static IParamMap *pmapParam; </b>\n\n
Points to the parameter map used to handle the user interface for this WSM.
These are the parameter block indices for the pmap : \n\n
\code
#define PB_OSMTOWSM_LENGTH 0
#define PB_OSMTOWSM_WIDTH 1
#define PB_OSMTOWSM_HEIGHT 2
#define PB_OSMTOWSM_DECAY 3 
\endcode 
*/
class SimpleOSMToWSMObject : public SimpleWSMObject {
	public:
		SimpleModBase *mod;
		static IParamMap *pmapParam;

		/*! \remarks Constructor. */
		CoreExport SimpleOSMToWSMObject();
		/*! \remarks Constructor.
		\par Parameters:
		<b>SimpleModBase *m</b>\n\n
		This is a pointer to the <b>SimpleModBase</b> instance this WSM is based on. */
		CoreExport SimpleOSMToWSMObject(SimpleModBase *m);

		int NumRefs() {return 2;}
		CoreExport RefTargetHandle GetReference(int i);
protected:
		CoreExport virtual void SetReference(int i, RefTargetHandle rtarg);
public:
		CoreExport IOResult Load(ILoad *iload);

		int NumSubs() {return 2;}  
		CoreExport Animatable* SubAnim(int i);
		CoreExport MSTR SubAnimName(int i, bool localized = true) override;

		CoreExport void BuildMesh(TimeValue t);
		CoreExport Modifier *CreateWSMMod(INode *node);

		CoreExport CreateMouseCallBack* GetCreateMouseCallBack();
		CoreExport void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		CoreExport void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		CoreExport ParamDimension *GetParameterDim(int pbIndex) {return stdWorldDim;}
		CoreExport MSTR GetParameterName(int pbIndex, bool localized = true);

		/*! \remarks Implemented by the System.\n\n
		This class enhances the deformation done by the object space modifier to
		include a decay parameter. This allows the deformation to decay over distance.
		This helper method is used internally in this. */
		CoreExport Deformer &GetDecayDeformer(TimeValue t,Deformer &mdef,Point3 origin,Interval &iv);
		inline Deformer& GetDecayDeformer(TimeValue t, Deformer& mdef, Point3 origin)
		{
			Interval interval = FOREVER;
			return GetDecayDeformer(t, mdef, origin, interval);
		}

		CoreExport SimpleOSMToWSMObject* SimpleOSMToWSMClone(SimpleOSMToWSMObject *from,RemapDir& remap);

		CoreExport void InvalidateUI();
	};

#define PB_OSMTOWSM_LENGTH	0
#define PB_OSMTOWSM_WIDTH	1
#define PB_OSMTOWSM_HEIGHT	2
#define PB_OSMTOWSM_DECAY	3


/*! \sa  Class ParticleObject, Marker Types, Class Mesh, Class ParticleSys, Class Interval, Template Class Tab, Class ParamDimension,.\n\n
\par Description:
This class provides a base class from which you may derive Particle System
plug-ins. This class may be used by particle systems that fit within its form.
The form is primarily dictated by the data members maintain by the class. The
class maintains an instance of class ParticleSys that describes the particles.
It also has a table of force fields and collision objects. The emitter for the
particles is represented by a mesh. There is also a parameter block pointer
available.\n\n
Particle system plug-ins that don't fit this form may derive from a base class
without any constraints. See Class ParticleObject for more details.
\par Data Members:
<b>IParamBlock *pblock;</b>\n\n
The parameter block pointer.\n\n
<b>ParticleSys parts;</b>\n\n
This is a description of the particles themselves (their count, position,
velocities, ...).\n\n
<b>TimeValue tvalid;</b>\n\n
A particle system derived from SimpleParticle is valid at a particular time
only (it does not have a validity interval). It is assumed to be always
changing. This data member holds the time at which it is valid (when
<b>valid</b> is TRUE).\n\n
<b>BOOL valid;</b>\n\n
This flag indicates if the particle system is valid. If TRUE, <b>tvalid</b>
should contain the time it is valid for.\n\n
<b>Tab\<ForceField*\> fields;</b>\n\n
The table of force fields affecting the particles.\n\n
<b>Tab\<CollisionObject*\> cobjs;</b>\n\n
The table of collision objects affecting the particles.\n\n
<b>Mesh mesh;</b>\n\n
The mesh object that represents the emitter.\n\n
<b>Interval mvalid;</b>\n\n
The validity interval for the emitter mesh. If the mesh is invalid
<b>BuildEmitter()</b> will be called.\n\n
<b>static SimpleParticle *editOb;</b>\n\n
The SimpleParticle object that is being edited between <b>BeginEditParams()</b>
and <b>EndEditParams()</b>.\n\n
<b>static IObjParam *ip;</b>\n\n
Storage for the interface pointer passed into <b>BeginEditParams()</b>. This
pointer is only valid between <b>BeginEditParams()</b> and
<b>EndEditParams()</b>.  */
class SimpleParticle : public ParticleObject {
	public:
		IParamBlock *pblock;
		ParticleSys parts;
		TimeValue tvalid;
		BOOL valid;
		Tab<ForceField*> fields;		
		Tab<CollisionObject*> cobjs;
		Mesh mesh;
		Interval mvalid;
		MetaParticle metap;
		
		CoreExport static SimpleParticle *editOb;
		CoreExport static IObjParam *ip;

		/*! \remarks Constructor. The <b>pblock</b> is initialized to nullptr,
		the <b>mvalid</b> interval is set to empty, and <b>valid</b> is set to
		FALSE. */
		CoreExport SimpleParticle();
		CoreExport ~SimpleParticle();
				
		CoreExport void Update(TimeValue t,INode *node=nullptr);
		CoreExport void UpdateMesh(TimeValue t);
		CoreExport void GetBBox(TimeValue t, Matrix3& tm, Box3& box);		
		void MeshInvalid() {mvalid.SetEmpty();}
		void ParticleInvalid() {valid=FALSE;}

		// From BaseObject
		CoreExport void BeginEditParams( IObjParam *objParam, ULONG flags, Animatable *prev);
		CoreExport void EndEditParams( IObjParam *objParam, ULONG flags, Animatable *next);
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);		
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);		
		CoreExport unsigned long GetObjectDisplayRequirement() const;
		CoreExport bool UpdatePerNodeItems(
			const MaxSDK::Graphics::UpdateDisplayContext& updateDisplayContext,
			MaxSDK::Graphics::UpdateNodeContext& nodeContext,
			MaxSDK::Graphics::IRenderItemContainer& targetRenderItemContainer);
		CoreExport bool PrepareDisplay(
			const MaxSDK::Graphics::UpdateDisplayContext& prepareDisplayContext);
		CoreExport IParamArray *GetParamBlock();
		CoreExport int GetParamBlockIndex(int id);

		// From Object
		int DoOwnSelectHilite() {return TRUE;}
		CoreExport ObjectState Eval(TimeValue time);
		void InitNodeName(MSTR& s) {s = GetObjectName(true);}
		CoreExport Interval ObjectValidity(TimeValue t);
		CoreExport int CanConvertToType(Class_ID obtype);
		CoreExport Object* ConvertToType(TimeValue t, Class_ID obtype);
		CoreExport Object *MakeShallowCopy(ChannelMask channels);
		void WSStateInvalidate() {valid = FALSE;}
		BOOL IsWorldSpaceObject() {return TRUE;}

		// From GeomObject				
		CoreExport void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		CoreExport void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );

		// From ParticleObject
		CoreExport void ApplyForceField(ForceField *ff);
		CoreExport BOOL ApplyCollisionObject(CollisionObject *co);
		CoreExport TimeValue ParticleAge(TimeValue t, int i);		
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the position of the specified particle at the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to set the particle position.\n\n
		<b>int i</b>\n\n
		The zero based index of the particle to set.\n\n
		<b>Point3 pos</b>\n\n
		The position to set. */
		CoreExport void SetParticlePosition(TimeValue t, int i, Point3 pos);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the velocity of the specified particle at the specified time (in
		3ds Max units per tick).
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to set the particle velocity.\n\n
		<b>int i</b>\n\n
		The zero based index of the particle to set.\n\n
		<b>Point3 vel</b>\n\n
		The velocity to set. */
		CoreExport void SetParticleVelocity(TimeValue t, int i, Point3 vel);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the age of the specified particle at the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to set the particle age.\n\n
		<b>int i</b>\n\n
		The zero based index of the particle to set.\n\n
		<b>TimeValue age</b>\n\n
		The age to set. */
		CoreExport void SetParticleAge(TimeValue t, int i, TimeValue age);

		// Animatable methods
		void GetClassName(MSTR& s, bool localized = true) const override {s = GetObjectName(localized);}
		int NumSubs() {return 1;}
		// upcast pointer
		Animatable* SubAnim(int i) { return (Animatable*)pblock; }
		CoreExport MSTR SubAnimName(int i, bool localized = true) override;

        using ParticleObject::GetInterface;
		CoreExport void* GetInterface(ULONG id);

		// From ref
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return (RefTargetHandle)pblock;}
protected:
		virtual void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}	
public:
		CoreExport RefResult NotifyRefChanged(const Interval& changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message, BOOL propagate);
		

		// --- These must be implemented by the derived class ----------------
		/*! \remarks This method is called so the particle system can update its state to
		reflect the current time passed. This may involve generating new
		particle that are born, eliminating old particles that have expired,
		computing the impact of collisions or force field effects, and modify
		the positions and velocities of the particles.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The particles should be updated to reflect this time.\n\n
		<b>INode *node</b>\n\n
		This is the emitter node. Particles system are world space objects so
		they are not instanced. This means that the particle system can depend
		on the node's world space position.
		\par Sample Code:
		For example code see <b>/MAXSDK/SAMPLES/OBJECTS/RAIN.CPP</b>. */
		virtual void UpdateParticles(TimeValue t,INode *node)=0;
		/*! \remarks This method is called to allow the plug-in to provide a representation
		of its emitter in the 3D viewports.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		Specifies the time to build the emitter.\n\n
		<b>Mesh\& amesh</b>\n\n
		Store the built mesh representation here. */
		virtual void BuildEmitter(TimeValue t, Mesh& amesh)=0;
		/*! \remarks This method is called to retrieve the validity time of the particle
		system emitter.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to compute the validity interval.
		\return  The validity interval of the particle system emitter at the
		specified time. */
		virtual Interval GetValidity(TimeValue t)=0;
		/*! \remarks Returns one of the defined marker types to use when displaying
		particles.
		\return  One of the following values:\n\n
		See the ::MarkerType enum.
		\par Default Implementation:
		<b>{return POINT_MRKR;}</b> */
		virtual MarkerType GetMarkerType() {return POINT_MRKR;}
		/*! \remarks This method is called to determine if the particle emitter is okay to
		display at the specified time. If at certain times it is not okay to
		display this method should return FALSE. This might occur if a size
		goes to 0. Normally however it is always okay to display so the default
		implementation returns TRUE.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to display the emitter.
		\return  TRUE if it is okay to display, FALSE otherwise.
		\par Default Implementation:
		<b>{return TRUE;}</b> */
		virtual BOOL OKtoDisplay(TimeValue t) {return TRUE;}
		/*! \remarks This method is called to determine if the particle emitter is visible
		in the viewports. If the plug-in provides a UI control to toggle the
		emitter on and off, this method should return the state of this
		control.
		\return  TRUE if the emitter is visible; otherwise FALSE.
		\par Default Implementation:
		<b>{return TRUE;}</b> */
		virtual BOOL EmitterVisible() {return TRUE;}
		/*! \remarks It is important the user interface controls display values that reflect
		the current time. This method is called if the user interface
		parameters needs to be updated because the user moved to a new time.
		\par Example:
		If the plug-in uses a parameter map for handling its UI, it may call a
		method of the parameter map to handle this:
		<b>pmapParam-\>Invalidate();</b>\n\n
		If the plug-in does not use parameter maps, it should call the
		<b>SetValue()</b> method on each of its controls that display a value,
		for example the spinner controls. This will cause to the control to
		update the value displayed. The code below shows how this may be done
		for a spinner control. Note that ip and pblock are assumed to be
		initialized interface and parameter block pointers\n\n
		<b>(IObjParam *ip, IParamBlock *pblock).</b>\n\n
		<b>float newval;</b>\n\n
		<b>Interval valid=FOREVER;</b>\n\n
		<b>TimeValue t=ip-\>GetTime();</b>\n\n
		<b>// Get the value from the parameter block at the current
		time.</b>\n\n
		<b>pblock-\>GetValue( PB_ANGLE, t, newval, valid );</b>\n\n
		<b>// Set the value. Note that the notify argument is passed as
		FALSE.</b>\n\n
		<b>// This ensures no messages are sent when the value changes.</b>\n\n
		<b>angleSpin-\>SetValue( newval, FALSE );</b> */
		virtual void InvalidateUI() {}
		/*! \remarks This method returns the parameter dimension of the parameter whose
		index is passed.
		\par Parameters:
		<b>int pbIndex</b>\n\n
		The index of the parameter to return the dimension of.
		\return  Pointer to a ParamDimension.
		\par Example:
		<b>return stdNormalizedDim;</b>
		\par Default Implementation:
		The default implementation returns <b>defaultDim</b>.\n\n
		\sa  ParamDimension */
		virtual	ParamDimension *GetParameterDim(int pbIndex) {return defaultDim;}
		//! \note This method has been deprecated in terms of implementation as of 3ds Max 2022. Plugin developers should implement GetParameterName(int pbIndex, bool localized) instead.
		//!       This method can no longer be overriden and calls to it are now forwarded to the function that replaced it with a "bool localized" value of true.
		//!       This is done so that plugin developers who do not localize their plugins don't have to update all the places where they call this method.
		//!       Plugin developers who do localize their plugins should analyze the places where they call this method to decide what value to pass it for the "bool localized" parameter.
		//! \sa SimpleParticle::GetParameterName(int pbIndex, bool localized)
#pragma warning(push)
#pragma warning(disable: 4481)
		MAX_DEPRECATED virtual MSTR GetParameterName(int pbIndex) MAX_SEALED
		{
			return GetParameterName(pbIndex, true);
		}
#pragma warning(pop)
		/*! \remarks This method returns the name of the parameter whose index is passed.
		\par Parameters:
		<b>int pbIndex</b>\n\n
		The index of the parameter to return the name of.
		<b>bool localized</b>\n\n
		If true, then the parameter name returned should be localized in the language 3ds Max is currently using. Otherwise it should be the parameter name in English.
		If a plugin does not provide localized string resources, it can disregard this parameter and always return the parameter name in English.
		\return  The name of the parameter.
		\par Default Implementation:
		The default implementation returns <b>MSTR(_M("Parameter"))</b> */
		virtual MSTR GetParameterName(int pbIndex, bool localized) {return MSTR(_M("Parameter"));}
	};
#pragma warning(pop)
/*! \sa  Class SimpleObject2.\n\n
\par Description:
This class provides a method that is used to set the parameters of a box
object. This allows the object to be created programmatically. All methods of
this class are implemented by the system.  */
class GenBoxObject: public SimpleObject2 {	
	public:
	/*! \remarks This method is used to set the parameter of the box object.
	\par Parameters:
	<b>float width</b>\n\n
	The width of the box.\n\n
	<b>float height</b>\n\n
	The height of the box.\n\n
	<b>float length</b>\n\n
	The length of the box.\n\n
	<b>int wsegs=1</b>\n\n
	The number of width segments.\n\n
	<b>int lsegs=1</b>\n\n
	The number of length segments.\n\n
	<b>int hsegs=1</b>\n\n
	The number of height segments.\n\n
	<b>BOOL genUV=TRUE</b>\n\n
	Generate UV coordinates flag.
	\par Sample Code:
	The following code demonstrates the creation of the box object and the
	setting of its parameters:\n\n
	<b>GenBoxObject *gb = (GenBoxObject *)CreateInstance(GEOMOBJECT_CLASS_ID,BOXOBJ_CLASS_ID);</b>\n\n
	<b>gb-\>SetParams(10.0f, 10.0f, 10.0f, 1, 1, 1, TRUE);</b> */
	virtual void SetParams(float width, float height, float length, int wsegs=1,int lsegs=1, 
		int hsegs=1, BOOL genUV=TRUE)=0; 
	};

/*! \sa  Class SimpleObject.\n\n
\par Description:
This class provides a method that is used to set the parameters of a cylinder
object. This allows the object to be created programmatically. All methods of
this class are implemented by the system.  */
class GenCylinder: public SimpleObject2 {	
	public:
	/*! \remarks This method is used to set the parameter of the cylinder
	object.
	\par Parameters:
	<b>float rad</b>\n\n
	The radius.\n\n
	<b>float height</b>\n\n
	The height.\n\n
	<b>int segs</b>\n\n
	The number of segments.\n\n
	<b>int sides</b>\n\n
	The number of sides.\n\n
	<b>int capsegs=1</b>\n\n
	The number of segments in the cylinder cap.\n\n
	<b>BOOL smooth=TRUE</b>\n\n
	Smoothing flag.\n\n
	<b>BOOL genUV=TRUE</b>\n\n
	Generate UV coordinates flag.\n\n
	<b>BOOL sliceOn= FALSE</b>\n\n
	Slice the cylinder flag.\n\n
	<b>float slice1 = 0.0f</b>\n\n
	Slice angle1 in radians.\n\n
	<b>float slice2 = 0.0f</b>\n\n
	Slice angle2 in radians.
	\par Sample Code:
	The following code demonstrates the creation of the object and the setting
	of its parameters:\n\n
	<b>GenCylinder *gc = (GenCylinder *)CreateInstance(GEOMOBJECT_CLASS_ID,CYLINDER_CLASS_ID);</b>\n\n
	<b>gc-\>SetParams(10.0f, 50.0f, 1, 1);</b> */
	virtual void SetParams(float rad, float height, int segs, int sides, int capsegs=1, BOOL smooth=TRUE, 
		BOOL genUV=TRUE, BOOL sliceOn= FALSE, float slice1 = 0.0f, float slice2 = 0.0f)=0;
	};

/*! \sa  Class SimpleObject2.\n\n
\par Description:
This class provides a method that is used to set the parameters of a sphere
object. This allows the object to be created programmatically. All methods of
this class are implemented by the system.  */
class GenSphere: public SimpleObject2 {	
	public:
	/*! \remarks This method is used to set the parameter of the sphere
	object.
	\par Parameters:
	<b>float rad</b>\n\n
	The radius.\n\n
	<b>int segs</b>\n\n
	The number of segments.\n\n
	<b>BOOL smooth=TRUE</b>\n\n
	Smoothing flag.\n\n
	<b>BOOL genUV=TRUE</b>\n\n
	Generate UV coordinates flag.\n\n
	<b>float hemi=0.0f</b>\n\n
	The hemisphere setting where 0.0 is a full sphere, 0.5 is a half sphere and
	1.0 is nonexistent.\n\n
	<b>BOOL squash=FALSE</b>\n\n
	If TRUE the number of segments is kept constant when the sphere is a
	hemisphere. If FALSE the number of segments is truncated based on the
	hemisphere setting.\n\n
	<b>BOOL recenter=FALSE</b>\n\n
	If FALSE, the 'Base To Pivot' setting if off; otherwise it is on.
	\par Sample Code:
	The following code demonstrates the creation of the object and the setting
	of its parameters:\n\n
	\code
	#include "simpobj.h"   
	GenSphere *gs = (GenSphere *)CreateInstance(GEOMOBJECT_CLASS_ID,   
		Class_ID(SPHERE_CLASS_ID,0));   
	gs->SetParams(10.0f, //rad
		24, //segs  
		TRUE, //smooth,
		TRUE //genUV
		); 
	\endcode
	 */
	virtual void SetParams(float rad, int segs, BOOL smooth=TRUE, BOOL genUV=TRUE,
		 float hemi=0.0f, BOOL squash=FALSE, BOOL recenter=FALSE)=0;
	};


