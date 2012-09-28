#pragma once

#include <cgv/type/func/drop_const.h>
#include <cgv/type/func/drop_ref.h>

namespace cgv {
	namespace type {
		/// namespace for functions that act on types
		namespace func {

/// drop const and reference modifier from type
template <typename T>
struct clean
{
	typedef typename func::drop_const<T>::type drop_const_type;
	typedef typename func::drop_ref<drop_const_type>::type type;
};
/// defines value, reference and pointer types for potentially const and reference types
template <typename T>
struct clean_traits : public clean<T>
{
	typedef typename clean<T>::type* pointer_type;
	typedef const typename clean<T>::type* const_pointer_type;
	typedef typename clean<T>::type& reference_type;
	typedef const typename clean<T>::type& const_reference_type;
};

		}
	}
}
