#pragma once

#include <cgv/type/invalid_type.h>

namespace cgv {
	namespace type {
		namespace traits {

/** defines for a method_pointer pointer type the return type and the
    list of argument types. Only functions up to a maximum number of
	eight arguments are supported. */
template <typename M> struct method_pointer
{
	static const bool is_method_pointer = false;
	static const bool is_const_method_pointer = false;
	typedef invalid_type this_type;
	typedef invalid_type return_type;
	static const unsigned int nr_arguments = 0;
};

template <typename M, int> struct method_pointer_argument_list { typedef invalid_type type; };


/// specialize method_pointer template for non const method with 0 arguments
template <typename X, typename R>
struct method_pointer<R (X::*)()>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = false;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 0;
};


/// specialize method_pointer template for const method with 0 arguments
template <typename X, typename R>
struct method_pointer<R (X::*)() const>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = true;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 0;
};



/// specialize method_pointer template for non const method with 1 argument
template <typename X, typename R, typename T1>
struct method_pointer<R (X::*)(T1)>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = false;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 1;
};

template <typename X, typename R, typename T1>
struct method_pointer_argument_list<R (X::*)(T1),0> { typedef T1 type; };


/// specialize method_pointer template for const method with 1 argument
template <typename X, typename R, typename T1>
struct method_pointer<R (X::*)(T1) const>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = true;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 1;
};

template <typename X, typename R, typename T1>
struct method_pointer_argument_list<R (X::*)(T1) const, 0> { typedef T1 type; };



/// specialize method_pointer template for non const method with 2 arguments
template <typename X, typename R, typename T1, typename T2>
struct method_pointer<R (X::*)(T1,T2)>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = false;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 2;
};

template <typename X, typename R, typename T1, typename T2>
struct method_pointer_argument_list<R (X::*)(T1,T2),0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2>
struct method_pointer_argument_list<R (X::*)(T1,T2),1> { typedef T2 type; };


/// specialize method_pointer template for const method with 2 arguments
template <typename X, typename R, typename T1, typename T2>
struct method_pointer<R (X::*)(T1,T2) const>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = true;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 2;
};

template <typename X, typename R, typename T1, typename T2>
struct method_pointer_argument_list<R (X::*)(T1,T2) const, 0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2>
struct method_pointer_argument_list<R (X::*)(T1,T2) const, 1> { typedef T2 type; };



/// specialize method_pointer template for non const method with 3 arguments
template <typename X, typename R, typename T1, typename T2, typename T3>
struct method_pointer<R (X::*)(T1,T2,T3)>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = false;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 3;
};

template <typename X, typename R, typename T1, typename T2, typename T3>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3),0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3),1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3),2> { typedef T3 type; };


/// specialize method_pointer template for const method with 3 arguments
template <typename X, typename R, typename T1, typename T2, typename T3>
struct method_pointer<R (X::*)(T1,T2,T3) const>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = true;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 3;
};

template <typename X, typename R, typename T1, typename T2, typename T3>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3) const, 0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3) const, 1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3) const, 2> { typedef T3 type; };



/// specialize method_pointer template for non const method with 4 arguments
template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_pointer<R (X::*)(T1,T2,T3,T4)>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = false;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 4;
};

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4),0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4),1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4),2> { typedef T3 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4),3> { typedef T4 type; };


/// specialize method_pointer template for const method with 4 arguments
template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_pointer<R (X::*)(T1,T2,T3,T4) const>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = true;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 4;
};

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4) const, 0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4) const, 1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4) const, 2> { typedef T3 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4) const, 3> { typedef T4 type; };



/// specialize method_pointer template for non const method with 5 arguments
template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer<R (X::*)(T1,T2,T3,T4,T5)>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = false;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 5;
};

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5),0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5),1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5),2> { typedef T3 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5),3> { typedef T4 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5),4> { typedef T5 type; };


/// specialize method_pointer template for const method with 5 arguments
template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer<R (X::*)(T1,T2,T3,T4,T5) const>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = true;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 5;
};

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5) const, 0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5) const, 1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5) const, 2> { typedef T3 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5) const, 3> { typedef T4 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5) const, 4> { typedef T5 type; };



/// specialize method_pointer template for non const method with 6 arguments
template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer<R (X::*)(T1,T2,T3,T4,T5,T6)>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = false;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 6;
};

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6),0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6),1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6),2> { typedef T3 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6),3> { typedef T4 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6),4> { typedef T5 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6),5> { typedef T6 type; };


/// specialize method_pointer template for const method with 6 arguments
template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer<R (X::*)(T1,T2,T3,T4,T5,T6) const>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = true;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 6;
};

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6) const, 0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6) const, 1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6) const, 2> { typedef T3 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6) const, 3> { typedef T4 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6) const, 4> { typedef T5 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6) const, 5> { typedef T6 type; };



/// specialize method_pointer template for non const method with 7 arguments
template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer<R (X::*)(T1,T2,T3,T4,T5,T6,T7)>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = false;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 7;
};

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7),0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7),1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7),2> { typedef T3 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7),3> { typedef T4 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7),4> { typedef T5 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7),5> { typedef T6 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7),6> { typedef T7 type; };


/// specialize method_pointer template for const method with 7 arguments
template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer<R (X::*)(T1,T2,T3,T4,T5,T6,T7) const>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = true;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 7;
};

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7) const, 0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7) const, 1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7) const, 2> { typedef T3 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7) const, 3> { typedef T4 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7) const, 4> { typedef T5 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7) const, 5> { typedef T6 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7) const, 6> { typedef T7 type; };



/// specialize method_pointer template for non const method with 8 arguments
template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8)>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = false;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 8;
};

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8),0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8),1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8),2> { typedef T3 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8),3> { typedef T4 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8),4> { typedef T5 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8),5> { typedef T6 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8),6> { typedef T7 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8),7> { typedef T8 type; };


/// specialize method_pointer template for const method with 8 arguments
template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8) const>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = true;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = 8;
};

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8) const, 0> { typedef T1 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8) const, 1> { typedef T2 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8) const, 2> { typedef T3 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8) const, 3> { typedef T4 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8) const, 4> { typedef T5 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8) const, 5> { typedef T6 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8) const, 6> { typedef T7 type; };

template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_pointer_argument_list<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8) const, 7> { typedef T8 type; };



		}
	}
}
