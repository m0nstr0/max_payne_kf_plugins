/*		Value.h - metaclass system  MAXScript values
 *
 *	All MAXScript-specific C++ objects are subclasses of a single root class, Value, 
 *  and allocated & automatically freed in a specially maintained heap.  There is also
 *  a metaclass system to provide a runtime type calculus for the scripter.  Value subclasses
 *  are divided into those that are scripter-visible, (ie, may wind up as objects that the
 *  scripter may pass around or store in variables, etc.), and those that are entirely
 *  internal to the scripter operation (such as thunks, etc.).  The scripter-visible
 *  classes (the majority) are represented in the metasystem by instances of separate
 *  metaclasses.  The metaclasses are all subclasses of ValueMetaClass, the metaclass of
 *  a class X is named XClass and its sole instance is X_class. The class instances are
 *  made visible in globals (usually) named X.  
 *
 *  Each Value instance has a tag word that either contains a pointer to the instance's
 *  class instance (in the case of scripter-visible classes) or the reserved value INTERNAL_CLASS_TAG.
 *  This value is used in performing runtimne type tests and for yielding results to classOf 
 *  methods.
 *
 *  The metaclass, its instance and some of the class calculus methods are usually defined via
 *  a bunch of macros defined here (see visible_class, visible_class_instance, etc.)
 *
 *  Some of the classes are can be instanced directly as literals in a script, such as strings, 
 *  Point3s, arrays, etc.  Some others are instantiable directly by applying the class value
 *  to a set of initializing parameters, ie, using the class as a function in a function call,
 *	for example, ray, quat, interval, etc.  These are defined via a variant macro: applyable_class().
 *  A special case of this is provided in the MAXWrapper subsytem for creatable MAX objects, such as
 *  boxes, lights, camera, etc..  These are represnted by instances of the class MAXClass, and again, thses
 *  instances are exposed in globals to be applied to creation paramters.  These instances
 *  contain a lot of property metadata and are defined in MAX_classes.cpp.  See MAXObject.h for more
 *  info.
 *	
 *		Copyright (c) John Wainwright, 1996
 *
 */

#pragma once

#include "../ScripterExport.h"
#include "collectable.h"
#include "interupts.h"
#include "MAXScript_TLS.h"
#include "../macros/value_locals.h"
#include "MaxscriptTypedefs.h"
#include "../../geom/matrix3.h"
#include "../../geom/box2.h"
#include "../../geom/acolor.h"
#include "../../geom/quat.h"
#include "../../interval.h"
#include "../../ifnpub.h"

// forward declarations
class BitArray;
class CharStream;
class Name;
class PathName;
class Undefined;
class UserProp;
class UserGeneric;
class CallContext;
class ValueMetaClass;
struct node_map;
class Mtl;
class Texmap;
class MtlBase;
class Modifier;
class Control;
class Atmospheric;
class Effect;
class IMultiPassCameraEffect;
class ShadowType;
class FPInterfaceDesc;
class FilterKernel;
class ITrackViewNode;
class NURBSIndependentPoint;
class NURBSPoint;
class NURBSObject;
class NURBSControlVertex;
class NURBSCurve;
class NURBSCVCurve;
class NURBSSurface;
class NURBSTexturePoint;
class NURBSSet;
class ReferenceTarget;
class Mesh;
class BezierShape;
class Thunk;
class Renderer;
class NURBSTextureSurface;
class NURBSDisplay;
class TessApprox;
class SelectionIterator;
class HashTable;

//! \cond DOXYGEN_IGNORE
#include "../macros/define_external_functions.h"
#	include "../protocols/corenames.inl"
//! \endcond

// forward declarations...

extern ScripterExport Undefined undefined;  // corresponds to the mxs system global variable 'undefined'
extern ScripterExport bool dontThrowAccessorError; // used internally by is_prop_cf in order to not throw exception when testing to see if name is used as a property
ScripterExport Value* _get_key_arg_or_default(Value** arg_list, int count, Value* key_name, Value* def);

// the root MAXScript class
class Value : public Collectable
{
private:
	ScripterExport static Matrix3  s_error_matrix;
	ScripterExport static Box2  s_error_box2;
public:
	#pragma warning(push)
	#pragma warning(disable:4100)
	ValueMetaClass* tag;		// runtime type tag; filled in by subclasses

	ScripterExport virtual BOOL	is_kind_of(ValueMetaClass* c);
	ScripterExport virtual ValueMetaClass* local_base_class(); // local base class in this class's plug-in
	virtual Value*	eval() { check_interrupts(); return this; }
	virtual Value*	eval_no_wrapper() { check_interrupts(); return this; }
	ScripterExport virtual Value*  apply(Value** arglist, int count, CallContext* cc=nullptr);
	ScripterExport virtual Value*  apply_no_alloc_frame(Value** arglist, int count, CallContext* cc=nullptr);
	virtual void	export_to_scripter() { }

	virtual Value*  map(node_map& m) { unimplemented(_M("map"), this) ; return this; }
	virtual Value*	map_path(PathName* path, node_map& m) { unimplemented(_M("map_path"), this) ; return this; }
	virtual Value*	find_first(BOOL (*test_fn)(INode* node, int level, const void* arg), const void* test_arg) { unimplemented(_M("find_first"), this) ; return this; }
	virtual Value*	get_path(PathName* path) { unimplemented(_M("get"), this) ; return this; }

	ScripterExport virtual void sprin1(CharStream* stream);
	ScripterExport virtual void	sprint(CharStream* stream);

	virtual void	prin1() { sprin1(thread_local(current_stdout)); }
	virtual void	print() { sprint(thread_local(current_stdout)); }
	
	/* include all the protocol declarations */

//! \cond DOXYGEN_IGNORE
#include "../macros/define_abstract_functions.h"
#	include "../protocols/math.inl"
#	include "../protocols/vector.inl"
#	include "../protocols/matrix.inl"
#	include "../protocols/quat.inl"
#	include "../protocols/arrays.inl"
#	include "../protocols/streams.inl"
#	include "../protocols/strings.inl"
#	include "../protocols/time.inl"
#	include "../protocols/color.inl"
#	include "../protocols/node.inl"
#	include "../protocols/controller.inl"
#	include "../protocols/primitives.inl"
#	include "../protocols/generics.inl"
#	include "../protocols/bitmaps.inl"
#	include "../protocols/textures.inl"
#	include "../protocols/atmospherics.inl"
#	// Moved to ../maxwrapper/mxsnurbs.h into class NURBSObjectValue
#	include "../protocols/cameratracker.inl"
#	include "../protocols/bigmatrix.inl"
#	include	"../protocols/box.inl"
#	include "../protocols/physiqueblend.inl"
#	include "../protocols/physiquemod.inl"
#	include	"../protocols/biped.inl"
#	include "../protocols/notekey.inl"
#	include "../protocols/xrefs.inl"
//! \endcond

	virtual Class_ID get_max_class_id() { return Class_ID(0, 0); }
	virtual Value* delete_vf(Value** arglist, int arg_count) { ABSTRACT_FUNCTION(_M("delete"), this, Value*); }	  
	virtual Value* clearSelection_vf(Value** arglist, int arg_count) { ABSTRACT_FUNCTION(_M("clearSelection"), this, Value*); }	  

//! \cond DOXYGEN_IGNORE
#undef def_generic
#define def_generic(fn, name) ScripterExport virtual Value* fn##_vf(Value** arglist, int arg_count);
#	include "../protocols/kernel.inl"
//! \endcond
	
	virtual float		to_float() { ABSTRACT_CONVERTER(float, Float); }
	virtual double		to_double() { ABSTRACT_CONVERTER(double, Double); }
	virtual const MCHAR*		to_string() { ABSTRACT_CONVERTER(const MCHAR*, String); }
	virtual MSTR		to_mstr() { return MSTR(to_string()); }
	virtual MSTR		to_filename() { ABSTRACT_CONVERTER(const MCHAR*, FileName); }
	virtual int			to_int() { ABSTRACT_CONVERTER(int, Integer); }
	virtual INT64		to_int64() { ABSTRACT_CONVERTER(INT64, Integer64); }	
	virtual INT_PTR		to_intptr() { ABSTRACT_CONVERTER(INT_PTR, IntegerPtr); }	
	virtual BOOL		to_bool() { ABSTRACT_CONVERTER(BOOL, Boolean); }
	virtual BitArray&	to_bitarray() { throw ConversionError (this, _M("BitArray")); return *(BitArray*)nullptr; }
	virtual Point4		to_point4() { ABSTRACT_CONVERTER(Point4, Point4); }
	virtual Point3		to_point3() { ABSTRACT_CONVERTER(Point3, Point3); }
	virtual Point2		to_point2() { ABSTRACT_CONVERTER(Point2, Point2); }
	virtual AColor		to_acolor() { throw ConversionError (this, _M("Color")); return AColor(0,0,0); }
	virtual COLORREF	to_colorref() { throw ConversionError (this, _M("Color")); return RGB(0,0,0); }
	virtual INode*		to_node() { ABSTRACT_CONVERTER(INode*, <node>); }
	virtual Ray			to_ray() { throw ConversionError (this, _M("Ray")); return Ray(); }
	virtual Interval	to_interval() { throw ConversionError (this, _M("Interval")); return Interval();  }
	virtual Quat		to_quat() { throw ConversionError (this, _M("Quaternion")); return Quat();  }
	virtual AngAxis		to_angaxis() { throw ConversionError (this, _M("AngleAxis")); return AngAxis();  }
	virtual Matrix3&	to_matrix3() { throw ConversionError (this, _M("Matrix")); return s_error_matrix;  }
	virtual Box3		to_box3() { throw ConversionError (this, _M("Box3")); return Box3();  }
	virtual float*		to_eulerangles() { ABSTRACT_CONVERTER(float*, Float); }
	virtual Mtl*		to_mtl() { ABSTRACT_CONVERTER(Mtl*, Material); }
	virtual Texmap*		to_texmap() { ABSTRACT_CONVERTER(Texmap*, TextureMap); }
	virtual MtlBase*	to_mtlbase() { ABSTRACT_CONVERTER(MtlBase*, MtlBase); }
	virtual Modifier*	to_modifier() { ABSTRACT_CONVERTER(Modifier*, Modifier); }
	virtual TimeValue	to_timevalue() { ABSTRACT_CONVERTER(TimeValue, Time); }
	virtual Control*	to_controller() { ABSTRACT_CONVERTER(Control*, Controller); }
	virtual Atmospheric* to_atmospheric() { ABSTRACT_CONVERTER(Atmospheric*, Atmospheric); }
	virtual Effect*		to_effect() { ABSTRACT_CONVERTER(Effect*, Effect); }						// RK: Added this
	virtual IMultiPassCameraEffect*	to_mpassCamEffect() { ABSTRACT_CONVERTER(IMultiPassCameraEffect*, Effect); }	// LAM: Added this
	virtual ShadowType*	to_shadowtype() { ABSTRACT_CONVERTER(ShadowType*, ShadowType); }			// RK: Added this
	virtual FilterKernel*	to_filter() { ABSTRACT_CONVERTER(FilterKernel*, FilterKernel); }		// RK: Added this
	virtual INode*		to_rootnode() { ABSTRACT_CONVERTER(INode*, <root>); }						// RK: Added this
	virtual ITrackViewNode* to_trackviewnode() { ABSTRACT_CONVERTER(ITrackViewNode*, TrackViewNode); }
	virtual NURBSIndependentPoint* to_nurbsindependentpoint() { throw ConversionError (this, _M("NURBSIndependentPoint")); return (NURBSIndependentPoint*)nullptr;  }
	virtual NURBSPoint*	to_nurbspoint() { throw ConversionError (this, _M("NURBSPoint")); return (NURBSPoint*)nullptr;  }
	virtual NURBSObject* to_nurbsobject() { throw ConversionError (this, _M("NURBSObject")); return (NURBSObject*)nullptr;  }
	virtual NURBSControlVertex* to_nurbscontrolvertex() { throw ConversionError (this, _M("NURBSControlVertex")); return (NURBSControlVertex*)nullptr;  }
	virtual NURBSCurve* to_nurbscurve() { throw ConversionError (this, _M("NURBSCurve")); return (NURBSCurve*)nullptr;  }
	virtual NURBSCVCurve* to_nurbscvcurve() { throw ConversionError (this, _M("NURBSCVCurve")); return (NURBSCVCurve*)nullptr;  }
	virtual NURBSSurface* to_nurbssurface() { throw ConversionError (this, _M("NURBSSurface")); return (NURBSSurface*)nullptr;  }
	virtual NURBSTexturePoint* to_nurbstexturepoint() { throw ConversionError (this, _M("NURBSTexturePoint")); return (NURBSTexturePoint*)nullptr;  }
	virtual NURBSSet*	to_nurbsset() { throw ConversionError (this, _M("NURBSSet")); return (NURBSSet*)nullptr;  }
	virtual ReferenceTarget* to_reftarg() { ABSTRACT_CONVERTER(ReferenceTarget*, MaxObject); }
	virtual Mesh*		to_mesh() { ABSTRACT_CONVERTER(Mesh*, Mesh); }
	virtual BezierShape*	to_beziershape() { ABSTRACT_CONVERTER(BezierShape*, BezierShape); }
	virtual Thunk*		to_thunk() { ABSTRACT_CONVERTER(Thunk*, &-reference); }
	virtual void		to_fpvalue(FPValue& v) { throw ConversionError (this, _M("FPValue")); }

	virtual Renderer*	to_renderer() { ABSTRACT_CONVERTER(Renderer*, Renderer); }	// LAM: Added this 9/15/01

	virtual Box2&	to_box2() { throw ConversionError (this, _M("Box2")); return s_error_box2;  }
	virtual NURBSTextureSurface* to_nurbstexturesurface() { throw ConversionError (this, _M("NURBSTextureSurface")); return (NURBSTextureSurface*)nullptr;  }
	virtual NURBSDisplay* to_nurbsdisplay() { throw ConversionError (this, _M("NURBSDisplay")); return (NURBSDisplay*)nullptr;  }
	virtual TessApprox*	 to_tessapprox() { throw ConversionError (this, _M("TessApprox")); return (TessApprox*)nullptr;  }

	virtual Value*	widen_to(Value* arg, Value** arg_list) { ABSTRACT_WIDENER(arg); }
	virtual BOOL	comparable(Value* arg) { return (tag == arg->tag); }
	// if the following method is defined and returns TRUE, the value returned in specialCaseCompareResult is used
	// by def_eq_op and def_ne_op to determine if the values are equal. See these implementations in 
	// maxsdk\include\maxscript\macros\define_implementations.h
	virtual BOOL	special_case_compare(Value* arg, bool &specialCaseCompareResult) { return FALSE; }

	// The deep_copy method is used when creating deep copies of values. This method needs to be implemented for value classes that 
	// hold a collection of values, and when creating a deep copy of the class instance deep copies of those values also need to be made.
	// For example, this method is implemented for Array, Struct, DataPair, and MXSDictionaryValue.
	// The remapper argument is used to ensure that only a single deep copy of a value is made. So, for example, if you have an
	// array of #(p3, p3), where p3 is a Point3 value, the resulting array would contain #(<deep copied p3>, <deep copied p3>), where the
	// two elements are the same value.
	// Example implementation:
	// Value* DataPair::deep_copy(HashTable* remapper)
	// {
	//   DbgAssert(remapper);
	//   MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
	//   one_typed_value_local_tls(DataPair* valueCopy);  // protect new value being created from garbage collection
	//   if (remapper)
	//   {
	//     vl.valueCopy = (DataPair*)remapper->get(this);  // see whether a deep copy of this value was already made
	//     if (vl.valueCopy)
	//       return_value_tls(vl.valueCopy);  // yes, return that value
	//   }
	//   vl.valueCopy = new DataPair();  // no, create new value
	//   if (remapper)
	//     remapper->put(this, vl.valueCopy);  // add to remapper. This must be done before deep copying contents
	//   vl.valueCopy->v1 = heap_ptr(v1->deep_copy(remapper)); // deep copy value 1
	//   vl.valueCopy->v2 = heap_ptr(v2->deep_copy(remapper)); // deep copy value 2
	//   vl.valueCopy->v1_name = v1_name; // v1_name and v2_name are name values, name values are non-mutable and 
	//   vl.valueCopy->v2_name = v2_name; // copying a name value just returns the same value
	//   return_value_tls(vl.valueCopy);  // return the new value
	// }
	ScripterExport virtual Value* deep_copy(HashTable* remapper);

	virtual BOOL	is_const() { return FALSE; }

	// get_property and set_property are implemented by derived class to get/set its properties. Derived class implementations
	// should call method on parent class if not handled by the class. 
	ScripterExport virtual Value*	get_property(Value** arg_list, int count);
	ScripterExport virtual Value*	set_property(Value** arg_list, int count);

	// _get_property looks for property accessor for property, and if found calls that. If not found, calls get_property.
	// prop will be a Name value
	ScripterExport		   Value*	_get_property(Value* prop);

	// _set_property looks for property accessor for property, and if found calls that. If not found, calls set_property
	// This method is virtual and so can be overridden by derived classes
	// prop will be a Name value and val is the Value to assign to that property.
	ScripterExport virtual Value*	_set_property(Value* prop, Value* val);

	virtual Value*	get_container_property(Value* prop, Value* cur_prop) { if (!dontThrowAccessorError) throw AccessorError (cur_prop, prop); return nullptr; }
	virtual Value*	set_container_property(Value* prop, Value* val, Value* cur_prop) { throw AccessorError (cur_prop, prop); return nullptr;}

	// polymorphic default type predicates - abstracted over by is_x(v) macros as needed
	virtual BOOL	_is_collection() { return FALSE; }
	virtual BOOL	_is_charstream() { return FALSE; }
	virtual BOOL	_is_rolloutcontrol() { return FALSE; }
	virtual BOOL	_is_rolloutthunk() { return FALSE; }
	virtual BOOL	_is_function()	 { return FALSE; }
	virtual BOOL	_is_selection()	 { return FALSE; }
	virtual BOOL	_is_thunk()		{ return FALSE; }
	virtual BOOL	_is_indirect_thunk() { return FALSE; }

	// yield selection set iterator if you can
	virtual SelectionIterator* selection_iterator() { throw RuntimeError (_M("Operation requires a selection (Array or BitArray)")); return nullptr; }

	// scene persistence functions
	ScripterExport virtual IOResult Save(ISave* isave);
	// the Load fn is a static method on loadbale classes, see SceneIO.cpp & .h and each loadable class

	// called during MAX exit to have all MAXScript-side refs dropped (main implementation in MAXWrapper)
	virtual void drop_MAX_refs() { }

	// access ID'd FPInterface if supported
	virtual BaseInterface* GetInterface(Interface_ID id) { return nullptr; }

	// stack allocation routines
	ScripterExport Value* make_heap_permanent();
	ScripterExport Value* make_heap_static() { Value* v = make_heap_permanent(); v->flags |= GC_STATIC; return v; }

	ScripterExport Value* get_heap_ptr() { if (!has_heap_copy()) return migrate_to_heap(); return is_on_stack() ? get_stack_heap_ptr() : this; } 
	ScripterExport Value* get_stack_heap_ptr() { return (Value*)next; }
	ScripterExport Value* migrate_to_heap();
	ScripterExport Value* get_live_ptr() { return is_on_stack() && has_heap_copy() ? get_stack_heap_ptr() : this; } 

	// Case sensitive object property and method naming
	// If an object supports case sensitive object property and method names, the object wrapper class must implement this method
	// and return TRUE. When TRUE, the case sensitive name as specified in the script is passed to the get/set methods through the argument 
	// list as a keyword argument. The keyword is n_propCaseSensitiveName and keyword value will either be null or a ShallowString instance that 
	// holds a pointer to the case sensitive name.
	// The object wrapper class can also override get_case_sensitive_property and set_case_sensitive_property if this method returns true
	// for better performance.
	virtual BOOL object_supports_case_sensitive_names() { return FALSE; }

	// _get_property, _set_property, get_case_sensitive_property, set_case_sensitive_property, invoke_getter_case_sensitive_property and
	// invoke_setter_case_sensitive_property allow proper property and method access when using a case sensitive language such as python.
	// The functions receive the arg_list and a case sensitive version of the property name. These methods build a new arg_list adding 
	// a ShallowString instance that holds a pointer to the case sensitive name as a keyword argument, keyword n_propCaseSensitiveName.
	// Most classes that implement get_property and set_property will ignore the keyword arguments. Those that do want the
	// case sensitive property name can access it via the keyword argument.

	// Calls _get_property(Value* prop) if prop_name_as_parsed is null or object_supports_case_sensitive_names() returns false. 
	// Otherwise, calls the property accessor associated with the property if exists, passing the case sensitive property name as a keyword argument.
	// Otherwise, calls get_property, passing the case sensitive property name as a keyword argument. 
	// Argument prop will be a Name value and prop_name_as_parsed will be a ShallowString value or null
	ScripterExport Value*	_get_property(Value* prop, Value* prop_name_as_parsed);

	// Calls _set_property(Value* prop, Value* val) if prop_name_as_parsed is null or object_supports_case_sensitive_names() returns false. 
	// Otherwise, calls the property accessor associated with the property if exists, passing the case sensitive property name as a keyword argument.
	// Otherwise, calls set_property, passing the case sensitive property name as a keyword argument. 
	// Argument prop will be a Name value, val is the Value to assign to that property, and prop_name_as_parsed will be a ShallowString value or null
	ScripterExport Value*	_set_property(Value* prop, Value* val, Value* prop_name_as_parsed);

	// If object_supports_case_sensitive_names() returns true, adds the case_sensitive_name to the arg list as a keyword argument and calls 
	// get/set_property(Value** arg_list, int count).
	// Otherwise, calls get/set_property(Value** arg_list, int count)
	// Note that if an object wrapper class supports case sensitive object property and method names, the class can override this
	// method for improved performance. In some cases get/set_property(Value** arg_list, int count) will still be called with 
	// the case sensitive property name as a keyword argument
	ScripterExport virtual Value*	get_case_sensitive_property(Value** arg_list, int count, const TCHAR* case_sensitive_name);
	ScripterExport virtual Value*	set_case_sensitive_property(Value** arg_list, int count, const TCHAR* case_sensitive_name);

	// If object_supports_case_sensitive_names() returns true, adds the case_sensitive_name and nested_prop_case_sensitive_name to the arg 
	// list as keyword arguments and calls the getter/setter method
	// Otherwise, calls the getter/setter method
	ScripterExport Value*	invoke_getter_case_sensitive_property(Value* target, getter_vf getter, Value** arg_list, int count, const TCHAR* case_sensitive_name, const TCHAR* nested_prop_case_sensitive_name);
	ScripterExport Value*	invoke_setter_case_sensitive_property(Value* target, setter_vf setter, Value** arg_list, int count, const TCHAR* case_sensitive_name, const TCHAR* nested_prop_case_sensitive_name);

protected:
	ScripterExport static Value* keyarg_marker_value; // needed for compilation of get/set_##p1##_##p2 and get/set_nested_##p1

#pragma warning(pop)
};

inline Value* heap_ptr(Value* v) { return v ? v->get_heap_ptr() : nullptr; }	// ensure v is in heap, migrate if not, return heap pointer
inline Value* live_ptr(Value* v) { return  v->get_live_ptr(); }				// get live pointer, if on stack & migrated, heap copy is live

/* ---------- the base class for all metaclasses  ---------- */

class MetaClassClass;
extern ScripterExport MetaClassClass value_metaclass;  // the metaclass class

class ValueMetaClass : public Value
{
// Whether the generic functions and property setters of class instances can be called from debugger thread stored 
// in Collectable::flags3 - bit 0. Default is false.
public:
	const MCHAR*			name;
	UserProp*		user_props;		// additional, user defined property accessors
	short			uprop_count;
	UserGeneric*	user_gens;		//     "        "      "    generic fns
	short			ugen_count;
	Tab<FPInterfaceDesc*> prop_interfaces;	// static interfaces who methods appear as properties on instances of the class

					ValueMetaClass() { }
	ScripterExport	ValueMetaClass(const MCHAR* iname);
	ScripterExport	~ValueMetaClass();

	ScripterExport BOOL	is_kind_of(ValueMetaClass* c) override;
#	define		is_valueclass(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == (ValueMetaClass*)&value_metaclass)
	ScripterExport void sprin1(CharStream* s) override;
	ScripterExport void	export_to_scripter() override;
	ScripterExport void add_user_prop(const MCHAR* prop, value_cf getter, value_cf setter);
	ScripterExport void add_user_generic(const MCHAR *pName, value_cf fn);
	ScripterExport UserGeneric* find_user_gen(Value *pName);
	ScripterExport UserProp* find_user_prop(Value* prop);

	// static property interfaces
	ScripterExport void add_prop_interface(FPInterfaceDesc* fpd) { prop_interfaces.Append(1, &fpd); }
	ScripterExport int num_prop_interfaces() { return prop_interfaces.Count(); }
	ScripterExport FPInterfaceDesc* get_prop_interface(int i) { return prop_interfaces[i]; }
};
#define CHECK_ARG_COUNT(fn, w, g)	if ((w) != (g)) throw ArgCountError (_M(#fn), w, g)

#define classof_methods(_cls, _super)					\
	Value* classOf_vf(Value** arg_list, int count) override	\
	{													\
		UNUSED_PARAM(arg_list);							\
		CHECK_ARG_COUNT(classOf, 1, count + 1);			\
		return &_cls##_class;							\
	}													\
	Value* superClassOf_vf(Value** arg_list, int count) override	\
	{													\
		UNUSED_PARAM(arg_list);							\
		CHECK_ARG_COUNT(superClassOf, 1, count + 1);	\
		return &_super##_class;							\
	}													\
	Value* isKindOf_vf(Value** arg_list, int count) override	\
	{													\
		CHECK_ARG_COUNT(isKindOf, 2, count + 1);		\
		return (arg_list[0] == &_cls##_class) ?			\
			&true_value :								\
			_super::isKindOf_vf(arg_list, count);		\
	}													\
	BOOL is_kind_of(ValueMetaClass* c) override			\
	{													\
		return (c == &_cls##_class) ? 1					\
					: _super::is_kind_of(c);			\
	}

#define visible_class(_cls)												\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(const MCHAR* name) : ValueMetaClass (name) { }	\
		void		collect() override { delete this; }					\
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define visible_class_debug_ok(_cls)									\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(const MCHAR* name) : ValueMetaClass (name) { flags3 |= VALUE_FLAGBIT_0; }	\
		void		collect() override { delete this; }					\
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define visible_class_s(_cls, _super)									\
	class _cls##Class;													\
	extern ScripterExport _cls##Class _cls##_class;						\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(const MCHAR* name) : ValueMetaClass (name) { }	\
		void		collect() override { delete this; }					\
		Value* classOf_vf(Value** arg_list, int count) override			\
		{																\
			UNUSED_PARAM(arg_list);										\
			CHECK_ARG_COUNT(classOf, 1, count + 1);						\
			return &_super##_class;										\
		}																\
		Value* superClassOf_vf(Value** arg_list, int count) override	\
		{																\
			UNUSED_PARAM(arg_list);										\
			UNUSED_PARAM(count);										\
			return _super##_class.classOf_vf(nullptr, 0);				\
		}																\
		Value* isKindOf_vf(Value** arg_list, int count) override		\
		{																\
			CHECK_ARG_COUNT(isKindOf, 2, count + 1);					\
			Value* arg0 = arg_list[0];									\
			return (arg0 == &_cls##_class || arg0 == &_super##_class) ?	\
				&true_value :											\
				_super##_class.isKindOf_vf(arg_list, count);			\
		}																\
		BOOL is_kind_of(ValueMetaClass* c) override						\
		{																\
			return (c == &_cls##_class) ? 1								\
						: _super##_class.is_kind_of(c);					\
		}																\
	};

#define applyable_class(_cls)											\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(const MCHAR* name) : ValueMetaClass (name) { }\
		void		collect() override { delete this; }					\
		ScripterExport Value* apply(Value** arglist, int count, CallContext* cc = nullptr) override; \
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define applyable_class_debug_ok(_cls)									\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(const MCHAR* name) : ValueMetaClass (name) { flags3 |= VALUE_FLAGBIT_0; }\
		void		collect() override { delete this; }					\
		ScripterExport Value* apply(Value** arglist, int count, CallContext* cc = nullptr) override; \
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define applyable_class_s(_cls, _super)									\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(const MCHAR* name) : ValueMetaClass (name) { }\
		Value* classOf_vf(Value** arg_list, int count) override			\
		{																\
			UNUSED_PARAM(arg_list);										\
			CHECK_ARG_COUNT(classOf, 1, count + 1);						\
			return &_super##_class;										\
		}																\
		Value* superClassOf_vf(Value** arg_list, int count) override	\
		{																\
			UNUSED_PARAM(arg_list);										\
			UNUSED_PARAM(count);										\
			return _super##_class.classOf_vf(nullptr, 0);					\
		}																\
		void		collect() override { delete this; }					\
		ScripterExport Value* apply(Value** arglist, int count, CallContext* cc = nullptr) override; \
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define visible_class_instance(_cls, _name)				\
	ScripterExport _cls##Class _cls##_class (_M(_name));

#define invisible_class(_cls)											\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(const MCHAR* name) : ValueMetaClass (name) { }	\
		void		collect() override { delete this; }					\
		void		export_to_scripter() override { }					\
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define invisible_class_instance(_cls, _name)				\
	ScripterExport _cls##Class _cls##_class (_M(_name));


#define class_tag(_cls)				&_cls##_class

#define INTERNAL_CLASS_TAG				((ValueMetaClass*)0L)
#define INTERNAL_INDEX_THUNK_TAG		((ValueMetaClass*)1L)
#define INTERNAL_PROP_THUNK_TAG			((ValueMetaClass*)2L)
#define INTERNAL_LOCAL_THUNK_TAG		((ValueMetaClass*)3L)
#define INTERNAL_FREE_THUNK_TAG			((ValueMetaClass*)4L)
#define INTERNAL_RO_LOCAL_THUNK_TAG		((ValueMetaClass*)5L)
#define INTERNAL_CODE_TAG				((ValueMetaClass*)6L)
#define INTERNAL_SOURCEFILEWRAPPER_TAG	((ValueMetaClass*)7L)
#define INTERNAL_PIPE_TAG				((ValueMetaClass*)8L)
#define INTERNAL_TOOL_LOCAL_THUNK_TAG	((ValueMetaClass*)9L)
#define INTERNAL_GLOBAL_THUNK_TAG		((ValueMetaClass*)10L)
#define INTERNAL_CONST_GLOBAL_THUNK_TAG ((ValueMetaClass*)11L)
#define INTERNAL_SYS_GLOBAL_THUNK_TAG	((ValueMetaClass*)12L)
#define INTERNAL_PLUGIN_LOCAL_THUNK_TAG	((ValueMetaClass*)13L)
#define INTERNAL_PLUGIN_PARAM_THUNK_TAG	((ValueMetaClass*)14L)
#define INTERNAL_RCMENU_LOCAL_THUNK_TAG	((ValueMetaClass*)15L)
#define INTERNAL_STRUCT_MEM_THUNK_TAG	((ValueMetaClass*)16L)
#define INTERNAL_MSPLUGIN_TAG			((ValueMetaClass*)17L)
#define INTERNAL_STRUCT_TAG				((ValueMetaClass*)18L)
#define INTERNAL_MAKER_TAG				((ValueMetaClass*)19L)
#define INTERNAL_CODEBLOCK_LOCAL_TAG	((ValueMetaClass*)20L)
#define INTERNAL_CODEBLOCK_TAG			((ValueMetaClass*)21L)
#define INTERNAL_THUNK_REF_TAG			((ValueMetaClass*)22L)
#define INTERNAL_THUNK_DEREF_TAG		((ValueMetaClass*)23L)
#define INTERNAL_STRUCT_METHOD_TAG		((ValueMetaClass*)24L) // LAM - defect 307069
#define INTERNAL_MSPLUGIN_METHOD_TAG	((ValueMetaClass*)25L) // LAM - 9/6/02 - defect 291499
#define INTERNAL_CONTEXT_THUNK_TAG		((ValueMetaClass*)26L) // LAM - 2/8/05
#define INTERNAL_OWNER_THUNK_TAG		((ValueMetaClass*)27L) // LAM - 2/28/05
#define INTERNAL_RCMENU_ITEM_THUNK_TAG	((ValueMetaClass*)28L) // LAM - 3/21/05
#define INTERNAL_STANDINMSPLUGINCLASS_TAG	((ValueMetaClass*)29L) // LAM - 8/29/06
#define INTERNAL_SOURCEPOSWRAPPER_TAG		((ValueMetaClass*)30L) // LAM - 9/4/08
#define INTERNAL_PYWRAPPERBASE_TAG		((ValueMetaClass*)31L) // fangm - 12/23/15
#define INTERNAL_PROP_EX_THUNK_TAG			((ValueMetaClass*)32L)
#define INTERNAL_READONLY_THUNK_TAG		((ValueMetaClass*)33L)

#define INTERNAL_TAGS					((ValueMetaClass*)100L)  // must be higher than all internal tags

#ifndef DOXYGEN
visible_class_debug_ok (Value)
#endif

#define is_sourcepositionwrapper(v) ((v)->tag == INTERNAL_SOURCEPOSWRAPPER_TAG)

#define is_pywrapperbase(v) ((v)->tag == INTERNAL_PYWRAPPERBASE_TAG)

#define pywrapperbase_equal(va, vb) \
( \
	is_pywrapperbase(va) && \
	is_pywrapperbase(vb) && \
	(va)->eq_vf(&(vb), 1) \
)

/* ---------- the distinguished value subclasses ---------- */

#ifndef DOXYGEN
visible_class_debug_ok (Boolean)
#endif

class Boolean;
class ValueLoader;
extern ScripterExport Boolean true_value;  // corresponds to the mxs system global variable 'true'
extern ScripterExport Boolean false_value;  // corresponds to the mxs system global variable 'false'
#pragma warning(push)
#pragma warning(disable:4100)

// Have singletons of this class corresponding to the 'true' and 'false' values
class Boolean : public Value
{
public:
			Boolean() { tag = &Boolean_class; }
			classof_methods (Boolean, Value);
	void	collect() override;
	void	sprin1(CharStream* s) override;

#	define	is_bool(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == &Boolean_class)
	Value*	op_not_vf(Value**arg_list, int count) override;
	Value*	copy_vf(Value** arg_list, int count) override { return this; }
	BOOL		to_bool() override { return this == &true_value; }
	int		to_int() override { return (this == &true_value); }
	INT64		to_int64() override { return (this == &true_value); }
	INT_PTR	to_intptr() override { return (this == &true_value); }

	void	to_fpvalue(FPValue& v) override { v.i = (this == &true_value) ? 1 : 0; v.type = (ParamType2)TYPE_BOOL; }
	def_generic(coerce, "coerce");
	def_generic(eq, "=");
	def_generic(ne, "!=");

	BOOL	special_case_compare(Value* arg, bool &specialCaseCompareResult) override;

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ----- */

#ifndef DOXYGEN
visible_class_debug_ok (Undefined)
#endif

// Have singleton instances of this class for the 'undefined' and 'dontCollect' variables, and for flow control
class Undefined : public Value
{
public:
			Undefined() { tag = &Undefined_class; }
			classof_methods (Undefined, Value);
	void	collect() override;
	void	sprin1(CharStream* s) override;
	Value*  copy_vf(Value** arg_list, int count) override { return this; }

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
	Mtl*	to_mtl() override { return nullptr; }		// undefined is a NULL material
	void	to_fpvalue(FPValue& v) override;
};

extern ScripterExport Undefined dontCollect;  // corresponds to the mxs system global variable 'dontCollect' which is used in for loops
extern ScripterExport Undefined loopExit;   // used internally to flag that a for loop was exited via the while conditional

/* ----- */

#ifndef DOXYGEN
visible_class_debug_ok (Ok)
#endif

// Have singleton of this class corresponding to the 'ok' value
class Ok : public Value
{
public:
			Ok() { tag = &Ok_class; }
			classof_methods (Ok, Value);
	void	collect() override;
	void	sprin1(CharStream* s) override;
	Value*  copy_vf(Value** arg_list, int count) override { return this; }

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
	void	to_fpvalue(FPValue& v) override;
};

extern ScripterExport Ok ok;  // corresponds to the mxs system global variable 'ok'

/* ----- */

#ifndef DOXYGEN
visible_class_debug_ok (Empty)
#endif

// Have singleton of this class corresponding to the 'empty' value
class Empty : public Value
{
public:
			Empty() { tag = &Empty_class; }
			classof_methods (Empty, Value);
	void	collect() override;
	void	sprin1(CharStream* s) override;
	Value*  copy_vf(Value** arg_list, int count) override { return this; }

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
	void	to_fpvalue(FPValue& v) override;
};

extern ScripterExport Empty empty;  // corresponds to the mxs system global variable 'emptyVal'. Used when node user property key exists, but there is no value associated with it

/* ----- */

#ifndef DOXYGEN
visible_class_debug_ok (Unsupplied)
#endif

// Have singleton of this class corresponding to the 'unsupplied' value
class Unsupplied : public Value
{
public:
			Unsupplied() { tag = &Unsupplied_class; }
			classof_methods (Unsupplied, Value);
	void	collect() override;
	void	sprin1(CharStream* s) override;
	Value*  copy_vf(Value** arg_list, int count) override { return this; }

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
	void	to_fpvalue(FPValue& v) override;
};

extern ScripterExport Unsupplied unsupplied;   // corresponds to the mxs system global variable 'unsupplied'. Typically used for keyword params that are not specified


/* ----- */


#ifndef DOXYGEN
visible_class_debug_ok (NoValue)
#endif

/*
  The Class NoValue is for the Max Script function returning silent value, no output and printing log, 
  even without printing "OK" and "\n".
*/

class NoValue : public Value
{
public:
	NoValue() { tag = &NoValue_class; }
	classof_methods (NoValue, Value);
	void	collect() override;
	void	sprin1(CharStream* s) override;
	Value*  copy_vf(Value** arg_list, int count) override { return this; }

	// scene I/O 
	IOResult Save(ISave* isave) override;
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
	void	to_fpvalue(FPValue& v) override;

	void	sprint(CharStream* stream) override;
};
#pragma warning(pop)
extern ScripterExport NoValue noValue;   // corresponds to the mxs system global variable 'silentValue'. Printing this value results in no output.
