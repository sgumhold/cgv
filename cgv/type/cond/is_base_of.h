#pragma once

#if !defined(_MSC_VER) || (_MSC_VER < 1400)
#include <tr1/type_traits>
#include <cgv/type/func/drop_const.h>
#endif

namespace cgv {
	namespace type {
		namespace cond {

/// template condition returning, whether the first argument is a base class of the second argument
template <typename base_type, typename derived_type>
struct is_base_of 
{ 
	static const bool value =  
#if _MSC_VER >= 1400
			__is_base_of(base_type,derived_type);
#else
		std::tr1::is_base_of<typename func::drop_const<base_type>::type,
								   typename func::drop_const<derived_type>::type>::value;
#endif
};

		}
	}
}
