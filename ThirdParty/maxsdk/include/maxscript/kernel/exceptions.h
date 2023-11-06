/*		Exception.h - exception class for MAXScript
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#pragma once

#include "../ScripterExport.h"
#include "../../strclass.h"
#include "../../WindowsDefines.h"
#include "../foundation/ValueHolderMember.h"

class Value;
class Thunk;
class ValueMetaClass;
class CharStream;
class MAXScriptException;
struct MAXScript_TLS;

//! \deprecated This has been deprecated as of 3ds Max 2024, please use _T("") instead
MAX_DEPRECATED extern MCHAR* null_string;

// all maxscript flow control and error exceptions derive from MAXScriptException
class ScripterExport MAXScriptException : public MaxHeapOperators
{
protected:
	MSTR mCapturedMXSCallstack; // the captured mxs callstack
	MSTR mCapturedCplusplusCallstack; // the captured C++ callstack
	static bool sMXSCallstackCaptureEnabled; // whether MXS Callstack Capture is enabled
	static bool sCplusplusCallstackCaptureEnabled; // whether C++ Callstack Capture is enabled
	static bool sInMXSCallstackCapture; // whether MXS Callstack Capture currently being performed
	static bool sInCplusplusCallstackCapture; // whether C++ Callstack Capture is currently being performed
public:
	MAXScriptException() {}
	virtual ~MAXScriptException() {}
	virtual void sprin1(CharStream* s) const;
	void CaptureMXSCallstack(bool forceCapture = false, bool captureLocals = true); // capture the mxs callstack if MXS Callstack Capture is enabled
	// skipFirstNStackLevels - How many stack levels to not capture. The default value of 4 causes the capture to start at the call to this method  
	void CaptureCplusplusCallstack(int skipFirstNStackLevels = 4, bool forceCapture = false); // capture the C++ callstack if C++ Callstack Capture is enabled
	const MSTR& MXSCallstack() const { return mCapturedMXSCallstack; }
	const MSTR& CplusplusCallstack() const { return mCapturedCplusplusCallstack; }
	static void MXSCallstackCaptureEnabled( bool enabled);
	static bool isMXSCallstackCaptureEnabled( );
	static void CplusplusCallstackCaptureEnabled( bool enabled);
	static bool isCplusplusCallstackCaptureEnabled( );

	/**
	helper class for disabling mxs callstack capture if an exception is thrown. 
	useful in code where exceptions are not unexpected, and the exception is 'eaten'.
	for example:
	\code
	try
	{
		MAXScriptException::ScopedMXSCallstackCaptureDisable scopedMXSCallstackCaptureDisable;
		UUID = MaxSDK::Util::MaxString::FromMCHAR(res->to_string());
	}
	catch (...)
	{
		// Return value wasn't a string (e.g. "undefined").
	}
	\endcode
	**/
	class ScripterExport ScopedMXSCallstackCaptureDisable : public MaxHeapOperators
	{
	private:
		bool m_disabled;
		bool m_clearErrorSourceData;
		bool m_oldCaptureState;
		MAXScript_TLS* m_tls;
	public:
		/**
		ScopedMXSCallstackCaptureDisable ctor:
		* \param disable	If true, disable mxs callstack capture in ctor
		* \param clearErrorSourceData	If true, call clear_error_source_data() in Enable() if an 
		exception is thrown.
		**/
		ScopedMXSCallstackCaptureDisable(bool disable = true, bool clearErrorSourceData = true);
		/**
		ScopedMXSCallstackCaptureDisable ctor:
		* \param disable	If true, disable mxs callstack capture in ctor
		* \param clearErrorSourceData	If true, call clear_error_source_data() in Enable() if an 
		* \param _tls The current MAXScript_TLS*
		exception is thrown.
		**/
		ScopedMXSCallstackCaptureDisable(MAXScript_TLS* _tls, bool disable = true, bool clearErrorSourceData = true);
		~ScopedMXSCallstackCaptureDisable() { Enable(); }
		void Enable();
	};

	/**
	helper class for disabling c++ callstack capture if a system exception is thrown. 
	useful in code where exceptions are not unexpected, and the exception is 'eaten'.
	for example:
	\code
	try
	{
		MAXScriptException::ScopedCplusplusCallstackCaptureDisable scopedCplusplusCallstackCaptureDisable;
		UUID = MaxSDK::Util::MaxString::FromMCHAR(res->to_string());
	}
	catch (...)
	{
		// Return value wasn't a string (e.g. "undefined").
	}
	\endcode
	**/
	class ScripterExport ScopedCplusplusCallstackCaptureDisable : public MaxHeapOperators
	{
	private:
		bool m_disabled;
		bool m_oldCaptureState;
	public:
		/**
		ScopedCplusplusCallstackCaptureDisable ctor:
		* \param disable	If true, disable c++ callstack capture in ctor
		exception is thrown.
		**/
		ScopedCplusplusCallstackCaptureDisable(bool disable = true);
		~ScopedCplusplusCallstackCaptureDisable() { Enable(); }
		void Enable();
	};
};

// Classes that derived from MAXScriptRuntimeErrorException automatically capture the mxs callstack. 
// Compile errors, FunctionReturn, etc. don't need to capture the mxs callstack.
// NOTE: If a class derives from MAXScriptRuntimeErrorException and takes const MCHAR* args in the ctor, the
// class ctor should call MAXScriptRuntimeErrorException(false) and then manually call CaptureMXSCallstack after acquiring the argument strings.
// This is to prevent potential problems where the string passed comes from a typical GetString implementation
// that returns a pointer to a static buffer, and that GetString function is called again while doing the stack dump.
// This would result in the wrong description being used.
class ScripterExport MAXScriptRuntimeErrorException : public MAXScriptException
{
public:
	MAXScriptRuntimeErrorException();
	MAXScriptRuntimeErrorException(bool doStackCapture);
};

// this class is thrown if a non-maxscript exception (such as a c++ access violation) occurs
class ScripterExport UnknownSystemException : public MAXScriptRuntimeErrorException
{
	MSTR mAdditionalExceptionInformation;
public:
	UnknownSystemException() 
	{
		// if occurs, system exception occurred outside the scope of an 
		// ScopedMaxScriptSEHandler instance
		DbgAssert(false); 
	} 
	UnknownSystemException(int skipFirstNStackLevels, const MSTR& info) 
	{
		CaptureCplusplusCallstack(skipFirstNStackLevels);
		SetAdditionalExceptionInformation(info);
	}
	void	sprin1(CharStream* s) const override;
	void	SetAdditionalExceptionInformation(const MSTR& info)
	{
		mAdditionalExceptionInformation = info;
	}
	const MSTR& GetAdditionalExceptionInformation() const
	{
		return mAdditionalExceptionInformation;
	}
};

/// This is the exception that is thrown if the user breaks execution by pressing and holding the escape key
class ScripterExport SignalException : public MAXScriptException
{
public:
	void	sprin1(CharStream* s) const override;
};

class ScripterExport CompileError : public MAXScriptException
{
public:
	MSTR description;
	MSTR info;
	MSTR line;
	MSTR file;
	CompileError (const MCHAR* d, const MCHAR* i, const MCHAR* l, const MCHAR* f = _T(""));
	CompileError () = default;
	~CompileError ();

	void	sprin1(CharStream* s) const override;
	void	set_file(const MCHAR* f);
};

class ScripterExport SyntaxError : public CompileError
{
	MSTR wanted;
	MSTR got;
public:
	SyntaxError (const MCHAR* w, const MCHAR* g, const MCHAR* l = _T(""), const MCHAR * f = _T(""));
	~SyntaxError ();

	void	sprin1(CharStream* s) const override;
};

class ScripterExport TypeError : public MAXScriptRuntimeErrorException
{
	ValueHolderMember	target;
	ValueMetaClass* wanted_class;
	MSTR description;
public:
			TypeError (const MCHAR* d, Value* t, ValueMetaClass* c = nullptr);
		   ~TypeError ();

	void	sprin1(CharStream* s) const override;
};

class ScripterExport NoMethodError : public MAXScriptRuntimeErrorException
{
	ValueHolderMember	target;
	MSTR fn_name;
public:
			NoMethodError (const MCHAR* fn, Value* t);
		   ~NoMethodError ();

	void	sprin1(CharStream* s) const override;
};

#define unimplemented(m, t) throw NoMethodError (m, t)

class ScripterExport AccessorError : public MAXScriptRuntimeErrorException
{
	ValueHolderMember	target;
	ValueHolderMember	prop;
public:
			AccessorError (Value* t, Value* p);

	void	sprin1(CharStream* s) const override;
};

class ScripterExport AssignToConstError : public MAXScriptRuntimeErrorException
{
	ValueHolderMember	thunk;
public:
			AssignToConstError (Thunk* t);

	void	sprin1(CharStream* s) const override;
};

class ScripterExport ArgCountError : public MAXScriptRuntimeErrorException
{
	int		wanted;
	int		got;
	MSTR	fn_name;
public:
			ArgCountError (const MCHAR* fn, int w, int g);
		   ~ArgCountError ();

	void	sprin1(CharStream* s) const override;
};

class ScripterExport RuntimeError : public MAXScriptRuntimeErrorException
{
public:
			MSTR desc1;
			MSTR desc2;
			ValueHolderMember  info;
			RuntimeError (const MCHAR* d1);
			RuntimeError (const MCHAR* d1, const MCHAR* d2);
			RuntimeError (const MCHAR* d1, Value* ii);
			RuntimeError (const MCHAR* d1, const MCHAR* d2, Value* ii);
			RuntimeError (Value* ii);
		   ~RuntimeError ();

	void	init(const MCHAR* d1, const MCHAR* d2, Value* ii);
	void	sprin1(CharStream* s) const override;
};

class ScripterExport UserThrownError : public MAXScriptRuntimeErrorException
{
public:
			MSTR desc;
			ValueHolderMember  info;
			BOOL	debugBreak;
			UserThrownError (const MCHAR* d1, Value* ii, BOOL dbgBreak);
			UserThrownError (const MCHAR* di, BOOL dbgBreak);
		   ~UserThrownError ();

	void	init(const MCHAR* d1, Value* ii, BOOL dbgBreak);
	void	sprin1(CharStream* s) const override;
};

class ScripterExport DebuggerRuntimeError : public MAXScriptException
{
public:
	MSTR desc1;
	MSTR desc2;
	ValueHolderMember  info;
	DebuggerRuntimeError (const MCHAR* d1);
	DebuggerRuntimeError (const MCHAR* d1, const MCHAR* d2);
	DebuggerRuntimeError (const MCHAR* d1, Value* ii);
	DebuggerRuntimeError (const MCHAR* d1, const MCHAR* d2, Value* ii);
	DebuggerRuntimeError (Value* ii);
	~DebuggerRuntimeError ();

	void	init(const MCHAR* d1, const MCHAR* d2, Value* ii);
	void	sprin1(CharStream* s) const override;
};

class ScripterExport IncompatibleTypes : public MAXScriptRuntimeErrorException
{
	ValueHolderMember  val1;
	ValueHolderMember  val2;
public:
			IncompatibleTypes (Value* v1, Value* v2);

	void	sprin1(CharStream* s) const override;
};

class ScripterExport ConversionError : public MAXScriptRuntimeErrorException
{
	ValueHolderMember	val;
	MSTR type;
public:
			ConversionError (Value* v, const MCHAR* t);
		   ~ConversionError ();

	void	sprin1(CharStream* s) const override;
};

class FunctionReturn : public MAXScriptException
{
public:
	ValueHolderMember	return_result;
			FunctionReturn (Value* v);

	void	sprin1(CharStream* s) const override;
};

class LoopExit : public MAXScriptException
{
public:
	ValueHolderMember	loop_result;
			LoopExit (Value* v);

	void	sprin1(CharStream* s) const override;
};

class LoopContinue : public MAXScriptException
{
public:
			LoopContinue () { }

	void	sprin1(CharStream* s) const override;
};

/*! \remarks Exception thrown if a script calls quitmax resulting in 3ds Max shutting down
Code that handles MAXScriptException via code looking like:
\code
catch (MAXScriptException& e)
{
    ...
    ProcessMAXScriptException(e, MaxSDK::GetResourceStringAsMSTR(IDS_ERROR_OCCURRED_DURING_FILEIN), false, true, false);
    throw;
}
\endcode
should add handling of MaxShutdownException:
\code
catch (MaxShutdownException)
{
    throw;
}
\endcode
The implementation of ProcessMAXScriptException simply returns if the exception passed is a MaxShutdownException. So if your 
catch handling doesn't do anything other than call ProcessMAXScriptException the explicit catch of MaxShutdownException is
not strictly necessary, but is recommended for clarity. 
The 3ds Max user interface has been taken down by the time this exception has been raised, so plugins must not access it.
*/

class MaxShutdownException : public MAXScriptException
{
public:
	MaxShutdownException() { }

	void	sprin1(CharStream* s) const override;
};


//! \brief Exception thrown when calling a scripted function with less than 256k program call stack remaining. 
class CallStackOverflowException : public MAXScriptRuntimeErrorException
{
public:
	CallStackOverflowException() { }
	/**
	CallStackOverflowException ctor:
	* \param capturedMXSCallstack	Do not capture mxs callstack, use specified callstack instead:
	**/
	CallStackOverflowException(const MSTR& capturedMXSCallstack) : MAXScriptRuntimeErrorException(false) { mCapturedMXSCallstack = capturedMXSCallstack; }

	void	sprin1(CharStream* s) const override;
};

//! \brief Exception thrown when attempting to execute MAXScript commands that are disallowed by ISceneScriptSecurityManager.
class ScripterExport SecurityException : public MAXScriptRuntimeErrorException
{
	friend ScripterExport void ProcessMAXScriptException(MAXScriptException& e, const MCHAR* caption, bool displayErrorMessageBox, bool showSourcePosition, bool isTerminalHandling, bool quietErrors);
public:
	SecurityException(const MCHAR* msg, const MCHAR* command = nullptr);
	~SecurityException();

	void sprin1(CharStream* s) const override;
private:
	MSTR msg;
	MSTR command;
	bool wasDisplayedAsError = false; // We always want to display SecurityException to listener, even in try/catch and when ignoring errors
};

/**
These methods centralize the handling of maxscript exceptions in c++ code. Typical usage looks like:

ScopedMaxScriptEvaluationContext scopedMaxScriptEvaluationContext;
MAXScript_TLS* _tls = scopedMaxScriptEvaluationContext.Get_TLS();
two_value_locals_tls( thingValue, propNameValue );
try
{
  ScopedSaveCurrentFrames scopedSaveCurrentFrames(_tls);
  vl.thingValue = InterfaceFunction::FPValue_to_val( thing );
  vl.propNameValue = Name::intern( propName );
  vl.thingValue = vl.thingValue->get_property( &vl.propNameValue, 1 );
  vl.thingValue->to_fpvalue( retVal );
}
catch (MAXScriptException& e)
{
  ProcessMAXScriptException(e, _T("GetScriptedProperty"), false, false, true);
}
catch (...)
{
  ProcessUnknownMAXScriptException(_T("GetScriptedProperty"), false, false, true);
}

* \param e	The maxscript exception to process
* \param caption	The string to be used in the title bar of the error message box, and is used as part of
						the verbose description of the exception. Can be null.
* \param displayErrorMessageBox	If true, an error message box is displayed (unless max is in quiet mode).
* \param showSourcePosition	If true, source position of error is shown in Scripting editor.
* \param isTerminalHandling	If true, terminal handling of the exception is occurring. If false, the catch will be rethrowing the exception.
* \param quietErrors	If true, error information is not written to Listener, a brief description of the exception and its source location is written to the log file.
*/
ScripterExport void ProcessMAXScriptException(MAXScriptException& e, const MCHAR* caption, bool displayErrorMessageBox, bool showSourcePosition, bool isTerminalHandling, bool quietErrors = false);
inline void ProcessUnknownMAXScriptException(const MCHAR* caption, bool displayErrorMessageBox, bool showSourcePosition, bool isTerminalHandling, bool quietErrors = false)
{
	UnknownSystemException e{};
	ProcessMAXScriptException(e, caption, displayErrorMessageBox, showSourcePosition, isTerminalHandling, quietErrors);
}
