#pragma once 

namespace cgv {
	namespace type {
		namespace func {

/** the drop ref template defines a type without reference modifier */
template <typename T>
struct drop_ref
{
	/// define value type
	typedef T type;
};
/** specialize drop ref template for reference types */
template <typename T>
struct drop_ref<T&>
{
	/// define type without reference
	typedef T type;
};

		}
	}
}
