/*	
 *		mxsCustomAttributes.h - MAXScript scriptable custom attributes MAX objects
 *
 *			Copyright (c) Autodesk, Inc, 2000.  John Wainwright.
 *
 */

#pragma once

// ---------- scripter Custom Attribute classes -------------------

#include <map>

#include "../../custattrib.h"
#include "../../ICustAttribContainer.h"
#include "mxsplugin.h"

#define I_SCRIPTEDCUSTATTRIB	0x000010C1

#define MSCUSTATTRIB_CHUNK	0x0110
#define MSCUSTATTRIB_NAME_CHUNK	0x0010

ScripterExport bool HaveScriptedCustAttribDefs();

// special subclass for holding custom attribute definitions
// these can be applied to any existing object, adding a CustAttrib to it
//   instances of MSCustAttrib (an MSPlugin subclass) refer to CustAttribDefs for metadata
visible_class (MSCustAttribDef)

class MSCustAttribDef;
using MSCustAttribDefMap = std::map<Class_ID, MSCustAttribDef*>;
using MSCustAttribDefMapIter = MSCustAttribDefMap::iterator;
using MSCustAttribDefMapConstIter = MSCustAttribDefMap::const_iterator;
using MSCustAttribDefMapValue = MSCustAttribDefMap::value_type;

class MSCustAttribDef : public MSPluginClass
{
	friend class MSCustAttrib;
	friend ScripterExport bool HaveScriptedCustAttribDefs();
	friend Value* ca_getSceneDefs_cf(Value** arg_list, int count);
	friend Value* ca_deleteDef_cf(Value** arg_list, int count);
private:
	// Map of existing scripted attribute defs, indexed on the ClassID.  This is used in redefining attributes
	ScripterExport static MSCustAttribDefMap ms_attrib_defs;
public:
	// define the versioning behavior of scripted attribute defs on scene file load/merge 
	enum version_handling_behavior 
	{ 
		never_update, // never update existing scripted attribute defs, regardless of version numbers
		always_update, // always update existing scripted attribute defs with load def, regardless of version numbers
		update_when_load_version_greater_than_current_version, // update existing scripted attribute def if load version > existing version
		update_when_load_version_greater_than_or_equal_to_current_version, // update existing scripted attribute def if load version >= existing version
	};

	ScripterExport static version_handling_behavior sceneLoadVersionHandling; // version handling behavior when loading a scene file
	ScripterExport static version_handling_behavior sceneMergeVersionHandling; // version handling behavior when merging a scene file

	Value*		defData;			// persistent definition data, used by the scripter attribute editor
	MSTR			source;			// extracted definition source, stored persistently with the def & automatically recompiled on reload

	ScripterExport	MSCustAttribDef(const Class_ID& attrib_id);
	ScripterExport	~MSCustAttribDef();

	// definition and redefinition
	static MSCustAttribDef* intern(Class_ID& attrib_id);
	ScripterExport void		init(int local_count, Value** inits, HashTable* local_scope, HashTable* handlers, Array* pblock_defs, Array* iremap_param_names, Array* rollouts, CharStream* source);

	// returns existing MSCustAttribDef* with attrib_id if it exists
	static ScripterExport MSCustAttribDef* find(Class_ID& attrib_id);

	// MAXScript required
//	BOOL			is_kind_of(ValueMetaClass* c) { return (c == class_tag(MSCustAttribDef)) ? 1 : Value::is_kind_of(c); } // LAM: 2/23/01
	BOOL			is_kind_of(ValueMetaClass* c) override { return (c == class_tag(MSCustAttribDef)) ? 1 : MSPluginClass::is_kind_of(c); }
#	define			is_attribute_def(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MSCustAttribDef))
	void	collect() override;
	void			gc_trace() override;
	void			sprin1(CharStream* s) override;

	bool			is_custAttribDef() override { return true; }
	ScripterExport	MSCustAttribDef* unique_clone();

	// from Value 
	Value*			apply(Value** arg_list, int count, CallContext* cc=nullptr) override { return Value::apply(arg_list, count, cc); }  // CustAttribDef's are not applyable

	// scene I/O
	static IOResult	save_custattrib_defs(ISave* isave);
	static IOResult	load_custattrib_defs(ILoad* iload);

	// ClassDesc delegates
	RefTargetHandle	Create(BOOL loading) override;

	Value*			get_property(Value** arg_list, int count) override;
	Value*			set_property(Value** arg_list, int count) override;

	def_property ( name );
};


#pragma warning(push)
#pragma warning(disable:4239)

// MSCustAttrib - instances contain individual custom attribute blocks 
//   that are added to customized objects.
class MSCustAttrib : public MSPlugin, public CustAttrib, public ISubMap
{
public:
	IObjParam*				cip;		// ip for any currently open command panel dialogs

	static MSAutoMParamDlg* mainMDlg; // main dialog containing all scripted rollout
	IMtlParams*				mip;		// ip for any open mtlEditor panel dlgs
	MSTR					name;		// name of the custom attribute

	ScripterExport	MSCustAttrib() : cip(nullptr), mip(nullptr) { }
	ScripterExport	MSCustAttrib(MSCustAttribDef* pc, BOOL loading);
	ScripterExport	~MSCustAttrib() { DeleteAllRefsFromMe(); }
#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")
	ScripterExport	void	sprin1(CharStream* s) override;

	// From MSPlugin
	ScripterExport	HWND			AddRollupPage(HINSTANCE hInst, const MCHAR *dlgTemplate, DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD flags=0, int category=ROLLUP_CAT_CUSTATTRIB - 1) override;
	ScripterExport	void			DeleteRollupPage(HWND hRollup) override;
	ScripterExport	IRollupWindow*  GetRollupWindow() override;
	ScripterExport	void			RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) override;

	ReferenceTarget* get_delegate() override { return nullptr; }  // no delegates in MSCustAttribs 

	// from CustAttrib
	const MCHAR*			GetName(bool /*localized*/) override { return name; }
	ScripterExport	ParamDlg*		CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) override;

	void			SetName(const MCHAR* newName)	{ name = newName ; }

	// from InterfaceServer
	ScripterExport	BaseInterface* GetInterface(Interface_ID id) override;

	// From Animatable
    
    using CustAttrib::GetInterface;

	void				GetClassName(MSTR& s, bool localized = true) const override { s = localized ? pc->class_name->to_string() : pc->nonLocalized_class_name->to_string(); }
	Class_ID			ClassID() override { return pc->class_id; }
	SClass_ID		SuperClassID() override { return pc->sclass_id; }
	void				FreeCaches() override { }
	int				NumSubs() override { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) override { return pblocks[i]; }
	MSTR				SubAnimName(int i, bool localized = true) override
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
	BOOL				CanCopyAnim() override { return FALSE; }

	ScripterExport	void*			GetInterface(ULONG id) override;
	ScripterExport	void			DeleteThis() override;
	ScripterExport	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev) override;
	ScripterExport	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next) override;

	// From ReferenceMaker
	RefResult		NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override 
					{ 
						if (!(pc->mpc_flags & MPC_REDEFINITION))
							return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message, propagate); 
						return REF_SUCCEED;
					}

	// From ReferenceTarget
	int				NumRefs() override { return pblocks.Count(); }
	RefTargetHandle GetReference(int i) override { return pblocks[i]; }
protected:
	void	SetReference(int i, RefTargetHandle rtarg) override 
					{ 
						if (i >= pblocks.Count())
							pblocks.SetCount(i+1); 
						pblocks[i] = (IParamBlock2*)rtarg; 
					}
public:
	ScripterExport	void			RefAdded(RefMakerHandle rm) override;
	ScripterExport	void			RefDeleted(RefMakerHandle rm) override;
	ScripterExport	void			RefAddedUndoRedo(RefMakerHandle rm) override;
	ScripterExport	void			RefDeletedUndoRedo(RefMakerHandle rm) override;
	ScripterExport	RefTargetHandle Clone(RemapDir& remap) override;
	
	ScripterExport	IOResult		Save(ISave *isave) override;
	ScripterExport	IOResult		Load(ILoad *iload) override;

	// from ISubMap
	ScripterExport	int				NumSubTexmaps() override;
	ScripterExport	Texmap*			GetSubTexmap(int i) override;
	ScripterExport	void			SetSubTexmap(int i, Texmap *m) override;
	ScripterExport	MSTR			GetSubTexmapSlotName(int i, bool localized = true) override;
	int				MapSlotType(int i) override { UNUSED_PARAM(i); return MAPSLOT_TEXTURE; }
	MSTR			GetSubTexmapTVName(int i, bool localized = true) { return GetSubTexmapSlotName(i, localized); }
	ReferenceTarget *GetRefTarget() override { return this; }
};

#pragma warning(pop)

// ref enumerator to find an owner for this Custom Attribute
class FindCustAttribOwnerDEP : public DependentEnumProc 
{
public:		
	ReferenceMaker* rm;
	CustAttrib* targ_ca;
	FindCustAttribOwnerDEP(CustAttrib* ca) { rm = nullptr; targ_ca = ca; }
	int proc(ReferenceMaker* rmaker) override
	{
		if (rmaker == targ_ca)
			return DEP_ENUM_CONTINUE;

		// russom - 08/27/04 - 579271
		// Make sure we only eval real dependencies.
		// Note: We might need to also add a SKIP for restore class ids, but that
		// is not part of this defect fix.
		if( !rmaker->IsRealDependency(targ_ca) ) 
			return DEP_ENUM_SKIP;

		if (rmaker->ClassID() == CUSTATTRIB_CONTAINER_CLASS_ID )
		{
			ICustAttribContainer* cac = (ICustAttribContainer*)rmaker;
			Animatable* owner = cac->GetOwner();
			if (owner)
			{
				SClass_ID sid = owner->SuperClassID();
				if (sid != MAKEREF_REST_CLASS_ID  && sid != MAXSCRIPT_WRAPPER_CLASS_ID && sid != DELREF_REST_CLASS_ID)
				{
					for (int j = 0; j < cac->GetNumCustAttribs(); j++)
					{
						CustAttrib* ca = cac->GetCustAttrib(j);
						if (ca == targ_ca)
						{
							rm = (ReferenceMaker*)owner;
							return DEP_ENUM_HALT;
						}
					}
				}
			}
		}
		return DEP_ENUM_SKIP; // only need to look at immediate dependents
	}

};

