/*	
 *		ObjectSets.h - ObjectSet classes for MAXScript
 *
 *			Copyright (c) John Wainwright 1996
 *
 */

#pragma once

#include "../kernel/value.h"
#include "../foundation/collection.h"
#include "../foundation/streams.h"

visible_class (Set)

class Set : public Value, public Collection
{
public:
			classof_methods (Set, Value);
	BOOL	_is_collection() override { return 1; }

#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
	def_generic  (coerce,   "coerce"); 
};

#ifndef DOXYGEN
visible_class_s (ObjectSet, Set)
#endif

class ObjectSet : public Set
{
protected:
				ObjectSet() { }
public:
	MSTR		set_name;
	BOOL		(*selector)(INode* node, int level, const void* arg);	// set selector function
	const void*		selector_arg;									// selector fn argument

				ObjectSet(const MCHAR* name, SClass_ID class_id);
				ObjectSet(const MCHAR* init_name, BOOL (*sel_fn)(INode*, int, const void*), const void* init_arg = nullptr);
	void		init(const MCHAR* name);

				classof_methods (ObjectSet, Set);
	static void	setup();
	const MCHAR*		name() { return set_name; }
	void		collect() override;
	void		sprin1(CharStream* s) override { s->printf(_M("$%s"), set_name.data()); }
	void		export_to_scripter() override;
#	define		is_objectset(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(ObjectSet))

	ScripterExport Value* map(node_map& m) override;
	ScripterExport Value* map_path(PathName* path, node_map& m) override;
	ScripterExport Value* find_first(BOOL (*test_fn)(INode* node, int level, const void* arg), const void* test_arg) override;
	ScripterExport Value* get_path(PathName* path) override;

#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
	def_generic  (get,		"get");   // indexed get (no put or append)
	def_property ( count );
	def_property ( center );
	def_property ( min );
	def_property ( max );
	def_property ( boundingBox );
};

class CurSelObjectSet : public ObjectSet
{
public:
				CurSelObjectSet(const MCHAR* name);

	void	collect() override;

	ScripterExport Value* map(node_map& m) override;

#ifndef DOXYGEN
#include "../macros/define_implementations.h"
#endif
	def_generic  (get,		"get");   // indexed get (no put or append)
	def_generic  (coerce,   "coerce"); 

	def_property ( count );
};

extern ObjectSet all_objects;
extern ObjectSet all_geometry;
extern ObjectSet all_lights;
extern ObjectSet all_cameras;
extern ObjectSet all_helpers;
extern ObjectSet all_shapes;
extern ObjectSet all_systems;
extern ObjectSet all_spacewarps;
extern CurSelObjectSet current_selection;

