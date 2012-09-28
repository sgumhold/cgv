#pragma once

#include "reflection_traits.h"

#include "lib_begin.h"

namespace cgv {
	namespace reflect {

/// this reflection traits implementation is used by the reflect_string function
template <typename D, typename B>
struct extern_reflection_traits_impl : public B
{
	static const ReflectionTraitsKind kind = RTK_EXTERNAL_SELF_REFLECT;
	/// compile information about external implementation
	static const bool has_external = true;
	/// compile time information about external type with self_reflect implementation
	typedef D external_self_reflect_type;
	/// whether type can be converted to string, defaults to false
	bool has_external_implementation() const { return true; }
	/// call the external implementation
	bool external_implementation(reflection_handler& rh, void* member_ptr) { return static_cast<D*>(member_ptr)->self_reflect(rh); }
};

/// this reflection traits implementation is used for external self_reflect implementations of instances of type T where the external implementation is a self_reflect function in type D
template <typename T, typename D>
struct extern_reflection_traits : public extern_reflection_traits_impl<D, reflection_traits<T,RTK_EXTERNAL_SELF_REFLECT,false> >
{
	/// clone function
	abst_reflection_traits* clone() { return new extern_reflection_traits<T, D>(); }
};

/// this reflection traits implementation is used for external self_reflect implementations together with string interface of instances of type T where the external implementation is a self_reflect function in type D
template <typename T, typename D>
struct extern_string_reflection_traits : public extern_reflection_traits_impl<D, reflection_traits<T,RTK_EXTERNAL_SELF_REFLECT> >
{
	/// clone function
	abst_reflection_traits* clone() { return new extern_string_reflection_traits<T, D>(); }
};

	}
}

#include <cgv/config/lib_end.h>