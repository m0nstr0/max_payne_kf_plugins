/*		MSTime.h - the time family of classes for MAXScript
*
*		Copyright (c) John Wainwright, 1996
*		
*
*/

#pragma once

#include "../kernel/value.h"
#include "../../units.h"

/* ------------------------ Time ------------------------------ */

#ifndef DOXYGEN
visible_class_debug_ok (MSTime)
#endif

class MSTime : public Value
{
public:
	TimeValue	time;
#pragma push_macro("new")
#undef new
	ENABLE_STACK_ALLOCATE(MSTime);
#pragma pop_macro("new")
	MSTime (TimeValue t);
	static ScripterExport Value* intern(TimeValue t);

#	define		is_time(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MSTime))
	classof_methods (MSTime, Value);
	void		collect() override;
	void sprin1(CharStream* s) override;

#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
#	include "..\protocols\time.inl"
	def_generic  ( coerce,	"coerce");

	def_property ( ticks );
	def_property ( frame );
	def_property ( normalized );

	TimeValue	to_timevalue() override { return time; }
	float	    to_float() override { return (float)time / GetTicksPerFrame(); }
	double	    to_double() override { return static_cast<double>(time) / GetTicksPerFrame(); }
	int			to_int() override { return (int)time / GetTicksPerFrame(); }
	INT64       to_int64() override { return static_cast<INT64>(time / GetTicksPerFrame()); }
	INT_PTR     to_intptr() override { return static_cast<INT_PTR>(time / GetTicksPerFrame()); }
	void		to_fpvalue(FPValue& v) override { v.i = time; v.type = TYPE_TIMEVALUE; }

	Value*	widen_to(Value* arg, Value** arg_list) override;
	BOOL	comparable(Value* arg) override;

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ Interval ------------------------------ */

applyable_class_debug_ok (MSInterval)

class MSInterval : public Value
{
public:
	Interval	interval;
#pragma push_macro("new")
#undef new
	ENABLE_STACK_ALLOCATE(MSInterval);
#pragma pop_macro("new")

	MSInterval () {};
	ScripterExport MSInterval (Interval i);
	ScripterExport MSInterval (TimeValue s, TimeValue e);

#	define		is_interval(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MSInterval))
	classof_methods (MSInterval, Value);
	void		collect() override;
	void sprin1(CharStream* s) override;

#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
	def_property ( start );
	def_property ( end );

	Interval	to_interval() override { return interval; }
	void to_fpvalue(FPValue& v) override;

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

