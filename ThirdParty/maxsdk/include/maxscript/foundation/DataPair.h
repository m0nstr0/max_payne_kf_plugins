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

/* ------------------- DataPair class instance ----------------------- */

applyable_class_debug_ok (DataPair)

class DataPair : public Value
{
public:
	Value*		v1;
	Value*		v2;
	Value*		v1_name;
	Value*		v2_name;

	ScripterExport DataPair();
	ScripterExport DataPair(Value* v1, Value* v2, Value* v1_name = nullptr, Value* v2_name = nullptr);

	classof_methods(DataPair, Value);
#	define	is_datapair(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(DataPair))
	static Value* make(Value** arg_list, int count);

	void		collect() override { delete this; }
	void		sprin1(CharStream* s) override;
	void		gc_trace() override;
	Value*		deep_copy(HashTable* remapper) override;

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);

	use_generic ( get_props,	"getPropNames");
	use_generic ( copy,			"copy");
	Value*	get_property(Value** arg_list, int count) override;
	Value*	set_property(Value** arg_list, int count) override;
};

