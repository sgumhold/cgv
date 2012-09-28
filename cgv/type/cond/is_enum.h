#pragma once

#if !defined(_MSC_VER) || (_MSC_VER < 1400)
#include <tr1/type_traits>
#endif

namespace cgv {
	namespace type {
		namespace cond {

/// template condition returning, whether the given type is an enum type
template <typename T>
struct is_enum
{ 
	static const bool value =  
#if _MSC_VER >= 1400
			__is_enum(T);
#else
			std::tr1::is_enum<T>::value;
#endif
};

		}
	}
}
