/*		MAXScript.h - main include for MAXScript sources
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// trims win32 includes
#endif

#include "../WindowsDefines.h"
#include <cstdlib> 
#include <cstdio>
#include <cfloat>
#include <cmath>
#include "ScriptSource.h"
#include "../ISceneScriptSecurityManager.h"

// forward declarations
class Value;
class CharStream;
class Rollout;
class MAXScriptException;
class MSPlugin;
class Struct;
class MSZipPackage;
class String;
class StringStream;
class IMXSDebugger;
class Object;
class Interface;
class FPValue;
class Control;
class INode;
class Bitmap;
class BitmapInfo;
class RandGenerator;
class Listener;

// Includes to make this file compile
#include "ScripterExport.h"
#include "kernel/exceptions.h"
#include "kernel/interupts.h"
#include "../strbasic.h"
#include "../ref.h"
#include "../DefaultActions.h" // for DEFAULTACTIONS_LOGMSG

// Includes that are useful for the maxscript library
#include "../dllutilities.h"
#include "../iFnPub.h"
#include "../IParamm2.h"
#include "kernel/value.h"
#include "util/sceneio.h"
#include "foundation/arrays.h"
#include "foundation/hashtable.h"
#include "foundation/name.h"
#include "foundation/streams.h"
#include "foundation/strings.h"
#include "foundation/functions.h"
#include "compiler/thunks.h"

#define NOT_SUPPORTED_BY_PRODUCT(fn)	\
    throw RuntimeError (_M("Feature not available: "), _M(#fn)); 

#define END				NULL					// null varargs arg list terminator

#define MAXSCRIPT_UTILITY_CLASS_ID	Class_ID(0x4d64858, 0x16d1751d)
#define MAX_SCRIPT_DIR				_M("scripts")
#define SCRIPT_AUTOLOAD_DIR			_M("Startup\\")

// check whether we are UNICODE or Code page 0 (==> no mbcs code)
#ifdef _UNICODE
#	define	no_mb_chars		TRUE
#	define	bytelen(s)		(sizeof(wchar_t) * wcslen(s))
#else
#	define	no_mb_chars     (MB_CUR_MAX == 1)
#	define	bytelen(s)		strlen(s)
#endif

// the output from these functions are used for %g float printing, so must return double
inline double EPS(float v)	{ return (std::isnan)(v) ? (v) : fabs(v) < FLT_EPSILON ? 0.0 : (v); }  // small number round down for %g float printing
inline double EPS(double v) { return (std::isnan)(v) ? (v) : fabs(v) < DBL_EPSILON ? 0.0 : (v); }  // small number round down for %g double printing
inline double EPS(int v)	{ return (double)v; }
inline double EPS(INT64 v)	{ return (double)v; }
inline double EPS(DWORD v)	{ return (double)v; }
inline double EPS(short v)	{ return (double)v; }

/* MAXScript-specific window messages */

#define MXS_ADD_ROLLOUT_PAGE		(WM_USER + 0x100)
#define MXS_DELETE_ROLLOUT_PAGE		(WM_USER + 0x101)
#define MXS_REDRAW_VIEWS			(WM_USER + 0x102)
#define MXS_EDIT_SCRIPT				(WM_USER + 0x103)
#define MXS_NEW_SCRIPT				(WM_USER + 0x104)
#define MXS_DISPLAY_BITMAP			(WM_USER + 0x105)
#define MXS_ERROR_MESSAGE_BOX		(WM_USER + 0x106)
#define MXS_PRINT_STRING			(WM_USER + 0x107)
#define MXS_LISTENER_EVAL			(WM_USER + 0x108)
#define MXS_MESSAGE_BOX				(WM_USER + 0x109)
#define MXS_INITIALIZE_MAXSCRIPT	(WM_USER + 0x10A)
#define MXS_KEYBOARD_INPUT			(WM_USER + 0x10B)
#define MXS_SHOW_SOURCE				(WM_USER + 0x10C)
#define MXS_TAKE_FOCUS				(WM_USER + 0x10D)
#define MXS_STOP_CREATING			(WM_USER + 0x10E)
#define MXS_CLOSE_DOWN				(WM_USER + 0x10F)
#define MXS_STOP_EDITING			(WM_USER + 0x110)
#define MXS_LOAD_STARTUP_SCRIPTS	(WM_USER + 0x111)
#define MXS_EXECUTE_MACRO			(WM_USER + 0x112)
#define MXS_RESTART_EDITING			(WM_USER + 0x113)
#define MXS_NOTIFY_REF_DEPENDENTS	(WM_USER + 0x114)
#define MSPLUGINCLASS_STOPEDITING	(WM_USER + 0x115)
#define MSPLUGINCLASS_RESTARTEDITING	(WM_USER + 0x116)
#define MXS_MESSAGE_BOX_EX				(WM_USER + 0x117)

// bits in message_box_data.flags in addition to standard win32 MessageBox flag values
#define MXS_MESSAGEBOX_DONT_SHOW_AGAIN      0x01000000L //!< show 'Do not show this message again' check box in message box (not supported for win32 MessageBox), bit set on return if check box checked
#define MXS_MESSAGEBOX_HOLD_BUTTON          0x02000000L //!< show 'Hold' button (not supported for win32 MessageBox)
#define MXS_MESSAGEBOX_WIN32_MESSAGEBOX     0x10000000L //!< show message box using win32 MessageBox
#define MXS_MESSAGEBOX_MAX_MAXMSGBOX        0x20000000L //!< show message box using MaxMsgBox

struct message_box_data 	// LPARAM for MXS_MESSAGE_BOX contains a pointer to this structure
{
	const MCHAR* title = nullptr;
	const MCHAR* message = nullptr;
	BOOL beep = TRUE;
	int flags = 0;
	int result = 0;
};

// bits in message_box_data_ex.flags2
#define MXS_MESSAGEBOX_PARENT_HWND_SET     0x00000001L //!< message_box_data_ex.parent contains parent HWND to use

struct message_box_data_ex : public message_box_data // LPARAM for MXS_MESSAGE_BOX_EX contains a pointer to this structure
{
	HWND parent = nullptr; //!< the specified parent hwnd, if any.
	DWORD flags2 = 0;
	DWORD_PTR reserved = 0; // reserved for future use.
};

class mxs_notify_ref_data			// LPARAM for MXS_NOTIFY_REF_DEPENDENTS contains a pointer to this class
{
public:
	RefTargetHandle hSender;
	Interval changeInt;
	PartID partID;
	RefMessage message;
	SClass_ID sclass;
	BOOL propagate;
	RefTargetHandle hTarg;
	RefResult result;
	
	mxs_notify_ref_data(RefTargetHandle hSender, PartID partID = PART_ALL, RefMessage message = REFMSG_CHANGE) : 
		changeInt(FOREVER), partID(partID), message(message), sclass(NOTIFY_ALL), propagate(TRUE),
		hTarg(nullptr), hSender(hSender), result(REF_SUCCEED)
		{
			DbgAssert(hSender);
		}
};

#include "kernel/MAXScript_TLS.h"

/* error handlers */

void out_of_memory();
void bad_delete();

/* arg count check & keyword arg accessors (assume conventional names for arg_list & count in using function) */

ScripterExport Value* _get_key_arg(Value** arg_list, int count, Value* key_name);
ScripterExport Value* _get_key_arg_or_default(Value** arg_list, int count, Value* key_name, Value* def);

#define key_arg(key)					_get_key_arg(arg_list, count, n_##key)
#define key_arg_or_default(key, def)	_get_key_arg_or_default(arg_list, count, n_##key##, def)
#define int_key_arg(key, var, def)		((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_int())
#define intptr_key_arg(key, var, def)	((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_intptr())
#define float_key_arg(key, var, def)	((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_float())
#define bool_key_arg(key, var, def)		((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_bool())
#define interval_key_arg(key, var, def)		((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_interval())
#define timevalue_key_arg(key, var, def)	((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_timevalue())
#define node_key_arg(key, var, def)			((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_node())
#define string_key_arg(key, var, def)		((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_string())
#define filename_key_arg(key, var, def)		((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_filename())
#define point4_key_arg(key, var, def)		((var = _get_key_arg(arg_list, count, n_##key)) == &unsupplied ? def : var->to_point4())

#define check_arg_count(fn, w, g)			if ((w) != (g)) throw ArgCountError (_M(#fn), w, g)
#define check_gen_arg_count(fn, w, g)		if ((w) != (g + 1)) throw ArgCountError (_M(#fn), w, g + 1)
#define check_arg_count_with_keys(fn, w, g) if (!(g == w || (g > w && arg_list[w] == &keyarg_marker))) throw ArgCountError (_M(#fn), w, count_with_keys())
#define check_gen_arg_count_with_keys(fn, w, g) if (!(g == w || (g > w && arg_list[w-1] == &keyarg_marker))) throw ArgCountError (_M(#fn), w, count_with_keys() + 1)
#define count_with_keys()					_count_with_keys(arg_list, count)

/* for functions that return a boolean */

#define bool_result(val)					((val) ? &true_value : &false_value)

/* for testing a value to ensure that it is within range */

// following only valid for integer and float range checking
#define MXS_range_check(_val, _lowerLimit, _upperLimit, _desc)		\
	if (_val < _lowerLimit || _val > _upperLimit) {					\
		MCHAR buf[256];												\
		MCHAR buf2[128];											\
		_tcscpy(buf,_desc);											\
		_tcscat(buf,_M(" < "));										\
		_sntprintf(buf2, 128, _M("%g"), EPS(_lowerLimit));			\
		_tcscat(buf,buf2);											\
		_tcscat(buf,_M(" or > "));									\
		_sntprintf(buf2, 128, _M("%g"), EPS(_upperLimit));			\
		_tcscat(buf,buf2);											\
		_tcscat(buf,_M(": "));										\
		_sntprintf(buf2, 128, _M("%g"), EPS(_val));					\
		_tcscat(buf,buf2);											\
		throw RuntimeError (buf);									\
	}


/* value local macros - for managing C local variable references to Value*'s for the collector - see Collectable.cpp */

#include "macros/value_locals.h"

/* general utilities */

ScripterExport MCHAR* save_string(const MCHAR* str);
MCHAR  wputch(HWND w, const MCHAR* buf, const MCHAR* bufp, const MCHAR c);					/* edit window output... */
MCHAR* wputs(HWND w, const MCHAR* buf, const MCHAR* bufp, const MCHAR *str);		
int    wprintf(HWND w, const MCHAR* buf, const MCHAR* bufp, const MCHAR *format, ...);
void   wflush(HWND w, const MCHAR* buf, const MCHAR* bufp);		
#define mputs	thread_local(current_stdout)->puts	/* current MAXScript stdout output... */
#define mputch	thread_local(current_stdout)->putch	/* current MAXScript stdout output... */
#define mprintf	thread_local(current_stdout)->printf
#define mflush	thread_local(current_stdout)->flush

ScripterExport Object* Get_Object_Or_XRef_BaseObject(Object* obj);

// Converts string to valid property or class name, replacing invalid characters with an underscore.
// For example, 'Vol. Select' would be converted to 'Vol__Select'
// Type corresponds to PROPNAME, CLASSNAME, or DROPSPACES where:
//	PROPNAME - spaces in string are converted to underscore
//	CLASSNAME - spaces and punctuation in string are converted to underscore
//	DROPSPACES - removes underscore characters in string
// if bStripTrailing is true, after conversion any underscore characters at end of string are removed
// Return value is how many characters were converted to an underscore
// This method is MBCS-aware
static const int PROPNAME = 1;
static const int CLASSNAME = 2;
static const int DROPSPACES	= 3;

ScripterExport int namify(MCHAR* n, int type, bool bStripTrailing = true);
 
ScripterExport void  install_utility_page(Rollout* rollout);

#include "kernel/MaxscriptTypedefs.h"

/* MAXScript signal flags */

#define INTERRUPT_EVAL		0x0001

ScripterExport int		mxs_rand(); // resolution is 0 - 2^31-1
ScripterExport INT64	mxs_rand64(); // resolution is 0 - 2^63-1
ScripterExport void		mxs_seed(int);
ScripterExport int		random_range(int from, int to);
ScripterExport INT64	random_range(INT64 from, INT64 to);
ScripterExport float	random_range(float from, float to);
ScripterExport double	random_range(double from, double to);
ScripterExport void		dlx_detaching(HINSTANCE hinstance);
ScripterExport void		define_system_global(const MCHAR* name, Value* (*getter)(), Value* (*setter)(Value*));
// LAM 4/1/00 - added following to be able to overwrite existing global value in hash table.
ScripterExport void		define_system_global_replace(const MCHAR* name, Value* (*getter)(), Value* (*setter)(Value*));
ScripterExport void		define_struct_global(const MCHAR* name, const MCHAR* struct_name, Value* (*getter)(), Value* (*setter)(Value*));
ScripterExport void	    printable_name(MSTR& name);
ScripterExport void		show_source_pos();
ScripterExport void		show_source_pos(bool clear_err);
ScripterExport void		show_listener();

ScripterExport void		init_MAXScript();
using utility_installer = void (*)(Rollout* ro);
ScripterExport void		set_utility_installer(utility_installer ui);
ScripterExport void		reset_utility_installer();
ScripterExport void		error_message_box(MAXScriptException& e, const MCHAR* caption);
using autocad_point_reader = Value* (*)(MCHAR* str);
ScripterExport void		set_autocad_point_reader(autocad_point_reader apr);

extern ScripterExport Interface* MAXScript_interface;
extern ScripterExport Interface7* MAXScript_interface7;
extern ScripterExport Interface8* MAXScript_interface8;
extern ScripterExport Interface9* MAXScript_interface9;
extern ScripterExport Interface11* MAXScript_interface11;
extern ScripterExport Interface13* MAXScript_interface13;
extern ScripterExport Interface14* MAXScript_interface14;
extern ScripterExport Interface17* MAXScript_interface17;
//! \deprecated This has been deprecated as of 3ds Max 2022, with Interface18 specific methods moved to Interface13
MAX_DEPRECATED extern ScripterExport Interface17* MAXScript_interface18;
//! \deprecated This has been deprecated as of 3ds Max 2022, with Interface19 specific methods moved to Interface14
MAX_DEPRECATED extern ScripterExport Interface17* MAXScript_interface19;

extern ScripterExport BOOL		escape_enabled; // RK: 05/20/02, 5.0 or later only
extern ScripterExport BOOL		MAXScript_detaching;
extern ScripterExport BOOL		MAXScript_running;
extern ScripterExport HWND		main_thread_window;
extern ScripterExport BOOL		progress_bar_up;

//! \brief Get whether the listener is displayed in a viewport.
//! Returns true if listener displayed in viewport
ScripterExport BOOL				IsMAXScriptListenerInViewport();

// accessors for thread values for error handling
ScripterExport BOOL	get_error_trace_back_disabled(MAXScript_TLS* _tls = nullptr);
ScripterExport void	set_error_trace_back_disabled(BOOL isTraceBackDisabled, MAXScript_TLS* _tls = nullptr);

/*! Class that uses the RAII idiom to push/pop the error traceback disable state. 
	This ensures that the error traceback disable state is properly popped in the event of
	an exception. 
*/
class [[nodiscard]] ScopedErrorTracebackDisable : public MaxSDK::Util::Noncopyable
{
	MAXScript_TLS* m_tls;
	BOOL m_save_disable_trace_back;
	bool m_state_popped;
public:
	ScripterExport ScopedErrorTracebackDisable(BOOL doTraceBackDisable, MAXScript_TLS* _tls = nullptr);
	ScripterExport ~ScopedErrorTracebackDisable();
	ScripterExport void PopState();
};


//! \brief Function compiles and evaluates the specified string.
/*! If the string is successfully compiled and evaluated, and a pointer to an FPValue is provided, 
	the return value is converted to an FPValue and returned through the referenced FPValue. 
	If the string is not successfully compiled and evaluated:
	  Error messages are by default logged to LogSys if net rendering. 
	  If not net rendering, error messages are written to Listener or by default logged to LogSys if in quiet mode.
	  If a pointer to an FPValue is provided, the error message is stored in the FPValue as a TYPE_TSTR.
	Note: This must be called from the main max thread (That contains UI Controls). This is due
	to the possibility that results from the Maxscript actions can cause the UI to be updated. And since
	Much of the UI contains WPF Elements which will throw an exception if NOT updated from the main thread.
\param s - Points to a null-terminated string that specifies the MAXScript commands to compile and evaluate. 
	This expects a string containing Maxscript expressions, NOT a file path.
\param scriptSource - the source of the script string.
\param quietErrors - If TRUE, or net rendering, errors are by default logged to LogSys. If FALSE and not net rendering, errors are logged to Listener.
\param fpv - Optionally points to an FPValue. If not NULL, the result of the script evaluation is converted to an FPValue and 
	stored in the specified FPValue. If an error occurs, the error message is stored in the FPValue as a TYPE_TSTR.
\param logQuietErrors - If TRUE and quietErrors is TRUE, errors are logged to LogSys. If FALSE and quietErrors is TRUE, errors are not logged to LogSys.
\return Returns TRUE if script was executed successfully. 
\note As of 3ds Max 2016, if the result of the script evaluation cannot be converted to an FPValue, the FPValue is set 
	to TYPE_VALUE and the Value* is stored in it. The Value* is guaranteed to be protected from garbage collection
	only until a new Value* is created or a new expression is evaluated in the calling thread. So the calling 
	code must protect the Value* from garbage collection as soon as possible. Typically this is done via value
	local storage (for example, one_value_local), a ValueHolder, or a ValueHolderMember. For example:
	\code
	FPValue result;
	if (!ExecuteMAXScriptScript(fileReader.ReadFull().ToMCHAR(), MAXScript::ScriptSource::Dynamic, false, &result))
		return;
	if (result.type != TYPE_VALUE)
		return;
	ValueHolderMember res(result.v); 
	\endcode
	In the above example, a scriptSource value of Dynamic was specified. This will cause the file contents to be executed 
	with restricted security rights, regardless of Safe Scene Script Execution settings. This would protect against a scene embedded malicious
	script modifying the file contents, adding malicious commands. 
*/
ScripterExport BOOL		ExecuteMAXScriptScript(const MCHAR* s, MAXScript::ScriptSource scriptSource, BOOL quietErrors = FALSE, FPValue* fpv = nullptr, BOOL logQuietErrors = TRUE);

//! \brief Function that compiles and evaluates a CharStream Maxscript expression.
/*!
\param source - Contains the script to compile and evaluate. The method will protect this value from garbage collection.
\param[in, out] res - If res is non-NULL, will contain TRUE on return if script was executed successfully, FALSE if not.
\param scriptSource - the source of the script string.
\return Returns Value* resulting from evaluating source. Evaluation is within a try/catch, so this function will not
throw. */
ScripterExport Value* ExecuteScript(CharStream* source, bool* res, MAXScript::ScriptSource scriptSource);

//! \brief Function that compiles and evaluates a Maxscript expression string.
/*!
\param source - Contains the script to compile and evaluate. If null, returns 'undefined'.
\param[in, out] res - If res is non-NULL, will contain TRUE on return if script was executed successfully, FALSE if not.
\param scriptSource - the source of the script string.
\return Returns Value* resulting from evaluating source. Evaluation is within a try/catch, so this function will not throw. */
ScripterExport Value*	ExecuteScript(const MCHAR* source, bool* res, MAXScript::ScriptSource scriptSource);

//! \brief Executes the given script file
/*! Internally this calls filein_script_ex.
	\param filename - If filename is NULL, this will open a dialog prompting the user to choose a file path to an existing script.
	If the parameter is non-null, then the given file will be evaluated and compiled. */
ScripterExport void filein_script(const MCHAR* filename);

//! \brief Executes the given script file
/*! \param[in] filename - The fully qualified path to the existing file. This can be maxscript files (*.ms, .mxs), maxscript zip files (*.mzp), 
	encrypted maxscript files (*.mse), or python files (*.py, *.pyc, *.pyw).
	\param[in, out] captured_error_message - String for capturing error message, can be nullptr. 
	\param[in] performEmbeddedScriptSecurityChecks - If true, if thread's scriptSource is Embedded and Safe Scene Script Execution Mode is enabled 
	validate script file extension is valid to run and run script as embedded.
	\return - TRUE if the script was successfully executed, FALSE if not. Note that this does not reflect the result of the script.
	\note - if captured_error_message is specified and an error occurs, no error dialog is displayed. If 
	captured_error_message is not specified and an error occurs, whether an error dialog is displayed depends
	on current quiet mode state. */
ScripterExport BOOL filein_script_ex(const MCHAR* filename, MSTR* captured_error_message = nullptr, bool performEmbeddedScriptSecurityChecks = false);

//! \brief Executes the given script file using given ScriptSource
/*! \param[in] filename - The fully qualified path to the existing file. This can be maxscript files (*.ms, .mxs), maxscript zip files (*.mzp), 
	encrypted maxscript files (*.mse), or python files (*.py, *.pyc, *.pyw).
	\param[in] scriptSource - The ScriptSource value to use when executing the script
	\param[in, out] captured_error_message - String for capturing error message, can be nullptr. 
	\return - TRUE if the script was successfully executed, FALSE if not. Note that this does not reflect the result of the script.
	\note - if captured_error_message is specified and an error occurs, no error dialog is displayed. If 
	captured_error_message is not specified and an error occurs, whether an error dialog is displayed depends
	on current quiet mode state. */
ScripterExport BOOL filein_script_ex(const MCHAR* filename, MAXScript::ScriptSource scriptSource, MSTR* captured_error_message = nullptr);

//! \brief Executes the given script file specified as an AssetUser. This function should be used for executing script files whose name is stored in the scene file.
/*! \param[in] file - The AssetUser specifying the script file. This can be maxscript files (*.ms, .mxs), maxscript zip files (*.mzp), 
	encrypted maxscript files (*.mse), or python files (*.py, *.pyc, *.pyw). The AssetType would typically be kPreRenderScript, kPostRenderScript, or kOtherAsset.
	\param[in, out] capturedErrorMessage - String for capturing error message, can be nullptr. 
	\return - true if the script was successfully executed, false otherwise. Note that this does not reflect the result of the script.
	\note - If MaxSDK::ISceneScriptSecurityManager::IsSafeSceneScriptExecutionEnabled(SettingType) returns true, the script file's extension is validated 
	to run as embedded and the script is run as embedded.
	\note - if captured_error_message is specified and an error occurs, no error dialog is displayed. If 
	captured_error_message is not specified and an error occurs, whether an error dialog is displayed depends
	on current quiet mode state. 
*/
ScripterExport bool ExecuteScriptAsset(const MaxSDK::AssetManagement::AssetUser& file, MSTR* capturedErrorMessage = nullptr);

//! \brief Opens the specified file in the Scripting editor
/*! \param filename - If filename is NULL, this will open a dialog prompting the user to choose a file path to an existing script.
	If the filename is not found, and empty script is opened in the script editor with the specified filename.
	If the parameter is non-null, then the given file will be opened in the Scripting editor. */
ScripterExport void open_script_editor(const MCHAR* filename = nullptr);

// Method for processing input value or array for default action value. Current recognized values are:
// #logMsg, #logToFile, #abort, and integer values. Actions are converted to DWORD where bit 0 is log to Default Action 
// system log, bit 1 is log to log file, and bit 2 is abort/cancel (if applicable).
ScripterExport DWORD		ProcessDefaultActionVal (Value* inpActionVal, DWORD defaultAction = DEFAULTACTIONS_LOGMSG);

//! The MXS debugger interface. A Singleton.
extern ScripterExport IMXSDebugger* theMXSDebugger;

// type_check is a general Value type checking macro
// prefer integer_type_check macro to check for integer types validity for all Integer types (Integer, Inter64 and IntegerPtr)
// prefer float_type_check macro to check for floating types validity for all Float types (Float and Double)
#define type_check(val, cl, where) if (val->tag != class_tag(cl)) throw TypeError (where, val, &cl##_class);

#define SOURCE_STREAM_ENCRYPTED_FLAG 1 /* flag set in thread local source_flags when source is encrypted  */

extern ScripterExport HashTable* globals;
extern ScripterExport HashTable* persistents;
extern ScripterExport Listener*  the_listener;
extern ScripterExport HWND		 the_listener_window;
ScripterExport void       listener_message(UINT iMsg, WPARAM wParam, LPARAM lParam, BOOL block_flag);

extern ScripterExport RandGenerator* ClassIDRandGenerator;

inline int _count_with_keys(Value** arg_list, int count)
{
	// compute # args before any key-args
	for (int i = 0; i < count; i++)
		if (arg_list[i] == (Value*)&keyarg_marker)
			return i;
	return count;
}

ScripterExport BOOL GetPrintAllElements(); // get whether to print all elements of arrays, meshselection, BigMatrix, etc.
ScripterExport BOOL SetPrintAllElements(BOOL); // set whether to print all elements of arrays, meshselection, BigMatrix, etc. Returns old value

ScripterExport bool GetDontRepeatMessages(); // get whether to allow repeated ref messages inside a maxscript context.
ScripterExport bool SetDontRepeatMessages(bool); // set whether to allow repeated ref messages inside a maxscript context. Returns old value

ScripterExport bool CanChangeGroupFlags(INode* node); // Since grouping break the node hierarchy used to represent ALOs, we don't allow it

// get MXS path to object. This is the path shown in macro recorder. If resolved, returns TRUE and places path in name
ScripterExport BOOL Find_MXS_Name_For_Obj(Animatable* obj, MSTR &name, BOOL explicitName = TRUE); 

// methods to convert LF strings to CR/LF strings and back
ScripterExport void Replace_LF_with_CRLF(MSTR &string);
ScripterExport void Replace_CRLF_with_LF(MSTR &string);

ScripterExport void checkFileOpenModeValidity(const MCHAR* mode); // checks validity of mode for file open commands, and throws error if not valid

// returns true if name matches pattern. Will test original and translated name. Pattern can include '*', '?', and sets
// such as [a-m]
ScripterExport bool max_name_match(const MCHAR* max_name, const MCHAR* pattern, bool caseSensitive = false);
// returns true if name matches pattern. Will test original and translated name. Pattern can include '*' and '?', but
// does not include sets such as [a-m]
ScripterExport bool max_node_name_match(const MCHAR* max_name, const MCHAR* pattern, bool caseSensitive = false);

// create and return a Bitmap* based on the BitmapInfo. Basically just a wrapped version of
// TheManager->Create(bitmapInfo) that throws a runtime error if creation is unsuccessful.
ScripterExport Bitmap* CreateBitmapFromBitmapInfo(BitmapInfo& bitmapInfo);

// get the storable bitmapinfo type for the specified bitmapinfo type
ScripterExport int GetStorableBitmapInfoTypeForBitmapInfoType(int type);

// MAXScript preferences.  An instance of this class lives in CORE and is accessible to both CORE and MAXScript
class MAXScriptPrefs
{
public:
	int		loadStartupScripts;
	int		loadSaveSceneScripts;
	int		loadSavePersistentGlobals;
	MSTR	font;
	int		fontSize;
	int		autoOpenListener;
	float	initialHeapSize;
	int		enableMacroRecorder;
	int		showCommandPanelSwitch;
	int		showToolSelections;
	int		showMenuSelections;
	int		absoluteSceneNames;
	int		absoluteSubObjects;
	int		absoluteTransforms;
	int		useFastNodeNameLookup;
	int		showGCStatus;
	int		showEditorPath;
	int		useMultiThreadedUsedMacroscriptCompile; // not currently enabled because of gc issues
	int		logUsedMacroscriptCompiles;
	int		preValidateResourceValues;
	int		displayScriptEditorOnError;

	MAXScriptPrefs() { Reset(); }
	virtual ~MAXScriptPrefs() { }
	void	Reset()
	{
		// MAXScript preference defaults
		loadStartupScripts =		TRUE;
		loadSaveSceneScripts =		TRUE;
		loadSavePersistentGlobals = TRUE;
		font =						_M("Consolas");
		fontSize =					9;
		initialHeapSize =			100.0f;
		autoOpenListener =			FALSE;
		enableMacroRecorder =		FALSE;
		showCommandPanelSwitch =	FALSE;
		showToolSelections =		FALSE;
		showMenuSelections =		FALSE;
		absoluteSceneNames =		FALSE;
		absoluteSubObjects =		FALSE;
		absoluteTransforms =		FALSE;
		useFastNodeNameLookup =		TRUE;
		showGCStatus =				FALSE;
		showEditorPath =			TRUE;
		useMultiThreadedUsedMacroscriptCompile =	FALSE; // not currently enabled because of gc issues
		logUsedMacroscriptCompiles =				FALSE;
		preValidateResourceValues =					FALSE;
		displayScriptEditorOnError =					TRUE;
	}

	virtual void LoadMAXScriptPreferences();
	virtual void SaveMAXScriptPreferences();
};

// temporarily adds license for various activeX controls
// ctor registers the license if needed, dtor removes them
class ActiveXLicensor
{
	DWORD licenseAdded; // acts as bitfield. Each bit signifies whether a specifc license was added.
public:
	ScripterExport ActiveXLicensor();
	ScripterExport ~ActiveXLicensor();
};

// Exception safe helper class for controlling temporary changing of quiet mode. ctor sets quiet mode to 
// specified state, Restore resets to initial state, and dtor resets to initial state if Restore has not been run.
class TempQuietMode
{
public:
	ScripterExport TempQuietMode (BOOL newState = TRUE);
	ScripterExport ~TempQuietMode();
	ScripterExport void Restore();
private:
	BOOL oldState;
	bool needsRestore;
};

// Exception safe helper class for controlling temporary changing of BitmapManager silent mode. ctor sets silent mode to 
// specified state, Restore resets to initial state, and dtor resets to initial state if Restore has not been run.
class TempBitmapManagerSilentMode
{
public:
	ScripterExport TempBitmapManagerSilentMode (BOOL newState = TRUE);
	ScripterExport ~TempBitmapManagerSilentMode();
	ScripterExport void Restore();
private:
	BOOL oldState;
	bool needsRestore;
};

// Helper structure/methods for looking up a Value/int in a table, and returning an int/Value. 
// Typically used for converting function argument name value to its corresponding int value.
// Example:
/*
#define def_cpanel_types()							\
mxs_value_id_pair cpanelTypes[] = {						\
{ TASK_MODE_MODIFY,		n_modify },			\
{ TASK_MODE_HIERARCHY,	n_hierarchy },		\
{ TASK_MODE_MOTION,		n_motion },			\
};

def_cpanel_types();	
DWORD which = (DWORD)ConvertValueToID(cpanelTypes, elements(cpanelTypes), key_arg(which),0x00FF);
*/
struct mxs_value_id_pair
{
	int	id;			// the integer id for the value
	Value* val;		// the value
};

// Look up specified value in table. If not found, if def_id != -1, return def_id, otherwise throw a runtime error that includes a list of the valid values.
ScripterExport int		ConvertValueToID(mxs_value_id_pair pairs[], int count, Value* val, int def_id=-1);
// Look up specified id in table. If not found, if def_val != NULL, return def_val, otherwise throw a runtime error that includes a list of the valid values.
ScripterExport Value*	ConvertIDToValue(mxs_value_id_pair pairs[], int count, int id, Value* def_val=nullptr);

namespace MAXScript
{
	ScripterExport int GetUIScaledValue(float value);  // rounds up
	ScripterExport int GetValueUIUnscaled(float value);  // rounds up
	ScripterExport float GetUIScaledValueFloat(float value); // no rounding
	ScripterExport float GetValueUIUnscaledFloat(float value); // no rounding
}

// get the list of reserved maxscript parser keywords.
ScripterExport Tab<const MCHAR*> GetMAXScriptParserKeywords();

// returns true if the name is a valid maxscript variable name
ScripterExport bool IsVariableNameValid(const MCHAR *var_name);

// returns true if the global variable name is settable. Returns true if 
// either the global variable name does not exist or it does not contain a 
// value that can be overwritten (like 'red', 'sysinfo', etc.)
ScripterExport bool IsGlobalVariableNameSettable(const MCHAR* var_name);


namespace MAXScript
{
	//! \brief Sets Scripting Listener log file to specified file
	/*! \param[in] filename - The fully qualified path for the listener log file
	\param[in] mode - The file open mode specification, typically "w" or "a". See 
	the MAXScript Help file, topic FileStream Values, for a description of the mode string values.
	Note that this method automatically adds 't' to the specified mode string.
	\return - true if the listener log file was created, false otherwise. */
	ScripterExport bool SetMAXScriptListenerLogFile(const MCHAR* filename, const MCHAR* mode);

	//! \brief Gets the Scripting Listener log file name if present
	/*! \param[out] listenerLogFileName - Returns the fully qualified path for the listener log file
	\return - true if a listener log file name was acquired, false otherwise. */
	ScripterExport bool GetMAXScriptListenerLogFile(MSTR& listenerLogFileName);

	//! \brief Test whether the specified ScriptingFeature can be executed
	/*! This function checks to see whether the specified ScriptingFeature can be executed.
	The specified ScriptingFeature is disabled in the following cases:
	1. When MAXScript is executing a scene file embedded script, Safe Scene Script Execution Mode is enabled in
	ISceneScriptSecurityManager, and the specified ScriptingFeature in embedded scripts is disabled in
	ISceneScriptSecurityManager. 
	2. When MAXScript is executing a dynamic script, which can be embedded in a scene file or not.
	3. When 3ds Max is running in SecureMode.
	\param[in] scriptingFeature - The ScriptingFeature.
	\return - true if commands associated with the specified ScriptingFeature can be executed, false otherwise.
	\sa MAXScript::ScriptSource
	*/
	ScripterExport bool CanRunScriptingFeature(MaxSDK::ISceneScriptSecurityManager::ScriptingFeature scriptingFeature);

	//! \brief Validate whether MAXScript system commands can be executed
	/*! This function checks to see whether MAXScript system commands can be executed, and if not throws an exception.
	MAXScript system command execution is disabled in the following cases:
	1. When MAXScript is executing a scene file embedded script, Safe Scene Script Execution Mode is enabled in
	ISceneScriptSecurityManager, and MAXScript System Commands in embedded scripts is disabled in
	ISceneScriptSecurityManager. If this is the case, a SecurityException exception is thrown.
	2. When MAXScript is executing a dynamic script, which can be embedded in a scene file or not. If this is the case,
	a SecurityException exception is thrown.
	3. When 3ds Max is running in SecureMode. If this is the case, a RuntimeError exception is thrown.
	This function should be called from the code that implements the MAXScript command in question.
	This override of the function would be used when the code is a standard MAXScript primitive implementation. For example:
	\code 
	Value* set_file_attribute_cf(Value** arg_list, int count)
	{
		check_arg_count(setFileAttribute, 3, count);
		MAXScript::ValidateCanRunMAXScriptSystemCommand(_T("setFileAttribute"), arg_list, count);
		...
	\endcode
	If MAXScript system commands can not be executed, the arguments in arg_list are converted to their string representation 
	and are appended to the command name for error reporting purposes by 3ds Max.
	\param[in] command - The name of the MAXScript system command. Note that this name is used only for error reporting purposes by 3ds Max.
	\param[in] arg_list - The list of arguments to the command. 
	\param[in] count - The count of arguments in arg_list.
	\sa MAXScript::ScriptSource
	*/
	ScripterExport void ValidateCanRunMAXScriptSystemCommand(const MCHAR* command, Value** arg_list, int count);

	//! \brief Validate whether MAXScript system commands can be executed
	/*! This function checks to see whether MAXScript system commands can be executed, and if not throws an exception.
	MAXScript system command execution is disabled in the following cases:
	1. When MAXScript is executing a scene file embedded script, Safe Scene Script Execution Mode is enabled in 
	ISceneScriptSecurityManager, and MAXScript System Commands in embedded scripts is disabled in 
	ISceneScriptSecurityManager. If this is the case, a SecurityException exception is thrown. 
	2. When MAXScript is executing a dynamic script, which can be embedded in a scene file or not. If this is the case,
	a SecurityException exception is thrown.
	3. When 3ds Max is running in SecureMode. If this is the case, a RuntimeError exception is thrown.
	This function should be called from the code that implements the MAXScript command in question.
	This override of the function would be used when the code is not a standard MAXScript primitive implementation, and the string 
	representation of the argument list is manually constructed. For example:
	\code 
	MAXScript::ValidateCanRunMAXScriptSystemCommand(_T("executeScriptFile"), TSTR(_T("\"")) + filename +  _T("\""));
	\endcode
	If MAXScript system commands can not be executed, the arguments string is appended to the command name for error reporting purposes by 3ds Max.
	\param[in] command - The name of the MAXScript system command. Note that this name is used only for error reporting purposes by 3ds Max.
	\param[in] arguments - A string representation of the list of arguments to the command. Note that this value can be nullptr.
	\sa MAXScript::ScriptSource
	*/
	ScripterExport void ValidateCanRunMAXScriptSystemCommand(const MCHAR* command, const MCHAR* arguments);

	//! \brief Validate whether Python scripts can be executed
	/*! This function checks to see whether Python scripts can be executed, and if not throws an exception.
	Python script execution through the python MAXScript interface is disabled in the following cases: 
	1. When the script is embedded in a scene file, Safe Scene Script Execution Mode is enabled in ISceneScriptSecurityManager, 
	and Python script execution in embedded scripts is disabled in ISceneScriptSecurityManager. If this is the case, a 
	SecurityException exception is thrown. 
	2. When MAXScript is executing a dynamic script, which can be embedded in a scene file or not. If this is the case, 
	a SecurityException exception is thrown.
	3. When 3ds Max is running in SecureMode. If this is the case, a RuntimeError exception is thrown. 
	This function should be called from the code that attempts to run Python scripts.
	This override of the function would be used when the code is a standard MAXScript primitive implementation. For example:
	\code 
	Value* execute_python_script_cf(Value** arg_list, int count)
	{
		check_arg_count(executePythonScript, 3, count);
		MAXScript::ValidateCanRunPythonScripts(_T("executePythonScript"), arg_list, count);
		...
	\endcode
	If Python scripts can not be executed, the arguments in arg_list are converted to their string representation and are
	appended to the command name for error reporting purposes by 3ds Max.
	\param[in] command - The name of the Python script execution command. Note that this name is used only for error reporting purposes by 3ds Max.
	\param[in] arg_list - The list of arguments to the command. 
	\param[in] count - The count of arguments in arg_list.
	\sa MAXScript::ScriptSource
	*/
	ScripterExport void ValidateCanRunPythonScripts(const MCHAR* command, Value** arg_list, int count);

	//! \brief Validate whether Python scripts can be executed
	/*! This function checks to see whether Python scripts can be executed, and if not throws an exception.
	Python script execution through the python MAXScript interface is disabled in the following cases:
	1. When the script is embedded in a scene file, Safe Scene Script Execution Mode is enabled in
	ISceneScriptSecurityManager, and Python script execution in embedded scripts is disabled in
	ISceneScriptSecurityManager. If this is the case, a SecurityException exception is thrown.
	2. When MAXScript is executing a dynamic script, which can be embedded in a scene file or not. If this is the case,
	a SecurityException exception is thrown.
	3. When 3ds Max is running in SecureMode. If this is the case, a RuntimeError exception is thrown. 
	This function should be called from the code that attempts to run Python scripts.
	This override of the function would be used when the code is not a standard MAXScript primitive implementation, and
	the string representation of the argument list is manually constructed. For example: 
	\code
	MAXScript::ValidateCanRunPythonScripts(_T("executePythonScript"), TSTR(_T("\"")) + pythonScript +  _T("\""));
	\endcode
	If Python scripts can not be executed, the arguments string is appended to the command name for error reporting purposes by 3ds Max.
	\param[in] command - The name of the Python script execution command. Note that this name is used only for error reporting purposes by 3ds Max.
	\param[in] arguments - A string representation of the list of arguments to the command. Note that this value can be nullptr.
	\sa MAXScript::ScriptSource
	*/
	ScripterExport void ValidateCanRunPythonScripts(const MCHAR* command, const MCHAR* arguments);

	//! \brief Create string representation of argument list
	/*! This function creates a string representation of the specified argument list. 
	Note that this can be a fairly expensive call, and should only be called when necessary.
	This function would be typically be used in code that is a standard MAXScript primitive implementation. For
	example:
	\code
	Value* dotNet_loadAssembly_cf(Value** arg_list, int count)
	{
		check_arg_count_with_keys(dotNet.loadAssembly, 1, count);
		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		if (!MAXScript::CanRunScriptingFeature(MaxSDK::ISceneScriptSecurityManager::ScriptingFeature::DotNet))
		{
			TSTR msg = _T("dotNet.loadAssembly ");
			TSTR args = MAXScript::CreateStringRepresentationOfArgumentList(arg_list, count);
			msg.append(args);

			static MaxSDK::ISceneScriptSecurityManager* spISceneScriptSecurityManager =
			MaxSDK::ISceneScriptSecurityManager::GetInstance(); spISceneScriptSecurityManager->LogBlockedCommand(msg,
				MaxSDK::ISceneScriptSecurityManager::ScriptingFeature::MAXScriptSystemCommands);
			...
	\endcode
	\param[in] arg_list - The list of arguments. 
	\param[in] count - The count of arguments in arg_list. */
	ScripterExport MSTR CreateStringRepresentationOfArgumentList(Value** arg_list, int count);
}
