/*	
 *		SceneIO.h - MAXScript-related scene file I/O (persistent globals, on-open script, etc.)
 *
 *			Copyright (c) Autodesk, Inc, 1998.  John Wainwright.
 *
 */

#pragma once
#include "mxsMaxFileVersion.h"

#include "../ScripterExport.h"
#include "../kernel/value.h"
#include "../../maxtypes.h"
#include "../../tab.h"
#include "../../ioapi.h"

// forward declarations
class ValueLoader;
class ILoad;
class ISave;

/* --------- Scene I/O chunk ID's ---------- */

#define OPENSCRIPT_CHUNK			0x0010    // obsoleted by CALLBACKSCRIPT_CHUNK
#define SAVESCRIPT_CHUNK			0x0020    // obsoleted by CALLBACKSCRIPT_CHUNK
#define PSGLOBALS_CHUNK				0x0030
#define MSPLUGINCLASS_CHUNK			0x0040
#define MSPLUGINCLASSHDR_CHUNK		0x0050
#define LENGTH_CHUNK				0x0060
#define CALLBACKSCRIPT_A_CHUNK		0x0070
#define CALLBACKSCRIPT_W_CHUNK		0x0071
#define CUSTATTRIBDEF_CHUNK			0x0080
#define SOURCE_A_CHUNK				0x00a0
#define SOURCE_W_CHUNK				0x00a1

/* ---- persistent global value loading ----- */

using load_fn = Value* (*)(ILoad* iload, USHORT chunkID, ValueLoader* vl);

// one to one correspondence between LoadableClassID and array loadable_classes entries in sceneio.cpp
enum LoadableClassID 
{
	Undefined_Chunk = 0,	Boolean_Chunk,			Ok_Chunk,
	Integer_Chunk,			Float_Chunk,			String_A_Chunk, 
	Name_A_Chunk,			Array_Chunk,			Point3Value_Chunk, 
	QuatValue_Chunk,		RayValue_Chunk,			AngAxisValue_Chunk,
	EulerAnglesValue_Chunk, Matrix3Value_Chunk,		Point2Value_Chunk,
	ColorValue_Chunk,		MSTime_Chunk,			MSInterval_Chunk,
	MAXWrapper_Chunk,		Unsupplied_Chunk,		Struct_Chunk,
	Point4Value_Chunk,		Empty_Chunk,			Integer64_Chunk,
	DoubleValue_Chunk,		String_W_Chunk,			Name_W_Chunk,
	NoValue_Chunk,			IntegerPtr_Chunk,		Box3Value_Chunk,
	MXSDictionaryValue_Chunk,	DataPair_Chunk,

	// add more here...

	HIGH_CLASS_CHUNK  // must be last
};


extern ScripterExport Value* load_value(ILoad* iload, ValueLoader* vload);
extern void save_persistent_callback_scripts(ISave* isave);
extern IOResult load_persistent_callback_script(ILoad* iload);
extern Tab<ValueLoader*> value_loaders;

#ifdef _DEBUG
  extern BOOL dump_load_postload_callback_order;
#endif

// post global load callback scheme, allows different loaders to 
// permit ::Load() fns to register a callback to clean-up a load.  
// Eg, Array loader gets such a callback from MAXWrapper::Load() which
// uses this to build the MAXWrapper at post-load time, after object pointers
// have been back-patched.

// ::Load()'s that need to specialize this to provide a callback
class ValueLoadCallback
{
public:
	virtual Value* post_load() { return &undefined; }  // return the cleaned-up value
};

// each loader specializes this and gives it to the ::Load()
class ValueLoader
{
public:
	virtual void register_callback(ValueLoadCallback* cb) { UNUSED_PARAM(cb); }
	virtual void call_back() { }
};

// A post load callback to process persistent value load callbacks
class ValueLoadPLCB : public PostLoadCallback 
{
public:
	ScripterExport void proc(ILoad *iload) override;
};

// callback script  (see MAXCallbacks.cpp)
class CallbackScript 
{ 
public:
	MSTR	script;		// callback script, script filename, or name of function
	Value*  code;		// cached compiled code or a function value wrapped in a ValueHolder
	Value*  id;			// script ID
	short	flags;		// flags
	const MAXScript::ScriptSource scriptSource; // the ScriptSource when the CallbackScript was created if specified as script or filename.
						// If specified as function, then scriptSource that will be used is built into the function itself.

	CallbackScript(const MSTR& iscript, MAXScript::ScriptSource iscriptSource, Value* iid, short iflags) : 
		script(iscript), code(nullptr), id(iid), flags(iflags), scriptSource(iscriptSource)
	{
	}
	CallbackScript(Value* ifunction, const MSTR& function_name, Value* iid, short iflags) : 
		script(function_name), code(ifunction), id(iid), flags(iflags), scriptSource(MAXScript::ScriptSource::NotSpecified)
	{
	}
};

#define MCB_SCRIPT_IS_FILE		0x0001
#define MCB_PERSISTENT			0x0002
#define MCB_HAS_ID				0x0004
#define MCB_INVALID				0x0008
#define MCB_SCENE_FILE_EMBEDDED	0x0010
#define MCB_PROCESSED			0x0020 // temp flag set when running callbacks - internal use only

extern Tab<CallbackScript*>* callback_scripts[];

