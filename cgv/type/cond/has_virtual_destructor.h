#pragma once

#if !defined(_MSC_VER) || (_MSC_VER < 1400)
#include <tr1/type_traits>
#endif

namespace cgv {
	namespace type {
		/// namespace for conditions that act on types
		namespace cond {
/// template condition returning, whether the passed type has a virtual destructor
template <typename T>
struct has_virtual_destructor
{ 
	static const bool value = 
#if _MSC_VER >= 1400
		__has_virtual_destructor(T); 
#else
		std::tr1::has_virtual_destructor<T>::value;
#endif
};
		}
	}
}
