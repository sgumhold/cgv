#pragma once 

namespace cgv {
	namespace type {
		namespace func {

/** the drop pointer template defines a type that drops one pointer modifier */
template <typename T>
struct drop_pointer
{
	/// define value type
	typedef T type;
};
/** specialize drop pointer template for pointer types */
template <typename T>
struct drop_pointer<T*>
{
	/// define type without reference
	typedef T type;
};

		}
	}
}
