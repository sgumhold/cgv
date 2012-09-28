#pragma once 

#include <cgv/type/func/drop_const.h>

namespace cgv {
	namespace type {
		namespace func {
			namespace MakeConst {
				template <typename T>
				struct make_const_impl 
				{
					typedef const T type;
				};
				template <typename T>
				struct make_const_impl<T&>
				{
					typedef const T& type;
				};
			}
			/** the drop const traits defines a type without const modifier */
			template <typename T>
			struct make_const : public MakeConst::make_const_impl<typename drop_const<T>::type> {};
		}
	}
}
