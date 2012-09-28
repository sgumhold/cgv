#pragma once

#include <cgv/type/invalid_type.h>

namespace cgv {
	namespace type {
		namespace traits {

			/// traits class that allows to check for member pointer types and to query the type \c this_type of the class and \c member_type of the member
			template <typename T>
			struct member_pointer
			{
				static const bool is_member_pointer = false;
				typedef invalid_type this_type;
				typedef invalid_type member_type;
			};

			template <class T, typename M>
			struct member_pointer<M T::*>
			{
				static const bool is_member_pointer = true;
				typedef T this_type;
				typedef M member_type;
			};
		}
	}
}