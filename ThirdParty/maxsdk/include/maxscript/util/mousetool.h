/*	
 *		MouseTool.h - scriptable mouse CommandModes for MAX
 *
 *			Copyright (c) Autodesk, Inc, 1998.  John Wainwright.
 *
 */

#pragma once

#include "../kernel/value.h"
#include "../../mouseman.h"
#include "../../geom/point3.h"
#include "../../cmdmode.h"
#include "../../maxapi.h"

// forward declarations
class MouseTool;
class MSPlugin;
class ViewExp;
class Matrix3;
class HashTable;

// tool context local indexes - MUST match order in Parser::tool_body()
enum {
	cl_viewPoint, cl_worldPoint, cl_worldDist, cl_worldAngle, cl_gridPoint, cl_gridDist, cl_gridAngle, cl_nodeTM, cl_shift, cl_ctrl, cl_alt, cl_lbutton, cl_mbutton, cl_rbutton, cl_viewPointUnscaled, };

/* --------- MouseTool command mode & callback classes ------------- */

class MouseToolCallBack : public MouseCallBack
{
	public:
		MouseTool*	tool;
		IPoint2		last_mp;
		Point3		last_wp;
		Point3		last_cpp;

		MouseToolCallBack() {}
		int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m) override;
		int mouse_proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat, BOOL createMouseCallback = FALSE);
		void set_context_locals(ViewExp* vpx, int snap, int point, int flag, IPoint2 mp, Point3 cpp, Matrix3& mat);
		void reset_context_locals();
};

#define MOUSE_TOOL_COMMAND	7364
#define CID_MOUSE_TOOL		CID_USER + 45237

class MouseToolCommandMode : public CommandMode, public CommandModeChangedCallback 
{
public:
	MouseToolCallBack	proc;
	BOOL				active;
	MSTR				prompt;
	int					num_points;
	int					cmd_class;

	// -------------------------------------------------------------------------
	// We need to create an empty constructor here, or VS2019 does not setup the
	// vtable of the second implemented base class correctly... this is scary!
	// -------------------------------------------------------------------------
	MouseToolCommandMode() {}

	int Class() override { return cmd_class; }
	int ID() override { return CID_MOUSE_TOOL; }
	MouseCallBack *MouseProc(int *points) override { *points = num_points; return &proc; }
	ChangeForegroundCallback *ChangeFGProc() override { return nullptr; }
	BOOL ChangeFG(CommandMode* oldMode) override { UNUSED_PARAM(oldMode); return FALSE; }
	void EnterMode() override; 
	void ExitMode() override;
	void ModeChanged(CommandMode *oldM, CommandMode *newM) override;
};

/* ------------- MouseTool MAXScript value class -------------- */

visible_class (MouseTool)

class MouseTool : public Value
{
public:
	Value*		name;						// tool name
	HashTable*  local_scope;				// local name space	
	Value**		locals;						// local var array	
	Value**		local_inits;				//   "    "    "  init vals	
	int			local_count;				//   "    "  count	
	HashTable*	handlers;					// handler tables	
	short		flags;						// tool flags
	int			cmd_class;					// command mode class
	int			num_points;					// number of points
	Value*		prompt;						// staus line prompt if non-null
	BOOL		init_values;				// whether to init ctrl/local values on (re)open 
	BOOL		end_tool_mode;				// signals end of tool cmd mode
	MouseToolCommandMode cmdmode;			// my command mode
	// command mode locals...
	Value*		result;						// tool result
	Value*		snap_mode;					// #2D or #3D or #none
	MSPlugin*	plugin;						// current plugin under manip if non-NULL

				MouseTool(short iflags);
	void		init(Value* name, int local_count, Value** inits, HashTable* local_scope, HashTable* handlers);
			   ~MouseTool();

#	define		is_mousetool(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MouseTool))
				classof_methods (MouseTool, Value);
	void	collect() override;
	void		gc_trace() override;
	ScripterExport void sprin1(CharStream* s) override;

	BOOL		call_event_handler(Value* handler, Value** arg_list, int count, BOOL enable_redraw = TRUE);
	void		init_locals();

	Value*	get_property(Value** arg_list, int count) override;
	Value*	set_property(Value** arg_list, int count) override;

	// added 3/21/05. Used by debugger to dump locals and externals to standard out
	void		dump_local_vars_and_externals(int indentLevel);
};

