/*    Numbers.h - the number family of classes - numbers for MAXScript
*
*    Copyright (c) John Wainwright, 1996
*    
*
*/

#pragma once

#include "../kernel/value.h"
#include "mxstime.h"

// forward declarations
// This file defines the following classes:
class Float;
class Double;
class Integer;
class Integer64;
class IntegerPtr;

#define FLOAT_CACHE_SIZE   1024  // must be power of 2
#define DOUBLE_CACHE_SIZE  512   //   "       "      "
#define INT_CACHE_SIZE     512   //   "       "      "
#define INT64_CACHE_SIZE   128   //   "       "      "
#define LOW_INT_RANGE      100

extern Float* float_cache[];
extern Double* double_cache[];
extern Integer* int_cache[];
extern Integer64* int64_cache[];

#ifndef DOXYGEN
visible_class_debug_ok (Number)
#endif

class Number : public Value
{
public:
#  define  is_integer_number(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Integer) || (v)->tag == class_tag(Integer64) || (v)->tag == class_tag(IntegerPtr))
// integer_type_check macro checks integer types validity for all Integer types
// For integer value number converted to int using value->to_int(); covers Integer, Inter64 and IntegerPtr
// When converted this way it is the preferred Value type checking macro over using type_check macro.
#  define  integer_type_check(v, where) if (!is_integer_number(v)) type_check(v, Integer, where)

#  define  is_float_number(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Float) || (v)->tag == class_tag(Double))
// float_type_check macro checks float types validity for all Floating types
// For floating value number converted to float using value->to_float(); covers for Float, Double
// When converted this way it is the preferred Value type checking macro over using type_check macro.
#  define  float_type_check(v, where) if (!is_float_number(v)) type_check(v, Float, where)

#  define  is_number(o) (is_integer_number(o) || is_float_number(o))

	classof_methods (Number, Value);

	static   Value* read(const MCHAR* str, bool heapAlloc = false);
	static   void setup();

#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
	def_generic( coerce, "coerce");
	def_generic( copy,   "copy");
};

	applyable_class_debug_ok (Float)
	applyable_class_debug_ok (Double)
	applyable_class_debug_ok (Integer)     // forward decls for float class 
	applyable_class_debug_ok (Integer64)            
	applyable_class_debug_ok (IntegerPtr)
#define  is_double(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Double))
#define  is_integer(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Integer))
#define  is_integer64(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Integer64))
#define  is_integerptr(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(IntegerPtr))

class Float : public Number
{
public:
	float value;
#pragma push_macro("new")
#undef new
	ENABLE_STACK_ALLOCATE(Float);
#pragma pop_macro("new")
	Float() { }
	ScripterExport Float(float init_val);

	static ScripterExport Value* intern(float init_val);
	static ScripterExport Value* heap_intern(float init_val);

	classof_methods (Float, Number);
#  define  is_float(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Float))
	void  collect() override;
	void  sprin1(CharStream* s) override;

	/* include all the protocol declarations */

#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#  include "..\protocols\math.inl"
#endif

	float    to_float() override { return value; }
	double   to_double() override { return static_cast<double>(value); }
	int      to_int() override;
	INT64    to_int64() override { return static_cast<INT64>(value); }
	INT_PTR  to_intptr() override
	{
#ifndef _WIN64
		return to_int();
#else
		return to_int64();
#endif
	}
	TimeValue to_timevalue() override { return (TimeValue)(value * GetTicksPerFrame()); }  // numbers used as times are in frames
	void  to_fpvalue(FPValue& v) override { v.f = to_float(); v.type = (ParamType2)TYPE_FLOAT; }

	Value*   widen_to(Value* arg, Value** arg_list) override;
	BOOL  comparable(Value* arg) override { return (is_number(arg) || is_time(arg)); }

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

class Double : public Number
{
public:
	double value;
#pragma push_macro("new")
#undef new
	ENABLE_STACK_ALLOCATE(Double);
#pragma pop_macro("new")
	Double() { }
	ScripterExport Double(double init_val);

	static ScripterExport Value* intern(double init_val);
	static ScripterExport Value* heap_intern(double init_val);

	classof_methods (Double, Number);
#  define  is_double(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Double))
	void  collect() override;
	void  sprin1(CharStream* s) override;

	/* include all the protocol declarations */

#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
#  include "..\protocols\math.inl"

	float    to_float() override { return static_cast<float>(value); }
	double   to_double() override { return value; }
	int      to_int() override;
	INT64    to_int64() override { return static_cast<INT64>(value); }
	INT_PTR  to_intptr() override
	{
#ifndef _WIN64
		return to_int();
#else
		return to_int64();
#endif
	}
	TimeValue to_timevalue() override { return (TimeValue)(value * GetTicksPerFrame()); }  // numbers used as times are in frames
	void  to_fpvalue(FPValue& v) override { v.dbl = to_double(); v.type = (ParamType2)TYPE_DOUBLE; }

	Value*   widen_to(Value* arg, Value** arg_list) override;
	BOOL  comparable(Value* arg) override { return (is_number(arg) || is_time(arg)); }

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};


class Integer : public Number
{
public:
	int value;
#pragma push_macro("new")
#undef new
	ENABLE_STACK_ALLOCATE(Integer);
#pragma pop_macro("new")
	Integer() { };
	ScripterExport Integer(int init_val);

	static  ScripterExport Value* intern(int init_val);
	static  ScripterExport Value* heap_intern(int init_val);

	classof_methods (Integer, Number);
#  define  is_int(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Integer))
	void  collect() override;
	void  sprin1(CharStream* s) override;

	/* include all the protocol declarations */

#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
#  include "..\protocols\math.inl"

	float    to_float() override { return static_cast<float>(value); }
	double   to_double() override { return static_cast<double>(value); }
	int      to_int() override { return value; }
	INT_PTR  to_intptr() override { return static_cast<INT_PTR>(value); }
	INT64    to_int64() override { return static_cast<INT64>(value); }
	TimeValue to_timevalue() override { return (TimeValue)(value * GetTicksPerFrame()); }  // numbers used as times are in frames
	void  to_fpvalue(FPValue& v) override { v.i = to_int(); v.type = (ParamType2)TYPE_INT; }

	BOOL      to_bool() override { return value != 0; }

	Value*   widen_to(Value* arg, Value** arg_list) override;
	BOOL  comparable(Value* arg) override { return (is_number(arg) || is_time(arg)); }
	BOOL	special_case_compare(Value* arg, bool &specialCaseCompareResult) override;

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

class Integer64 : public Number     
{
public:
	INT64 value;
#pragma push_macro("new")
#undef new
	ENABLE_STACK_ALLOCATE(Integer64);
#pragma pop_macro("new")
	Integer64() { };
	ScripterExport Integer64(INT64 init_val);

	static  ScripterExport Value* intern(INT64 init_val);
	static  ScripterExport Value* heap_intern(INT64 init_val);

	classof_methods (Integer64, Number);
#  define  is_int64(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Integer64))
	void  collect() override;
	void  sprin1(CharStream* s) override;

	/* include all the protocol declarations */

#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
#  include "..\protocols\math.inl"

	float    to_float() override { return static_cast<float>(value); }
	double   to_double() override { return static_cast<double>(value); }
	int      to_int() override { return static_cast<int>(value); }
	INT_PTR  to_intptr() override { return static_cast<INT_PTR>(value); }
	INT64    to_int64() override { return value; }
	TimeValue to_timevalue() override { return (TimeValue)(value * GetTicksPerFrame()); }
	void  to_fpvalue(FPValue& v) override { v.i64 = to_int64(); v.type = (ParamType2)TYPE_INT64; }

	BOOL      to_bool() override { return value != 0; }

	Value*   widen_to(Value* arg, Value** arg_list) override;
	BOOL  comparable(Value* arg) override { return (is_number(arg) || is_time(arg)); }
	BOOL	special_case_compare(Value* arg, bool &specialCaseCompareResult) override;

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/*
Used to store temporary values which cannot/should not be streamed out.  This includes
HWND, pointers, etc.  Everything that changes size between Win32 and Win64, in other
words.
*/
class IntegerPtr : public Number
{
public:
	INT_PTR value;
#pragma push_macro("new")
#undef new
	ENABLE_STACK_ALLOCATE(IntegerPtr);
#pragma pop_macro("new")
	IntegerPtr() { };
	ScripterExport IntegerPtr(INT_PTR init_val);

	static  ScripterExport Value* intern(INT_PTR init_val);
	static  ScripterExport Value* heap_intern(INT_PTR init_val);

	classof_methods (IntegerPtr, Number);
#  define  is_intptr(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(IntegerPtr))
	void  collect() override;
	void  sprin1(CharStream* s) override;

	/* include all the protocol declarations */

#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
#  include"..\protocols\math.inl"

	float    to_float() override  { return static_cast<float>(value); }
	double   to_double() override { return static_cast<double>(value); }
	int      to_int() override    { return static_cast<int>(value); }
	INT_PTR  to_intptr() override { return value; }
	INT64    to_int64() override  { return static_cast<INT64>(value); }
	TimeValue to_timevalue() override { return (TimeValue)(value * GetTicksPerFrame()); }
	void  to_fpvalue(FPValue& v) override { v.intptr = to_intptr(); v.type = (ParamType2)TYPE_INTPTR; }

	BOOL      to_bool() override { return value != 0; }

	Value*   widen_to(Value* arg, Value** arg_list) override;
	BOOL  comparable(Value* arg) override { return (is_number(arg) || is_time(arg)); }
	BOOL	special_case_compare(Value* arg, bool &specialCaseCompareResult) override;

	// scene I/O -- throws exceptions (we're not supposed to load/save IntegerPtr)
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};


