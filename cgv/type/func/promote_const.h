#pragma once 

#include <cgv/type/cond/is_const.h>
#include <cgv/type/func/drop_const.h>
#include <cgv/type/func/make_const.h>
#include <cgv/type/ctrl/if_.h>

namespace cgv {
	namespace type {
		namespace func {
			/** promote the const modifier from type T1 to T2 */
			template <typename T1, typename T2>
			struct promote_const : public
				ctrl::if_<cond::is_const<T1>::value,
						  typename make_const<T2>::type,
						  typename drop_const<T2>::type> {};
		}
	}
}
