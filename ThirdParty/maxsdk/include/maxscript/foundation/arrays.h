/*		Arrays.h - the Array family of classes for MAXScript
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#pragma once

#include "../ScripterExport.h"
#include "../kernel/value.h"
#include "collection.h"
#include "../../geom/bitarray.h"

/* ------------------------ Array ------------------------------ */

applyable_class_debug_ok(Array)

class Array : public Value, public Collection
{
public:
	int			volatile size;			// array size
	int			data_size;				// allocated array buffer size (in Value*'s)
	Value**		volatile data;			// the array elements (uninitialized are set to undefined)

	ScripterExport	static CRITICAL_SECTION array_update;	// for syncing array updates

	ScripterExport	 Array(int init_size);
	~Array();

				classof_methods (Array, Value);

	static Value* make(Value** arg_list, int count); // does not call eval() on args, used by Maker which does its own eval() calls
	static Value* make_eval_args(Value** arg_list, int count); // calls eval() on args, used by ArrayClass::apply which does not do its own eval() calls
	static void	setup();
	// access ith array entry.
	ScripterExport Value*& operator[](const int i) const;  // index is 0-based
	
#	define		is_array(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Array))
	BOOL		_is_collection() override { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
	BOOL		_is_selection() override { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
	void		gc_trace() override;
	void		collect() override;
	void sprin1(CharStream* s) override;

	// operations
#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
#	include "../protocols/arrays.inl"
	use_generic( plus,		"+" );
	use_generic( copy,		"copy" );
	use_generic( coerce,	"coerce");
	use_generic( free,		"free");

	Value* map(node_map& m) override;
	Value* map_path(PathName* path, node_map& m) override;
	Value* find_first(BOOL (*test_fn)(INode* node, int level, const void* arg), const void* test_arg) override;
	Value* get_path(PathName* path) override;

	// built-in property accessors 
	def_property ( count );

	ScripterExport Value* append(Value*);
	ScripterExport Value* join(Value*);
	ScripterExport Value* sort();
	ScripterExport Value* push(Value*);
	ScripterExport Value* drop();
	ScripterExport Value* get(int index); // index is 1-based
	ScripterExport BOOL	  deep_eq(Value* other);

	Value* deep_copy(HashTable* remapper) override;

	// get selection iterator for an array
	SelectionIterator* selection_iterator() override;

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);

	void	 to_fpvalue(FPValue& v) override;
};

/* ------------------------ BitArray ------------------------------ */

applyable_class_debug_ok(BitArrayValue)

class BitArrayValue : public Value
{
public:
	BitArray	bits;		// the bits

	ScripterExport BitArrayValue();
	ScripterExport BitArrayValue(const BitArray& b);
	ScripterExport BitArrayValue(int count);

				classof_methods (BitArrayValue, Value);

	static Value* make(Value** arg_list, int count);

#	define		is_BitArrayValue(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(BitArrayValue))
//	BOOL		_is_collection() { return 1; }
	BOOL		_is_selection() override { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
	void		collect() override;
	void		sprin1(CharStream* s) override;
	ScripterExport void can_hold(int index); // resize if need to hold specified index

	// operations
#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
#	include "../protocols/arrays.inl"
	use_generic( plus, "+" );
	use_generic( minus, "-" );
	def_generic( uminus, "u-");
	use_generic( times, "*" );
	use_generic( copy, "copy" );
	use_generic( coerce,	"coerce");
	use_generic( free,		"free");

	Value* map(node_map& m) override;

	// built-in property accessors
	def_property ( count );
	def_property ( numberSet );
	def_property ( isEmpty );

	SelectionIterator* selection_iterator() override;

	BitArray&	to_bitarray() override { return bits; }
	void	    to_fpvalue(FPValue& v) override { v.bits = &bits; v.type = TYPE_BITARRAY; }
#	define		is_bitarray(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(BitArrayValue))

};

