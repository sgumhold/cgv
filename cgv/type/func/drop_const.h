#pragma once 

namespace cgv {
	namespace type {
		namespace func {

/** the drop const template defines a type without const modifier */
template <typename T>
struct drop_const
{
	/// define value type
	typedef T type;
};
/** specialize drop const type for const types */
template <typename T>
struct drop_const<const T>
{
	/// define type without const 
	typedef T type;
};
/** specialize drop const type for reference types */
template <typename T>
struct drop_const<T&>
{
	/// define type without const 
	typedef typename drop_const<T>::type& type;
};

		}
	}
}
