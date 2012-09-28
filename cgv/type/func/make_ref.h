#pragma once 

#include <cgv/type/func/drop_ref.h>

namespace cgv {
	namespace type {
		namespace func {
			/** ensure the reference modifier for a type without changing the const modifier */
			template <typename T>
			struct make_ref
			{
				typedef typename drop_ref<T>::type& type;
			};
		}
	}
}
