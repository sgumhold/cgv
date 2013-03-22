#pragma once

#if !defined(_MSC_VER) || (_MSC_VER < 1400)
#include <tr1/type_traits>
#endif

namespace cgv {
	namespace type {
		namespace cond {

/// template condition returning, whether the first argument is a base class of the second argument
template <typename T>
struct is_abstract
{ 
	static const bool value =  
#if _MSC_VER >= 1400
			__is_abstract(T);
#else
		std::tr1::is_abstract<T>::value;
#endif
};

		}
	}
}
