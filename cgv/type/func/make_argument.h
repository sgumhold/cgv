#pragma once 

#include <cgv/type/func/clean.h>

namespace cgv {
	namespace type {
		namespace func {

/** the make_argument type function converts a given type to a
    constant reference if it is not a non constant reference 
	 type. In the latter case the type is preserved. */
template <typename T>
struct make_argument
{
	typedef const typename cgv::type::func::clean<T>::type& type;
};
template <typename T>
struct make_argument<T&>
{
	typedef T& type;
};

		}
	}
}
