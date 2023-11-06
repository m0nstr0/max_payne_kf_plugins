/*		Numbers.h - the Thunk family of classes - variable accessors for MAXScript
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#pragma once

#include "../kernel/value.h"
#include "../foundation/streams.h"
#include "../ui/uiextend.h"
#include "../../Noncopyable.h"

// forward declarations
class MouseTool;
class Rollout;

#pragma warning(push)
#pragma warning(disable:4100)

/* ----------------------- Thunk  ---------------------- */
visible_class (Thunk)

class Thunk : public Value
{
public:
	Value*	name;
	BOOL	clear_container; // outer-level prop in a prop sequence, clear current_container when done

			classof_methods (Thunk, Value);
#	define	is_thunk(v) ((v)->_is_thunk())
#	define  is_indirect_thunk(v) ((v)->_is_indirect_thunk())
	BOOL	_is_thunk() override { DbgAssert(!is_sourcepositionwrapper(this)); return TRUE; }
	ScripterExport void	gc_trace() override;
	Thunk() : clear_container(FALSE), name(nullptr) { }

	Thunk*	to_thunk() override {return this; }
	virtual Thunk* make_free_thunk(int level) { return nullptr; }
	void    assign(Value* val) { assign_vf(&val, 1); }

	ScripterExport Value*	get_property(Value** arg_list, int count) override;
	ScripterExport Value*	set_property(Value** arg_list, int count) override;
};

/* -------------------- GlobalThunk  ------------------- */

class GlobalThunk : public Thunk
{
public:
	Value*	cell;

	ScripterExport		GlobalThunk(Value* init_name) { init(init_name); }
	ScripterExport		GlobalThunk(Value* init_name, Value* init_val);
	ScripterExport void	init(Value* init_name);
#	define	is_globalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_GLOBAL_THUNK_TAG)

	ScripterExport Value* eval() override;

	ScripterExport void	gc_trace() override;
	void	collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;

};

class ConstGlobalThunk : public GlobalThunk
{
public:
			ConstGlobalThunk(Value* iname) : GlobalThunk(iname) { tag = INTERNAL_CONST_GLOBAL_THUNK_TAG; }
			ConstGlobalThunk(Value* iname, Value* ival) : GlobalThunk(iname, ival) { tag = INTERNAL_CONST_GLOBAL_THUNK_TAG; }
#	define	is_constglobalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_CONST_GLOBAL_THUNK_TAG)

	Value*	eval() override { return cell->is_const() ? cell->copy_vf(nullptr, 0) : cell; }
	ScripterExport void	collect() override;

	Value*	assign_vf(Value**arg_list, int count) override { throw AssignToConstError (this); return &undefined; }
};

/* -------------------- SystemGlobalThunk  ------------------- */

/* system globals are abstractions over some system state accessing functions, such as 
 * animation_range, current_renderer,e tc. */

class SystemGlobalThunk : public Thunk
{
	Value* (*get_fn)();
	Value* (*set_fn)(Value*);
public:
	ScripterExport		SystemGlobalThunk(Value* init_name, Value* (*iget)(), Value* (*iset)(Value*));
// LAM 4/1/00 - added following to prevent AF in name clash debugging output in HashTable::put_new()
#	define	is_systemglobalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_SYS_GLOBAL_THUNK_TAG)

	ScripterExport Value* eval() override;

	void	collect() override;
	ScripterExport void	sprin1(CharStream* s) override { s->printf(_M("SystemGlobal:%s"), name->to_string()); }

	Value*	assign_vf(Value**arg_list, int count) override;
};

/* -------------------- LocalThunk  ------------------- */

class LocalThunk : public Thunk
{
public:
	int		frame_level;	// frame nest level at declaration
	int		index;			// local var's index in local frame

			LocalThunk(Value* init_name, int init_index, int iframe_lvl);
#	define	is_localthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_LOCAL_THUNK_TAG)

	Thunk*	make_free_thunk(int level) override;

	Value*	eval() override;
	void	collect() override;
	void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;
};

class IndirectLocalThunk : public LocalThunk
{
public:
	IndirectLocalThunk(Value* init_name, int init_index, int iframe_lvl) :
				LocalThunk(init_name, init_index, iframe_lvl) { }

	BOOL	_is_indirect_thunk() override { DbgAssert(!is_sourcepositionwrapper(this)); return TRUE; }
	Thunk*	make_free_thunk(int level) override;

	Value*	eval() override;
	void	collect() override;
	void	sprin1(CharStream* s) override { s->printf(_M("&")); LocalThunk::sprin1(s); }
	Value*	assign_vf(Value**arg_list, int count) override;
};

// ContextThunk created from an IndirectLocal/FreeThunk on entry to a MAXScript function apply
//   to contain the callers frame context for evals and assigns
class ContextThunk : public Thunk
{
public:
	Thunk*  thunk;			// the wrapped thunk
	Value**	frame;			// callers frame
#pragma push_macro("new")
#undef new
	ENABLE_STACK_ALLOCATE(ContextLocalThunk);
#pragma pop_macro("new")
	ContextThunk(Thunk*  thunk, Value** frame) :
				thunk(thunk), frame(frame) { tag = INTERNAL_CONTEXT_THUNK_TAG;  }
#	define	is_contextthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_CONTEXT_THUNK_TAG)

	void	collect() override;
	void	sprin1(CharStream* s) override { s->printf(_M("&")); thunk->sprin1(s); }

	Value*	eval() override;
	Value*	assign_vf(Value**arg_list, int count) override;
};

/* -------------------- FreeThunk  ------------------- */

class FreeThunk : public Thunk
{
public:
	int		level;		// how many levels to reach back
	int		index;		// index there
			FreeThunk(Value* init_name, int level, int index);
#	define	is_freethunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_FREE_THUNK_TAG)

	Thunk*	make_free_thunk(int iLevel) override;

	void	collect() override;
	void	sprin1(CharStream* s) override;

	Value*	eval() override;
	Value*	assign_vf(Value**arg_list, int count) override;
};

class IndirectFreeThunk : public FreeThunk
{
public:
			IndirectFreeThunk(Value* init_name, int level, int index) :
				FreeThunk(init_name, level, index) { }

	BOOL	_is_indirect_thunk() override { DbgAssert(!is_sourcepositionwrapper(this)); return TRUE; }
	Thunk*	make_free_thunk(int iLevel) override;

	void	collect() override;
	void	sprin1(CharStream* s) override { s->printf(_M("&")); FreeThunk::sprin1(s); }

	Value* eval() override;
	Value*	assign_vf(Value**arg_list, int count) override;
};

/* -------------------- PropertyThunk  ------------------- */

class PropertyThunk : public Thunk
{
public:
	Value*		target_code;	// code to eval to get target
	Value*		property_name;	// property name
	getter_vf	getter;			// getter virtual fn for built-in properties
	setter_vf	setter;			// setter    "     "       "        "

				PropertyThunk(Value* target, Value* prop_name);
				PropertyThunk(Value* target, Value* prop_name, getter_vf get_fn, setter_vf set_fn);

	void		gc_trace() override;
	void		collect() override;
	ScripterExport void sprin1(CharStream* s) override;
#	define		is_propertythunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_PROP_THUNK_TAG || (v)->tag == INTERNAL_PROP_EX_THUNK_TAG)

	ScripterExport Value* eval(Value**arg_list, int count);
	ScripterExport Value* eval() override { return eval(nullptr,0); }
	Value*		assign_vf(Value**arg_list, int count) override;
	Value*		op_assign_vf(Value**arg_list, int count) override;
};

class PropertyThunkEx : public PropertyThunk
{
public:
	MSTR		property_name_as_parsed;		// property name (raw string as parsed)
	MSTR		nested_property_name_as_parsed;	// getter/setter property name (raw string as parsed)
	Value*		nested_prop_name;				// getter/setter nested property name

	PropertyThunkEx(Value* target, Value* prop_name, const MCHAR* prop_name_as_parsed);
	PropertyThunkEx(Value* target, Value* prop_name, const MCHAR* prop_name_as_parsed, getter_vf get_fn, setter_vf set_fn, Value* nested_prop_name, const MCHAR* nested_prop_name_as_parsed);

	void		gc_trace() override;
#	define		is_propertythunkex(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_PROP_EX_THUNK_TAG)

	ScripterExport Value* eval(Value**arg_list, int count);
	ScripterExport Value* eval() override { return eval(nullptr, 0); }
	Value* assign_vf(Value**arg_list, int count) override;
	Value* op_assign_vf(Value**arg_list, int count) override;
};

// a PropThunk subclass that is used when a Prop access occurs in a function call
// this is basically a hack to support OLE client method calls, since OLE IDISPATCH
// cannot distinguish methods from props
class FnCallPropertyThunk : public PropertyThunk
{
public:
	FnCallPropertyThunk(Value* target, Value* prop_name, getter_vf get_fn, setter_vf set_fn)
					: PropertyThunk (target, prop_name, get_fn, set_fn) {}
	void		collect() override;
	ScripterExport Value* eval() override;
};

class FnCallPropertyThunkEx : public PropertyThunkEx
{
public:
	FnCallPropertyThunkEx(Value* target, Value* prop_name, getter_vf get_fn, setter_vf set_fn, const MCHAR* prop_name_as_parsed, Value* nested_prop_name, const MCHAR* nested_prop_name_as_parsed)
		: PropertyThunkEx(target, prop_name, prop_name_as_parsed, get_fn, set_fn, nested_prop_name, nested_prop_name_as_parsed) {}
	ScripterExport Value* eval() override;
};

/* -------------------- IndexThunk  ------------------- */

class IndexThunk : public Thunk
{
	Value*		target_code;	// code to eval to get target
	Value*		index_code;		// code to eval to get index

public:
				IndexThunk(Value* index);

#	define		is_indexthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_INDEX_THUNK_TAG)
	void		gc_trace() override;
	void	collect() override;
	ScripterExport void sprin1(CharStream* s) override;

	Value*		set_target(Value* targ) { target_code = targ; return this; }
	ScripterExport Value* eval() override;
	Value*		assign_vf(Value**arg_list, int count) override;
};

/* -------------------- RolloutControlThunk  ------------------- */

class RolloutControlThunk : public Thunk
{
public:
	int		index;
	Rollout* rollout;

			RolloutControlThunk(Value* name, int control_index, Rollout* rollout);
	BOOL	_is_rolloutthunk() override { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
#	define	is_rolloutthunk(v) ((v)->_is_rolloutthunk())

	Value*	eval() override;

	ScripterExport void gc_trace() override;
	void	collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;
};

/* -------------------- RolloutLocalThunk  ------------------- */

class RolloutLocalThunk : public Thunk
{
public:
	int		index;
	Rollout* rollout;

			RolloutLocalThunk(Value* name, int control_index, Rollout* rollout);
	BOOL	_is_rolloutthunk() override { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
#	define	is_rolloutlocalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_RO_LOCAL_THUNK_TAG)

	ScripterExport Value* eval() override;

	void	gc_trace() override;
	void collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;
};

class ConstRolloutLocalThunk : public RolloutLocalThunk
{
public:
	ScripterExport ConstRolloutLocalThunk(Value* name, int control_index, Rollout* rollout);
	void	collect() override;

	Value*	assign_vf(Value**arg_list, int count) override { throw AssignToConstError (this); return &undefined; }
};

/* -------------------- ToolLocalThunk  ------------------- */

class ToolLocalThunk : public Thunk
{
public:
	int			index;
	MouseTool*	tool;

			ToolLocalThunk(Value* name, int iindex, MouseTool* tool);

	ScripterExport Value* eval() override;

#	define	is_toollocalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_TOOL_LOCAL_THUNK_TAG)
	void	gc_trace() override;
	void	collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;
};

/* -------------------- CodeBlockLocalThunk  ------------------- */

class CodeBlock;

class CodeBlockLocalThunk : public Thunk
{
public:
	int			index;
	CodeBlock*	block;

			CodeBlockLocalThunk(Value* name, int iindex, CodeBlock* block);

	ScripterExport Value* eval() override;

#	define	is_codeblocklocalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_CODEBLOCK_LOCAL_TAG)
	void	gc_trace() override;
	void	collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;
};

/* -------------------- RCMenuItemThunk  ------------------- */

class RCMenuItemThunk : public Thunk
{
public:
	int			index;
	RCMenu*		rcmenu;

			RCMenuItemThunk(Value* name, int item_index, RCMenu* menu);
	BOOL	_is_rcmenuthunk() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
#	define	is_rcmenuitemthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_RCMENU_ITEM_THUNK_TAG)

	Value*	eval() override { return rcmenu->items[index]; }

	ScripterExport void gc_trace() override;
	void	collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;
};

/* -------------------- RCMenuLocalThunk  ------------------- */

class RCMenuLocalThunk : public Thunk
{
public:
	int			index;
	RCMenu*		rcmenu;

			RCMenuLocalThunk(Value* name, int iindex, RCMenu* menu);
	BOOL	_is_rcmenuthunk() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
#	define	is_rcmenulocalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_RCMENU_LOCAL_THUNK_TAG)
#	define	is_rcmenuthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_RCMENU_LOCAL_THUNK_TAG || (v)->tag == INTERNAL_RCMENU_ITEM_THUNK_TAG)

	ScripterExport Value* eval() override;

	void	gc_trace() override;
	void	collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;
};

/* -------------------- PluginLocalThunk  ------------------- */

class PluginLocalThunk : public Thunk
{
public:
	int		index;    // access via current_plugin thread local
	BOOL	re_init;  // indicate whether this local needs re-initialization on a redefinition (say for local rollouts, fns, etc.)

			PluginLocalThunk(Value* name, int iindex, BOOL re_init = FALSE);
#	define	is_pluginlocalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_PLUGIN_LOCAL_THUNK_TAG)

	ScripterExport Value* eval() override;

	void	collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;
};

class ConstPluginLocalThunk : public PluginLocalThunk
{
public:
	ConstPluginLocalThunk(Value* name, int iindex, BOOL re_init = FALSE) : PluginLocalThunk(name, iindex, re_init) { }
	void	collect() override;
	Value*	assign_vf(Value**arg_list, int count) override { throw AssignToConstError (this); return &undefined; }
};

/* -------------------- PluginParamThunk  ------------------- */

class PluginParamThunk : public Thunk
{
public:
			PluginParamThunk(Value* name);
#	define	is_pluginparamthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_PLUGIN_PARAM_THUNK_TAG)

	ScripterExport Value* eval() override;

	void collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;

	Value*	get_container_property(Value* prop, Value* cur_prop) override;
	Value*	set_container_property(Value* prop, Value* val, Value* cur_prop) override;
};

/*! Class that uses the RAII idiom to push/pop the plugin stored in thread local current_plugin. 
	This ensures that the current_plugin thread local is properly popped in the event of
	an exception. 
*/
class [[nodiscard]] ScopedPushPlugin : public MaxSDK::Util::Noncopyable
{
public:
	ScripterExport ScopedPushPlugin(MSPlugin* plugin, MAXScript_TLS* tls = nullptr);
	ScripterExport ~ScopedPushPlugin();
	ScripterExport void PopPlugin();
private:
	bool m_PluginPopped;
	MSPlugin* m_save_cp;
	MAXScript_TLS* m_tls;
};

/*! Class that uses the RAII idiom to push/pop the struct stored in thread local current_struct. 
	This ensures that the current_struct thread local is properly popped in the event of
	an exception. 
*/
class [[nodiscard]] ScopedPushStruct : public MaxSDK::Util::Noncopyable
{
public:
	ScripterExport ScopedPushStruct(Struct* _struct, MAXScript_TLS* tls = nullptr);
	ScripterExport ~ScopedPushStruct();
	ScripterExport void PopStruct();
private:
	bool m_StructPopped;
	Struct* m_save_struct;
	MAXScript_TLS* m_tls;
};

/* -------------------- StructMemberThunk  ------------------- */

class StructDef;
class StructMemberThunk : public Thunk
{
private:
	StructDef*	m_owning_def;  // the owning StructDef, backpatched by parser
	int			m_index;    // access via current_struct thread local
public:

	ScripterExport StructMemberThunk(Value* name, int iindex);
#	define	is_structmemberthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_STRUCT_MEM_THUNK_TAG)

	ScripterExport Value* eval() override;

	void	gc_trace() override;
	void	collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;

	int GetIndex();
	void SetOwningDef(StructDef* owning_def);
	StructDef* GetOwningDef();
};

/* -------------------- ThunkReference  ------------------- */
// indirect thunk (eg, &foo)

class ThunkReference : public Thunk
{
public:
	Thunk*	target;			// the target thunk

	ScripterExport ThunkReference(Thunk* target);
#	define	is_thunkref(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_THUNK_REF_TAG)

	void	gc_trace() override;
	void	collect() override;
	void	sprin1(CharStream* s) override;

	Value*  eval() override;
};

class DerefThunk : public Thunk  // generated by a '*' prefix operator
{
public:
	Value*	target;    // the target to deref

			ScripterExport DerefThunk(Value* target);
#	define	is_derefthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_THUNK_DEREF_TAG)
	void	gc_trace() override;
	void	collect() override;
	void	sprin1(CharStream* s) override { s->printf(_M("*")); target->sprin1(s); }

	Value*  eval() override;
	Value*	assign_vf(Value**arg_list, int count) override;
};

/* -------------------- OwnerThunk  ------------------- */
// used by wrap owner of Functions for use by debugger

class OwnerThunk : public Thunk
{
public:
	Value*	cell;

	ScripterExport		OwnerThunk(Value* owner);
#	define	is_ownerthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_OWNER_THUNK_TAG)

	ScripterExport Value* eval() override;

	ScripterExport void	gc_trace() override;
	void	collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override;

};

/* -------------------- ReadOnlyThunk  ------------------- */
// used in for loops for holding index and filtered index values
// when building codetree, can assign value to ReadOnlyThunk via assignReadOnlyThunk_pf. For example:
//	vl.loop_index_thunk = new ReadOnlyThunk(vl.loop_index)
//	vl.code = new CodeTree(source, &assignReadOnlyThunk_pf, vl.loop_index_thunk, vl.op, END);
// at source code level, can assign value to ReadOnlyThunk via ReadOnlyThunk::assign.

class ReadOnlyThunk : public Thunk
{
public:
	Value*	value;

	ScripterExport		ReadOnlyThunk(Value* init_name);
#	define	is_readonlythunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_READONLY_THUNK_TAG)

	ScripterExport Value* eval() override;

	ScripterExport void	gc_trace() override;
	void	collect() override;
	ScripterExport void	sprin1(CharStream* s) override;

	Value*	assign_vf(Value**arg_list, int count) override { throw AssignToConstError(this); return &undefined; }

	Value*  assign(Value* val);

};

#pragma warning(pop) // for C4100

class Primitive;
ScripterExport extern Primitive assignReadOnlyThunk_pf;
