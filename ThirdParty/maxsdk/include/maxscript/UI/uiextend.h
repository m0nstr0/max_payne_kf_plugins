/**********************************************************************
 *<
	FILE: UIExtend.h

	DESCRIPTION: MaxScript user interface extensions

	CREATED BY: Ravi Karra, 1998

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "../kernel/value.h"
// forward declarations
class RCMenu;
class HashTable;

#define MF_SUBMENU_START	(MF_SEPARATOR+10)
#define MF_SUBMENU_END		(MF_SEPARATOR+11)

#ifndef DOXYGEN
visible_class_debug_ok (MenuItem)
#endif

class MenuItem : public Value
{	
public:
	Value	*name, *caption, *flt_fn;
	Value	**keyparms;
	RCMenu	*menu;
	HMENU	hmenu;
	UINT	hmenu_index;

	int		keyparm_count;
	UINT	flags;
	
			MenuItem (RCMenu *m, Value* n, Value* c, Value	**keyparms, int keyparm_count, UINT f=0);
			MenuItem () {menu= nullptr; name=caption=nullptr; keyparms=nullptr; flags=keyparm_count=hmenu_index=0; hmenu = nullptr;} 
			~MenuItem ();

	ScripterExport void	setup_params();
	void	collect() override;
	void	gc_trace() override;
	ScripterExport void sprin1(CharStream* s) override;

	Value*	get_property(Value** arg_list, int count) override;
	Value*	set_property(Value** arg_list, int count) override;
};


class MSRightClickMenu : public RightClickMenu
{
public:
	RCMenu	*menu;
	void	Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m) override;
	void	Selected(UINT id) override;
	Value*	call_filt_fn(Value* fn);
};


#ifndef DOXYGEN
visible_class_debug_ok (RCMenu)
#endif

class RCMenu : public Value
{
public:
	Value*		name;						// menu name
	HashTable*  local_scope;				// local name space	
	MenuItem**	items;						// menu item array
	int			item_count;					//   "	  "  count
	Value**		locals;						// local var array	
	Value**		local_inits;				//   "    "    "  init vals	
	int			local_count;				//   "    "  count	
	HashTable*	handlers;					// handler tables	
	short		flags;						// menu flags
	BOOL		init_values;				// whether to init ctrl/local values on (re)open 
	BOOL		end_rcmenu_mode;			// signals end of rcmenu mode
	MSRightClickMenu msmenu;				// right-click menu
	// command mode locals...
	Value*		result;						// rcmenu result
	MSPlugin*	current_plugin;				// current plugin for rcmenu if non-NULL
	Struct*		current_struct;				// current struct for rcmenu if non-NULL

				RCMenu(short iflags);
	void		init(Value *pName, int iLocal_count, Value** inits, HashTable *pLocal_scope, MenuItem** iitems, int iitem_count, HashTable *pHandlers);
			   ~RCMenu();

#	define is_RCMenu(o) ((o)->tag == class_tag(RCMenu))  // LAM - defect 307069
				classof_methods (RCMenu, Value);
	void	collect() override;
	void		gc_trace() override;
	ScripterExport void sprin1(CharStream* s) override;

	Value*		get_event_handler(Value *pName, Value* event);
	BOOL		call_event_handler(Value *pName, Value* event, Value** arg_list, int count);
	

	Value*	get_property(Value** arg_list, int count) override;
	Value*	set_property(Value** arg_list, int count) override;

	// added 3/21/05. Used by debugger to dump locals and externals to standard out
	void		dump_local_vars_and_externals(int indentLevel);
};

// LAM - 9/10/01

// The following classes have been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.
class MSSelectFilterCallback : public SelectFilterCallback
{
public:
	MSSelectFilterCallback()
	{
	}
	virtual ~MSSelectFilterCallback() {}
	MCHAR dname[128];
	const MCHAR* GetName() override {return dname;};
	BOOL IsFiltered(SClass_ID isid, Class_ID icid, INode *node) const override;
	void Unregistered() override;
	Value* selectFilters_fns;
};

class MSDisplayFilterCallback : public DisplayFilterCallback
{

public:
	MSDisplayFilterCallback() 
	{
	}
	virtual ~MSDisplayFilterCallback() {}

	MCHAR dname[128];
	const MCHAR* GetName() override {return dname;};

	BOOL IsHidden(SClass_ID isid, Class_ID icid, INode *node) override;
	void Unregistered() override;
	Value* displayFilters_fns;
};

// End of 3ds max 4.2 Extension

