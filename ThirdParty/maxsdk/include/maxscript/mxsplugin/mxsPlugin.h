/*	
 *		MSPlugin.h - MAXScript scriptable plugins for MAX
 *
 *	 A typical scripted plugin is defined by 3 MAXScript classes:
 *
 *		MSPlugin			base mixin for all scripted MAX plug-in classes
 *							this class inherits from Value so it can live
 *							in the MAXScript garbage-collected heap.
 * 
 *		MSPluginGeomObject	scripted GeomObjects
 *		MSPluginModifier	scripted Modifiers
 *		MSPluginControl		scripted Controls
 *		MSPluginLight		scripted Lights
 *		MSPluginMtl			scripted Materials, etc...
 *
 *		MSGeomObjectXtnd
 *		MSModifierXtnd
 *		MSControlXtnd		... These are subclasses of the above MSPlugin classes that
 *							extend an existing class (specified in the extends: param).  
 *							Instances contain a delegate, a ref to an owned instance of 
 *							the class under extension and
 *							bounce all calls to it (delegation) then specialize UI calls 
 *							to allow UI replacement or extra rollouts for the delegate.  
 *
 *      MSPluginClass		MAXClass specialization whose instances represent scripted
 *							plugin classes, contains all the definition-level stuff
 *							and a pointer to the MSPluginDesc for the class.
 *							It is applyable for scripted instance creation.  It is also 
 *							kept in a special 
 *							hashtable to enable repeated re-definition in the scripter,
 *							the same value is updated on each re-definition.
 *							This class inherits also from MAXClass and so lives in
 *							the MAXScript heap. 
 *
 *		MSPluginDesc		ClassDesc2 specialization for MSPlugin classes.
 *							Unlike most ClassDescs, many instances of this are
 *						    created, one per scripted plugin.
 *							Instances contain a pointer to the corresponding
 *							MSPluginClass instance from which info for the implementing
 *							the standard ClassDesc2 interface is derived.
 *
 *			Copyright (c) Autodesk, Inc, 1998.  John Wainwright.
 *
 */

#pragma once

#include "../kernel/value.h"
#include "../maxwrapper/mxsobjects.h"
#include "../../ref.h"
#include "../../iparamm2.h"
#include "../../iparamb2.h"
#include "../../iparamb2Typedefs.h"
#include "../../IMtlRender_Compatibility.h"
#include "../../genlight.h"
#include "../../gencam.h"
#include "../../simpobj.h"
#include "../../simpspl.h"
#include "../../manipulator.h"
#include "../../simpmod.h"
#include "../../tvutil.h"
#include "../../control.h"
#include "../../ILockedTracks.h"
#include "../../ILockedContainerUpdate.h"

class MSPlugin;
class MSPluginDesc;
class MSPluginClass;
class MSAutoMParamDlg;
class MSAutoEParamDlg;
class Point3Value;
class Matrix3Value;
class Box3Value;
class MouseTool;

// plugin context predefined local indexes - MUST match order in Parser::plugin_def() and Parser::attributes_body()
enum { 
	pl_this, pl_version, pl_loading, pl_delegate			// common
};

enum { 
	pl_extent = pl_delegate + 1, pl_min, pl_max, pl_center   // for SimpleMods
};

enum { 
	pl_mesh2 = pl_center + 1, pl_transform, pl_inverseTransform, pl_bbox, pl_owningNode,  	// for SimpleMeshMods
};

enum { 
	pl_mesh = pl_delegate + 1								// for SimpleObjects 
};

enum { 
	pl_beziershape = pl_delegate + 1							// for SimpleSplines 
};

enum { 
	pl_target = pl_delegate + 1, pl_node,				// for SimpleManipulators 
	pl_gizmoDontDisplay, pl_gizmoDontHitTest, pl_gizmoScaleToViewport,
	pl_gizmoUseScreenSpace, pl_gizmoActiveViewportOnly, pl_gizmoUseRelativeScreenSpace, pl_gizmoApplyUIScaling,
};

enum { 
	pl_isLeaf = pl_delegate + 1,							// for scripted controller plugins 
	pl_isKeyable,
	pl_method,
	pl_parentTransform,										// these 2 are actively used only for P/R/S/Transform controllers
	pl_usesParentTransform
};


using creator_fn = RefTargetHandle (*)(MSPluginClass* pic, BOOL loading);

// parameter reference (used by subtex and submtl mechanism in scripted texmaps & mtls)
class ParamRef
{
public:
	BlockID	block_id;
	ParamID param_id;
	int		tabIndex;
	ParamRef(BlockID bid, ParamID pid, int index) { block_id = bid; param_id = pid; tabIndex = index; }
};

// ----------------------- MSPluginClass -----------------------
//	MAXClass specialization for scripted plug-in classes

visible_class (MSPluginClass)

class MSPluginClass : public MAXClass
{
protected:
					MSPluginClass();
	void			ctor_init();	// initializes member variables
	bool			validate_ExtendingClass_failed; // true if last call to ValidateIfExtendingClass failed to set extend_cd
public:
	Value*			class_name;		// UI-visible class name - Localized 
	Value*			nonLocalized_class_name;		// UI-visible class name - Non-Localized 
	HINSTANCE		hInstance;		// owning module
	ClassDesc*		extend_cd;		// if extending, ClassDesc of class to extend
	MAXClass*		extend_maxclass;	// if extending, MAXClass of class to extend
	creator_fn		obj_creator;	// obj maker for the appropriate MSPlugin subclass	
	HashTable*		local_scope;	// local name space	
	Value**			local_inits;	// local var init vals	
	int				local_count;	//   "    "  count	
	HashTable*		handlers;		// handler tables	
	Array*			rollouts;		// UI rollouts
	MouseTool*		create_tool;	// scripted creation tool if non-NULL
	Array*			pblock_defs;	// parameter block definition data from compiler (used to build the PB2 descs)
	Array*			remap_param_names;	// defines the mapping of old param names to new param names
	Array*			loading_pblock_defs; // parameter block definition data for currently loading scene file (to permit old version schema migration)
	Tab<ParamBlockDesc2*> pbds;		// parameter block descriptors
	ReferenceTarget* alternate;		// any alternate UI object system-style during create mode
	Tab<ParamRef>	sub_texmaps;	// param references to any texmaps in pblocks in instances of this class in subobjno order
	Tab<ParamRef>	sub_mtls;		// param references to any mtls in pblocks in instances of this class in subobjno order
	int				version;		// plugin version (from version: param on def header)
	DWORD			mpc_flags;		// flags	
	DWORD			rollup_state;	// initial rollup state

	static HashTable* msp_classes;	// table of existing scripted plugin classes to enable redefinition
	static MSPlugin* creating;		// object currently being created if non-NULL
	static bool		 loading;		// currently loading defs from a scene file, delay TV & other updates
	static bool		 loadingLoadingScene;		// if loading, doing a load scene as opposed to a merge

					MSPluginClass(Value* name, MAXSuperClass* supcls, creator_fn cfn);
				   ~MSPluginClass();

	// definition and redefinition
	static MSPluginClass* intern(Value* name, MAXSuperClass* supcls, creator_fn cfn);
	void			init(int iLocal_count, Value** inits, HashTable *pLocal_scope, HashTable *pHandlers, Array *pBlock_defs, Array* iremap_param_names, Array *pRollouts, MouseTool *pCreate_tool);

	// MAXScript required
	BOOL			is_kind_of(ValueMetaClass* c) override { return (c == class_tag(MSPluginClass)) ? 1 : MAXClass::is_kind_of(c); }
#	define			is_msplugin_class(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MSPluginClass))
	void			gc_trace() override;
	void			collect() override;

	// from Value 
	Value*			apply(Value** arglist, int count, CallContext* cc=nullptr) override;		// object creation by applying class object

	// local
	void			SetClassID(Value* cidv);
	void			SetExtension(Value* cls); // sets the MAXClass this MSPluginClass extends, if any.
	// during startup, we can run into case where the MAXClass we are extending does not point at a valid ClassDesc yet. For example, if have a 
	// scripted plugin extending DirectX_9_Shader in stdplugs/stdscripts, the 'DirectX 9 Shader' classDesc hasn't been created yet. That is created
	// later during startup via a GUP. ValidateIfExtendingClass is used to set extend_cd if we are extending a class, but we couldn't set it when 
	// the MSPluginClass was created. If ValidateIfExtendingClass fails to set extend_cd, it sets a flag that it failed to do so, and returns false. 
	// Later calls check that flag first, and if false, and forceRevalidate is false, it simply returns false. 
	bool			ValidateIfExtendingClass(bool forceRevalidate = false); 
	void			SetVersion(Value* ver) { version = ver->to_int(); }
	void			StopEditing(int stop_flags = 0);
	void			RestartEditing();
	static int			lookup_assetType(Value* name);
	//! \deprecated This function was deprecated in Max 2021. Please use MSPluginClass::lookup_paramType_from_name() instead.
	MAX_DEPRECATED static ParamType2	lookup_type(Value* name);
	ScripterExport static ParamType2	lookup_paramType_from_name(Value* name); // returns (ParamType2)-1 if not found
	ScripterExport static const Name*		lookup_name_from_paramType(ParamType2 type); // returns nullptr if not found
	void			call_handler(Value* handler, Value** arg_list, int count, TimeValue t, BOOL disable_on_error=FALSE);
	// low level handler call, returns result from handler, but needs init_thread_locals() & push/pop_alloc_frame around it
	Value*			_call_handler(Value* handler, Value** arg_list, int count, TimeValue t, BOOL disable_on_error=FALSE);

	virtual	bool	is_custAttribDef() { return false; }

	// alternate UI
	void			install_alternate(ReferenceTarget* ref);
	// schema migration
	void			build_paramblk_descs();
	void			redefine(MSPlugin* val, HashTable* old_locals, Array* old_pblock_defs);
	Array*			find_pblock_def(Value *pName, Array *pBlock_defs);
	// scene I/O
	static void		save_class_defs(ISave* isave);
	static IOResult load_class_def(ILoad* iload);
	static void		post_load(ILoad *iload, int which);

	// ClassDesc delegates
	virtual BOOL	OkToCreate(Interface *i);	
	virtual RefTargetHandle	Create(BOOL isloading);
	const MCHAR*	ClassName() { return class_name->to_string(); }
	const MCHAR*	NonLocalizedClassName() { return nonLocalized_class_name->to_string(); }
	const MCHAR*	InternalName() { return name->to_string(); }
	SClass_ID		SuperClassID() { return sclass_id; }
	Class_ID		ClassID() { return class_id; }
	const MCHAR* 	Category() { return category ? category->to_string() : _M(""); }
	HINSTANCE		HInstance() { return hInstance; }
	BOOL			IsManipulator();
	BOOL            CanManipulate(ReferenceTarget* hTarget);
	BOOL			CanManipulateNode(INode* pNode);
	Manipulator*	CreateManipulator(ReferenceTarget* hTarget, INode* pNode);
	Manipulator*	CreateManipulator(INode* pNode);

	Value*			get_property(Value** arg_list, int count) override;
	Value*			set_property(Value** arg_list, int count) override;

#include "../macros/define_implementations.h"
	// props
	def_prop_getter(name);
};

#ifdef _DEBUG
//#define DEBUG_PARAMDEF_SIZES
#ifdef DEBUG_PARAMDEF_SIZES
void PrintMSPluginClassPDBParamMetrics(ParamBlockDesc2* pdb);
#endif // DEBUG_PARAMDEF_SIZES
#endif // _DEBUG

// plugin class flags  //AF (4/24/01) redefined these flags because there were conflicts when bit-fiddling with them...
#define MPC_TEMPORARY			(1<<0) //0x0001	// no classID: temporary, cannot be saved in a scene file, shouldn't be wired in to the scene anywhere
#define MPC_REDEFINITION		(1<<1) //0x0002	// class redefinition under way
#define MPC_MS_CREATING			(1<<2) //0x0004	// currently creating new object
#define MPC_MS_WAS_CREATING		(1<<3) //0x0008	// MAXScript was creating during a redefinition
#define MPC_EDITING_IN_CP		(1<<4) //0x0010	// currently editing obj in command panel
#define MPC_EDITING_IN_MTLEDT	(1<<5) //0x0020	// currently editing obj in material editor
#define MPC_EDITING_IN_EFX		(1<<6) //0x0040	// currently editing obj in render effects/environment editor
#define MPC_REPLACE_UI			(1<<7) //0x0080	// for extending classes, scripted UI completely replaces delegates UI
#define MPC_INVISIBLE			(1<<8) //0x0100	// doesn't show up in create panel buttons, useful for controlling dummies, etc.
#define MPC_SILENTERRORS		(1<<9) //0x0200  // don't report errors
#define MPC_MAX_CREATING		(1<<10) //0x0400  // in default MAX creation mode
#define MPC_ABORT_CREATE		(1<<11) //0x0800  // plugin should abort MAX create mode on next GetMouseCreateCallback
#define MPC_LEVEL_6				(1<<12) //0x1000  // level 6 plugin; supports full, stand-alone creatability
#define MPC_IS_MANIP			(1<<13) //0x2000  // is a manipulator plugin
#define MPC_ALTERNATE			(1<<14) //0x4000  // is currently an alternate
#define MPC_CAD_FILESAVE		(1<<15) //0x8000  // custom attribute def used by saved instance of scripted plugin
#define MPC_PROMOTE_DEL_PROPS			(1<<16) //0x00010000  // If set, automatically search delegate props on prop miss in scripted plugin
#define MPC_USE_PB_VALIDITY				(1<<17) //0x00020000  // If set, AND delegate's validity interval with param blocks' validity interval
#define MPC_CAD_FILELOAD_LOADDEFDATA	(1<<18) //0x00040000  // custom attribute definition defined or redefined during scene file load. Load its defData member.
#define MPC_CAD_NO_ATTRIBID				(1<<19) //0x00080000  // custom attribute definition attribID not specified.

// for accessing keyword params in pblock_defs
#define key_parm(_key)	_get_key_param(keys, n_##_key)
#define bool_key_parm(_key, var, def)		((var = _get_key_param(keys, n_##_key)) == &unsupplied ? def : var->to_bool())
extern Value* _get_key_param(Array* keys, Value* key_name);

// ----------------------- MSPluginDesc -----------------------
//	ClassDescs for scripted classes, created dynamically for each scripted class

class MSPluginDesc : public ClassDesc2, public IMtlRender_Compatibility_MtlBase
{
public:
	MSPluginClass*	pc;			// my MAXScript-side plugin class
	MSPlugin*		plugin;		// object under creation, MSPlugin interface
	RefTargetHandle base_obj;	//   "      "       "     base object interface

	MSPluginDesc(MSPluginClass* ipc) { pc = ipc; Init(*this); }

	// from ClassDesc
	int 			IsPublic() override;
	BOOL	OkToCreate(Interface *i) override { return pc->OkToCreate(i); }
	void*			Create(BOOL loading = FALSE) override { return pc->Create(loading); }
	const MCHAR*	ClassName() override { return pc->ClassName(); }
	const MCHAR*	NonLocalizedClassName() override { return pc->NonLocalizedClassName(); }
	const MCHAR*	InternalName() override { return pc->InternalName(); }
	SClass_ID		SuperClassID() override { return pc->SuperClassID(); }
	Class_ID		ClassID() override { return pc->ClassID(); }
	const MCHAR* 	Category() override { return pc->Category(); }
	int 			BeginCreate(Interface *i) override;
	int 			EndCreate(Interface *i) override;
	void			ResetClassParams(BOOL fileReset) override;
	DWORD			InitialRollupPageState() override;
	// manipulator methods
	BOOL			IsManipulator() override { return pc->IsManipulator(); }
	BOOL			CanManipulate(ReferenceTarget* hTarget) override { return pc->CanManipulate(hTarget); }
	BOOL			CanManipulateNode(INode* pNode) override { return pc->CanManipulateNode(pNode); }
	Manipulator*	CreateManipulator(ReferenceTarget* hTarget, INode* pNode) override { return pc->CreateManipulator(hTarget, pNode); }
	Manipulator*	CreateManipulator(INode* pNode) override { return pc->CreateManipulator(pNode); }

	HINSTANCE		HInstance() override { return pc->HInstance(); }
	const MCHAR*	GetString(INT_PTR id) { return id != 0 ? (const MCHAR*)id : nullptr; } // resIDs are actual string ptrs in msplugins...
	const MCHAR*	GetRsrcString(INT_PTR id) override { return id != 0 ? (const MCHAR*)id : nullptr; }

	// local 
	void			StartTool(IObjCreate *iob);   // start up scripted create tool
	void			StopTool(IObjCreate *iob);   // stop scripted create tool

	Class_ID		SubClassID() override;
	// Class descriptor of a Renderer plugin
	bool IsCompatibleWithRenderer(ClassDesc& rendererClassDesc) override;
	bool GetCustomMtlBrowserIcon(HIMAGELIST& hImageList, int& inactiveIndex, int& activeIndex, int& disabledIndex) override;
};

// ----------------------- MSPluginPBAccessor -----------------------
//	paramblock accessor topass gets & sets to scripted handlers

class MSPluginPBAccessor : public PBAccessor
{
	BlockID			bid;
	MSPluginClass*	pc;
public:
	MSPluginPBAccessor(MSPluginClass* ipc, BlockID id);
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override;
	void PreSet(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override;
	void PostSet(const PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override;
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval& valid) override;
	BOOL KeyFrameAtTime(ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override;
	void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, ReferenceMaker* owner, ParamID id, int tabIndex, int count) override;
	void DeleteThis() override;

private:
	Value* to_value(const PB2Value& v, const ParamDef& pd);
	void from_value(PB2Value& v, const ParamDef& pd, Value* val, bool inGetHandler);
};

// ----------------------- MSPlugin -----------------------
//	base mixin class for MAX-side scripted classes

#define MSP_LOADING		0x0001			// instance is currently being loaded from a scene
#define MSP_DISABLED	0x0002			// general disablement flag, used to diable plugin in case of handler errors, reset by redefinition
#define MSP_DELETED		0x0004			// I'm deleted in the MAX world

class MSPlugin : public Value
{
public:
	MSPluginClass*	pc;				// my class
	Value**			locals;			// local var array	
	short			flags;			// plugin flags	
	int				version;		// plugin version
	ReferenceTarget* ref;			// ReferenceTarget interface to me
	Tab<IParamBlock2*> pblocks;		// parameter blocks
	ILoad*			iload;			// ILoad that the plugin instance was created from
	
					MSPlugin() { flags = 0; }
	virtual		   ~MSPlugin();
	void			init(MSPluginClass *pClass);

	ScripterExport	void			gc_trace() override;
	ScripterExport	void			collect() override;

	void			DeleteThis();	// drops all references to/from me

	// code management
	void			init_locals();
	void			call_handler(Value* handler, Value** arg_list, int count, TimeValue t, BOOL disable_on_error=FALSE);
	// low level handler call, returns result from handler, but needs init_thread_locals() & push/pop_alloc_frame around it
	Value*			_call_handler(Value* handler, Value** arg_list, int count, TimeValue t, BOOL disable_on_error=FALSE);
	FPStatus		call_handler(const MCHAR* handler_name, FPParams* params, FPValue& result, TimeValue t, BOOL disable_on_error=FALSE);
	FPStatus		call_handler(Value* handler, FPParams* params, FPValue& result, TimeValue t, BOOL disable_on_error=FALSE);
	void			post_create(ReferenceTarget* me, BOOL loading);
	void			call_all_set_handlers();
	void			disable() { flags |= MSP_DISABLED; }
	void			enable() { flags &= ~MSP_DISABLED; }
	BOOL			enabled() { return !(flags & MSP_DISABLED); }

	// locals
	int				get_local_index(Value* prop);
	Value*			get_local(int index) { return locals[index]; }
	void			set_local(int index, Value* val) { locals[index] = heap_ptr(val); }

	// block management
	ScripterExport	IParamBlock2*	GetParamBlockByID(BlockID id);

	// UI 
	virtual HWND	AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0, DWORD vflags=0, int category=ROLLUP_CAT_STANDARD)=0;
	virtual void	DeleteRollupPage(HWND hRollup)=0;
	virtual IRollupWindow* GetRollupWindow()=0;
	virtual void	RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )=0;

	// factored ReferenceTarget stuff
	ScripterExport	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);
	ScripterExport	RefTargetHandle clone_from(MSPlugin* obj, ReferenceTarget* obj_as_ref, RemapDir& remap);
	void			RefAdded(RefMakerHandle rm);
	ScripterExport	void			RefDeleted(RefMakerHandle rm);
	void			NotifyTarget(int msg, RefMakerHandle rm);

	// delegate access 
	virtual ReferenceTarget* get_delegate()=0;

	// I/O
	IOResult		Save(ISave *isave) override;
	IOResult		Load(ILoad *iLoad);
	void			post_load(ILoad *iLoad, int which);

	// added 3/21/05. Used by debugger to dump locals and externals to standard out
	void			dump_local_vars_and_externals(int indentLevel);
};

// used for in-memory instance migration when a scripted plugin class is redefined
// NOTE: MSPluginValueMigrator is obsolete - just leaving in for now to prevent sdk break
class MSPluginValueMigrator : public ValueMapper
{
	MSPluginClass*	pc;
	HashTable*		old_locals;
	Array*			old_pblock_defs;
public:

	MSPluginValueMigrator(MSPluginClass* pc, HashTable* old_locals, Array* old_pblock_defs)
	{
		this->pc = pc;
		this->old_locals = old_locals;
		this->old_pblock_defs = old_pblock_defs;
	}

	void map(Value* val) override 
	{ 
		if (((MSPlugin*)val)->pc == pc) 
			pc->redefine((MSPlugin*)val, old_locals, old_pblock_defs); 
	}
};

// used for in-memory instance migration when a scripted plugin class is redefined
class MSPluginValueMigratorEx : public ValueMapper
{
	MSPluginClass* pc;
	HashTable* old_locals;
	Array* old_pblock_defs;
	Array* plugins_to_migrate;

public:
	MSPluginValueMigratorEx(MSPluginClass* pc, HashTable* old_locals, Array* old_pblock_defs, Array* plugins_to_migrate)
			: pc(pc)
			, old_locals(old_locals)
			, old_pblock_defs(old_pblock_defs)
			, plugins_to_migrate(plugins_to_migrate)
	{
	}
	void map(Value* val) override;
	void DoMigration();
};

#define MSPLUGIN_CHUNK 0x0010
#pragma warning(push)
#pragma warning(disable: 4239  4100)
// ----------------------- MSPluginObject ----------------------

//	template for MSPlugin classes derived from Object
template <class TYPE> class MSPluginObject : public MSPlugin, public TYPE
{
public:
	IObjParam*		ip;					// ip for any currently open command panel dialogs

	void			DeleteThis() override;
#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")
	MSPluginObject() : TYPE(){}
	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD vflags=0, int category=ROLLUP_CAT_STANDARD) override;
	void			DeleteRollupPage(HWND hRollup) override;
	IRollupWindow*	GetRollupWindow() override;
	void			RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) override;

	ReferenceTarget* get_delegate() override { return nullptr; }  // no delegate 

	// From Animatable
	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	void			FreeCaches() override { } 		
	int				NumSubs() override { return pblocks.Count(); }
	Animatable*		SubAnim(int i) override { return pblocks[i]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (pblocks[i]->GetDesc()->local_name != 0)
		{
			return localized ? pblocks[i]->GetLocalName() : pblocks[i]->GetDesc()->int_name.data();
		}
		return _T("");
	}
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return TYPE::GetInterface(id); }
    
    
	BaseInterface*GetInterface(Interface_ID id) override { 
        ///////////////////////////////////////////////////////////////////////////
        // GetInterface(Interface_ID) was added after the MAX 4
        // SDK shipped. This did not break the SDK because
        // it only calls the base class implementation. If you add
        // any other code here, plugins compiled with the MAX 4 SDK
        // that derive from MSPluginObject and call Base class
        // implementations of GetInterface(Interface_ID), will not call
        // that code in this routine. This means that the interface
        // you are adding will not be exposed for these objects,
        // and could have unexpected results.
        return TYPE::GetInterface(id);  
        /////////////////////////////////////////////////////////////////////////////
    }
   

	// From ReferenceMaker
	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override 
					{ 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
					}

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
	void			SetReference(int i, RefTargetHandle rtarg) override;
	void			RefDeleted(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); }
	void			RefAdded(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); }
	void			RefAddedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); };
	void			RefDeletedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); };
	IOResult		Save(ISave* iSave) override { return MSPlugin::Save(iSave); }
	IOResult		Load(ILoad *iLoad) override { return MSPlugin::Load(iLoad); }
	void			NotifyTarget(int msg, RefMakerHandle rm) override { MSPlugin::NotifyTarget(msg, rm); }

	// From BaseObject
	const MCHAR*	GetObjectName(bool localized = true) const override { return (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	void			BeginEditParams( IObjParam *objParam, ULONG vflags, Animatable *pPrev) override;
	void			EndEditParams( IObjParam *objParam, ULONG vflags,Animatable *pNext) override;
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt) override { return 0; }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags) override { return 0; }
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) override { }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) override { }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override { }
	CreateMouseCallBack* GetCreateMouseCallBack() override { return nullptr; } 	
	BOOL			HasUVW() override { return 1; }
	void			SetGenUVW(BOOL sw) override { }
	
	// From Object
	ObjectState		Eval(TimeValue time) override { return ObjectState(this); }
	void			InitNodeName(MSTR& s) override {s = GetObjectName(true);} // non-localized name
	Interval		ObjectValidity(TimeValue t) override { return FOREVER; }
	int				CanConvertToType(Class_ID obtype) override { return 0; }
	Object*			ConvertToType(TimeValue t, Class_ID obtype) override { return nullptr; }
	void			GetCollapseTypes(Tab<Class_ID> &clist, Tab<MSTR*> &nlist) override { }
	void			GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel) override { Object::GetDeformBBox(t, box, tm, useSel); }
	int				IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm) override { return 0; }

};

//	template for MSPlugin Xtnd classes derived from Object
template <class TYPE, class MS_SUPER> class MSObjectXtnd : public MS_SUPER
{
public:
	TYPE*			delegate;		// my delegate

	void			DeleteThis() override;

	MSObjectXtnd() : MS_SUPER() {}

	// From MSPlugin
	ReferenceTarget* get_delegate() override { return delegate; } 

	// From Animatable
	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? MS_SUPER::pc->ClassName() : MS_SUPER::pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return MS_SUPER::pc->class_id; }
	void			FreeCaches() override { }; // { delegate->FreeCaches(); }
	int				NumSubs() override { return MS_SUPER::pblocks.Count() + 1; }
	Animatable*		SubAnim(int i) override { if (i == 0) return delegate; return MS_SUPER::pblocks[i-1]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (i == 0)
		{
			return delegate->GetObjectName(localized);
		}
		if (MS_SUPER::pblocks[i - 1]->GetDesc()->local_name != 0)
		{
			return localized ? MS_SUPER::pblocks[i - 1]->GetLocalName() : MS_SUPER::pblocks[i - 1]->GetDesc()->int_name.data();
		}
		return _T("");
	}
	int				NumParamBlocks() override { return MS_SUPER::pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return MS_SUPER::pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return MS_SUPER::GetInterface(id); }
	
    
	BaseInterface* GetInterface(Interface_ID id) override{
        ///////////////////////////////////////////////////////////////////////////
        // GetInterface(Interface_ID) was added after the MAX 4
        // SDK shipped. This did not break the SDK because
        // it only calls the base class implementation. If you add
        // any other code here, plugins compiled with the MAX 4 SDK
        // that derive from MSObjectXtnd and call Base class
        // implementations of GetInterface(Interface_ID), will not call
        // that code in this routine. This means that the interface
        // you are adding will not be exposed for these objects,
        // and could have unexpected results.
        return MS_SUPER::GetInterface(id); 
	    ////////////////////////////////////////////////////////////////////////////
    }

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
	void			SetReference(int i, RefTargetHandle rtarg) override; 

	// From BaseObject
	const MCHAR*	GetObjectName(bool localized = true) const override { return (localized) ? MS_SUPER::pc->ClassName() : MS_SUPER::pc->NonLocalizedClassName(); }
	void			BeginEditParams( IObjParam *objParam, ULONG vflags, Animatable *pPrev) override;
	void			EndEditParams( IObjParam *objParam, ULONG vflags, Animatable *pNext) override;
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt) override
						{ return delegate->HitTest(t, inode, type, crossing, vflags, p, vpt); }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags) override 
					{ return delegate->Display(t, inode, vpt, vflags); }
	// IObjectDisplay2 entries
	unsigned long GetObjectDisplayRequirement() const override;
	bool PrepareDisplay(
		const MaxSDK::Graphics::UpdateDisplayContext& prepareDisplayContext) override;

	bool UpdatePerNodeItems(
		const MaxSDK::Graphics::UpdateDisplayContext& updateDisplayContext,
		MaxSDK::Graphics::UpdateNodeContext& nodeContext,
		MaxSDK::Graphics::IRenderItemContainer& targetRenderItemContainer) override;

	bool UpdatePerViewItems(
		const MaxSDK::Graphics::UpdateDisplayContext& updateDisplayContext,
		MaxSDK::Graphics::UpdateNodeContext& nodeContext,
		MaxSDK::Graphics::UpdateViewContext& viewContext,
		MaxSDK::Graphics::IRenderItemContainer& targetRenderItemContainer) override;
	const MaxSDK::Graphics::RenderItemHandleArray& GetRenderItems() const override
					{ return delegate->GetRenderItems(); }
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) override { delegate->GetWorldBoundBox(t, inode, vpt, box); }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) override { delegate->GetLocalBoundBox(t, inode, vpt,  box ); }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override { delegate->Snap(t, inode, snap, p, vpt); }
	CreateMouseCallBack* GetCreateMouseCallBack() override;
	BOOL			HasUVW() override { return delegate->HasUVW(); }
	void			SetGenUVW(BOOL sw) override { delegate->SetGenUVW(sw); }
	void			SetExtendedDisplay(int vflags) override { delegate->SetExtendedDisplay( vflags); }      // for setting mode-dependent display attributes

	// From Object
	ObjectState		Eval(TimeValue time) override;
	void			InitNodeName(MSTR& s) override {s = GetObjectName(true);} // non-localized name
	Interval		ObjectValidity(TimeValue t) override;
	int				CanConvertToType(Class_ID obtype) override 
					{ 
						if (obtype == anyObjectClassID)
							return TRUE;
						return delegate->CanConvertToType(obtype); 
					}
	Object*			ConvertToType(TimeValue t, Class_ID obtype) override 
					{
						if (obtype == anyObjectClassID)
							return this;
						// CAL-10/1/2002: Don't return the delegate, because it might be deleted.
						//		Return a copy of the delegate instead. (422964)
						Object* obj = delegate->ConvertToType(t, obtype);
						if (obj == delegate) 
						{
							obj = delegate->MakeShallowCopy(OBJ_CHANNELS);
							obj->LockChannels(OBJ_CHANNELS); //if we shallow copy these channels they need to be locked since they will get double deleted
						}
						return obj;
					}
	void			GetCollapseTypes(Tab<Class_ID> &clist, Tab<MSTR*> &nlist) override { delegate->GetCollapseTypes(clist, nlist); }
	void			GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel) override { delegate->GetDeformBBox(t, box, tm, useSel); }
	int				IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm) override { return delegate->IntersectRay(t, r, at, norm); }

};

// ----------------------- MSPluginGeomObject ----------------------
//  scripted GeomObject 

class MSPluginGeomObject : public MSPluginObject<GeomObject>
{
public:
					MSPluginGeomObject() { }
					MSPluginGeomObject(MSPluginClass* pc, BOOL loading);
				   ~MSPluginGeomObject() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap) override;

	// From GeomObject
	int				IsRenderable() override { return 0; }		
	Mesh*			GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete) override { return GeomObject::GetRenderMesh(t, inode, view, needDelete); }
};

class MSGeomObjectXtnd : public MSObjectXtnd<GeomObject, MSPluginGeomObject>
{
public:
					MSGeomObjectXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSGeomObjectXtnd() { DeleteAllRefsFromMe(); }

	RefTargetHandle Clone(RemapDir& remap) override;
	// From GeomObject
	int				IsRenderable() override { return delegate->IsRenderable(); }		
	Mesh*			GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete) override { return delegate->GetRenderMesh(t, inode, view, needDelete); }
};

// ----------------------- MSPluginHelper ----------------------
// scripted HelperObject

class MSPluginHelper : public MSPluginObject<HelperObject>
{
public:
					MSPluginHelper() { }
					MSPluginHelper(MSPluginClass* pc, BOOL loading);
				   ~MSPluginHelper() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap) override;

	// From HelperObject
	int				UsesWireColor() override { return HelperObject::UsesWireColor(); }   // TRUE if the object color is used for display
	BOOL			NormalAlignVector(TimeValue t,Point3 &pt, Point3 &norm) override { return HelperObject::NormalAlignVector(t, pt, norm); }
};

class MSHelperXtnd : public MSObjectXtnd<HelperObject, MSPluginHelper>
{
public:
					MSHelperXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSHelperXtnd() { DeleteAllRefsFromMe(); }

	RefTargetHandle Clone(RemapDir& remap) override;

	// From BaseObject
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags) override;

	void			GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& abox ) override;
	void			GetLocalBoundBox(TimeValue t, INode *inode, ViewExp *vpt, Box3& abox ) override;
	void			GetDeformBBox(TimeValue t, Box3& abox, Matrix3 *tm, BOOL useSel ) override;
	int				HitTest(TimeValue t, INode *inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt) override;
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override;

	// From Object
	ObjectState		Eval(TimeValue time) override;
	Interval		ObjectValidity(TimeValue t) override;

	// From HelperObject
	int				UsesWireColor() override;
	BOOL			NormalAlignVector(TimeValue t,Point3 &pt, Point3 &norm) override;
};

// ----------------------- MSPluginLight ----------------------
// scripted GenLight

class MSPluginLight : public MSPluginObject<GenLight>
{
public:
	ExclList			exclusionList;

					MSPluginLight() { }
					MSPluginLight(MSPluginClass* pc, BOOL loading);
				   ~MSPluginLight() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap) override;

	// From LightObject
	RefResult		EvalLightState(TimeValue time, Interval& valid, LightState *ls) override { return REF_SUCCEED; }
	ObjLightDesc *	CreateLightDesc(INode *n, BOOL forceShadowBuf) override { return LightObject::CreateLightDesc(n, forceShadowBuf); }
	void			SetUseLight(int onOff) override { }
	BOOL			GetUseLight() override { return FALSE; }
	void			SetHotspot(TimeValue time, float f) override { } 
	float			GetHotspot(TimeValue t, Interval& valid) override { return 0.0f; }
	void			SetFallsize(TimeValue time, float f) override { }
	float			GetFallsize(TimeValue t, Interval& valid) override { return 0.0f; }
	void			SetAtten(TimeValue time, int which, float f) override { }
	float			GetAtten(TimeValue t, int which, Interval& valid) override { return 0.0f; }
	void			SetTDist(TimeValue time, float f) override { }
	float			GetTDist(TimeValue t, Interval& valid) override { return 0.0f; }
	void			SetConeDisplay(int s, int notify=TRUE) override { }
	BOOL			GetConeDisplay() override { return FALSE; }
	int 			GetShadowMethod() override {return LIGHTSHADOW_NONE;}
	void 			SetRGBColor(TimeValue t, const Point3& rgb) override { }
	Point3 			GetRGBColor(TimeValue t, Interval& valid) override {return Point3(0,0,0);}        
	void 			SetIntensity(TimeValue time, float f) override { }
	float 			GetIntensity(TimeValue t, Interval& valid) override {return 0.0f;}
	void 			SetAspect(TimeValue t, float f) override { }
	float			GetAspect(TimeValue t, Interval& valid) override {return 0.0f;}    
	void 			SetUseAtten(int s) override { }
	BOOL 			GetUseAtten() override {return FALSE;}
	void 			SetAttenDisplay(int s) override { }
	BOOL 			GetAttenDisplay() override {return FALSE;}      
	void 			Enable(int enab) override { }
	void 			SetMapBias(TimeValue t, float f) override { }
	float 			GetMapBias(TimeValue t, Interval& valid) override {return 0.0f;}
	void 			SetMapRange(TimeValue t, float f) override { }
	float 			GetMapRange(TimeValue t, Interval& valid) override {return 0.0f;}
	void 			SetMapSize(TimeValue t, int f) override { }
	int 			GetMapSize(TimeValue t, Interval& valid) override {return 0;}
	void 			SetRayBias(TimeValue t, float f) override { }
	float 			GetRayBias(TimeValue t, Interval& valid) override {return 0.0f;}
	int 			GetUseGlobal() override {return 0;}
	void 			SetUseGlobal(int a) override { }
	int 			GetShadow() override {return 0;}
	void 			SetShadow(int a) override { }
	int 			GetShadowType() override {return 0;}
	void 			SetShadowType(int a) override { }
	int 			GetAbsMapBias() override {return 0;}
	void 			SetAbsMapBias(int a) override { }
	int 			GetOvershoot() override {return 0;}
	void 			SetOvershoot(int a) override { }
	int 			GetProjector() override {return 0;}
	void 			SetProjector(int a) override { }
	ExclList* 		GetExclList() override { return &exclusionList; }
	BOOL 			Include() override {return FALSE;}
	Texmap* 		GetProjMap() override {return nullptr;}
	void 			SetProjMap(Texmap* pmap) override { }
	void 			UpdateTargDistance(TimeValue t, INode* inode) override { }
	int				UsesWireColor() override { return LightObject::UsesWireColor(); }   // TRUE if the object color is used for display

	// From GenLight

	GenLight*		NewLight(int type) override { return nullptr; }
	int				Type() override { return 0; }  // OMNI_LIGHT, TSPOT_LIGHT, DIR_LIGHT, FSPOT_LIGHT, TDIR_LIGHT
	void			SetType(int tp) override { } // OMNI_LIGHT, TSPOT_LIGHT, DIR_LIGHT, FSPOT_LIGHT, TDIR_LIGHT      
	BOOL			IsSpot() override { return FALSE; }
	BOOL			IsDir() override { return FALSE; }
	void			SetSpotShape(int s) override { }
	int				GetSpotShape() override { return 0; }
	void 			SetHSVColor(TimeValue t, Point3& hsv) override { }
	Point3 			GetHSVColor(TimeValue t, Interval& valid) override { return Point3(0.0f, 0.0f, 0.0f); }
	void 			SetContrast(TimeValue time, float f) override { }
	float 			GetContrast(TimeValue t, Interval& valid) override { return 0.0f; }
	void 			SetUseAttenNear(int s) override { }
	BOOL 			GetUseAttenNear() override { return FALSE; }
	void 			SetAttenNearDisplay(int s) override { }
	BOOL 			GetAttenNearDisplay() override { return FALSE; }

	ExclList&		GetExclusionList() override { return exclusionList; }
	void 			SetExclusionList(ExclList &list) override { }

	BOOL 			SetHotSpotControl(Control *c) override { return FALSE; }
	BOOL 			SetFalloffControl(Control *c) override { return FALSE; }
	BOOL 			SetColorControl(Control *c) override { return FALSE; }
	Control* 		GetHotSpotControl() override { return nullptr; }
	Control* 		GetFalloffControl() override { return nullptr; }
	Control* 		GetColorControl() override { return nullptr; }
	
	void 			SetAffectDiffuse(BOOL onOff) override { }
	BOOL 			GetAffectDiffuse() override { return FALSE; }
	void 			SetAffectSpecular(BOOL onOff) override { }
	BOOL 			GetAffectSpecular() override { return FALSE; }

	void 			SetDecayType(BOOL onOff) override { }
	BOOL 			GetDecayType() override { return FALSE; }
	void 			SetDecayRadius(TimeValue time, float f) override {}
	float 			GetDecayRadius(TimeValue t, Interval& valid) override { return 0.0f;}
	void 			SetDiffuseSoft(TimeValue time, float f) override {}
	float 			GetDiffuseSoft(TimeValue t, Interval& valid) override { return 0.0f; }

	void 			SetShadColor(TimeValue t, const Point3& rgb) override {}
	Point3 			GetShadColor(TimeValue t, Interval& valid) override { return Point3(0,0,0); }
	BOOL 			GetLightAffectsShadow() override { return FALSE; }
	void 			SetLightAffectsShadow(BOOL b) override {  }
	void			SetShadMult(TimeValue t, float m) override {  }
	float			GetShadMult(TimeValue t, Interval& valid) override { return 1.0f; }

	Texmap* 		GetShadowProjMap() override { return nullptr;  }
	void 			SetShadowProjMap(Texmap* pmap) override {}

	void 			SetAmbientOnly(BOOL onOff) override {  }
	BOOL 			GetAmbientOnly() override { return FALSE; }

	void			SetAtmosShadows(TimeValue t, int onOff) override { }
	int				GetAtmosShadows(TimeValue t) override { return 0; }
	void			SetAtmosOpacity(TimeValue t, float f) override { }
	float			GetAtmosOpacity(TimeValue t, Interval& valid) override { return 0.0f; }
	void			SetAtmosColAmt(TimeValue t, float f) override { }
	float			GetAtmosColAmt(TimeValue t, Interval& valid) override { return 0.0f; }

	void			SetUseShadowColorMap(TimeValue t, int onOff) override { GenLight::SetUseShadowColorMap(t, onOff); }
	int				GetUseShadowColorMap(TimeValue t) override { return GenLight::GetUseShadowColorMap(t); }

	void			SetShadowGenerator(ShadowType *s) override { GenLight::SetShadowGenerator(s); };
	ShadowType*		GetShadowGenerator() override { return GenLight::GetShadowGenerator(); } 
};

class MSLightXtnd : public MSObjectXtnd<GenLight, MSPluginLight>
{
public:
					MSLightXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSLightXtnd() { DeleteAllRefsFromMe(); }
	RefTargetHandle Clone(RemapDir& remap) override;

	// From BaseObject
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags) override;

	void			GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& abox ) override;
	void			GetLocalBoundBox(TimeValue t, INode *inode, ViewExp *vpt, Box3& abox ) override;
	void			GetDeformBBox(TimeValue t, Box3& abox, Matrix3 *tm, BOOL useSel ) override;
	int				HitTest(TimeValue t, INode *inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt) override;
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override;

	// From LightObject

	RefResult		EvalLightState(TimeValue time, Interval& valid, LightState *ls) override;
	ObjLightDesc *	CreateLightDesc(INode *n, BOOL forceShadowBuf) override { return delegate->CreateLightDesc(n, forceShadowBuf); }
	void			SetUseLight(int onOff) override { delegate->SetUseLight(onOff); }
	BOOL			GetUseLight() override { return delegate->GetUseLight(); }
	void			SetHotspot(TimeValue time, float f) override { delegate->SetHotspot(time, f); } 
	float			GetHotspot(TimeValue t, Interval& valid) override { return delegate->GetHotspot(t, valid); }
	void			SetFallsize(TimeValue time, float f) override { delegate->SetFallsize(time, f); }
	float			GetFallsize(TimeValue t, Interval& valid) override { return delegate->GetFallsize(t, valid); }
	void			SetAtten(TimeValue time, int which, float f) override { delegate->SetAtten(time, which, f); }
	float			GetAtten(TimeValue t, int which, Interval& valid) override { return delegate->GetAtten(t, which, valid); }
	void			SetTDist(TimeValue time, float f) override { delegate->SetTDist(time, f); }
	float			GetTDist(TimeValue t, Interval& valid) override { return delegate->GetTDist(t, valid); }
	void			SetConeDisplay(int s, int notify=TRUE) override { delegate->SetConeDisplay(s, notify); }
	BOOL			GetConeDisplay() override { return delegate->GetConeDisplay(); }
	int 			GetShadowMethod() override {return delegate->GetShadowMethod();}
	void 			SetRGBColor(TimeValue t, const Point3& rgb) override { delegate->SetRGBColor(t, rgb); }
	Point3 			GetRGBColor(TimeValue t, Interval& valid) override {return delegate->GetRGBColor(t, valid);}        
	void 			SetIntensity(TimeValue time, float f) override { delegate->SetIntensity(time, f); }
	float 			GetIntensity(TimeValue t, Interval& valid) override { return delegate->GetIntensity(t, valid); }
	void 			SetAspect(TimeValue t, float f) override { delegate->SetAspect(t, f); }
	float			GetAspect(TimeValue t, Interval& valid) override { return delegate->GetAspect(t, valid); }    
	void 			SetUseAtten(int s) override { delegate->SetUseAtten(s); }
	BOOL 			GetUseAtten() override { return delegate->GetUseAtten(); }
	void 			SetAttenDisplay(int s) override { delegate->SetAttenDisplay(s); }
	BOOL 			GetAttenDisplay() override { return delegate->GetAttenDisplay(); }      
	void 			Enable(int enab) override { delegate->Enable(enab); }
	void 			SetMapBias(TimeValue t, float f) override { delegate->SetMapBias(t, f); }
	float 			GetMapBias(TimeValue t, Interval& valid) override { return delegate->GetMapBias(t, valid); }
	void 			SetMapRange(TimeValue t, float f) override { delegate->SetMapRange(t, f); }
	float 			GetMapRange(TimeValue t, Interval& valid) override { return delegate->GetMapRange(t, valid); }
	void 			SetMapSize(TimeValue t, int f) override { delegate->SetMapSize(t, f); }
	int 			GetMapSize(TimeValue t, Interval& valid) override { return delegate->GetMapSize(t, valid); }
	void 			SetRayBias(TimeValue t, float f) override { delegate->SetRayBias(t, f); }
	float 			GetRayBias(TimeValue t, Interval& valid) override { return delegate->GetRayBias(t, valid); }
	int 			GetAbsMapBias() override { return delegate->GetAbsMapBias(); }
	void 			SetAbsMapBias(int a) override { delegate->SetAbsMapBias(a); }
	int 			GetOvershoot() override { return delegate->GetOvershoot(); }
	void 			SetOvershoot(int a) override { delegate->SetOvershoot(a); }
	int 			GetProjector() override { return delegate->GetProjector(); }
	void 			SetProjector(int a) override { delegate->SetProjector(a); }
	ExclList* 		GetExclList() override { return delegate->GetExclList(); }
	BOOL 			Include() override { return delegate->Include(); }
	Texmap* 		GetProjMap() override { return delegate->GetProjMap(); }
	void 			SetProjMap(Texmap* pmap) override { delegate->SetProjMap(pmap); }
	void 			UpdateTargDistance(TimeValue t, INode* inode) override { delegate->UpdateTargDistance(t, inode); }
	int				UsesWireColor() override;

	// From GenLight

	GenLight*		NewLight(int type) override { return delegate->NewLight(type); }
	int				Type() override { return delegate->Type(); }  // OMNI_LIGHT, TSPOT_LIGHT, DIR_LIGHT, FSPOT_LIGHT, TDIR_LIGHT
	void			SetType(int tp) override { delegate->SetType(tp); } // OMNI_LIGHT, TSPOT_LIGHT, DIR_LIGHT, FSPOT_LIGHT, TDIR_LIGHT      
	BOOL			IsSpot() override { return delegate->IsSpot(); }
	BOOL			IsDir() override { return delegate->IsDir(); }
	void			SetSpotShape(int s) override { delegate->SetSpotShape(s); }
	int				GetSpotShape() override { return delegate->GetSpotShape(); }
	void 			SetHSVColor(TimeValue t, Point3& hsv) override { delegate->SetHSVColor(t, hsv); }
	Point3 			GetHSVColor(TimeValue t, Interval& valid) override { return delegate->GetHSVColor(t, valid); }
	void 			SetContrast(TimeValue time, float f) override { delegate->SetContrast(time, f); }
	float 			GetContrast(TimeValue t, Interval& valid) override { return delegate->GetContrast(t, valid); }
	void 			SetUseAttenNear(int s) override { delegate->SetUseAttenNear(s); }
	BOOL 			GetUseAttenNear() override { return delegate->GetUseAttenNear(); }
	void 			SetAttenNearDisplay(int s) override { delegate->SetAttenNearDisplay(s); }
	BOOL 			GetAttenNearDisplay() override { return delegate->GetAttenNearDisplay(); }

	ExclList&		GetExclusionList() override { return delegate->GetExclusionList(); }
	void 			SetExclusionList(ExclList &list) override { delegate->SetExclusionList(list); }

	BOOL 			SetHotSpotControl(Control *c) override { return delegate->SetHotSpotControl(c); }
	BOOL 			SetFalloffControl(Control *c) override { return delegate->SetFalloffControl(c); }
	BOOL 			SetColorControl(Control *c) override { return delegate->SetColorControl(c); }
	Control* 		GetHotSpotControl() override { return delegate->GetHotSpotControl(); }
	Control* 		GetFalloffControl() override { return delegate->GetFalloffControl(); }
	Control* 		GetColorControl() override { return delegate->GetColorControl(); }
	
	void 			SetAffectDiffuse(BOOL onOff) override { delegate->SetAffectDiffuse(onOff); }
	BOOL 			GetAffectDiffuse() override { return delegate->GetAffectDiffuse(); }
	void 			SetAffectSpecular(BOOL onOff) override { delegate->SetAffectSpecular(onOff); }
	BOOL 			GetAffectSpecular() override { return delegate->GetAffectSpecular(); }

	void 			SetDecayType(BOOL onOff) override { delegate->SetDecayType(onOff); }
	BOOL 			GetDecayType() override { return delegate->GetDecayType(); }
	void 			SetDecayRadius(TimeValue time, float f) override { delegate->SetDecayRadius(time, f); }
	float 			GetDecayRadius(TimeValue t, Interval& valid) override { return delegate->GetDecayRadius(t, valid);}
	void 			SetDiffuseSoft(TimeValue time, float f) override { delegate->SetDiffuseSoft(time, f); }
	float 			GetDiffuseSoft(TimeValue t, Interval& valid) override { return delegate->GetDiffuseSoft(t, valid); }

	int 			GetUseGlobal() override { return delegate->GetUseGlobal(); }
	void 			SetUseGlobal(int a) override { delegate->SetUseGlobal(a); }
	int 			GetShadow() override { return delegate->GetShadow(); }
	void 			SetShadow(int a) override { delegate->SetShadow(a); }
	int 			GetShadowType() override { return delegate->GetShadowType(); }
	void 			SetShadowType(int a) override { delegate->SetShadowType(a); }

	void 			SetShadColor(TimeValue t, const Point3& rgb) override { delegate->SetShadColor(t, rgb); }
	Point3 			GetShadColor(TimeValue t, Interval& valid) override { return delegate->GetShadColor(t, valid); }
	BOOL 			GetLightAffectsShadow() override { return delegate->GetLightAffectsShadow(); }
	void 			SetLightAffectsShadow(BOOL b) override { delegate->SetLightAffectsShadow(b); }
	void			SetShadMult(TimeValue t, float m) override { delegate->SetShadMult(t, m); }
	float			GetShadMult(TimeValue t, Interval& valid) override { return delegate->GetShadMult(t, valid); }

	Texmap* 		GetShadowProjMap() override { return delegate->GetShadowProjMap();  }
	void 			SetShadowProjMap(Texmap* pmap) override { delegate->SetShadowProjMap(pmap); }

	void 			SetAmbientOnly(BOOL onOff) override { delegate->SetAmbientOnly(onOff); }
	BOOL 			GetAmbientOnly() override { return delegate->GetAmbientOnly(); }

	void			SetAtmosShadows(TimeValue t, int onOff) override { delegate->SetAtmosShadows(t, onOff);}
	int				GetAtmosShadows(TimeValue t) override { return delegate->GetAtmosShadows(t); }
	void			SetAtmosOpacity(TimeValue t, float f) override { delegate->SetAtmosOpacity(t, f);}
	float			GetAtmosOpacity(TimeValue t, Interval& valid) override { return delegate->GetAtmosOpacity(t); }
	void			SetAtmosColAmt(TimeValue t, float f) override { delegate->SetAtmosColAmt(t, f);}
	float			GetAtmosColAmt(TimeValue t, Interval& valid) override { return delegate->GetAtmosColAmt(t); }
	
	void			SetUseShadowColorMap(TimeValue t, int onOff) override { delegate->SetUseShadowColorMap(t, onOff); }
	int				GetUseShadowColorMap(TimeValue t) override { return delegate->GetUseShadowColorMap(t); }

	void			SetShadowGenerator(ShadowType *s) override { delegate->SetShadowGenerator(s); }
	ShadowType*		GetShadowGenerator() override { return delegate->GetShadowGenerator(); } 
};

// ----------------------- MSPluginCamera ----------------------
// scripted GenCamera

class MSPluginCamera : public MSPluginObject<GenCamera>
{
public:
					MSPluginCamera() { }
					MSPluginCamera(MSPluginClass* pc, BOOL loading);
				   ~MSPluginCamera() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap) override;

	// From CameraObject

	RefResult		EvalCameraState(TimeValue time, Interval& valid, CameraState* cs) override { return REF_SUCCEED; }
	void			SetOrtho(BOOL b) override { }
	BOOL			IsOrtho() override { return FALSE; }
	void			SetFOV(TimeValue time, float f) override { } 
	float			GetFOV(TimeValue t, Interval& valid) override { return 0.0f; }
	void			SetTDist(TimeValue time, float f) override { } 
	float			GetTDist(TimeValue t, Interval& valid) override { return 0.0f; }
	int				GetManualClip() override {return 0;}
	void			SetManualClip(int onOff) override { }
	float			GetClipDist(TimeValue t, int which, Interval &valid) override { return 0.0f; }
	void			SetClipDist(TimeValue t, int which, float val) override { }
	void			SetEnvRange(TimeValue time, int which, float f) override { } 
	float			GetEnvRange(TimeValue t, int which, Interval& valid) override { return 0.0f; }
	void			SetEnvDisplay(BOOL b, int notify=TRUE) override { }
	BOOL			GetEnvDisplay() override { return FALSE; }
	void			RenderApertureChanged(TimeValue t) override { }
	void			UpdateTargDistance(TimeValue t, INode* inode) override { }
	int				UsesWireColor() override { return CameraObject::UsesWireColor(); }   // TRUE if the object color is used for display

	// From GenCamera

	GenCamera *		NewCamera(int type) override { return nullptr; }
	void			SetConeState(int s) override { }
	int				GetConeState() override { return 0; }
	void			SetHorzLineState(int s) override { }
	int				GetHorzLineState() override { return 0; }
	void			Enable(int enab) override { }
	BOOL			SetFOVControl(Control *c) override { return FALSE; }
	void			SetFOVType(int ft) override { }
	int				GetFOVType() override { return 0; }
	Control*		GetFOVControl() override { return nullptr; }
	int				Type() override { return 0; }
	void			SetType(int tp) override { }

	void			SetDOFEnable(TimeValue t, BOOL onOff) override {}
	BOOL			GetDOFEnable(TimeValue t, Interval& valid) override { return 0; }
	void			SetDOFFStop(TimeValue t, float fs) override {}
	float			GetDOFFStop(TimeValue t, Interval& valid) override { return 1.0f; }

};

class MSCameraXtnd : public MSObjectXtnd<GenCamera, MSPluginCamera>
{
public:
					MSCameraXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSCameraXtnd() { DeleteAllRefsFromMe(); }
	RefTargetHandle Clone(RemapDir& remap) override;

	// From BaseObject
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags) override;

	void			GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& abox ) override;
	void			GetLocalBoundBox(TimeValue t, INode *inode, ViewExp *vpt, Box3& abox ) override;
	void			GetDeformBBox(TimeValue t, Box3& abox, Matrix3 *tm, BOOL useSel ) override;
	int				HitTest(TimeValue t, INode *inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt) override;
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override;


	// From CameraObject

	RefResult		EvalCameraState(TimeValue time, Interval& valid, CameraState* cs) override;
	void			SetOrtho(BOOL b) override { delegate->SetOrtho(b); }
	BOOL			IsOrtho() override { return delegate->IsOrtho(); }
	void			SetFOV(TimeValue time, float f) override { delegate->SetFOV(time, f); } 
	float			GetFOV(TimeValue t, Interval& valid) override { return delegate->GetFOV(t, valid); }
	void			SetTDist(TimeValue time, float f) override { delegate->SetTDist(time, f); } 
	float			GetTDist(TimeValue t, Interval& valid) override { return delegate->GetTDist(t, valid); }
	int				GetManualClip() override { return delegate->GetManualClip(); }
	void			SetManualClip(int onOff) override { delegate->SetManualClip(onOff); }
	float			GetClipDist(TimeValue t, int which, Interval &valid) override { return delegate->GetClipDist(t, which, valid); }
	void			SetClipDist(TimeValue t, int which, float val) override { delegate->SetClipDist(t, which, val); }
	void			SetEnvRange(TimeValue time, int which, float f) override { delegate->SetEnvRange(time, which, f); } 
	float			GetEnvRange(TimeValue t, int which, Interval& valid) override { return delegate->GetEnvRange(t, which, valid); }
	void			SetEnvDisplay(BOOL b, int notify=TRUE) override { delegate->SetEnvDisplay(b, notify); }
	BOOL			GetEnvDisplay() override { return delegate->GetEnvDisplay(); }
	void			RenderApertureChanged(TimeValue t) override;
	void			UpdateTargDistance(TimeValue t, INode* inode) override { delegate->UpdateTargDistance(t, inode); }
	int				UsesWireColor() override;

	void			SetMultiPassEffectEnabled(TimeValue t, BOOL enabled) override { delegate->SetMultiPassEffectEnabled(t, enabled); }
	BOOL			GetMultiPassEffectEnabled(TimeValue t, Interval& valid) override { return delegate->GetMultiPassEffectEnabled(t, valid); }
	void			SetMPEffect_REffectPerPass(BOOL enabled) override { delegate->SetMPEffect_REffectPerPass(enabled); }
	BOOL			GetMPEffect_REffectPerPass() override { return delegate->GetMPEffect_REffectPerPass(); }
	void			SetIMultiPassCameraEffect(IMultiPassCameraEffect *pIMultiPassCameraEffect) override { delegate->SetIMultiPassCameraEffect(pIMultiPassCameraEffect); }
	IMultiPassCameraEffect *GetIMultiPassCameraEffect() override { return delegate->GetIMultiPassCameraEffect(); }

	// From GenCamera

	GenCamera*		NewCamera(int type) override { return delegate->NewCamera(type); }
	void			SetConeState(int s) override { delegate->SetConeState(s); }
	int				GetConeState() override { return delegate->GetConeState(); }
	void			SetHorzLineState(int s) override { delegate->SetHorzLineState(s); }
	int				GetHorzLineState() override { return delegate->GetHorzLineState(); }
	void			Enable(int enab) override { delegate->Enable(enab); }
	BOOL			SetFOVControl(Control *c) override { return delegate->SetFOVControl(c); }
	void			SetFOVType(int ft) override { delegate->SetFOVType(ft); }
	int				GetFOVType() override { return delegate->GetFOVType(); }
	Control*		GetFOVControl() override { return delegate->GetFOVControl(); }
	int				Type() override { return delegate->Type(); }
	void			SetType(int tp) override { delegate->SetType(tp); }

	void			SetDOFEnable(TimeValue t, BOOL onOff) override { delegate->SetDOFEnable(t, onOff); }
	BOOL			GetDOFEnable(TimeValue t, Interval& valid) override { return delegate->GetDOFEnable(t, valid); }
	void			SetDOFFStop(TimeValue t, float fs) override { delegate->SetDOFFStop(t, fs); }
	float			GetDOFFStop(TimeValue t, Interval& valid) override { return delegate->GetDOFFStop(t, valid); }
};

// ----------------------- MSPluginShape ----------------------
// scripted Shape

class MSPluginShape : public MSPluginObject<ShapeObject>
{
	ShapeHierarchy	sh;
public:
	MSPluginShape() : MSPluginObject<ShapeObject>() { sh.New(); }
					MSPluginShape(MSPluginClass* pc, BOOL loading);
				   ~MSPluginShape() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap) override;

	// From GeomObject
	int				IsRenderable() override { return ShapeObject::IsRenderable(); }		
	Mesh*			GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete) override { return ShapeObject::GetRenderMesh(t, inode, view, needDelete); }

	// from ShapeObject
	void			InitNodeName(MSTR& s) override { ShapeObject::InitNodeName(s); }
	SClass_ID		SuperClassID() override { return ShapeObject::SuperClassID(); }

	int				IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm) override { return ShapeObject::IntersectRay(t, ray, at, norm); }
	int				NumberOfVertices(TimeValue t, int curve = -1) override { return ShapeObject::NumberOfVertices(t, curve); }	// Informational only, curve = -1: total in all curves
	int				NumberOfCurves(TimeValue t) override { return 0; }      // Number of curve polygons in the shape
	BOOL			CurveClosed(TimeValue t, int curve) override { return FALSE; }     // Returns TRUE if the curve is closed
	Point3			InterpCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE) override { return Point3 (0,0,0); }    // Interpolate from 0-1 on a curve
	Point3			TangentCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE) override { return Point3 (0,0,0); }    // Get tangent at point on a curve
	float			LengthOfCurve(TimeValue t, int curve) override { return 0.0f; }  // Get the length of a curve
	int				NumberOfPieces(TimeValue t, int curve) override { return 0; }   // Number of sub-curves in a curve
	Point3			InterpPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE) override { return Point3 (0,0,0); }  // Interpolate from 0-1 on a sub-curve
	Point3			TangentPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE) override { return Point3 (0,0,0); }         // Get tangent on a sub-curve
	BOOL			CanMakeBezier() override { return ShapeObject::CanMakeBezier(); }                  // Return TRUE if can turn into a bezier representation
	void			MakeBezier(TimeValue t, BezierShape &shape) override { ShapeObject::MakeBezier(t, shape); }     // Create the bezier representation
	ShapeHierarchy& OrganizeCurves(TimeValue t, ShapeHierarchy *hier=nullptr) override { return sh; }       // Ready for lofting, extrusion, etc.
	void			MakePolyShape(TimeValue t, PolyShape &shape, int steps = PSHAPE_BUILTIN_STEPS, BOOL optimize = FALSE) override { } // Create a PolyShape representation with optional fixed steps & optimization
	int				MakeCap(TimeValue t, MeshCapInfo &capInfo, int capType) override { return 0; }  // Generate mesh capping info for the shape
	int				MakeCap(TimeValue t, PatchCapInfo &capInfo) override { return ShapeObject::MakeCap(t, capInfo); }	// Only implement if CanMakeBezier=TRUE -- Gen patch cap info

	
	MtlID			GetMatID(TimeValue t, int curve, int piece) override { return ShapeObject::GetMatID(t, curve, piece); }
	BOOL			AttachShape(TimeValue t, INode *thisNode, INode *attachNode) { return ShapeObject::AttachShape(t, thisNode, attachNode); }	// Return TRUE if attached
	// UVW Mapping switch access
	BOOL			HasUVW() override { return ShapeObject::HasUVW(); }
	void			SetGenUVW(BOOL sw) override { ShapeObject::SetGenUVW(sw); }

	// These handle loading and saving the data in this class. Should be called
	// by derived class BEFORE it loads or saves any chunks
	IOResult		Save(ISave *iSave) override { MSPlugin::Save(iSave); return ShapeObject::Save(iSave); }
	IOResult		Load(ILoad *iLoad) override {  MSPlugin::Load(iLoad); return ShapeObject::Load(iLoad); }

	Class_ID		PreferredCollapseType() override { return ShapeObject::PreferredCollapseType(); }
	BOOL			GetExtendedProperties(TimeValue t, MSTR &prop1Label, MSTR &prop1Data, MSTR &prop2Label, MSTR &prop2Data) override
						{ return ShapeObject::GetExtendedProperties(t, prop1Label, prop1Data, prop2Label, prop2Data); }
	void			RescaleWorldUnits(float f) override { ShapeObject::RescaleWorldUnits(f); }
};

class MSShapeXtnd : public MSObjectXtnd<ShapeObject, MSPluginShape>
{
public:
					MSShapeXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSShapeXtnd() { DeleteAllRefsFromMe(); }
	RefTargetHandle Clone(RemapDir& remap) override;

	// From GeomObject
	int				IsRenderable() override { return delegate->IsRenderable(); }		
	Mesh*			GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete) override { return delegate->GetRenderMesh(t, inode, view, needDelete); }

	// from ShapeObject
	void			InitNodeName(MSTR& s) override { delegate->InitNodeName(s); }
					// CAL-10/1/2002: delegate could be NULL while doing DeleteReference(0) (412668)
	SClass_ID		SuperClassID() override { return delegate ? delegate->SuperClassID() : MSPluginShape::SuperClassID(); }

	int				IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm) override { return delegate->IntersectRay(t, ray, at, norm); }
	int				NumberOfVertices(TimeValue t, int curve = -1) override { return delegate->NumberOfVertices(t, curve); }	
	int				NumberOfCurves(TimeValue t) override { return delegate->NumberOfCurves(t); }
	BOOL			CurveClosed(TimeValue t, int curve) override { return delegate->CurveClosed(t, curve); }
	Point3			InterpCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE) override { return delegate->InterpCurve3D(t, curve, param, ptype); }    
	Point3			TangentCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE) override { return delegate->TangentCurve3D(t, curve, param, ptype); }    
	float			LengthOfCurve(TimeValue t, int curve) override { return delegate->LengthOfCurve(t, curve); }  
	int				NumberOfPieces(TimeValue t, int curve) override { return delegate->NumberOfPieces(t, curve); }   
	Point3			InterpPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE) override { return delegate->InterpPiece3D(t, curve, piece, param, ptype); }  
	Point3			TangentPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE) override { return delegate->TangentPiece3D(t, curve, piece, param, ptype); }         
	BOOL			CanMakeBezier() override { return delegate->CanMakeBezier(); }                  
	void			MakeBezier(TimeValue t, BezierShape &shape) override { delegate->MakeBezier(t, shape); }     
	ShapeHierarchy& OrganizeCurves(TimeValue t, ShapeHierarchy *hier=nullptr) override { return delegate->OrganizeCurves(t, hier); }       
	void			MakePolyShape(TimeValue t, PolyShape &shape, int steps = PSHAPE_BUILTIN_STEPS, BOOL optimize = FALSE) override { delegate->MakePolyShape(t, shape, steps, optimize); } 
	int				MakeCap(TimeValue t, MeshCapInfo &capInfo, int capType) override { return delegate->MakeCap(t, capInfo, capType); }  
	int				MakeCap(TimeValue t, PatchCapInfo &capInfo) override { return delegate->MakeCap(t, capInfo); }	

	void			CloneSelSubComponents(TimeValue t) override { delegate->CloneSelSubComponents( t); }
	void			AcceptCloneSelSubComponents(TimeValue t) override { delegate->AcceptCloneSelSubComponents( t); }
	void			 SelectSubComponent(
		HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE) override { delegate->SelectSubComponent(hitRec, selected, all, invert); }
	void			ClearSelection(int selLevel) override { delegate->ClearSelection( selLevel); }
	void			SelectAll(int selLevel) override { delegate->SelectAll( selLevel); }
	void			InvertSelection(int selLevel) override { delegate->InvertSelection( selLevel); }
	int				SubObjectIndex(HitRecord *hitRec) override {return  delegate->SubObjectIndex(hitRec);}
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt, ModContext* mc) override
	{ return delegate->HitTest(t, inode, type, crossing, vflags, p, vpt, mc); }
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt) override
	{ return delegate->HitTest(t, inode, type, crossing, vflags, p, vpt); }
	void			ActivateSubobjSel(int level, XFormModes& modes ) override { delegate->ActivateSubobjSel( level, modes ); }
	BOOL			SupportsNamedSubSels() override {return  delegate->SupportsNamedSubSels();}
	void			ActivateSubSelSet(MSTR &setName) override { delegate->ActivateSubSelSet(setName); }
	void			NewSetFromCurSel(MSTR &setName) override { delegate->NewSetFromCurSel(setName); }
	void			RemoveSubSelSet(MSTR &setName) override { delegate->RemoveSubSelSet(setName); }
	void			SetupNamedSelDropDown() override { delegate->SetupNamedSelDropDown(); }
	int				NumNamedSelSets() override {return  delegate->NumNamedSelSets();}
	MSTR			GetNamedSelSetName(int i) override {return  delegate->GetNamedSelSetName( i);}
	void			SetNamedSelSetName(int i,MSTR &newName) override { delegate->SetNamedSelSetName( i, newName); }
	void			NewSetByOperator(MSTR &newName,Tab<int> &sets,int op) override { delegate->NewSetByOperator(newName, sets, op); }
	void			GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) override { delegate->GetSubObjectCenters(cb, t, node, mc); }
	void			GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) override { delegate->GetSubObjectTMs( cb, t, node, mc); }                          
	
	void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE ) override{ delegate->Move( t, partm, tmAxis, val, localOrigin ); }
	void Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE ) override{ delegate->Rotate( t, partm, tmAxis, val, localOrigin ); }
	void Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE ) override{ delegate->Scale( t, partm, tmAxis, val, localOrigin ); }
	void TransformStart(TimeValue t) override { delegate->TransformStart( t); }
	void TransformHoldingStart(TimeValue t) override { delegate->TransformHoldingStart( t); }
	void TransformHoldingFinish(TimeValue t) override { delegate->TransformHoldingFinish( t); } 
	void TransformFinish(TimeValue t) override { delegate->TransformFinish( t); } 
	void TransformCancel(TimeValue t) override { delegate->TransformCancel( t); }

	MtlID			GetMatID(TimeValue t, int curve, int piece) override { return delegate->GetMatID(t, curve, piece); }
	BOOL			AttachShape(TimeValue t, INode *thisNode, INode *attachNode) { return delegate->AttachShape(t, thisNode, attachNode); }	// Return TRUE if attached
	// UVW Mapping switch access
	BOOL			HasUVW() override { return delegate->HasUVW(); }
	void			SetGenUVW(BOOL sw) override { delegate->SetGenUVW(sw); }

	// These handle loading and saving the data in this class. Should be called
	// by derived class BEFORE it loads or saves any chunks
	IOResult		Save(ISave *iSave) override { MSPlugin::Save(iSave); return ShapeObject::Save(iSave); }
	IOResult		Load(ILoad *iLoad) override {  MSPlugin::Load(iLoad); return ShapeObject::Load(iLoad); }

	Class_ID		PreferredCollapseType() override { return delegate->PreferredCollapseType(); }
	BOOL			GetExtendedProperties(TimeValue t, MSTR &prop1Label, MSTR &prop1Data, MSTR &prop2Label, MSTR &prop2Data) override
						{ return delegate->GetExtendedProperties(t, prop1Label, prop1Data, prop2Label, prop2Data); }
	void			RescaleWorldUnits(float f) override { delegate->RescaleWorldUnits(f); }

	Object*			MakeShallowCopy(ChannelMask channels) override { return delegate->MakeShallowCopy(channels); }
	void			ShallowCopy(Object* fromOb, ChannelMask channels) override { delegate->ShallowCopy(fromOb, channels); }
	ObjectState		Eval(TimeValue time) override { delegate->Eval(time); return ObjectState(this); }

};

// ----------------------- MSPluginSimpleObject ----------------------
//	scriptable SimpleObject, mesh building and all

class MSPluginSimpleObject : public MSPlugin, public SimpleObject
{
public:
	IObjParam*		ip;			// ip for any currently open command panel dialogs

					MSPluginSimpleObject() { }
					MSPluginSimpleObject(MSPluginClass* pc, BOOL loading);
				   ~MSPluginSimpleObject() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);

	void			DeleteThis() override;
#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")
	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD vflags=0, int category=ROLLUP_CAT_STANDARD) override;
	void			DeleteRollupPage(HWND hRollup) override;
	IRollupWindow*	GetRollupWindow() override;
	void			RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) override;

	ReferenceTarget* get_delegate() override { return nullptr; }  // no delegate 

	// From Animatable
	using SimpleObject::GetInterface;

	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	int				NumSubs() override { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) override { return pblocks[i]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (pblocks[i]->GetDesc()->local_name != 0)
		{
			return localized ? pblocks[i]->GetLocalName() : pblocks[i]->GetDesc()->int_name.data();
		}
		
		return _T("");
	}
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return SimpleObject::GetInterface(id); }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override 
					{ 
						SimpleObject::NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
					}

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override;
public:
	void			RefDeleted(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); }
	void			RefAdded(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); }
	void			RefAddedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); };
	void			RefDeletedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); };
	RefTargetHandle Clone(RemapDir& remap) override;
	IOResult		Save(ISave *iSave) override { return MSPlugin::Save(iSave); }
	IOResult		Load(ILoad *iLoad) override { return MSPlugin::Load(iLoad); }
	void			NotifyTarget(int msg, RefMakerHandle rm) override { MSPlugin::NotifyTarget(msg, rm); }

	// From BaseObject
	const MCHAR*	GetObjectName(bool localized = true) const override { return (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	void			BeginEditParams( IObjParam *objParam, ULONG vflags,Animatable *pPrev) override;
	void			EndEditParams( IObjParam *objParam, ULONG vflags,Animatable *pNext) override;

	// From SimpleObject
	void			BuildMesh(TimeValue t) override;
	BOOL			OKtoDisplay(TimeValue t) override;
	void			InvalidateUI() override;
	CreateMouseCallBack* GetCreateMouseCallBack() override;
	BOOL			HasUVW() override;
	void			SetGenUVW(BOOL sw) override;
};

//	MSSimpleObjectXtnd
class MSSimpleObjectXtnd : public MSPluginSimpleObject
{
public:
	SimpleObject*	delegate;		// my delegate

					MSSimpleObjectXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSSimpleObjectXtnd() { DeleteAllRefsFromMe(); }

	void			DeleteThis() override;

	// From MSPlugin
	ReferenceTarget* get_delegate() override { return delegate; } 

	// From Animatable
	using MSPluginSimpleObject::GetInterface;

	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	void			FreeCaches() override { delegate->FreeCaches(); } 		
	int				NumSubs() override { return pblocks.Count() + 1; }  
	Animatable*		SubAnim(int i) override { if (i == 0) return delegate; return pblocks[i-1]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (i == 0)
		{
			return delegate->GetObjectName(localized);
		}
		if (pblocks[i - 1]->GetDesc()->local_name != 0)
		{
			return localized ? pblocks[i - 1]->GetLocalName() : pblocks[i - 1]->GetDesc()->int_name.data();
		}
		return _T("");
	}
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return MSPluginSimpleObject::GetInterface(id); }
	
	// From ReferenceMaker
//	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override; 
public:
	RefTargetHandle Clone(RemapDir& remap) override;

	// From BaseObject
	const MCHAR *			GetObjectName(bool localized = true) const override { return pc->class_name->to_string(); } // non-localized name
	void			BeginEditParams( IObjParam *objParam, ULONG vflags, Animatable *pPrev) override;
	void			EndEditParams( IObjParam *objParam, ULONG vflags, Animatable *pNext) override;
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt) override
						{ return delegate->HitTest(t, inode, type, crossing, vflags, p, vpt); }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags) override 
						{ return delegate->Display(t, inode, vpt, vflags); }		
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) override { delegate->GetWorldBoundBox(t, inode, vpt, box); }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) override { delegate->GetLocalBoundBox(t, inode, vpt,  box ); }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override { delegate->Snap(t, inode, snap, p, vpt); }
	CreateMouseCallBack* GetCreateMouseCallBack() override { return delegate->GetCreateMouseCallBack(); } 
	BOOL			HasUVW() override { return delegate->HasUVW(); }
	void			SetGenUVW(BOOL sw) override { delegate->SetGenUVW(sw); }
	
	// From Object
	ObjectState		Eval(TimeValue time) override;
	void			InitNodeName(MSTR& s) override {s = GetObjectName(true);} // non-localized name
	Interval		ObjectValidity(TimeValue t) override;
	int				CanConvertToType(Class_ID obtype) override 
					{ 
						if (obtype == anyObjectClassID)
							return TRUE;
						return delegate->CanConvertToType(obtype); 
					}
	Object*			ConvertToType(TimeValue t, Class_ID obtype) override 
					{
						if (obtype == anyObjectClassID)
							return this;
						// CAL-10/1/2002: Don't return the delegate, because it might be deleted.
						//		Return a copy of the delegate instead. (422964)
						Object* obj = delegate->ConvertToType(t, obtype);
						if (obj == delegate) 
						{
							obj = delegate->MakeShallowCopy(OBJ_CHANNELS);							
							obj->LockChannels(OBJ_CHANNELS); //if we shallow copy these channels they need to be locked since they will get double deleted
						}
						return obj;
					}
	void			GetCollapseTypes(Tab<Class_ID> &clist, Tab<MSTR*> &nlist) override { delegate->GetCollapseTypes(clist, nlist); }
	void			GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel) override { delegate->GetDeformBBox(t, box, tm, useSel); }
	int				IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm) override { return delegate->IntersectRay(t, r, at, norm); }

	void			BuildMesh(TimeValue t) override { delegate->BuildMesh(t); }
	BOOL			OKtoDisplay(TimeValue t) override { return delegate->OKtoDisplay(t); }
	void			InvalidateUI() override { delegate->InvalidateUI(); }
	ParamDimension* GetParameterDim(int pbIndex) override { return delegate->GetParameterDim(pbIndex); }
	MSTR			GetParameterName(int pbIndex, bool localized = true) override { return delegate->GetParameterName(pbIndex, localized); }

};

// ----------------------- MSPluginSimpleSpline ----------------------
//	scriptable SimpleSpline

class MSPluginSimpleSpline : public MSPlugin, public SimpleSpline
{
private:
	IObjParam*		ip;			// ip for any currently open command panel dialogs

public:

					MSPluginSimpleSpline();
					MSPluginSimpleSpline(MSPluginClass* pc, BOOL loading);
				   ~MSPluginSimpleSpline();

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);

	void			DeleteThis() override;
#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")
	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD flags=0, int category=ROLLUP_CAT_STANDARD) override;
	void			DeleteRollupPage(HWND hRollup) override;
	IRollupWindow*	GetRollupWindow() override;
	void			RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) override;

	ReferenceTarget* get_delegate() override;

	// From Animatable
    using SimpleSpline::GetInterface;

	void			GetClassName(MSTR& s, bool localized = true) const override;
	Class_ID		ClassID() override;
	int				NumSubs() override;
	Animatable*		SubAnim(int i) override;
	MSTR SubAnimName(int i, bool localized = true) override;
	int				NumParamBlocks() override;
	IParamBlock2*	GetParamBlock(int i) override;
	IParamBlock2*	GetParamBlockByID(BlockID id) override;
	void*			GetInterface(ULONG id) override;

	// From ReferenceMaker
	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override;

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void	SetReference(int i, RefTargetHandle rtarg) override;
public:
	void			RefDeleted(RefMakerHandle rm) override;
	void			RefAdded(RefMakerHandle rm) override;
	void			RefAddedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); };
	void			RefDeletedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); };
	RefTargetHandle Clone(RemapDir& remap) override;
	void			NotifyTarget(int msg, RefMakerHandle rm) override;

	// From BaseObject
	const MCHAR *	GetObjectName(bool localized = true) const override;
	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev) override;
	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next) override;

	// From SimpleSpline
	void			BuildShape(TimeValue t, BezierShape& ashape) override;
	BOOL			ValidForDisplay(TimeValue t) override;
	void			InvalidateUI() override;
	CreateMouseCallBack* GetCreateMouseCallBack() override;
	BOOL			HasUVW() override;
	void			SetGenUVW(BOOL sw) override;
	// I/O
	IOResult		Save(ISave *isave) override;
    IOResult		Load(ILoad *iload) override;
};

// ----------------------- MSPluginSimpleManipulator ----------------------
//	scriptable SimpleManipulator

class MSPluginSimpleManipulator : public MSPlugin, public SimpleManipulator
{
public:
	IObjParam*		ip;			// ip for any currently open command panel dialogs

					MSPluginSimpleManipulator() { }
					MSPluginSimpleManipulator(MSPluginClass* pc, BOOL loading, RefTargetHandle hTarget=nullptr, INode* pNode=nullptr);
				   ~MSPluginSimpleManipulator() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	static MSPluginSimpleManipulator* create(MSPluginClass* pc, RefTargetHandle hTarget, INode* pNode);

	void			DeleteThis() override;
#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
 #pragma pop_macro("new")
	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD vflags=0, int category=ROLLUP_CAT_STANDARD) override;
	void			DeleteRollupPage(HWND hRollup) override;
	IRollupWindow*	GetRollupWindow() override;
	void			RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) override;

	ReferenceTarget* get_delegate() override { return nullptr; }  // no delegate 

	// From Animatable
	using SimpleManipulator::GetInterface;

	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	int				NumSubs() override { return pblocks.Count() + SimpleManipulator::NumSubs(); }  
	Animatable*		SubAnim(int i) override;
	MSTR SubAnimName(int i, bool localized = true) override;
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return Manipulator::GetInterface(id); }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override 
					{ 
						SimpleManipulator::NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
					}

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override;
public:
	void			RefDeleted(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); }
	void			RefAdded(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); }
	void			RefAddedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); };
	void			RefDeletedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); };
	RefTargetHandle Clone(RemapDir& remap) override;
	IOResult		Save(ISave *iSave) override { return MSPlugin::Save(iSave); }
	IOResult		Load(ILoad *iLoad) override { return MSPlugin::Load(iLoad); }
	void			NotifyTarget(int msg, RefMakerHandle rm) override { MSPlugin::NotifyTarget(msg, rm); }

	// From BaseObject
	const MCHAR*	GetObjectName(bool localized = true) const override { return (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	void			BeginEditParams( IObjParam *objParam, ULONG vflags, Animatable *pPrev) override;
	void			EndEditParams( IObjParam *objParam, ULONG vflags, Animatable *pNext) override;
	CreateMouseCallBack* GetCreateMouseCallBack() override;

	// From HelperObject
	int				UsesWireColor() override { return HelperObject::UsesWireColor(); }   // TRUE if the object color is used for display
	BOOL			NormalAlignVector(TimeValue t,Point3 &pt, Point3 &norm) override { return HelperObject::NormalAlignVector(t, pt, norm); }

	// From SimpleManipulator
	void			UpdateShapes(TimeValue t, MSTR& toolTip) override;
	void			OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD vflags, ManipHitData* pHitData) override;
	void			OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD vflags, ManipHitData* pHitData) override;
	void			OnButtonUp(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD vflags, ManipHitData* pHitData) override;
};

//	MSSimpleManipulatorXtnd
class MSSimpleManipulatorXtnd : public MSPluginSimpleManipulator
{
public:
	SimpleManipulator*	delegate;		// my delegate

					MSSimpleManipulatorXtnd() { }
					MSSimpleManipulatorXtnd(MSPluginClass* pc, BOOL loading, RefTargetHandle hTarget=nullptr, INode* pNode=nullptr);
				   ~MSSimpleManipulatorXtnd() { DeleteAllRefsFromMe(); }

	static MSSimpleManipulatorXtnd* create(MSPluginClass* pc, RefTargetHandle hTarget);

	// From MSPlugin
	ReferenceTarget* get_delegate() override { return delegate; } 

	// From Animatable
	using MSPluginSimpleManipulator::GetInterface;

	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	void			FreeCaches() override { delegate->FreeCaches(); } 		
	int				NumSubs() override { return pblocks.Count() + SimpleManipulator::NumSubs() + 1; }  
	Animatable*		SubAnim(int i) override;
	MSTR SubAnimName(int i, bool localized = true) override;
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return MSPluginSimpleManipulator::GetInterface(id); }
	
	// From ReferenceMaker
//	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override;
public:
	RefTargetHandle Clone(RemapDir& remap) override;

	// From BaseObject
	const MCHAR*	GetObjectName(bool localized = true) const override { return (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	void			BeginEditParams(IObjParam *objParam, ULONG vflags, Animatable *pPrev) override;
	void			EndEditParams(IObjParam *objParam, ULONG vflags, Animatable *pNext) override;
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt) override
						{ return delegate->HitTest(t, inode, type, crossing, vflags, p, vpt); }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags) override 
						{ return delegate->Display(t, inode, vpt, vflags); }		
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) override { delegate->GetWorldBoundBox(t, inode, vpt, box); }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) override { delegate->GetLocalBoundBox(t, inode, vpt,  box ); }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override { delegate->Snap(t, inode, snap, p, vpt); }
	CreateMouseCallBack* GetCreateMouseCallBack() override { return delegate->GetCreateMouseCallBack(); } 
	BOOL			HasUVW() override { return delegate->HasUVW(); }
	void			SetGenUVW(BOOL sw) override { delegate->SetGenUVW(sw); }
	
	// From HelperObject
	int				UsesWireColor() override { return delegate->UsesWireColor(); }   // TRUE if the object color is used for display
	BOOL			NormalAlignVector(TimeValue t,Point3 &pt, Point3 &norm) override { return delegate->NormalAlignVector(t, pt, norm); }

	// From SimpleManipulator
	void			UpdateShapes(TimeValue t, MSTR& toolTip) override;
	void			OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD vflags, ManipHitData* pHitData) override;
	void			OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD vflags, ManipHitData* pHitData) override;
	void			OnButtonUp(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD vflags, ManipHitData* pHitData) override;
};

// ----------------------- MSPluginModifier ----------------------
// scripted Modifier

class MSPluginModifier : public MSPlugin, public Modifier
{
public:
	IObjParam*		ip;					// ip for any currently open command panel dialogs

					MSPluginModifier() { }
					MSPluginModifier(MSPluginClass* pc, BOOL loading);
				   ~MSPluginModifier() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);

	void			DeleteThis() override { 	MSPlugin::DeleteThis(); }
#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")
	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD vflags=0, int category=ROLLUP_CAT_STANDARD) override;
	void			DeleteRollupPage(HWND hRollup) override;
	IRollupWindow*	GetRollupWindow() override;
	void			RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) override;

	ReferenceTarget* get_delegate() override { return nullptr; }  // no delegate 

	// From Animatable
	using Modifier::GetInterface;

	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	SClass_ID		SuperClassID() override { return pc->sclass_id; }
	void			FreeCaches() override { } 		
	int				NumSubs() override { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) override { return pblocks[i]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (pblocks[i]->GetDesc()->local_name != 0)
		{
			return localized ? pblocks[i]->GetLocalName() : pblocks[i]->GetDesc()->int_name.data();
		}
		
		return _T("");
	}
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return Modifier::GetInterface(id); }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override 
					{ 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
					}

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override;
public:
	void			RefDeleted(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); }
	void			RefAdded(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); }
	void			RefAddedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); };
	void			RefDeletedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); };
	RefTargetHandle Clone(RemapDir& remap) override;
	IOResult		Save(ISave *iSave) override { MSPlugin::Save(iSave); return Modifier::Save(iSave); }
    IOResult		Load(ILoad *iLoad) override { MSPlugin::Load(iLoad); return Modifier::Load(iLoad); }
	void			NotifyTarget(int msg, RefMakerHandle rm) override { MSPlugin::NotifyTarget(msg, rm); }  // LAM - 9/7/01 - ECO 624

	// From BaseObject
	const MCHAR *			GetObjectName(bool localized = true) const override { return pc->class_name->to_string(); } // localized name
	void			BeginEditParams(IObjParam *objParam, ULONG vflags, Animatable *pPrev) override;
	void			EndEditParams(IObjParam *objParam, ULONG vflags, Animatable *pNext) override;
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt) override { return 0; }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags) override { return 0; }		
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) override { }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) override { }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override { }
	CreateMouseCallBack* GetCreateMouseCallBack() override { return nullptr; } 	
	BOOL			HasUVW() override { return 1; }
	void			SetGenUVW(BOOL sw) override { }

	// from Modifier
	Interval		LocalValidity(TimeValue t) override;
	ChannelMask		ChannelsUsed() override { return GEOM_CHANNEL; }   // pretend this thing mods geometry in order to get parameters eval'd 
	ChannelMask		ChannelsChanged() override { return GEOM_CHANNEL; } 
	// this is used to invalidate cache's in Edit Modifiers:
	void			NotifyInputChanged(const Interval& changeInt, PartID partID, RefMessage message, ModContext *mc) override { Modifier::NotifyInputChanged(changeInt, partID, message, mc); }

	// This is the method that is called when the modifier is needed to 
	// apply its effect to the object. Note that the INode* is always NULL
	// for object space modifiers.
	void			ModifyObject(TimeValue t, ModContext &mc, ObjectState* os, INode *node) override { os->obj->UpdateValidity(GEOM_CHAN_NUM, LocalValidity(t));	}

	// Modifiers that place a dependency on topology should return TRUE
	// for this method. An example would be a modifier that stores a selection
	// set base on vertex indices.
	BOOL			DependOnTopology(ModContext &mc) override { return Modifier::DependOnTopology(mc); }

	// this can return:
	//	anyObjectClassID  - (ANY_OBJECT_CLASS_ID,0) where ANY_OBJECT_CLASS_ID = 0x0000ac
	//	genericShapeClassID  - (GENERIC_SHAPE_CLASS_ID,0) where GENERIC_SHAPE_CLASS_ID = 0x0000ab
	//	triObjectClassID - (TRIOBJ_CLASS_ID, 0) where TRIOBJ_CLASS_ID = 0x0009
	//	polyObjectClassID - (POLYOBJ_CLASS_ID, 0) where POLYOBJ_CLASS_ID = 0x5d21369a
	//	mapObjectClassID - (MAPPABLE_OBJ_CLASS_ID,0) where MAPPABLE_OBJ_CLASS_ID = 0x000006
	//	defObjectClassID - (DEFORM_OBJ_CLASS_ID,0) where DEFORM_OBJ_CLASS_ID = 0x000005
	//	patchObjectClassID - (PATCHOBJ_CLASS_ID,0) where PATCHOBJ_CLASS_ID = 0x1030
	//	splineShapeClassID - (SPLINESHAPE_CLASS_ID,0) where SPLINESHAPE_CLASS_ID = 0x00000a
	Class_ID InputType() override
	{
		return anyObjectClassID;
	}
	bool AnyObjectFilter(Object* object) const override;

	IOResult		SaveLocalData(ISave *iSave, LocalModData *ld) override { return Modifier::SaveLocalData(iSave, ld); }
	IOResult		LoadLocalData(ILoad *iLoad, LocalModData **pld) override { return Modifier::LoadLocalData(iLoad, pld); }
};

class MSModifierXtnd : public MSPluginModifier
{
public:
	Modifier*		delegate;		// my delegate

					MSModifierXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSModifierXtnd() { DeleteAllRefsFromMe(); }

					void DeleteThis() override { MSPlugin::DeleteThis(); }

	// From MSPlugin
	ReferenceTarget* get_delegate() override { return delegate; } 

	// From Animatable
    using MSPluginModifier::GetInterface;

	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	SClass_ID		SuperClassID() override { return pc->sclass_id; }
	void			FreeCaches() override { delegate->FreeCaches(); } 		
	int				NumSubs() override { return pblocks.Count() + 1; }  
	Animatable*		SubAnim(int i) override { if (i == 0) return delegate; return pblocks[i-1]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (i == 0)
		{
			return delegate->GetObjectName(localized);
		}
		if (pblocks[i - 1]->GetDesc()->local_name != 0)
		{
			return localized ? pblocks[i - 1]->GetLocalName() : pblocks[i - 1]->GetDesc()->int_name.data();
		}
		return _T("");
	}
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return MSPluginModifier::GetInterface(id); }
	
	// From ReferenceMaker
//	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override;
public:
	RefTargetHandle Clone(RemapDir& remap) override;

	// From BaseObject
	const MCHAR *			GetObjectName(bool localized = true) const override { return pc->class_name->to_string(); } // localized name
	void			BeginEditParams(IObjParam *objParam, ULONG vflags, Animatable *pPrev) override;
	void			EndEditParams(IObjParam *objParam, ULONG vflags, Animatable *pNext) override;
	int			HitTest(TimeValue t, INode* inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt) override
					{ return delegate->HitTest(t, inode, type, crossing, vflags, p, vpt); }
	int			Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags) override;
	void			SetExtendedDisplay(int vflags) override { delegate->SetExtendedDisplay( vflags); }      // for setting mode-dependent display attributes
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) override { delegate->GetWorldBoundBox(t, inode, vpt, box); }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) override { delegate->GetLocalBoundBox(t, inode, vpt,  box ); }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override { delegate->Snap(t, inode, snap, p, vpt); }
	CreateMouseCallBack* GetCreateMouseCallBack() override { return delegate->GetCreateMouseCallBack(); } 
	BOOL			ChangeTopology() override {return delegate->ChangeTopology();}

	void			Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE ) override{ delegate->Move( t, partm, tmAxis, val, localOrigin ); }
	void			Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE ) override{ delegate->Rotate( t, partm, tmAxis, val, localOrigin ); }
	void			Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE ) override{ delegate->Scale( t, partm, tmAxis, val, localOrigin ); }
	void			TransformStart(TimeValue t) override { delegate->TransformStart( t); }
	void			TransformHoldingStart(TimeValue t) override { delegate->TransformHoldingStart( t); }
	void			TransformHoldingFinish(TimeValue t) override { delegate->TransformHoldingFinish( t); }             
	void			TransformFinish(TimeValue t) override { delegate->TransformFinish( t); }            
	void			TransformCancel(TimeValue t) override { delegate->TransformCancel( t); }            
	int			HitTest(TimeValue t, INode* inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt, ModContext* mc) override { return delegate->HitTest( t, inode, type, crossing, vflags, p, vpt, mc); }
	int			Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags, ModContext* mc) override; // quick render in viewport, using current TM.         
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) override { delegate->GetWorldBoundBox( t, inode, vpt, box, mc); }

	void			CloneSelSubComponents(TimeValue t) override { delegate->CloneSelSubComponents( t); }
	void			AcceptCloneSelSubComponents(TimeValue t) override { delegate->AcceptCloneSelSubComponents( t); }
	void			 SelectSubComponent(
					HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE) override { delegate->SelectSubComponent(hitRec, selected, all, invert); }
	void			ClearSelection(int selLevel) override { delegate->ClearSelection( selLevel); }
	void			SelectAll(int selLevel) override { delegate->SelectAll( selLevel); }
	void			InvertSelection(int selLevel) override { delegate->InvertSelection( selLevel); }
	int				SubObjectIndex(HitRecord *hitRec) override {return  delegate->SubObjectIndex(hitRec);}               
	void			ActivateSubobjSel(int level, XFormModes& modes ) override { delegate->ActivateSubobjSel( level, modes ); }
	BOOL			SupportsNamedSubSels() override {return  delegate->SupportsNamedSubSels();}
	void			ActivateSubSelSet(MSTR &setName) override { delegate->ActivateSubSelSet(setName); }
	void			NewSetFromCurSel(MSTR &setName) override { delegate->NewSetFromCurSel(setName); }
	void			RemoveSubSelSet(MSTR &setName) override { delegate->RemoveSubSelSet(setName); }
	void			SetupNamedSelDropDown() override { delegate->SetupNamedSelDropDown(); }
	int				NumNamedSelSets() override {return  delegate->NumNamedSelSets();}
	MSTR			GetNamedSelSetName(int i) override {return  delegate->GetNamedSelSetName( i);}
	void			SetNamedSelSetName(int i,MSTR &newName) override { delegate->SetNamedSelSetName( i, newName); }
	void			NewSetByOperator(MSTR &newName,Tab<int> &sets,int op) override { delegate->NewSetByOperator(newName, sets, op); }
	void			GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) override { delegate->GetSubObjectCenters(cb, t, node, mc); }
	void			GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) override { delegate->GetSubObjectTMs( cb, t, node, mc); }                          
	BOOL			HasUVW () override { return delegate->HasUVW(); }
	BOOL			HasUVW (int mapChannel) override { return delegate->HasUVW (mapChannel); }
	void			SetGenUVW(BOOL sw) override { delegate->SetGenUVW( sw);   }	// applies to mapChannel 1
	void			SetGenUVW (int mapChannel, BOOL sw) override { delegate->SetGenUVW ( mapChannel, sw); }
	void			ShowEndResultChanged (BOOL showEndResult) override { delegate->ShowEndResultChanged ( showEndResult);  }
	
	// from Modifier
	Interval		LocalValidity(TimeValue t) override;
	ChannelMask		ChannelsUsed() override { return delegate->ChannelsUsed(); }
	ChannelMask		ChannelsChanged() override { return delegate->ChannelsChanged(); }
	// this is used to invalidate cache's in Edit Modifiers:
	void			NotifyInputChanged(const Interval& changeInt, PartID partID, RefMessage message, ModContext *mc) override { delegate->NotifyInputChanged(changeInt, partID, message, mc); }
	void			ModifyObject(TimeValue t, ModContext &mc, ObjectState* os, INode *node) override; 

	// Modifiers that place a dependency on topology should return TRUE
	// for this method. An example would be a modifier that stores a selection
	// set base on vertex indices.
	BOOL			DependOnTopology(ModContext &mc) override { return delegate->DependOnTopology(mc); }

	// this can return:
	//	anyObjectClassID  - (ANY_OBJECT_CLASS_ID,0) where ANY_OBJECT_CLASS_ID = 0x0000ac
	//	genericShapeClassID  - (GENERIC_SHAPE_CLASS_ID,0) where GENERIC_SHAPE_CLASS_ID = 0x0000ab
	//	triObjectClassID - (TRIOBJ_CLASS_ID, 0) where TRIOBJ_CLASS_ID = 0x0009
	//	polyObjectClassID - (POLYOBJ_CLASS_ID, 0) where POLYOBJ_CLASS_ID = 0x5d21369a
	//	mapObjectClassID - (MAPPABLE_OBJ_CLASS_ID,0) where MAPPABLE_OBJ_CLASS_ID = 0x000006
	//	defObjectClassID - (DEFORM_OBJ_CLASS_ID,0) where DEFORM_OBJ_CLASS_ID = 0x000005
	//	patchObjectClassID - (PATCHOBJ_CLASS_ID,0) where PATCHOBJ_CLASS_ID = 0x1030
	//	splineShapeClassID - (SPLINESHAPE_CLASS_ID,0) where SPLINESHAPE_CLASS_ID = 0x00000a
	Class_ID InputType() override
	{
		return delegate->InputType();
	}

	IOResult		SaveLocalData(ISave *iSave, LocalModData *ld) override { return delegate->SaveLocalData(iSave, ld); }
	IOResult		LoadLocalData(ILoad *iLoad, LocalModData **pld) override { return delegate->LoadLocalData(iLoad, pld); }
};

// ----------------------- MSPluginSimpleMeshMod ----------------------
// scripted Modifier for meshes

class MSPluginSimpleMeshMod : public MSPluginModifier
{
public:
	IObjParam*		ip;					// ip for any currently open command panel dialogs
	Matrix3Value*	transform;			// cache for the local values
	Matrix3Value*	inverseTransform;
	Point3Value*	min;
	Point3Value*	max;
	Point3Value*	center;
	Point3Value*	extent;
	Box3Value*		bbox;

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);

	MSPluginSimpleMeshMod() { }
	MSPluginSimpleMeshMod(MSPluginClass* pc, BOOL loading);
	~MSPluginSimpleMeshMod();

	// From BaseObject
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt) override { return 0; }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags) override { return 0; }		
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) override { }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) override { }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override { }
	BOOL			HasUVW() override;
	void			SetGenUVW(BOOL sw) override;
	BOOL			ChangeTopology() override {return TRUE;}
	Interval		GetValidity(TimeValue t);
	RefTargetHandle Clone(RemapDir& remap) override;

	// from Modifier
	Interval		LocalValidity(TimeValue t) override;
	ChannelMask ChannelsUsed() override  {
		return GEOM_CHANNEL|TOPO_CHANNEL|SELECT_CHANNEL|SUBSEL_TYPE_CHANNEL|TEXMAP_CHANNEL|VERTCOLOR_CHANNEL;
	}
	ChannelMask ChannelsChanged() override {
		return GEOM_CHANNEL|TOPO_CHANNEL|SELECT_CHANNEL|TEXMAP_CHANNEL|VERTCOLOR_CHANNEL;
	}
	// this is used to invalidate cache's in Edit Modifiers:
	void			NotifyInputChanged(const Interval& changeInt, PartID partID, RefMessage message, ModContext *mc) override { Modifier::NotifyInputChanged(changeInt, partID, message, mc); }

	// This is the method that is called when the modifier is needed to 
	// apply its effect to the object. Note that the INode* is always NULL
	// for object space modifiers.
	void			ModifyObject(TimeValue t, ModContext &mc, ObjectState* os, INode *node) override;

	// Modifiers that place a dependency on topology should return TRUE
	// for this method. An example would be a modifier that stores a selection
	// set base on vertex indices.
	BOOL			DependOnTopology(ModContext &mc) override { return Modifier::DependOnTopology(mc); }

	Class_ID		InputType() override { return triObjectClassID; }
};

// ----------------------- MSPluginSimpleMod ----------------------
// scripted SimpleMod  (this one has full-implementation handler calls)

class MSPluginSimpleMod : public MSPlugin, public SimpleModBase
{
public:
	IObjParam*		ip;					// ip for any currently open command panel dialogs
	Point3Value*	vec;				// cache for the Map parameter & local values
	Point3Value*	extent;
	Point3Value*	min;
	Point3Value*	max;
	Point3Value*	center;
	BOOL			busy;
	TimeValue		last_time;
	Point3			last_in, last_out;

	static CRITICAL_SECTION def_sync;	// thread synch for Map parameter cache
	static BOOL		setup_sync;
	
					MSPluginSimpleMod() { vec = extent = min = max = center = nullptr; busy = FALSE; last_time = TIME_NegInfinity; }
					MSPluginSimpleMod(MSPluginClass* pc, BOOL loading);
				   ~MSPluginSimpleMod();

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);

	void			DeleteThis() override {  MSPlugin::DeleteThis(); }
#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")
	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD vflags=0, int category=ROLLUP_CAT_STANDARD) override;
	void			DeleteRollupPage(HWND hRollup) override;
	IRollupWindow*	GetRollupWindow() override;
	void			RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) override;

	ReferenceTarget* get_delegate() override { return nullptr; }  // no delegate 

	// From Animatable
	using SimpleModBase::GetInterface;
    
	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	SClass_ID		SuperClassID() override { return pc->sclass_id; }
	void			FreeCaches() override { } 		
	int				NumSubs() override { return pblocks.Count() + 2; }  
	Animatable*		SubAnim(int i) override;
	MSTR SubAnimName(int i, bool localized = true) override;
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return SimpleModBase::GetInterface(id); }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override 
					{ 
						if (message == REFMSG_CHANGE) 
							last_time = TIME_NegInfinity;
						SimpleModBase::NotifyRefChanged(changeInt, hTarget, partID, message, propagate);
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
					}

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override;
public:
	void			RefDeleted(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); }
	void			RefAdded(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); }
	void			RefAddedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); };
	void			RefDeletedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); };
	RefTargetHandle Clone(RemapDir& remap) override;
	IOResult		Save(ISave *iSave) override { MSPlugin::Save(iSave); return SimpleModBase::Save(iSave); }
	IOResult		Load(ILoad *iLoad) override { MSPlugin::Load(iLoad); return SimpleModBase::Load(iLoad); }
	void			NotifyTarget(int msg, RefMakerHandle rm) override { MSPlugin::NotifyTarget(msg, rm); }  // LAM - 9/7/01 - ECO 624

	// From BaseObject
	const MCHAR *			GetObjectName(bool localized = true) const override { return pc->class_name->to_string(); } // localized name
	void			BeginEditParams(IObjParam *objParam, ULONG vflags, Animatable *pPrev) override;
	void			EndEditParams(IObjParam *objParam, ULONG vflags, Animatable *pNext) override;

	// Clients of SimpleMod need to implement this method
	Deformer&		GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat) override;
	void			InvalidateUI() override;
	Interval		GetValidity(TimeValue t) override;
	BOOL			GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis) override;
};

class MSSimpleModXtnd : public MSPluginSimpleMod
{
public:
	SimpleMod*		delegate;		// my delegate

					MSSimpleModXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSSimpleModXtnd() { DeleteAllRefsFromMe(); }

					void DeleteThis() override {  MSPlugin::DeleteThis(); }

	// From MSPlugin
	ReferenceTarget* get_delegate() override { return delegate; } 

	// From Animatable
	using MSPluginSimpleMod::GetInterface;

	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	SClass_ID		SuperClassID() override { return pc->sclass_id; }
	void			FreeCaches() override { delegate->FreeCaches(); } 		
	int				NumSubs() override { return pblocks.Count() + 1; }  
	Animatable*		SubAnim(int i) override { if (i == 0) return delegate; return pblocks[i-1]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (i == 0)
		{
			return delegate->GetObjectName(localized);
		}
		if (pblocks[i - 1]->GetDesc()->local_name != 0)
		{
			return localized ? pblocks[i - 1]->GetLocalName() : pblocks[i - 1]->GetDesc()->int_name.data();
		}
		return _T("");
	}
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return MSPluginSimpleMod::GetInterface(id); }
	
	// From ReferenceMaker
//	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override;
public:
	RefTargetHandle Clone(RemapDir& remap) override;

	// From BaseObject
	const MCHAR *			GetObjectName(bool localized = true) const override { return pc->class_name->to_string(); } // localized name
	void			BeginEditParams(IObjParam *objParam, ULONG vflags, Animatable *pPrev) override;
	void			EndEditParams(IObjParam *objParam, ULONG vflags, Animatable *pNext) override;
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int vflags, IPoint2 *p, ViewExp *vpt, ModContext* mc) override
						{ return delegate->HitTest(t, inode, type, crossing, vflags, p, vpt, mc); }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int vflags, ModContext *mc) override 
						{ return delegate->Display(t, inode, vpt, vflags, mc); }		
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) override { delegate->GetWorldBoundBox(t, inode, vpt, box, mc); }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) override { delegate->GetLocalBoundBox(t, inode, vpt,  box ); }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override { delegate->Snap(t, inode, snap, p, vpt); }
	CreateMouseCallBack* GetCreateMouseCallBack() override { return delegate->GetCreateMouseCallBack(); } 
	BOOL			HasUVW() override { return delegate->HasUVW(); }
	void			SetGenUVW(BOOL sw) override { delegate->SetGenUVW(sw); }
		
	void			GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) override { delegate->GetSubObjectCenters(cb, t, node, mc); }
	void			GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) override { delegate->GetSubObjectTMs(cb, t, node, mc); }
	BOOL			ChangeTopology() override { return delegate->ChangeTopology(); }
	
	// from Modifier
	ChannelMask		ChannelsUsed() override { return delegate->ChannelsUsed(); }
	ChannelMask		ChannelsChanged() override { return delegate->ChannelsChanged(); }
	// this is used to invalidate cache's in Edit Modifiers:
	void			NotifyInputChanged(const Interval& changeInt, PartID partID, RefMessage message, ModContext *mc) override { delegate->NotifyInputChanged(changeInt, partID, message, mc); }

	// This is the method that is called when the modifier is needed to 
	// apply its effect to the object. Note that the INode* is always NULL
	// for object space modifiers.
	void			ModifyObject(TimeValue t, ModContext &mc, ObjectState* os, INode *node) override { delegate->ModifyObject(t, mc, os, node); }

	// Modifiers that place a dependency on topology should return TRUE
	// for this method. An example would be a modifier that stores a selection
	// set base on vertex indices.
	BOOL			DependOnTopology(ModContext &mc) override { return delegate->DependOnTopology(mc); }

	// this can return:
	//	anyObjectClassID  - (ANY_OBJECT_CLASS_ID,0) where ANY_OBJECT_CLASS_ID = 0x0000ac
	//	genericShapeClassID  - (GENERIC_SHAPE_CLASS_ID,0) where GENERIC_SHAPE_CLASS_ID = 0x0000ab
	//	triObjectClassID - (TRIOBJ_CLASS_ID, 0) where TRIOBJ_CLASS_ID = 0x0009
	//	polyObjectClassID - (POLYOBJ_CLASS_ID, 0) where POLYOBJ_CLASS_ID = 0x5d21369a
	//	mapObjectClassID - (MAPPABLE_OBJ_CLASS_ID,0) where MAPPABLE_OBJ_CLASS_ID = 0x000006
	//	defObjectClassID - (DEFORM_OBJ_CLASS_ID,0) where DEFORM_OBJ_CLASS_ID = 0x000005
	//	patchObjectClassID - (PATCHOBJ_CLASS_ID,0) where PATCHOBJ_CLASS_ID = 0x1030
	//	splineShapeClassID - (SPLINESHAPE_CLASS_ID,0) where SPLINESHAPE_CLASS_ID = 0x00000a
	Class_ID InputType() override
	{
		return delegate->InputType();
	}

	IOResult		SaveLocalData(ISave *iSave, LocalModData *ld) override { return delegate->SaveLocalData(iSave, ld); }
	IOResult		LoadLocalData(ILoad *iLoad, LocalModData **pld) override { return delegate->LoadLocalData(iLoad, pld); }

	// Clients of SimpleMod need to implement this method
	Deformer&		GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat) override { return delegate->GetDeformer(t, mc, mat, invmat); }
	void			InvalidateUI() override { delegate->InvalidateUI(); }
	Interval		GetValidity(TimeValue t) override;
	BOOL			GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis) override { return delegate->GetModLimits(t, zmin,  zmax,  axis); }

};

// ----------------------- MSPluginTexmap ----------------------
// scripted Texmap

class MSPluginTexmap : public MSPlugin, public Texmap
{
public:
	static MSAutoMParamDlg* mainMDlg; // main dialog containing all scripted rollout
	static IMtlParams*		ip;

					MSPluginTexmap() { }
					MSPluginTexmap(MSPluginClass* pc, BOOL loading);
				   ~MSPluginTexmap() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")
	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD vflags=0, int category=ROLLUP_CAT_STANDARD) override;
	void			DeleteRollupPage(HWND hRollup) override;
	IRollupWindow*	GetRollupWindow() override;
	void			RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) override;

	ReferenceTarget* get_delegate() override { return nullptr; } 


	// From Animatable
	using Texmap::GetInterface;

	void			DeleteThis() override;
	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	void			FreeCaches() override { } 		
	int				NumSubs() override { return pblocks.Count(); }
	Animatable*		SubAnim(int i) override { return pblocks[i]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (pblocks[i]->GetDesc()->local_name != 0)
		{
			return localized ? pblocks[i]->GetLocalName() : pblocks[i]->GetDesc()->int_name.data();
		}
		
		return _T("");
	}
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return Texmap::GetInterface(id); }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override 
					{ 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
					}

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override;
public:
	void			RefDeleted(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); }
	void			RefAdded(RefMakerHandle rm) override { Texmap::RefAdded(rm); MSPlugin::RefAdded(rm); }
	void			RefAddedUndoRedo(RefMakerHandle rm) override { Texmap::RefAddedUndoRedo(rm); MSPlugin::RefAdded(rm); };
	void			RefDeletedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); };
	RefTargetHandle Clone(RemapDir& remap) override;

	// From MtlBase

	MSTR			GetFullName(bool localized = true) override { return MtlBase::GetFullName(localized); }
	int				BuildMaps(TimeValue t, RenderMapsContext &rmc) override { return MtlBase::BuildMaps(t, rmc); }
	ULONG			Requirements(int subMtlNum) override { return MtlBase::Requirements(subMtlNum); }
	ULONG			LocalRequirements(int subMtlNum) override { return MtlBase::LocalRequirements(subMtlNum); }
	void			MappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) override { MtlBase::MappingsRequired(subMtlNum, mapreq, bumpreq); } 
	void			LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) override { MtlBase::LocalMappingsRequired(subMtlNum, mapreq, bumpreq); }
	BOOL			IsMultiMtl() override { return MtlBase::IsMultiMtl(); }
	int				NumSubTexmaps() override;
	Texmap*			GetSubTexmap(int i) override;
//	int				MapSlotType(int i) { return MtlBase::MapSlotType(i); }
	void			SetSubTexmap(int i, Texmap *m) override;
//	int				SubTexmapOn(int i) { return MtlBase::SubTexmapOn(i); }
//	void			DeactivateMapsInTree() { MtlBase::DeactivateMapsInTree(); }
	MSTR			GetSubTexmapSlotName(int i, bool localized = true) override;
	MSTR			GetSubTexmapTVName(int i, bool localized = true) { return GetSubTexmapSlotName(i, localized); }
//	void			CopySubTexmap(HWND hwnd, int ifrom, int ito) { MtlBase::CopySubTexmap(hwnd, ifrom, ito); }
	void			Update(TimeValue t, Interval& valid) override { }
	void			Reset() override { pc->cd2->Reset(this, TRUE); }
	Interval		Validity(TimeValue t) override;
	ParamDlg*		CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp) override;
	IOResult		Save(ISave *iSave) override { MSPlugin::Save(iSave); return MtlBase::Save(iSave); }
	IOResult		Load(ILoad *iLoad) override { MSPlugin::Load(iLoad); return MtlBase::Load(iLoad); }

	ULONG			GetGBufID() { return MtlBase::GetGBufID(); }
	void			SetGBufID(ULONG id) { MtlBase::SetGBufID(id); }

	void			EnumAuxFiles(AssetEnumCallback& assetEnum, DWORD vflags) override 
	{
		if ((vflags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/21/03
		ReferenceTarget::EnumAuxFiles(assetEnum, vflags);
	}
	PStamp*			GetPStamp(int sz) override { return MtlBase::GetPStamp(sz); }
	PStamp*			CreatePStamp(int sz) { return MtlBase::CreatePStamp(sz); }   		
	void			DiscardPStamp(int sz) override { MtlBase::DiscardPStamp(sz); }      		
	BOOL			SupportTexDisplay() override { return MtlBase::SupportTexDisplay(); }
	DWORD_PTR		GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) override { return MtlBase::GetActiveTexHandle(t, thmaker); }
	void			ActivateTexDisplay(BOOL onoff) override { MtlBase::ActivateTexDisplay(onoff); }
	BOOL			SupportsMultiMapsInViewport() override { return MtlBase::SupportsMultiMapsInViewport(); }
	void			SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb) override { MtlBase::SetupGfxMultiMaps(t, mtl, cb); }
	ReferenceTarget *GetRefTarget() override { return MtlBase::GetRefTarget(); }

	// From Texmap

	// Evaluate the color of map for the context.
	AColor			EvalColor(ShadeContext& sc) override  { return AColor (0,0,0); }

	// Evaluate the map for a "mono" channel.
	// this just permits a bit of optimization 
	float			EvalMono(ShadeContext& sc) override { return Texmap::EvalMono(sc); }
	
	// For Bump mapping, need a perturbation to apply to a normal.
	// Leave it up to the Texmap to determine how to do this.
	Point3			EvalNormalPerturb(ShadeContext& sc) override { return Point3(0,0,0); }

	// This query is made of maps plugged into the Reflection or 
	// Refraction slots:  Normally the view vector is replaced with
	// a reflected or refracted one before calling the map: if the 
	// plugged in map doesn't need this , it should return TRUE.
	BOOL			HandleOwnViewPerturb() override { return Texmap::HandleOwnViewPerturb(); }

	void			GetUVTransform(Matrix3 &uvtrans) override {Texmap::GetUVTransform(uvtrans); }
	int				GetTextureTiling() override { return Texmap::GetTextureTiling(); }
	void			InitSlotType(int sType) override { Texmap::InitSlotType(sType); }			   
	int				GetUVWSource() override { return Texmap::GetUVWSource(); }
	int				GetMapChannel () override { return Texmap::GetMapChannel (); }	// only relevant if above returns UVWSRC_EXPLICIT

	UVGen *			GetTheUVGen() override { return Texmap::GetTheUVGen(); }  // maps with a UVGen should implement this
	XYZGen *		GetTheXYZGen() override { return Texmap::GetTheXYZGen(); } // maps with a XYZGen should implement this

	// System function to set slot type for all subtexmaps in a tree.
	void			SetOutputLevel(TimeValue t, float v) override { Texmap::SetOutputLevel(t, v); }

	// called prior to render: missing map names should be added to NameAccum.
	// return 1: success,   0:failure. 
	int				LoadMapFiles(TimeValue t) override { return Texmap::LoadMapFiles(t); } 

	// render a 2-d bitmap version of map.
	void			RenderBitmap(TimeValue t, Bitmap *bm, float scale3D=1.0f, BOOL filter = FALSE) override { Texmap::RenderBitmap(t, bm, scale3D, filter); }

	// --- Texmap evaluation ---
	// The output of a texmap is meaningful in a given ShadeContext 
	// if it is the same as when the scene is rendered.			
	bool			IsLocalOutputMeaningful( ShadeContext& sc ) override { return Texmap::IsLocalOutputMeaningful( sc ); }
	bool			IsOutputMeaningful( ShadeContext& sc ) override { return Texmap::IsOutputMeaningful( sc ); }

};


class MSTexmapXtnd : public MSPluginTexmap
{
public:
	Texmap*			delegate;		// my delegate

					MSTexmapXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSTexmapXtnd() { DeleteAllRefsFromMe(); }

	void			DeleteThis() override;

	// From MSPlugin
	ReferenceTarget* get_delegate() override { return delegate; } 

	// From Animatable
	using MSPluginTexmap::GetInterface;

	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	void			FreeCaches() override { delegate->FreeCaches(); } 	

	int				NumSubs() override; 
	Animatable*		SubAnim(int i) override; 
	MSTR SubAnimName(int i, bool localized = true) override; 
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return MSPluginTexmap::GetInterface(id); }
	
	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override;
public:
	RefTargetHandle Clone(RemapDir& remap) override;
	RefResult NotifyDependents(const Interval& changeInt, PartID partID, RefMessage message,
		SClass_ID sclass, BOOL propagate, RefTargetHandle hTarg, NotifyDependentsOption notifyDependentsOption) override;

	// From MtlBase

	int				BuildMaps(TimeValue t, RenderMapsContext &rmc) override { return delegate->BuildMaps(t, rmc); }
	ULONG			Requirements(int subMtlNum) override { return delegate->Requirements(subMtlNum); }
	ULONG			LocalRequirements(int subMtlNum) override { return delegate->LocalRequirements(subMtlNum); }
	void			MappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) override { delegate->MappingsRequired(subMtlNum, mapreq, bumpreq); } 
	void			LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) override { delegate->LocalMappingsRequired(subMtlNum, mapreq, bumpreq); }
	BOOL			IsMultiMtl() override { return delegate->IsMultiMtl(); }
	void			Update(TimeValue t, Interval& valid) override;
	void			Reset() override { delegate->Reset(); pc->cd2->Reset(this, TRUE); }
	Interval		Validity(TimeValue t) override;
	ParamDlg*		CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp) override;
	IOResult		Save(ISave *iSave) override { return MSPluginTexmap::Save(iSave); }
	IOResult		Load(ILoad *iLoad) override { return MSPluginTexmap::Load(iLoad); }

	ULONG			GetGBufID() { return delegate->GetGBufID(); }
	void			SetGBufID(ULONG id) { delegate->SetGBufID(id); }
	
	void			EnumAuxFiles(AssetEnumCallback& assetEnum, DWORD vflags) override 
	{
		if ((vflags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/21/03
		ReferenceTarget::EnumAuxFiles(assetEnum, vflags);
	}
	PStamp*			GetPStamp(int sz) override { return delegate->GetPStamp(sz); }
	PStamp*			CreatePStamp(int sz) { return delegate->CreatePStamp(sz); }   		
	void			DiscardPStamp(int sz) override { delegate->DiscardPStamp(sz); }      		

	int				NumSubTexmaps() override;
	Texmap*			GetSubTexmap(int i) override;
//	int				MapSlotType(int i) { return MtlBase::MapSlotType(i); }
	void			SetSubTexmap(int i, Texmap *m) override;
//	int				SubTexmapOn(int i) { return MtlBase::SubTexmapOn(i); }
//	void			DeactivateMapsInTree() { MtlBase::DeactivateMapsInTree(); }     
	MSTR			GetSubTexmapSlotName(int i, bool localized = true) override;
	BOOL			SupportTexDisplay() override { return delegate->SupportTexDisplay(); }
	DWORD_PTR		GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) override { return delegate->GetActiveTexHandle(t, thmaker); }
	void			ActivateTexDisplay(BOOL onoff) override { delegate->ActivateTexDisplay(onoff); }
	BOOL			SupportsMultiMapsInViewport() override { return delegate->SupportsMultiMapsInViewport(); }
	void			SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb) override { delegate->SetupGfxMultiMaps(t, mtl, cb); }
	ReferenceTarget *GetRefTarget() override { return delegate->GetRefTarget(); }

	// From Texmap

	// Evaluate the color of map for the context.
	AColor			EvalColor(ShadeContext& sc) override  { return delegate->EvalColor(sc); }
	
	// Evaluate the map for a "mono" channel.
	// this just permits a bit of optimization 
	float			EvalMono(ShadeContext& sc) override { return delegate->EvalMono(sc); }
	
	// For Bump mapping, need a perturbation to apply to a normal.
	// Leave it up to the Texmap to determine how to do this.
	Point3			EvalNormalPerturb(ShadeContext& sc) override { return delegate->EvalNormalPerturb(sc); }

	// This query is made of maps plugged into the Reflection or 
	// Refraction slots:  Normally the view vector is replaced with
	// a reflected or refracted one before calling the map: if the 
	// plugged in map doesn't need this , it should return TRUE.
	BOOL			HandleOwnViewPerturb() override { return delegate->HandleOwnViewPerturb(); }

	BITMAPINFO*		GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono=FALSE, int forceW=0, int forceH=0) override
					{ return delegate->GetVPDisplayDIB(t, thmaker, valid, mono, forceW, forceH);  }

	void			GetUVTransform(Matrix3 &uvtrans) override {delegate->GetUVTransform(uvtrans); }
	int				GetTextureTiling() override { return delegate->GetTextureTiling(); }
	void			InitSlotType(int sType) override { delegate->InitSlotType(sType); }			   
	int				GetUVWSource() override { return delegate->GetUVWSource(); }
	int				GetMapChannel () override { return delegate->GetMapChannel (); }	// only relevant if above returns UVWSRC_EXPLICIT

	UVGen *			GetTheUVGen() override { return delegate->GetTheUVGen(); }  // maps with a UVGen should implement this
	XYZGen *		GetTheXYZGen() override { return delegate->GetTheXYZGen(); } // maps with a XYZGen should implement this

	void			SetOutputLevel(TimeValue t, float v) override { delegate->SetOutputLevel(t, v); }

	// called prior to render: missing map names should be added to NameAccum.
	// return 1: success,   0:failure. 
	int				LoadMapFiles(TimeValue t) override { return delegate->LoadMapFiles(t); } 

	// render a 2-d bitmap version of map.
	void			RenderBitmap(TimeValue t, Bitmap *bm, float scale3D=1.0f, BOOL filter = FALSE) override { delegate->RenderBitmap(t, bm, scale3D, filter); }

//	void			RefAdded(RefMakerHandle rm){ delegate->RefAdded(rm); }

	// --- Texmap evaluation ---
	// The output of a texmap is meaningful in a given ShadeContext 
	// if it is the same as when the scene is rendered.			
	bool			IsLocalOutputMeaningful( ShadeContext& sc ) override { return delegate->IsLocalOutputMeaningful( sc ); }
	bool			IsOutputMeaningful( ShadeContext& sc ) override { return delegate->IsOutputMeaningful( sc ); }

	int				IsHighDynamicRange( ) { return delegate->IsHighDynamicRange( ); }
};

// ----------------------- MSPluginMtl ----------------------
// scripted Mtl

class MSPluginMtl : public MSPlugin, public Mtl
{
public:
	static MSAutoMParamDlg* mainMDlg; // main dialog containing all scripted rollout
	static IMtlParams*		ip;

					MSPluginMtl() { }
					MSPluginMtl(MSPluginClass* pc, BOOL loading);
				   ~MSPluginMtl() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")
	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD vflags=0, int category=ROLLUP_CAT_STANDARD) override;
	void			DeleteRollupPage(HWND hRollup) override;
	IRollupWindow*	GetRollupWindow() override;
	void			RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) override;

	ReferenceTarget* get_delegate() override { return nullptr; } 


	// From Animatable
	using Mtl::GetInterface;

	void			DeleteThis() override;
	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	void			FreeCaches() override { } 		
	int				NumSubs() override { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) override { return pblocks[i]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (pblocks[i]->GetDesc()->local_name != 0)
		{
			return localized ? pblocks[i]->GetLocalName() : pblocks[i]->GetDesc()->int_name.data();
		}
		
		return _T("");
	}
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return Mtl::GetInterface(id); }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override 
					{ 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
					}

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override;
public:
	RefTargetHandle Clone(RemapDir& remap) override;

	// From MtlBase

	MSTR			GetFullName(bool localized = true) override { return MtlBase::GetFullName(localized); }
	int				BuildMaps(TimeValue t, RenderMapsContext &rmc) override { return MtlBase::BuildMaps(t, rmc); }
	ULONG			Requirements(int subMtlNum) override { return MtlBase::Requirements(subMtlNum); }
	ULONG			LocalRequirements(int subMtlNum) override { return MtlBase::LocalRequirements(subMtlNum); }
	void			MappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) override { MtlBase::MappingsRequired(subMtlNum, mapreq, bumpreq); } 
	void			LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) override { MtlBase::LocalMappingsRequired(subMtlNum, mapreq, bumpreq); }
	BOOL			IsMultiMtl() override { return MtlBase::IsMultiMtl(); }
	int				NumSubTexmaps() override;
	Texmap*			GetSubTexmap(int i) override;
//	int				MapSlotType(int i) { return MtlBase::MapSlotType(i); }
	void			SetSubTexmap(int i, Texmap *m) override;
//	int				SubTexmapOn(int i) { return MtlBase::SubTexmapOn(i); }
//	void			DeactivateMapsInTree() { MtlBase::DeactivateMapsInTree(); }
	MSTR			GetSubTexmapSlotName(int i, bool localized = true) override;
	MSTR			GetSubTexmapTVName(int i, bool localized = true) { return GetSubTexmapSlotName(i, localized); }
//	void			CopySubTexmap(HWND hwnd, int ifrom, int ito) { MtlBase::CopySubTexmap(hwnd, ifrom, ito); }
	void			Update(TimeValue t, Interval& valid) override { }
	void			Reset() override { pc->cd2->Reset(this, TRUE); }
	Interval		Validity(TimeValue t) override;
	ParamDlg*		CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp) override;
	IOResult		Save(ISave *iSave) override { MSPlugin::Save(iSave); return MtlBase::Save(iSave); }
	IOResult		Load(ILoad *iLoad) override { MSPlugin::Load(iLoad); return MtlBase::Load(iLoad); }

	ULONG			GetGBufID() { return MtlBase::GetGBufID(); }
	void			SetGBufID(ULONG id) { MtlBase::SetGBufID(id); }

	void			EnumAuxFiles(AssetEnumCallback& assetEnum, DWORD vflags) override 
	{	
		if ((vflags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/21/03
		ReferenceTarget::EnumAuxFiles(assetEnum, vflags);
	}
	PStamp*			GetPStamp(int sz) override { return MtlBase::GetPStamp(sz); }
	PStamp*			CreatePStamp(int sz) { return MtlBase::CreatePStamp(sz); }   		
	void			DiscardPStamp(int sz) override { MtlBase::DiscardPStamp(sz); }      		
	BOOL			SupportTexDisplay() override { return MtlBase::SupportTexDisplay(); }
	DWORD_PTR		GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) override { return MtlBase::GetActiveTexHandle(t, thmaker); }
	void			ActivateTexDisplay(BOOL onoff) override { MtlBase::ActivateTexDisplay(onoff); }
	BOOL			SupportsMultiMapsInViewport() override { return MtlBase::SupportsMultiMapsInViewport(); }
	void			SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb) override { MtlBase::SetupGfxMultiMaps(t, mtl, cb); }
	ReferenceTarget *GetRefTarget() override { return MtlBase::GetRefTarget(); }

	// From Mtl

	MtlBase*		GetActiveTexmap() { return Mtl::GetActiveTexmap(); } 
	void			SetActiveTexmap( MtlBase *txm) { Mtl::SetActiveTexmap(txm); } 
	void			RefDeleted(RefMakerHandle rm) override { Mtl::RefDeleted(rm); MSPlugin::RefDeleted(rm); } 
	void			RefAdded(RefMakerHandle rm) override;
	void			RefAddedUndoRedo(RefMakerHandle rm) override { Mtl::RefAddedUndoRedo(rm); MSPlugin::RefAdded(rm); };
	void			RefDeletedUndoRedo(RefMakerHandle rm) override { Mtl::RefDeletedUndoRedo(rm); MSPlugin::RefDeleted(rm); };
	Color			GetAmbient(int mtlNum=0, BOOL backFace=FALSE) override { return Color(0,0,0); }
	Color			GetDiffuse(int mtlNum=0, BOOL backFace=FALSE) override { return Color(0,0,0); }	    
	Color			GetSpecular(int mtlNum=0, BOOL backFace=FALSE) override { return Color(0,0,0); }
	float			GetShininess(int mtlNum=0, BOOL backFace=FALSE) override { return 0.0f; }
	float			GetShinStr(int mtlNum=0, BOOL backFace=FALSE) override { return 0.0f; }		
	float			GetXParency(int mtlNum=0, BOOL backFace=FALSE) override { return 0.0f; }
	BOOL			GetSelfIllumColorOn(int mtlNum=0, BOOL backFace=FALSE) override { return Mtl::GetSelfIllumColorOn(mtlNum, backFace); } 
	float			GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE) override { return Mtl::GetSelfIllum(mtlNum, backFace); } 
	Color			GetSelfIllumColor(int mtlNum=0, BOOL backFace=FALSE) override { return Mtl::GetSelfIllumColor(mtlNum, backFace); } 
	float			WireSize(int mtlNum=0, BOOL backFace=FALSE) override { return Mtl::WireSize(mtlNum, backFace); } 
	void			SetAmbient(Color c, TimeValue t) override { }		
	void			SetDiffuse(Color c, TimeValue t) override { }		
	void			SetSpecular(Color c, TimeValue t) override { }
	void			SetShininess(float v, TimeValue t) override { }	
	void			Shade(ShadeContext& sc) override { }
	int				NumSubMtls() override;
	Mtl*			GetSubMtl(int i) override;
	void			SetSubMtl(int i, Mtl *m) override;
	MSTR			GetSubMtlSlotName(int i, bool localized = true) override;
	MSTR			GetSubMtlTVName(int i, bool localized = true) { return GetSubMtlSlotName(i, localized); }
//	void			CopySubMtl(HWND hwnd, int ifrom, int ito) { Mtl::CopySubMtl(hwnd, ifrom, ito); }  
	float			EvalDisplacement(ShadeContext& sc) override { return Mtl::EvalDisplacement(sc); } 
	Interval		DisplacementValidity(TimeValue t) override { return Mtl::DisplacementValidity(t); } 

	// --- Material evaluation
	// Returns true if the evaluated color\value (output) is constant 
	// over all possible inputs. 
	bool			IsOutputConst( ShadeContext& sc, int stdID ) override { return Mtl::IsOutputConst( sc, stdID ); }
	// Evaluates the material on a single standard texmap channel (ID_AM, etc) 
	// at a UVW cordinated and over an area described in the ShadingContext
	bool			EvalColorStdChannel( ShadeContext& sc, int stdID, Color& outClr) override { return Mtl::EvalColorStdChannel( sc, stdID, outClr ); }
	bool			EvalMonoStdChannel( ShadeContext& sc, int stdID, float& outVal) override { return Mtl::EvalMonoStdChannel( sc, stdID, outVal ); }
};


class MSMtlXtnd : public MSPluginMtl
{
public:
	Mtl*			delegate;		// my delegate

					MSMtlXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSMtlXtnd() { DeleteAllRefsFromMe(); }

	void			DeleteThis() override;

	// From MSPlugin
	ReferenceTarget* get_delegate() override { return delegate; } 

	// From Animatable
	using MSPluginMtl::GetInterface;

	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	void			FreeCaches() override { if (delegate) delegate->FreeCaches(); } 		
	int				NumSubs() override; 
	Animatable*		SubAnim(int i) override;
	MSTR SubAnimName(int i, bool localized = true) override;
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override 
					{ 
						if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; 
						if (id == IID_IReshading) return delegate->GetInterface(id); 
						return MSPluginMtl::GetInterface(id); 
					}

	BaseInterface* GetInterface(Interface_ID id) override
	{
		return delegate ? delegate->GetInterface(id) : nullptr;
	}
	
	// From ReferenceMaker
//	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override;
public:
	RefTargetHandle Clone(RemapDir& remap) override;
	RefResult NotifyDependents(const Interval& changeInt, PartID partID, RefMessage message,
		SClass_ID sclass, BOOL propagate, RefTargetHandle hTarg, NotifyDependentsOption notifyDependentsOption) override;

	// From MtlBase

	int				BuildMaps(TimeValue t, RenderMapsContext &rmc) override { return delegate->BuildMaps(t, rmc); }
	ULONG			Requirements(int subMtlNum) override { return delegate->Requirements(subMtlNum); }
	ULONG			LocalRequirements(int subMtlNum) override { return delegate->LocalRequirements(subMtlNum); }
	void			MappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) override { delegate->MappingsRequired(subMtlNum, mapreq, bumpreq); } 
	void			LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) override { delegate->LocalMappingsRequired(subMtlNum, mapreq, bumpreq); }
	BOOL			IsMultiMtl() override { return delegate->IsMultiMtl(); }
	int				NumSubTexmaps() override;
	Texmap*			GetSubTexmap(int i) override;
//	int				MapSlotType(int i) { return delegate->MapSlotType(i); }
	void			SetSubTexmap(int i, Texmap *m) override;
//	int				SubTexmapOn(int i) { return delegate->SubTexmapOn(i); }
//	void			DeactivateMapsInTree() { delegate->DeactivateMapsInTree(); }
	MSTR			GetSubTexmapSlotName(int i, bool localized = true) override;
//	MSTR			GetSubTexmapTVName(int i, bool localized = true) override { return delegate->GetSubTexmapTVName(i, localized); }
//	void			CopySubTexmap(HWND hwnd, int ifrom, int ito) { delegate->CopySubTexmap(hwnd, ifrom, ito); }
	
	void			Update(TimeValue t, Interval& valid) override;
	void			Reset() override { delegate->Reset(); pc->cd2->Reset(this, TRUE); }
	Interval		Validity(TimeValue t) override;
	ParamDlg*		CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp) override;
	IOResult		Save(ISave *iSave) override { return MSPluginMtl::Save(iSave); }
	IOResult		Load(ILoad *iLoad) override { return MSPluginMtl::Load(iLoad); }

	ULONG			GetGBufID() { return delegate->GetGBufID(); }
	void			SetGBufID(ULONG id) { delegate->SetGBufID(id); }
	
	void			EnumAuxFiles(AssetEnumCallback& assetEnum, DWORD vflags) override 
	{
		if ((vflags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/21/03
		ReferenceTarget::EnumAuxFiles(assetEnum, vflags);
	}
	PStamp*			GetPStamp(int sz) override { return delegate->GetPStamp(sz); }
	PStamp*			CreatePStamp(int sz) { return delegate->CreatePStamp(sz); }   		
	void			DiscardPStamp(int sz) override { delegate->DiscardPStamp(sz); }      		
	BOOL			SupportTexDisplay() override { return delegate->SupportTexDisplay(); }
	DWORD_PTR		GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) override { return delegate->GetActiveTexHandle(t, thmaker); }
	void			ActivateTexDisplay(BOOL onoff) override { delegate->ActivateTexDisplay(onoff); }
	BOOL			SupportsMultiMapsInViewport() override { return delegate->SupportsMultiMapsInViewport(); }
	void			SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb) override { delegate->SetupGfxMultiMaps(t, mtl, cb); }
	ReferenceTarget *GetRefTarget() override { return delegate->GetRefTarget(); }

	// From Mtl

	BOOL			DontKeepOldMtl() override { return TRUE; }
	MtlBase*		GetActiveTexmap() { return delegate->GetActiveTexmap(); } 
	void			SetActiveTexmap( MtlBase *txm) { delegate->SetActiveTexmap(txm); } 
	Color			GetAmbient(int mtlNum=0, BOOL backFace=FALSE) override { return delegate->GetAmbient(mtlNum, backFace); }
	Color			GetDiffuse(int mtlNum=0, BOOL backFace=FALSE) override { return delegate->GetDiffuse(mtlNum, backFace); }	    
	Color			GetSpecular(int mtlNum=0, BOOL backFace=FALSE) override { return delegate->GetSpecular(mtlNum, backFace); }
	float			GetShininess(int mtlNum=0, BOOL backFace=FALSE) override { return delegate->GetShininess(mtlNum=0, backFace); }
	float			GetShinStr(int mtlNum=0, BOOL backFace=FALSE) override { return delegate->GetShinStr(mtlNum=0, backFace); }		
	float			GetXParency(int mtlNum=0, BOOL backFace=FALSE) override { return delegate->GetXParency(mtlNum=0, backFace); }
	BOOL			GetSelfIllumColorOn(int mtlNum=0, BOOL backFace=FALSE) override { return delegate->GetSelfIllumColorOn(mtlNum, backFace); } 
	float			GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE) override { return delegate->GetSelfIllum(mtlNum, backFace); } 
	Color			GetSelfIllumColor(int mtlNum=0, BOOL backFace=FALSE) override { return delegate->GetSelfIllumColor(mtlNum, backFace); } 

	Sampler*		GetPixelSampler(int mtlNum=0, BOOL backFace=FALSE) override{ return delegate->GetPixelSampler(mtlNum, backFace); } 

	float			WireSize(int mtlNum=0, BOOL backFace=FALSE) override { return delegate->WireSize(mtlNum, backFace); } 
	void			SetAmbient(Color c, TimeValue t) override { delegate->SetAmbient(c, t); }		
	void			SetDiffuse(Color c, TimeValue t) override { delegate->SetDiffuse(c, t); }		
	void			SetSpecular(Color c, TimeValue t) override { delegate->SetSpecular(c, t); }
	void			SetShininess(float v, TimeValue t) override { delegate->SetShininess(v, t); }

	BOOL			SupportsShaders() override { return delegate->SupportsShaders(); }
	BOOL			SupportsRenderElements() override { return delegate->SupportsRenderElements(); }

	void			Shade(ShadeContext& sc) override { delegate->Shade(sc); }
	int				NumSubMtls() override;
	Mtl*			GetSubMtl(int i) override;
	void			SetSubMtl(int i, Mtl *m) override;
	MSTR			GetSubMtlSlotName(int i, bool localized = true) override;
	MSTR			GetSubMtlTVName(int i, bool localized = true) { return GetSubMtlSlotName(i, localized); }
//	void			CopySubMtl(HWND hwnd, int ifrom, int ito) { delegate->CopySubMtl(hwnd, ifrom, ito); }  
	float			EvalDisplacement(ShadeContext& sc) override { return delegate->EvalDisplacement(sc); } 
	Interval		DisplacementValidity(TimeValue t) override { return delegate->DisplacementValidity(t); } 

	// --- Material evaluation
	// Returns true if the evaluated color\value (output) is constant over all possible inputs. 
	bool			IsOutputConst( ShadeContext& sc, int stdID ) override { return delegate->IsOutputConst( sc, stdID ); }
	// Evaluates the material on a single standard texmap channel (ID_AM, etc) 
	// at a UVW cordinated and over an area described in the ShadingContext
	bool			EvalColorStdChannel( ShadeContext& sc, int stdID, Color& outClr) override { return delegate->EvalColorStdChannel( sc, stdID, outClr ); }
	bool			EvalMonoStdChannel( ShadeContext& sc, int stdID, float& outVal) override { return delegate->EvalMonoStdChannel( sc, stdID, outVal ); }

	// Need to get/set properties for the delegate
	int			SetProperty(ULONG id, void *data) override { return delegate->SetProperty( id, data ); }
	void*			GetProperty(ULONG id) override { return delegate->GetProperty( id ); }

};

/* ------------- ParamDlg class for scripter material/texmap plug-ins --------------- */

class MSAutoMParamDlg : public IAutoMParamDlg 
{
	public:
		Tab<ParamDlg*>	secondaryDlgs;	// secondary ParamDlgs if this is a main
		ParamDlg*		delegateDlg;	// my delegate's dialog if any
		MSPlugin*		plugin;			// target plugin
		Rollout*		ro;				// rollout controlling dialog
		ReferenceTarget* mtl;			// material in the dialog
		IMtlParams*		ip;				// mtl interface
		TexDADMgr		texDadMgr;
		MtlDADMgr		mtlDadMgr;
		HWND			hwmedit;		// medit window

					MSAutoMParamDlg(HWND hMedit, IMtlParams *i, ReferenceTarget* mtl, MSPlugin* plugin, Rollout* ro);
				   ~MSAutoMParamDlg();

		// from ParamDlg
		Class_ID	ClassID() override { return mtl->ClassID(); }
		ReferenceTarget* GetThing() override { return mtl; }
		void		SetThing(ReferenceTarget *m) override;
		void		DeleteThis() override;
		void		SetTime(TimeValue t) override;
		void		ReloadDialog() override;
		void		ActivateDlg(BOOL onOff) override;
		int			FindSubTexFromHWND(HWND hw) override;	
		int			FindSubMtlFromHWND(HWND hw) override;	

		void		InvalidateUI() override { ReloadDialog(); }
		void		MtlChanged() override { ip->MtlChanged(); }
		// secondary dialog list management
		int			NumDlgs() override { return secondaryDlgs.Count(); }
		void		AddDlg(ParamDlg* dlg) override;
		ParamDlg*	GetDlg(int i) override;
		void		SetDlg(int i, ParamDlg* dlg) override;
		void		DeleteDlg(ParamDlg* dlg) override;

		// access to this dlg's parammap stuff
		IParamMap2* GetMap() override { return nullptr; }
};

// RK: Start
// ----------------------- MSPluginSpecialFX ----------------------

//	template for MSPlugin classes derived from SpecialFX
template <class TYPE> class MSPluginSpecialFX : public MSPlugin, public TYPE
{
public:
	IRendParams		*ip;

	void			DeleteThis() override;
#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")	
	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD vflags=0, int category=ROLLUP_CAT_STANDARD) override;
	void			DeleteRollupPage(HWND hRollup) override;
	IRollupWindow*	GetRollupWindow() override;
	void			RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) override;

	ReferenceTarget* get_delegate() override { return nullptr; }  // no delegate

	// From Animatable
	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	void			FreeCaches() override { } 		
	int				NumSubs() override { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) override { return pblocks[i]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (pblocks[i]->GetDesc()->local_name != 0)
		{
			return localized ? pblocks[i]->GetLocalName() : pblocks[i]->GetDesc()->int_name.data();
		}
		
		return _T("");
	}
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return TYPE::GetInterface(id); }
    
    
    BaseInterface*GetInterface(Interface_ID id) override{ 
        ///////////////////////////////////////////////////////////////////////////
        // GetInterface(Interface_ID) was added after the MAX 4
        // SDK shipped. This did not break the SDK because
        // it only calls the base class implementation. If you add
        // any other code here, plugins compiled with the MAX 4 SDK
        // that derive from MSPluginSpecialFX and call Base class
        // implementations of GetInterface(Interface_ID), will not call
        // that code in this routine. This means that the interface
        // you are adding will not be exposed for these objects,
        // and could have unexpected results.
        return TYPE::GetInterface(id); 
        /////////////////////////////////////////////////////////////////////////////
    }
    

	// From ReferenceMaker
	RefResult		NotifyRefChanged( const Interval& changeInt,RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override
					{ 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
					}

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
	void			SetReference(int i, RefTargetHandle rtarg) override;
	void			RefDeleted(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); }
	void			RefAdded(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); }
	void			RefAddedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefAdded(rm); };
	void			RefDeletedUndoRedo(RefMakerHandle rm) override { MSPlugin::RefDeleted(rm); };
	IOResult		Save(ISave *iSave) override { MSPlugin::Save(iSave); return SpecialFX::Save(iSave); }
	IOResult		Load(ILoad *iLoad) override { MSPlugin::Load(iLoad); return SpecialFX::Load(iLoad); }

	// From SpecialFX
	MSTR			GetName(bool localized = true) const override { return localized ? pc->class_name->to_string() : pc->nonLocalized_class_name->to_string(); }
	BOOL			Active(TimeValue t) override { return SpecialFX::Active(t); }
	void			Update(TimeValue t, Interval& valid) override { SpecialFX::Update(t, valid); }
	int				NumGizmos() override { return SpecialFX::NumGizmos(); }
	INode*			GetGizmo(int i) override { return SpecialFX::GetGizmo(i); }
	void			DeleteGizmo(int i) override { SpecialFX::DeleteGizmo(i); }
	void			AppendGizmo(INode *node) override { SpecialFX::AppendGizmo(node); }
	BOOL			OKGizmo(INode *node) override { return SpecialFX::OKGizmo(node); } 
	void			EditGizmo(INode *node) override { SpecialFX::EditGizmo(node); } 
};

//	template for MSPlugin Xtnd classes derived from SpecialFX
template <class TYPE, class MS_SUPER> class MSSpecialFXXtnd : public MS_SUPER
{
public:
	TYPE*			delegate;		// my delegate

	void			DeleteThis() override;

	// From MSPlugin
	ReferenceTarget* get_delegate() override { return delegate; } 

	// From Animatable
	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? MS_SUPER::pc->ClassName() : MS_SUPER::pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return MS_SUPER::pc->class_id; }
	void			FreeCaches() override { delegate->FreeCaches(); } 		
	int				NumSubs() override { return MS_SUPER::pblocks.Count() + 1; }  
	Animatable*		SubAnim(int i) override { if (i == 0) { return delegate; } return MS_SUPER::pblocks[i-1]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (i == 0)
		{
			MSTR n;
			delegate->GetClassName(n, localized);
			return n;
		}
		if (MS_SUPER::pblocks[i - 1]->GetDesc()->local_name != 0)
		{
			return localized ? MS_SUPER::pblocks[i - 1]->GetLocalName() : MS_SUPER::pblocks[i - 1]->GetDesc()->int_name.data();
		}
		return _T("");
	}
	int				NumParamBlocks() override { return MS_SUPER::pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return MS_SUPER::pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return MS_SUPER::GetInterface(id); }

    
	BaseInterface* GetInterface(Interface_ID id) override{ 
        ///////////////////////////////////////////////////////////////////////////
        // GetInterface(Interface_ID) was added after the MAX 4
        // SDK shipped. This did not break the SDK because
        // it only calls the base class implementation. If you add
        // any other code here, plugins compiled with the MAX 4 SDK
        // that derive from MSSpecialFXXtnd and call Base class
        // implementations of GetInterface(Interface_ID), will not call
        // that code in this routine. This means that the interface
        // you are adding will not be exposed for these objects,
        // and could have unexpected results.
        return MS_SUPER::GetInterface(id); 
	    //////////////////////////////////////////////////////////////////////////////
    }

	// From ReferenceMaker
//	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
	void			SetReference(int i, RefTargetHandle rtarg) override;

	// From SpecialFX
	BOOL			Active(TimeValue t) override { return delegate->Active(t); }
	void			Update(TimeValue t, Interval& valid) override;
	int				NumGizmos() override { return delegate->NumGizmos(); }
	INode*			GetGizmo(int i) override { return delegate->GetGizmo(i); }
	void			DeleteGizmo(int i) override { delegate->DeleteGizmo(i); }
	void			AppendGizmo(INode *node) override { delegate->AppendGizmo(node); }
	BOOL			OKGizmo(INode *node) override { return delegate->OKGizmo(node); } 
	void			EditGizmo(INode *node) override { delegate->EditGizmo(node); } 
};

// ----------------------- MSPluginEffect ----------------------
// scripted Effect

class MSPluginEffect : public MSPluginSpecialFX<Effect8>
{
public:
	MSAutoEParamDlg* mainFXDlg; // main dialog containing all scripted rollout

					MSPluginEffect() { }
					MSPluginEffect(MSPluginClass* pc, BOOL loading);
				   ~MSPluginEffect() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap) override;

	// from Effect
	EffectParamDlg*	CreateParamDialog(IRendParams* imp) override;
	DWORD			GBufferChannelsRequired(TimeValue t) override;
	void			Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *cb ) override;

	// from Effect8
	bool SupportsBitmap(Bitmap& bitmap) override;

	Effect*			to_effect() override { return this; }
};

class MSEffectXtnd : public MSSpecialFXXtnd<Effect, MSPluginEffect>
{
public:
					MSEffectXtnd(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap) override;

	// from Effect
	EffectParamDlg*	CreateParamDialog(IRendParams* imp) override;
	DWORD			GBufferChannelsRequired(TimeValue t) override;
	void			Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *cb ) override;

};

/* ------------- ParamDlg class for scripter effect plug-ins --------------- */

class MSAutoEParamDlg : public IAutoEParamDlg 
{
	public:
		Tab<SFXParamDlg*> secondaryDlgs; // secondary ParamDlgs if this is a main
		SFXParamDlg*	delegateDlg;	// my delegate's dialog if any
		MSPlugin*		plugin;			// target plugin
		Rollout*		ro;				// rollout controlling dialog
		SpecialFX*		sfx;			// effect/atmos in the dialog
		IRendParams*	ip;				// render interface

					MSAutoEParamDlg(IRendParams *i, SpecialFX* fx, MSPlugin* plugin, Rollout* ro);
				   ~MSAutoEParamDlg();

		// from ParamDlg
		Class_ID	ClassID() override { return sfx->ClassID(); }
		ReferenceTarget* GetThing() override { return sfx; }
		void		SetThing(ReferenceTarget *fx) override;
		void		DeleteThis() override;
		void		SetTime(TimeValue t) override;

		void		InvalidateUI() override; 
		// secondary dialog list management
		int			NumDlgs() override { return secondaryDlgs.Count(); }
		void		AddDlg(SFXParamDlg* dlg) override;
		SFXParamDlg* GetDlg(int i) override;
		void		SetDlg(int i, SFXParamDlg* dlg) override;
		void		DeleteDlg(SFXParamDlg* dlg) override;

		// access to this dlg's parammap stuff
		IParamMap2* GetMap() override { return nullptr; }
};

// RK: End 

// ----------------------- MSPluginAtmos ----------------------
// scripted Atmospheric

class MSPluginAtmos : public MSPluginSpecialFX<Atmospheric>
{
public:
	MSAutoEParamDlg* mainFXDlg; // main dialog containing all scripted rollout

					MSPluginAtmos() { }
					MSPluginAtmos(MSPluginClass* pc, BOOL loading);
				   ~MSPluginAtmos() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap) override;

	// from Atmospheric
	AtmosParamDlg *CreateParamDialog(IRendParams *rendParam) override;
	BOOL SetDlgThing(AtmosParamDlg* dlg) override;
	void Shade(ShadeContext& sc,const Point3& p0,const Point3& p1,Color& color, Color& trans, BOOL isBG=FALSE) override { };

	Atmospheric* to_atmospheric() override { return this; }
};

class MSAtmosXtnd : public MSSpecialFXXtnd<Atmospheric, MSPluginAtmos>
{
public:
					MSAtmosXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSAtmosXtnd() { DeleteAllRefsFromMe(); }
	RefTargetHandle Clone(RemapDir& remap) override;

	// from Atmospheric
	AtmosParamDlg *CreateParamDialog(IRendParams *rendParam) override;
	BOOL SetDlgThing(AtmosParamDlg* dlg) override;
	void Shade(ShadeContext& sc,const Point3& p0,const Point3& p1,Color& color, Color& trans, BOOL isBG=FALSE) override 
		{ delegate->Shade(sc, p0, p1, color, trans, isBG); }

};

// RK: End 

class MSPluginTrackViewUtility : public MSPlugin, public TrackViewUtility, public ReferenceTarget
{
public:
	Interface*		ip;					// ip for any currently open command panel dialogs
	ITVUtility*		iu;					// access to various trackview methods

	MSPluginTrackViewUtility() { }
	MSPluginTrackViewUtility(MSPluginClass* pc, BOOL loading);
	~MSPluginTrackViewUtility() { DeleteAllRefsFromMe(); }
#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")
	// From TrackViewUtility
	void BeginEditParams(Interface *pIp, ITVUtility *pIu) override;
	void EndEditParams(Interface *pIp, ITVUtility *pIu) override;
	void TrackSelectionChanged() override;
	void NodeSelectionChanged() override;
	void KeySelectionChanged() override;
	void TimeSelectionChanged() override;
	void MajorModeChanged() override;
	void TrackListChanged() override;
	int FilterAnim(Animatable* anim, Animatable* client, int subNum) override;

	static TrackViewUtility* create(MSPluginClass* pc, BOOL loading);


	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD vflags=0, int category=ROLLUP_CAT_STANDARD) override;
	void			DeleteRollupPage(HWND hRollup) override;
	IRollupWindow*	GetRollupWindow() override;
	void			RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) override;

	ReferenceTarget* get_delegate() override { return nullptr; }  // no delegate 

	// From Animatable
	using ReferenceTarget::GetInterface;

	void			DeleteThis() override { 	MSPlugin::DeleteThis(); }
	void GetClassName(MSTR& s, bool localized) const override { s = (localized) ? pc->ClassName() : pc->NonLocalizedClassName(); }
	Class_ID		ClassID() override { return pc->class_id; }
	void			FreeCaches() override { } 		
	int				NumSubs() override { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) override { return pblocks[i]; }
	MSTR SubAnimName(int i, bool localized = true) override
	{
		if (pblocks[i]->GetDesc()->local_name != 0)
		{
			return localized ? pblocks[i]->GetLocalName() : pblocks[i]->GetDesc()->int_name.data();
		}
		
		return _T("");
	}
	int				NumParamBlocks() override { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) override { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) override { return MSPlugin::GetParamBlockByID(id); }
	void*	GetInterface(ULONG id) override { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; return ReferenceTarget::GetInterface(id); }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override 
					{ 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
					}

	// From ReferenceTarget
	int				NumRefs() override;
	RefTargetHandle GetReference(int i) override;
protected:
	void			SetReference(int i, RefTargetHandle rtarg) override; 
public:
	RefTargetHandle Clone(RemapDir& remap) override;

};
#pragma warning(pop)

#ifdef _DEBUG
  extern BOOL dump_load_postload_callback_order;
#endif

// A pair of post load callback2 to process possible redefinition of loaded instances of scripted classes
// LAM - 3/7/03. Added per iload processing, 2 pass processing
// PluginClassDefPLCB1 - migrates parameter blocks, calls update handler if needed
// PluginClassDefPLCB2 - calls load handler, all set handlers, post load handler
class PluginClassDefPLCB1 : public PostLoadCallback 
{
public:
	Tab<ILoad*> registeredILoads;
	bool isRegistered(ILoad* iload)
	{	int count = registeredILoads.Count();
		for (int i = 0; i< count; i++)
			if (registeredILoads[i] == iload)
				return true;
		return false;
	}
	void Register(ILoad* iload)
	{	registeredILoads.Append(1,&iload);
	}
	void Unregister(ILoad* iload)
	{	int count = registeredILoads.Count();
		for (int i = 0; i< count; i++)
			if (registeredILoads[i] == iload)
			{
				registeredILoads.Delete(i,1);
				return;
			}
	}
	
	PluginClassDefPLCB1() { }
	int Priority() override { return 5; }

	void proc(ILoad *iload) override
	{
#ifdef _DEBUG
		if (dump_load_postload_callback_order) 
			DebugPrint(_M("MXS: PostLoadCallback1 run: thePluginClassDefPLCB1\n"));
#endif
		MSPluginClass::post_load(iload,0);
		Unregister(iload);
	}
};

extern PluginClassDefPLCB1 thePluginClassDefPLCB1;

class PluginClassDefPLCB2 : public PostLoadCallback 
{
public:
	
	PluginClassDefPLCB2() { }
	int Priority() override { return 10; }
	
	void proc(ILoad *iload) override
	{
#ifdef _DEBUG
		if (dump_load_postload_callback_order) 
			DebugPrint(_M("MXS: PostLoadCallback2 run: thePluginClassDefPLCB2\n"));
#endif
		MSPluginClass::post_load(iload,1);
	}
};
extern PluginClassDefPLCB2 thePluginClassDefPLCB2;

