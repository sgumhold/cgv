#pragma once

#include <type_traits>
#include <cgv/type/func/drop_const.h>

namespace cgv {
	namespace type {
		namespace cond {

/// template condition returning, whether the first argument is a base class of the second argument
template <typename base_type, typename derived_type>
struct is_base_of 
{ 
	static const bool value =  
		std::is_base_of<typename func::drop_const<base_type>::type,
								   typename func::drop_const<derived_type>::type>::value;
};

		}
	}
}
