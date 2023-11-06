// Protocols for Box classes

#pragma warning(push)
#pragma warning(disable:4100)

	use_generic						( eq,			"=");
	use_generic						( ne,			"!=");
	use_generic						( scale,		"scale");
	use_generic						( translate,	"translate");
	use_generic						( intersects,	"intersects");			
	use_generic						( copy,			"copy");
	use_generic						( plus,			"+" );

	def_visible_generic_debug_ok	( isEmpty,		"isEmpty");			
	def_visible_generic_debug_ok	( contains,		"contains");
	def_visible_generic_debug_ok	( rectify,		"rectify");
	def_visible_generic_debug_ok	( empty,		"empty");

	def_visible_generic_debug_ok	( makeCube,		"makeCube");
	def_visible_generic_debug_ok	( enlargeBy,	"enlargeBy");
	def_visible_generic_debug_ok	( expandToInclude,	"expandToInclude");
#pragma warning(pop)
