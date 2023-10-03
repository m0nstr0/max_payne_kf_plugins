//**************************************************************************/
// Copyright (c) 2016 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once
#include "../kernel/value.h"

applyable_class_debug_ok(MXSDictionaryValue)

class Array;

class MXSDictionaryValue : public Value
{
public:
	enum key_type { key_name, key_string, key_int};

	ScripterExport MXSDictionaryValue(key_type keyType = key_name);
	~MXSDictionaryValue() {}

	classof_methods(MXSDictionaryValue, Value);
	BOOL		_is_collection() override { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
#	define	is_dictionary(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MXSDictionaryValue))

	static Value* make(Value** arg_list, int count);

	void collect() override { delete this; }
	void gc_trace() override;
	void sprin1(CharStream* s) override;
	Value* deep_copy(HashTable* remapper) override;

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);

	// hashtable hash and compare fns for string keys
	static DWORD_PTR string_hash_fn(const void* key);
	static int dict_eq_fn_string(const void* key1, const void* key2);

	// hashtable hash and compare fns for integer keys
	static DWORD_PTR int_hash_fn(const void* key);
	static int dict_eq_fn_int(const void* key1, const void* key2);

	ScripterExport void put(Value* key, Value* val, bool putOnlyIfNew = false);
	ScripterExport void set(Value* key, Value* val);
	ScripterExport Value* get(Value* key);
	ScripterExport void remove(Value* key);
	ScripterExport bool has(Value* key);
	ScripterExport Array* getKeys(); // The returned Array* is not held by the dictionary, and therefore must be protected from gc
	ScripterExport void free();

	ScripterExport key_type getType() { return mKeyType; }
	ScripterExport int getCount();
	ScripterExport const HashTable* getHashTable();

	ScripterExport bool keyValueProperType(Value* key);
	ScripterExport static bool keyValueProperType(Value* key, key_type keyType);

	// built-in property accessors 
	def_property(count);
	def_property(type);
	def_property(keys);

	// helper function for mxs exposure - throws error if key not of proper type
	void type_check_key(Value* key, const MCHAR* where);

	/* ---------------------- MXSDictionaryValue primitives  --------------------- */
#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
#include "../protocols/MXSDictionaryValue.inl"
	use_generic(copy, "copy");
	use_generic(coerce, "coerce");
	use_generic(free, "free");

	Value* map(node_map& m) override;

	
protected:
	HashTable* mHashTable = nullptr;
	key_type mKeyType = key_name;
};
