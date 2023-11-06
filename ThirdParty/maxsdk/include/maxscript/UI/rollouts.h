/*	
 *		Rollouts.h - Rollout panel classes & functions for MAXScript
 *
 *			Copyright (c) John Wainwright 1996
 *
 */

#pragma once

#include "../kernel/value.h"
#include "../foundation/streams.h"
#include "../../custcont.h"
#include "../../control.h"
#include "../../maxapi.h"

// forward declarations
class Array;
class IMtlParams;
class TexDADMgr;
class MtlDADMgr;
struct ParamDef;

struct layout_data		// rollout control layout data
{
	int	left;
	int top;
	int	width;
	int height;
	int columns;
};

/* some layout constants (in pixels) ... */

#define TOP_MARGIN				2
#define SIDE_MARGIN				4
#define RADIO_DOT_WIDTH			23
#define CHECK_BOX_WIDTH			24
#define LEFT_ALIGN				13
#define RIGHT_ALIGN				13
#define GROUP_BOX_Y_MARGIN		6
#define GROUP_BOX_X_MARGIN		4
#define SPACING_BEFORE			5

enum class MXSScrollBarPolicy {
	kScrollBarAsNeeded,
	kScrollBarAlwaysOff,
	kScrollBarAlwaysOn
};

/* ---------------------- Rollout class ----------------------- */

/* contains the defintion of  rollout panel.  This includes:
 *     - an 'instance variable' array, these variables can be accessed as locals in rollout handlers
 *     - a control array, containing rolout control instances
 *     - a hashtable of event handler functions
 *   there are associated Thunks for the locals & controls so you can ref them as variables in
 *   handlers
 */

class RolloutControl;
class RolloutFloater;
class MSPlugin;
class RolloutChangeCallback;
class PB2Param;
struct NotifyInfo;

//! \cond DOXYGEN_IGNORE
visible_class_debug_ok (Rollout);
//! \endcond

class Rollout : public Value
{
public:
	Value*		name;						// rollout's variable name
	Value*		title;						// title factor			
	HashTable*  local_scope;				// local name space			
	Value**		locals;						// local var array			
	Value**		local_inits;				//   "    "    "  init vals	
	int			local_count;				//   "    "  count			
	RolloutControl** controls;				// control array			
	int			control_count;				//    "    "  count			
	HashTable*	handlers;					// handler tables			
	short		flags;						// rollout flags			
	short		order;						// rollout open order no.	
	Interface*	ip;							// Interface pointer		
	HWND		page;						// my dialog HWND when visible 
	HDC			rollout_dc;					// my dialog dev. context	
	HFONT		font;						// dialog's default font	
	int			text_height;				// metrics....				
	int			default_control_leading;
	int			rollout_width;
	int			rollout_height;
	int			rollout_category;
	int			current_width;				// used for across: processing...
	int			current_left;
	int			hOffset;			// used for offset in dialog. UI scaled values
	int			vOffset;

	int			max_y, last_y;
	int			across_count;
	WORD		close_button_ID;			// id of gen'd close button 
	BOOL		selected;					// selected to be open  
	BOOL		disabled;					// error in handler -> ro disabled 		
	CharStream* source;						// source text if available 
	BOOL		init_values;				// whether to init ctrl/local values on (re)open
	MSPlugin*	current_plugin;				// plugin I'm open on if non-NULL. If dealing with this, should also be dealing with current_struct.
	Struct*		current_struct;				// struct I'm open on if non-NULL. If dealing with this, should also be dealing with current_plugin.
	RolloutChangeCallback* tcb;				// timechange callback if rollout has controller-linked spinners
	IMtlParams* imp;						// MtlEditor interface if open in Mtl Editor and other stuff...
	TexDADMgr*	texDadMgr;
	MtlDADMgr*	mtlDadMgr;
	HWND		hwMtlEdit;
	RolloutFloater* rof;					// owning rolloutfloater window if present there
	WORD        next_id;					// dialog item ID allocators
	Tab<RolloutControl*> id_map;			// dialog item ID map for taking item ID's to associated RolloutControl

				Rollout(short iflags);
	void		init(Value *pName, Value *pTitle, int iLocal_count, Value** inits, HashTable *pLocal_scope, RolloutControl **ppControls, int iControl_count, HashTable *pHandlers, CharStream *pSource);
			   ~Rollout();

	static void	ColorChangeNotifyProc(void* param, NotifyInfo* pInfo);
#	define		is_rollout(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Rollout))
				classof_methods (Rollout, Value);
	void		collect() override;
	void		gc_trace() override;
	ScripterExport void		sprin1(CharStream* s) override;

	ScripterExport BOOL add_page(Interface *pIp, HINSTANCE hInst, int ro_flags = 0, RolloutFloater* pRof = nullptr);
	ScripterExport void delete_page(Interface *pIp, RolloutFloater* pRof = nullptr);
	void		open(Interface *pIp, BOOL rolled_up = FALSE);
	ScripterExport void close(Interface *pIp, RolloutFloater* pRof = nullptr);
	ScripterExport BOOL ok_to_close(RolloutFloater* pRof = nullptr);
	ScripterExport void run_event_handler(Value* event, Value** arg_list, int count);
	ScripterExport bool has_event_handler(Value* event); // return true if event handler defined for rollout

	Value*		call_event_handler(Value* event, Value** arg_list, int count);
	void		add_close_button(HINSTANCE hInst, int& current_y);
	void		edit_script();
	void		TimeChanged(TimeValue t);

	// this method is used to potentially update the layout of controls in the rollout, typically in response 
	// to a change in width of the container the rollout is in. If forceUpdate is true, the rollout layout is 
	// updated even if its RO_AUTO_LAYOUT_ON_RESIZE_ENABLED flag bit is not set.
	// The update of the layout occurs by calling adjust_control on each rollout control in the rollout.
	ScripterExport void UpdateLayout(bool forceUpdate = false);

	void		RollupMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

	// various open/close for scripted plug-in rollouts
	// command panel
	void		BeginEditParams(IObjParam *objParam, MSPlugin *pPlugin, ULONG vflags, Animatable *pPrev);
	void		EndEditParams(IObjParam *objParam, MSPlugin *pPlugin, ULONG vflags, Animatable *pNext);
	// mtl editor
	void		CreateParamDlg(HWND hMtlEdit, IMtlParams *pMtlParam, MSPlugin *pPlugin, TexDADMgr *pTexDadMgr, MtlDADMgr *pMtlDadMgr);
	void		SetThing(MSPlugin *pPlugin);
	void		ReloadDialog();
	void		SetTime(TimeValue t);
	void		DeleteThis();
	// update/reload
	void		InvalidateUI();
	void		InvalidateUI(ParamID id, int tabIndex=-1); // nominated param

	Value*	get_property(Value** arg_list, int count) override;
	Value*	set_property(Value** arg_list, int count) override;
	Value*	set_nested_controller(Value** arg_list, int count) override;

	// added 3/21/05. Used by debugger to dump locals and externals to standard out
	void		dump_local_vars_and_externals(int indentLevel);
};

#define RO_NO_CLOSEBUTTON						0x0001
#define RO_HIDDEN									0x0002
#define RO_ROLLED_UP								0x0004
#define RO_IN_FLOATER							0x0008
#define RO_INSTALLED								0x0010
#define RO_UTIL_MAIN								0x0020
#define RO_SILENT_ERRORS						0x0040
#define RO_HIDDEN2								0x0080
#define RO_PLUGIN_OWNED							0x0100
#define RO_CONTROLS_INSTALLED					0x0200
#define RO_AUTO_LAYOUT_ON_RESIZE_ENABLED	0x0400
#define RO_SOURCE_SCENE_EMBEDDED				0x0800

#define ROF_LOCK_WIDTH							0x0001
#define ROF_LOCK_HEIGHT							0x0002
#define ROF_AUTO_LAYOUT_ON_RESIZE			0x0004

/* --------------------- RolloutFloater class ------------------------ */
//! \cond DOXYGEN_IGNORE
visible_class (RolloutFloater);
//! \endcond

class RolloutFloater : public Value, public IRollupCallback
{
public:
	//NOTE: May be null.  All affected code should have null checking
	HWND			window = nullptr;					// modeless dialog window

	HWND			ru_window = nullptr;				// host rollup window cust control
	IRollupWindow*	irw = nullptr;				// rollup window control
	Tab<Rollout*>	rollouts;					// my rollouts
	int				width = 0, height = 0;		// window size...
	int				left = 0, top = 0;			// window position
	int				rollout_width_delta = 0;	// the RolloutFloater's client width - rollout width - used when resizing RolloutFloater to resize rollouts
	bool			inDelete = false;			// deleting RolloutFloater
	DWORD			options = ROF_LOCK_WIDTH;	// whether width and or height locked, whether to re-layout on width change

				RolloutFloater(HWND hWnd, DWORD ioptions = ROF_LOCK_WIDTH);
				RolloutFloater(const MCHAR* title, int left, int top, int width, int height, DWORD ioptions, MXSScrollBarPolicy scrollBarPolicy);
				RolloutFloater();
				RolloutFloater(const MCHAR* title, int left, int top, int width, int height, HWND parentHWnd, DWORD ioptions, MXSScrollBarPolicy scrollBarPolicy);
	 			~RolloutFloater();
#	define		is_rolloutfloater(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(RolloutFloater))

	void		Init(const MCHAR* ititle, int iLeft, int iTop, int iWidth, int iHeight, HWND parentHWnd, DWORD options, MXSScrollBarPolicy scrollBarPolicy);

	void		HandleDestroy() override; // from IRollupCallback

				classof_methods (RolloutFloater, Value);
	void		collect() override;
	void		gc_trace() override;

#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")
	void		add_rollout(Rollout* ro, BOOL rolledUp, BOOL borderless);
	void		remove_rollout(Rollout* ro);

	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;

	static void DeleteRolloutFloater(RolloutFloater* rof);
};
#pragma warning(push)
#pragma warning(disable:4100)

/* -------------------- RolloutControl classes ----------------------- */

/* represent controls such as buttons & spinners on rollout panels, RolloutControl is abstract
 * root of all controls */
 
//! \cond DOXYGEN_IGNORE
visible_class (RolloutControl);
//! \endcond

class RolloutControl : public Value
{
public:
	Value*		name;
	Value*		caption;
	Value*		init_caption;
	BOOL		enabled;
	Value**		keyparms;
	int			keyparm_count;
	Rollout*	parent_rollout;
	WORD	    control_ID;
	PB2Param*	pid;			// if non-NULL, indicates this control is associated with an MSPlugin parameter &
								//   points at ParamUIRep-like data for it
	short		flags;

	ScripterExport  RolloutControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	ScripterExport ~RolloutControl();

	// Value
				classof_methods (RolloutControl, Value);
	BOOL		_is_rolloutcontrol() override { return 1; }
#	define		is_rolloutcontrol(v) ((v)->_is_rolloutcontrol())
	void		collect() override;
	virtual	ScripterExport void gc_trace() override;
	virtual	ScripterExport Value* get_property(Value** arg_list, int count) override;
	virtual	ScripterExport Value* set_property(Value** arg_list, int count) override;

	// RolloutControl
	virtual ScripterExport void	add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y);
	// Adjust the layout of existing hwnds owned by the control that were created via add_control. The
	// implementation of this method would parallel add_control, except that it sets the position and 
	// size of existing controls.  The default implementation in Value adjusts a single hwnd and can
	// be used for derived classes that create the control via Value::add_control.
	virtual ScripterExport void adjust_control(int& current_y);
	virtual LPCMSTR	get_control_class() = 0;
	virtual DWORD	get_control_style() { return WS_TABSTOP; }
	virtual DWORD	get_control_ex_style() { return 0; }
	virtual void	init_control(HWND control) { }
	virtual void	compute_layout(Rollout *ro, layout_data* pos) { }
	virtual ScripterExport void	compute_layout(Rollout *ro, layout_data* pos, int& current_y);
	virtual ScripterExport void	process_layout_params(Rollout *ro, layout_data* pos, int& current_y);
	virtual ScripterExport void	setup_layout(Rollout *ro, layout_data* pos, int& current_y);
	virtual ScripterExport void	process_common_params();

	virtual ScripterExport void	call_event_handler(Rollout *ro, Value* event, Value** arg_list, int count);
	virtual ScripterExport void	run_event_handler(Rollout *ro, Value* event, Value** arg_list, int count);
	virtual BOOL	handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) { return FALSE; }

	ScripterExport WORD	next_id();

	virtual	ScripterExport void	set_text(const MCHAR* text, HWND ctl = nullptr, Value* align = nullptr);
	virtual	ScripterExport void	set_enable();
	virtual ScripterExport BOOL set_focus();
	virtual int num_controls() { return 1; }
	ScripterExport Value* get_event_handler(Value* event);
	ScripterExport Value* get_wrapped_event_handler(Value* event);

	virtual void ui_time_changed() { } // called when the ui time has changed, implement if control needs to be updated
	virtual Control* get_controller() { return nullptr; } // get the controller for this rollout control. Overrriden by AnimatableRolloutControl
	virtual BOOL controller_ok(Control* c) { return FALSE; } // return TRUE if controller 'c' can be assigned to the ui control

	// get primary hwnd associated with the rollout control
	ScripterExport HWND GetHWND();

	// returns true if the rollout control is disabled externally (usually because it's locked),
	// and should thus show itself in an disabled state.
	ScripterExport bool IsDisabledExternally();

	// PB2 UI update
	ScripterExport IParamBlock2* get_pblock();
	virtual void Reload() { }
	virtual void InvalidateUI() { }
	virtual void set_pb2_value() { }
	virtual void get_pb2_value(BOOL load_UI=TRUE) { }
	virtual void SetTexDADMgr(DADMgr* dad) { }
	virtual int FindSubTexFromHWND(HWND hw) { return -1; }
	virtual void SetMtlDADMgr(DADMgr* dad) { }
	virtual int FindSubMtlFromHWND(HWND hw) { return -1; } 
};

/* ui controls that have animation controllers, such as sliders and spinners, derive from AnimatableRolloutControl. This class handles the controller. 
We don't want the RolloutControl to derive from Animatable, nor create an Animatable in its ctor since RolloutControls are created in the Listener thread.
We only want to create Animatables in the main thread. A AnimatableRolloutControl::ControllerHolder instance is created when needed and is used to
hold the controller.
*/
class AnimatableRolloutControl : public RolloutControl
{
public:
	/* the AnimatableRolloutControl class uses instances of this class to hold their controller. 
	*/
	class ControllerHolder : public ReferenceMaker
	{
	public:
		explicit ControllerHolder(AnimatableRolloutControl* owner) : mOwner(owner) {}

		// Animatable
		Class_ID ClassID() override { return Class_ID(0x3d063ac9, 0x7136487c); }
		void GetClassName(MSTR& s, bool localized = true) const override { UNUSED_PARAM(localized); s = _M("AnimatableRolloutControlControllerHolder"); }

		// ReferenceMaker
		int NumRefs() override { return 1; }
		RefTargetHandle GetReference(int i) override { return mController; }

	protected:
		void SetReference(int i, RefTargetHandle rtarg) override;

	public:
		// REFMSG_CHANGE messages from the controller causes controller_changed() to be called on the owner
		RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID,
				RefMessage message, BOOL propagate) override;

		// ControllerHolder
		virtual Control* get_controller() { return mController; }
		virtual void set_controller(Control* c) { ReplaceReference(0, c); }
		virtual void gc_trace();

	private:
		Control* mController = nullptr; // the animation controller
		// AnimatableRolloutControl::process_common_params potentially replaces the reference to the controller. That is called while the 
		// hold system is suspended. That can result in undo/redo instability if there were restore objects using the previous controller.
		// We hold a MAXControl* here to prevent this instability, relying on the fact the the MAXControl will not be deleted
		// until a full gc occurs, at which time the undo system is already empty or is flushed.
		Value* mMAXControl = nullptr;
		AnimatableRolloutControl* mOwner = nullptr; // the owning AnimatableRolloutControl
	};
public:
	AnimatableRolloutControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~AnimatableRolloutControl();

#pragma push_macro("new")
#undef new
	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;
#pragma pop_macro("new")

	// Value
	ScripterExport void gc_trace() override;

	// RolloutControl
	ScripterExport void process_common_params() override;
	ScripterExport Value* get_property(Value** arg_list, int count) override;
	ScripterExport Value* set_property(Value** arg_list, int count) override;

	ScripterExport void ui_time_changed() override; // default implementation calls controller_changed
	Control* get_controller() override;
	BOOL controller_ok(Control* c) override { return FALSE; }

	// AnimatableRolloutControl
	void ScripterExport set_controller(Control* c);
	// following is called when controller or its value has changed, ui needs to update to new value.
	// derived classes must implement this method
	virtual void controller_changed() = 0;

protected:
	ParamDimension* dim = nullptr; // controller's dimension

private:
	ControllerHolder* mControllerHolder = nullptr; // the holder of the animation controller
};

// flag bits for RolloutControl::flags
#define ROC_FIXED_WIDTH				0x0001	// a specific width: supplied, don't resize buttons on .text =
#define ROC_MAKING_EDIT				0x0002
#define ROC_INVISIBLE				0x0004	// rollout control is set to invisible and is disabled
#define ROC_VAL_CHANGED				0x0008	// value was changed while control had focus
#define ROC_EDITTEXT_MULTILINE		0x0010	// edittext control is multiline
#define ROC_COLORSWATCH_POINT4		0x0010	// Set if color swatch is a Point4 (FRGBA)
#define ROC_PICKBUTTON_AUTODISP		0x0010	// Set if autoDisplay is turned on for pickbutton (autodisplay node name)
#define ROC_SPINNER_KEYBRACKETS		0x0010  // Set if spinner's setKeyBrakets is true(on).

#define ROC_IN_HANDLER				0x0020	// Set if currently running event handler
#define ROC_HANDLER_REENTRANT		0x0040	// Set if rollout control's event handlers are re-entrant. Only case is checkbutton.
#define ROC_EDITTEXT_READONLY		0x0080	// edittext control is read-only

#define ROC_DELETE_TOOLTIP			0x0100	// control has an extender-based tooltip to be manually deleted

extern LPCMSTR cust_button_class;

/* ------------- PB2Param class -------------------- */

// present in a UI control if rollout is part of a scripted plugin
// and this control is associated with a ParamBlock2  param

class PB2Param
{
public:
	ParamID id;			// pblock param ID
	int index;			// pblock direct index of param
	int tabIndex;		// element index if param is Tab<>
	int	block_id;		// owning block's ID
	int	subobjno;		// texmap or mtl param subobjno in the block
	ParamDimension* dim;// parameter's dimension
	ParamType2 type;	// parameter's type

	PB2Param(const ParamDef& pd, int index, int block_id, int subobjno, int tabIndex = -1);
}; 

/* -------------------- LabelControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (LabelControl, RolloutControl);
//! \endcond

class LabelControl : public RolloutControl
{
public:
	LabelControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~LabelControl();
    
	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

				classof_methods (LabelControl, RolloutControl);
	void		collect() override;
	ScripterExport void		sprin1(CharStream* s) override { s->printf(_M("LabelControl:%s"), name->to_string()); }

	LPCMSTR		get_control_class() override { return _M("STATIC"); }
	DWORD		get_control_style() override;
	void		compute_layout(Rollout *ro, layout_data* pos) override;
	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;
	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
};

/* -------------------- ButtonControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (ButtonControl, RolloutControl);
//! \endcond

class ButtonControl : public RolloutControl
{
public:
	HIMAGELIST	images;
	int			image_width, image_height;
	int			iOutEn, iInEn, iOutDis, iInDis;
	MSTR		m_toolTip;
	int			colortheme;
	MSTR		images_filename;
	MSTR		iconName;


	ButtonControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~ButtonControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

				classof_methods (ButtonControl, RolloutControl);
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("ButtonControl:%s"), name->to_string()); }

	void		ColorChangeNotifyProc(void* param, NotifyInfo* pInfo);
	LPCMSTR		get_control_class() override { return cust_button_class; }
	void		init_control(HWND control) override;
	void		compute_layout(Rollout *ro, layout_data* pos) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;
	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
	void		set_enable() override;
	void		ProcessIconSpecification();
};	

/* -------------------- CheckButtonControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (CheckButtonControl, RolloutControl);
//! \endcond

class CheckButtonControl : public RolloutControl
{
public:
	int			checked;	// LAM - 2/11/02 - changed from BOOL (which is actually an int) since now tristate
	HIMAGELIST	images;
	int			image_width, image_height;
	int			iOutEn, iInEn, iOutDis, iInDis;
	MSTR		m_toolTip;
	int			colortheme;
	MSTR		images_filename;
	MSTR		iconName;

	CheckButtonControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~CheckButtonControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

				classof_methods (CheckButtonControl, RolloutControl);
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("CheckButtonControl:%s"), name->to_string()); }

	void		ColorChangeNotifyProc(void* param, NotifyInfo* pInfo);
	LPCMSTR		get_control_class() override { return cust_button_class; }
	void		init_control(HWND control) override;
	void		compute_layout(Rollout *ro, layout_data* pos) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;
	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
	void		set_enable() override;
	void		ProcessIconSpecification();

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;
};	

/* -------------------- EditTextControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (EditTextControl, RolloutControl);
//! \endcond

class EditTextControl : public RolloutControl
{
public:
	Value*		text;
	Value*		bold;
	bool		in_setvalue;

	EditTextControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~EditTextControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

				classof_methods (EditTextControl, RolloutControl);
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("EditTextControl:%s"), name->to_string()); }
	void		gc_trace() override;

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y) override;
	void		adjust_control(int& current_y) override;
	LPCMSTR		get_control_class() override { return CUSTEDITWINDOWCLASS; }
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;
	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
	void		set_enable() override;
	int			num_controls() override { return 2; }

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;
};

/* -------------------- ComboBoxControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (ComboBoxControl, RolloutControl);
//! \endcond

class ComboBoxControl : public RolloutControl
{
public:
	Array*		item_array;
	int			selection;
	short		type;
	short		flags;

	ComboBoxControl(Value* name, Value* caption, Value** keyparms, int keyparm_count, int type = CBS_SIMPLE);
	~ComboBoxControl();

    static RolloutControl* create_cb(Value* name, Value* caption, Value** keyparms, int keyparm_count);
    static RolloutControl* create_dd(Value* name, Value* caption, Value** keyparms, int keyparm_count);

				classof_methods (ComboBoxControl, RolloutControl);
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("ComboBoxControl:%s"), name->to_string()); }
	void		gc_trace() override;

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y) override;
	void		adjust_control(int& current_y) override;
	LPCMSTR		get_control_class() override { return _M("COMBOBOX"); }
	DWORD		get_control_style() override { return CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT | WS_TABSTOP; }
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;
	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
	void		set_enable() override;
	int			num_controls() override { return 2; }

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;
};

#define CBF_EDIT_FIELD_CHANGING		0x0001

/* -------------------- ListBoxControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (ListBoxControl, RolloutControl);
//! \endcond

class ListBoxControl : public RolloutControl
{
public:
	Array*		item_array;
	int			selection;

	ListBoxControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~ListBoxControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

				classof_methods (ListBoxControl, RolloutControl);
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("ListBoxControl:%s"), name->to_string()); }
	void		gc_trace() override;

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y) override;
	void		adjust_control(int& current_y) override;
	LPCMSTR		get_control_class() override { return _M("LISTBOX"); }
	DWORD		get_control_style() override { return WS_TABSTOP; }
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;
	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
	void		set_enable() override;
	int			num_controls() override { return 2; }

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;
};

/* -------------------- SpinnerControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (SpinnerControl, RolloutControl);
//! \endcond

class SpinnerControl : public AnimatableRolloutControl
{
public:
	float		fvalue = 0.f;
	float		fmax = 0.f, fmin = 0.f;
	int			ivalue = 0;
	int			imax = 0, imin = 0;
	float		scale = 0.f;
	BOOL		autoScale = FALSE;
	EditSpinnerType spin_type = EDITTYPE_FLOAT;

	SpinnerControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~SpinnerControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

				classof_methods (SpinnerControl, RolloutControl);
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("SpinnerControl:%s"), name->to_string()); }

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y) override;
	void		adjust_control(int& current_y) override;
	LPCMSTR		get_control_class() override { return SPINNERWINDOWCLASS; }
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;

	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
	void		set_enable() override;
	BOOL		set_focus() override;
	int			num_controls() override { return 3; }

	void		controller_changed() override;
	BOOL		controller_ok(Control* c) override { return c->SuperClassID() == CTRL_FLOAT_CLASS_ID; }

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;
};

/* -------------------- SliderControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (SliderControl, RolloutControl);
//! \endcond

class SliderControl : public AnimatableRolloutControl
{
public:
	float		fvalue = 0.f;
	float		fmax = 0.f, fmin = 0.f;
	int			ivalue = 0;
	int			imax = 0, imin = 0;
	int			ticks = 0;
	int			slider_type = 0;
	bool		vertical = false;
	bool		sliding = false;

	SliderControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~SliderControl();
    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

				classof_methods (SliderControl, RolloutControl);
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("SliderControl:%s"), name->to_string()); }

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y) override;
	void		adjust_control(int& current_y) override;
	LPCMSTR		get_control_class() override;
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;

	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
	void		set_enable() override;
	int			num_controls() override { return 2; }

	void		controller_changed() override;
	BOOL		controller_ok(Control* c) override { return c->SuperClassID() == CTRL_FLOAT_CLASS_ID; }

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;
};

/* -------------------- PickerControl  ------------------- */

class PickerControl;

class PickerControlFilter : public PickNodeCallback
{
public:
		Value*	filter_fn;
		PickerControl*  picker;

			    PickerControlFilter(Value* filter, PickerControl* picker) : filter_fn(filter), picker(picker) { }
		BOOL	Filter(INode *node) override;
};

class PickerControlMode : public PickModeCallback
{
public:
		PickerControlFilter* pick_filter;
		MSTR			msg;
		PickerControl*  picker;

		PickerControlMode(PickerControlFilter* ifilter, const MCHAR* imsg, PickerControl* ipick);
		~PickerControlMode();

		BOOL	HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags) override;
		BOOL	Pick(IObjParam *ip, ViewExp *vpt) override;
		PickNodeCallback *GetFilter() override { return pick_filter; }
		BOOL	RightClick(IObjParam *ip, ViewExp *vpt) override { return TRUE; }
		void	EnterMode(IObjParam *ip) override;
		void	ExitMode(IObjParam *ip) override;
};
//! \cond DOXYGEN_IGNORE
visible_class_s (PickerControl, RolloutControl);
//! \endcond

class PickerControl : public RolloutControl
{
public:
	PickerControlFilter* pick_filter;
	PickerControlMode*   pick_mode;
    ICustButton*		 cust_button; 
	Value*				 picked_object;
	MSTR				 m_toolTip;

	PickerControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~PickerControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

				classof_methods (PickerControl, RolloutControl);
	void		collect() override;
	void		gc_trace() override;
	void		sprin1(CharStream* s) override { s->printf(_M("PickerControl:%s"), name->to_string()); }

	LPCMSTR		get_control_class() override { return cust_button_class; }
	void		compute_layout(Rollout *ro, layout_data* pos) override;
	void		init_control(HWND control) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;
	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
	void		set_enable() override;

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;
};

/* -------------------- ColorPickerControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (ColorPickerControl, RolloutControl);
//! \endcond

class ColorPickerControl : public RolloutControl
{
public:
	Value*		  color;
    IColorSwatch* csw;
	Value*		  title;
	BOOL		  notifyAfterAccept;

	ColorPickerControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~ColorPickerControl();

	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

	classof_methods (ColorPickerControl, RolloutControl);
	void		collect() override;
	void		gc_trace() override;
	void		sprin1(CharStream* s) override { s->printf(_M("ColorPickerControl:%s"), name->to_string()); }

	LPCMSTR		get_control_class() override { return COLORSWATCHWINDOWCLASS; }
	void		init_control(HWND control) override;
	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y) override;
	void		adjust_control(int& current_y) override;
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;
	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
	void		set_enable() override;
	int			num_controls() override { return 2; }

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;
};

/* -------------------- RadioControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (RadioControl, RolloutControl);
//! \endcond

class RadioControl : public RolloutControl
{
public:
	int			state;
	int			btn_count;
	MaxSDK::Array<IPoint2> offsets;

	RadioControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~RadioControl();

	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

	classof_methods (RadioControl, RolloutControl);
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("RadioControl:%s"), name->to_string()); }

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y) override;
	void		adjust_control(int& current_y) override;
	LPCMSTR		get_control_class() override { return _M("BUTTON"); }
	DWORD		get_control_style() override {  return BS_AUTORADIOBUTTON; }
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;

	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
	void		set_enable() override;
	BOOL		set_focus() override;
	int			num_controls() override { return btn_count + 1; } // buttons and caption. don't count 1 dummy button that ends group

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;
};

/* -------------------- CheckBoxControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (CheckBoxControl, RolloutControl);
//! \endcond

class CheckBoxControl : public RolloutControl
{
public:
	int			checked; // LAM - 2/11/02 - added 3rd state (indeterminate). Changed from BOOL to int just for clarity.

	CheckBoxControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~CheckBoxControl();
	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

	classof_methods (CheckBoxControl, RolloutControl);
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("CheckBoxControl:%s"), name->to_string()); }

	LPCMSTR		get_control_class() override { return _M("BUTTON"); }
	DWORD		get_control_style() override {  return BS_AUTO3STATE | BS_MULTILINE | WS_TABSTOP; }
	void		init_control(HWND control) override;
	void		compute_layout(Rollout *ro, layout_data* pos) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;

	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;
};

/* -------------------- BitmapControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (BitmapControl, RolloutControl);
//! \endcond

class BitmapControl : public RolloutControl
{
public:
	Value*		file_name;
	Value*		max_bitmap;    // if supplied
	Value*		iconName;
	HBITMAP		bitmap;
	BOOL		applyUIScaling;
	int			image_width, image_height;

	BitmapControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~BitmapControl();

	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

	classof_methods (BitmapControl, RolloutControl);
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("BitmapControl:%s"), name->to_string()); }
	void		gc_trace() override;

	LPCMSTR		get_control_class() override { return _M("STATIC"); }
	DWORD		get_control_style() override { return SS_BITMAP + SS_CENTERIMAGE; }
	DWORD		get_control_ex_style() override { return WS_EX_CLIENTEDGE; }
	void		compute_layout(Rollout *ro, layout_data* pos) override;
	void		process_layout_params(Rollout *ro, layout_data* pos, int& current_y) override;
	void		init_control(HWND control) override;
	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
};

/* -------------------- MapButtonControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class (MapButtonControl);
//! \endcond

class MapButtonControl : public ButtonControl
{
private:
    class MapButtonDADMgr : public DADMgr
    {
        MapButtonControl *mBtn;
    public:
        MapButtonDADMgr() {}
        void 		Init(MapButtonControl *btn) { mBtn = btn; }
        SClass_ID 	GetDragType(HWND hwnd, POINT p) override { return TEXMAP_CLASS_ID; }
        BOOL 		OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew) override;
        int 		SlotOwner() override { return OWNER_SCENE; }
        ReferenceTarget * GetInstance(HWND hwnd, POINT p, SClass_ID type) override;
        void 		Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, DADMgr* srcMgr = nullptr, BOOL bSrcClone = FALSE) override;
        BOOL 		AutoTooltip() override { return TRUE; }
    };

public:
	Value*       map;
	ICustButton* btn;
    MapButtonDADMgr mapButtonDADMgr;

	MapButtonControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~MapButtonControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

				classof_methods (MapButtonControl, RolloutControl);
	void		gc_trace() override;
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("MapButtonControl:%s"), name->to_string()); }

	void		init_control(HWND control) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;
	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;

	void		SetTexDADMgr(DADMgr* dad) override { if (btn) btn->SetDADMgr(dad); }
	int			FindSubTexFromHWND(HWND hw) override;
};	

/* -------------------- MtlButtonControl  ------------------- */
//! \cond DOXYGEN_IGNORE
visible_class (MtlButtonControl);
//! \endcond

class MtlButtonControl : public ButtonControl
{
private:
    class MtlButtonDADMgr : public DADMgr
    {
        MtlButtonControl *mBtn;
    public:
        MtlButtonDADMgr() {}
        void 		Init(MtlButtonControl *btn) { mBtn = btn; }
        SClass_ID 	GetDragType(HWND hwnd, POINT p) override { return MATERIAL_CLASS_ID; }
        BOOL 		OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew) override;
        int 		SlotOwner() override { return OWNER_SCENE; }
        ReferenceTarget * GetInstance(HWND hwnd, POINT p, SClass_ID type) override;
        void 		Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, DADMgr* srcMgr = nullptr, BOOL bSrcClone = FALSE) override;
        BOOL 		AutoTooltip() override { return TRUE; }
    };

public:
	Value*		 mtl;
	ICustButton* btn;
    MtlButtonDADMgr mtlButtonDADMgr;

	MtlButtonControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~MtlButtonControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count);

				classof_methods (MtlButtonControl, RolloutControl);
	void		gc_trace() override;
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("MtlButtonControl:%s"), name->to_string()); }

	void		init_control(HWND control) override;
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) override;
	Value*		get_property(Value** arg_list, int count) override;
	Value*		set_property(Value** arg_list, int count) override;
	void		set_enable() override;

	void		Reload() override;
	void		InvalidateUI() override;
	void		set_pb2_value() override;
	void		get_pb2_value(BOOL load_UI=TRUE) override;

	void		SetMtlDADMgr(DADMgr* dad) override { if (btn) btn->SetDADMgr(dad); }
	int			FindSubMtlFromHWND(HWND hw) override;
};	

/* ----------------------- GroupControls  ---------------------- */
//! \cond DOXYGEN_IGNORE
visible_class_s (GroupStartControl, RolloutControl);
//! \endcond

class GroupStartControl : public RolloutControl
{
public:
	int		start_y;			/* y coord of top of group */

	GroupStartControl(Value* caption);
	~GroupStartControl();

			classof_methods (GroupStartControl, RolloutControl);
	void	collect() override;
	void	sprin1(CharStream* s) override { s->printf(_M("GroupStartControl:%s"), caption->to_string()); }

	void	add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y) override;
	void	adjust_control(int& current_y) override;
	LPCMSTR	get_control_class() override { return _M(""); }
	void	compute_layout(Rollout *ro, layout_data* pos) override { }
};
//! \cond DOXYGEN_IGNORE
visible_class_s (GroupEndControl, RolloutControl);
//! \endcond

class GroupEndControl : public RolloutControl
{
private:
	GroupStartControl* my_start;			/* link back to my group start control */
public:
	GroupEndControl(GroupStartControl* starter);
	~GroupEndControl();

			classof_methods (GroupEndControl, RolloutControl);
	void	collect() override;
	void	sprin1(CharStream* s) override { s->printf(_M("GroupEndControl:%s"), caption->to_string()); }

	void	add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y) override;
	void	adjust_control(int& current_y) override;
	LPCMSTR	get_control_class() override { return _M(""); }
	void	compute_layout(Rollout *ro, layout_data* pos) override { }
};
#pragma warning(pop) // for C4100
// ---- time change callback for rollouts with controller-linked spinners in them ----

class RolloutChangeCallback : public TimeChangeCallback
{
public:
	Rollout*	ro;
				RolloutChangeCallback(Rollout* iro) { ro = iro; }
	void		TimeChanged(TimeValue t) override { ro->TimeChanged(t); }
};

/* control keyword parameter access macros... */

extern ScripterExport Value* _get_control_param(Value** keyparms, int count, Value* key_name);
extern ScripterExport Value* _get_control_param_or_default(Value** keyparms, int count, Value* key_name, Value* def);

#define control_param(key)					_get_control_param(keyparms, keyparm_count, n_##key)
#define control_param_or_default(key, def)	_get_control_param_or_default(keyparms, keyparm_count, n_##key##, def)
#define int_control_param(key, var, def)	((var = _get_control_param(keyparms, keyparm_count, n_##key)) == &unsupplied ? def : var->to_int())
#define float_control_param(key, var, def)	((var = _get_control_param(keyparms, keyparm_count, n_##key)) == &unsupplied ? def : var->to_float())
#define bool_control_param(key, var, def)	((var = _get_control_param(keyparms, keyparm_count, n_##key)) == &unsupplied ? def : var->to_bool())

// ----------- MXSToolTipExtender Support -----------------

class MXSToolTipExtender : public MaxSDK::Util::Noncopyable
{
public:
	// Sets an extender-based tooltip onto a rollout control
	ScripterExport void SetToolTipExt( RolloutControl* control, Value* toolTipVal );
	// Sets an extender-based tooltip onto a HWND
	ScripterExport void SetToolTip( HWND hWnd, MSTR toolTip );
	// Gets the extender-based tooltip, if any, from a rollout control
	ScripterExport Value* GetToolTipExt( RolloutControl* control );
	// Gets the extender-based tooltip, if any, from a HWND
	ScripterExport void GetToolTip( HWND hWnd, MSTR& toolTipOut );
	// Removes the extender-based tooltip, if any, from a rollout control
	ScripterExport void RemoveToolTipExt( RolloutControl* control );
	// Removes the extender-based tooltip, if any, from a HWND
	ScripterExport void RemoveToolTip( HWND hWnd );
	// get static singleton instance
	ScripterExport static MXSToolTipExtender& instance();
private:
	ToolTipExtender toolTipExt;
};

