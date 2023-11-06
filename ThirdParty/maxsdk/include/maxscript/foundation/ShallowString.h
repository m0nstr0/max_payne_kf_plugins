//**************************************************************************/
// Copyright (c) 2020 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/


/*
 *	ShallowString.h - holds a non-owned pointer to string
 *	Instances of this class are used internally for passing the case-insensitive name as a keyword arg to get_property and set_property.
 *  Instances stored on the stack can not outlive the source of the MCHAR*
 *  Instances stored in the heap create a copy of the string and delete that copy on deletion
 */

#pragma once

#include "../foundation/streams.h"

#ifndef DOXYGEN
invisible_class(ShallowString)
#endif

class ShallowString : public Value
{
	const MCHAR* m_string;

public:

#pragma push_macro("new")
#undef new
	ENABLE_STACK_ALLOCATE(ShallowString);
#pragma pop_macro("new")

	ScripterExport ShallowString(const MCHAR *init_string);
	~ShallowString();

	classof_methods (ShallowString, Value);
#	define		is_ShallowString(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(ShallowString))
	void		collect() override;
	void		sprin1(CharStream* s) override;

	const MCHAR*	to_string() override;
	MSTR			to_mstr() override;


	ScripterExport ShallowString* create_heap_copy();  // called when migrating to heap.
};
