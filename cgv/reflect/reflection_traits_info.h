#pragma once

#include <cgv/type/cond/is_base_of.h>
#include <cgv/type/cond/is_standard_type.h>
#include <cgv/config/cpp_version.h>
#include "reflection_traits.h"

namespace cgv {
	namespace reflect {

template <bool is_self,typename T> 
struct reflection_traits_info_self
{
	static const bool use_get = true; 
#ifdef CPP11
	typedef decltype(get_reflection_traits(T())) traits_type; 
	static const ReflectionTraitsKind kind = traits_type::kind; 
#else
	static const ReflectionTraitsKind kind = RTK_EXTERNAL_SELF_REFLECT; 
#endif
};

template <typename T>
struct reflection_traits_info_self<true,T>
{ 
	static const ReflectionTraitsKind kind = RTK_SELF_REFLECT; 
	static const bool use_get = false;
	typedef reflection_traits<T, RTK_SELF_REFLECT, false> traits_type;
};

template <bool is_std, typename T>
struct reflection_traits_info_std : public reflection_traits_info_self<cgv::type::cond::is_base_of<self_reflection_tag, T>::value, T> {};

template <typename T>
struct reflection_traits_info_std<true,T>
{ 
	static const ReflectionTraitsKind kind = RTK_STD_TYPE; 
	static const bool use_get = false; 
	typedef reflection_traits<T, RTK_STD_TYPE, true> traits_type;
};

//! the reflection_traits_info defines compile time information about reflection_traits for a given type T
/*! Each specialization defines three members:
    - static const ReflectionTraitsKind kind ... careful, the compatibility implementation without using C++11 does not split
	  into RTK_EXTERNAL_SELF_REFLECT and RTK_STRING. This difference can only be dispatched by a function call to get_reflection_traits
	  as for example done in the get_reflection_handler
	- const bool use_get ... whether the get_reflection_traits function needs to be called to determine the reflection_traits of T
	- typedef traits_type ... the reflection_traits for T. Take care! In compatibility mode this is only defined if no call to
	  get_reflection_traits is necessary to determine the reflection_traits of T.
*/
template <typename T>
struct reflection_traits_info : public reflection_traits_info_std<cgv::type::cond::is_standard_type<T>::value, T> 
{
};

	}
}

#include <cgv/config/lib_end.h>