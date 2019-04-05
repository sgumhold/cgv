#pragma once

#include <cgv/type/invalid_type.h>

namespace cgv {
	namespace type {
		namespace traits {

/** defines for a function_pointer pointer type the return type and the
    list of argument types. Only functions up to a maximum number of
	eight arguments are supported. */
template <typename T> struct function_pointer
{
	static const bool is_function_pointer = false;
	typedef invalid_type return_type;
	static const unsigned int nr_arguments = 0;
};

template <typename T, int> struct function_pointer_argument_list { typedef invalid_type type; };


/// specialize function_pointer template for 0 arguments
template <typename R>
struct function_pointer<R (*)()>
{
	static const bool is_function_pointer = true;
	typedef R return_type;
	static const unsigned int nr_arguments = 0;
};




/// specialize function_pointer template for 1 argument
template <typename R, typename T1>
struct function_pointer<R (*)(T1)>
{
	static const bool is_function_pointer = true;
	typedef R return_type;
	static const unsigned int nr_arguments = 1;
};


template <typename R, typename T1> 
struct function_pointer_argument_list<R (*)(T1), 0> { typedef T1 type; };



/// specialize function_pointer template for 2 arguments
template <typename R, typename T1, typename T2>
struct function_pointer<R (*)(T1,T2)>
{
	static const bool is_function_pointer = true;
	typedef R return_type;
	static const unsigned int nr_arguments = 2;
};


template <typename R, typename T1, typename T2> 
struct function_pointer_argument_list<R (*)(T1,T2), 0> { typedef T1 type; };

template <typename R, typename T1, typename T2> 
struct function_pointer_argument_list<R (*)(T1,T2), 1> { typedef T2 type; };



/// specialize function_pointer template for 3 arguments
template <typename R, typename T1, typename T2, typename T3>
struct function_pointer<R (*)(T1,T2,T3)>
{
	static const bool is_function_pointer = true;
	typedef R return_type;
	static const unsigned int nr_arguments = 3;
};


template <typename R, typename T1, typename T2, typename T3> 
struct function_pointer_argument_list<R (*)(T1,T2,T3), 0> { typedef T1 type; };

template <typename R, typename T1, typename T2, typename T3> 
struct function_pointer_argument_list<R (*)(T1,T2,T3), 1> { typedef T2 type; };

template <typename R, typename T1, typename T2, typename T3> 
struct function_pointer_argument_list<R (*)(T1,T2,T3), 2> { typedef T3 type; };



/// specialize function_pointer template for 4 arguments
template <typename R, typename T1, typename T2, typename T3, typename T4>
struct function_pointer<R (*)(T1,T2,T3,T4)>
{
	static const bool is_function_pointer = true;
	typedef R return_type;
	static const unsigned int nr_arguments = 4;
};


template <typename R, typename T1, typename T2, typename T3, typename T4> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4), 0> { typedef T1 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4), 1> { typedef T2 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4), 2> { typedef T3 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4), 3> { typedef T4 type; };



/// specialize function_pointer template for 5 arguments
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct function_pointer<R (*)(T1,T2,T3,T4,T5)>
{
	static const bool is_function_pointer = true;
	typedef R return_type;
	static const unsigned int nr_arguments = 5;
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5), 0> { typedef T1 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5), 1> { typedef T2 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5), 2> { typedef T3 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5), 3> { typedef T4 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5), 4> { typedef T5 type; };



/// specialize function_pointer template for 6 arguments
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct function_pointer<R (*)(T1,T2,T3,T4,T5,T6)>
{
	static const bool is_function_pointer = true;
	typedef R return_type;
	static const unsigned int nr_arguments = 6;
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6), 0> { typedef T1 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6), 1> { typedef T2 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6), 2> { typedef T3 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6), 3> { typedef T4 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6), 4> { typedef T5 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6), 5> { typedef T6 type; };



/// specialize function_pointer template for 7 arguments
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct function_pointer<R (*)(T1,T2,T3,T4,T5,T6,T7)>
{
	static const bool is_function_pointer = true;
	typedef R return_type;
	static const unsigned int nr_arguments = 7;
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7), 0> { typedef T1 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7), 1> { typedef T2 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7), 2> { typedef T3 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7), 3> { typedef T4 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7), 4> { typedef T5 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7), 5> { typedef T6 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7), 6> { typedef T7 type; };



/// specialize function_pointer template for 8 arguments
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct function_pointer<R (*)(T1,T2,T3,T4,T5,T6,T7,T8)>
{
	static const bool is_function_pointer = true;
	typedef R return_type;
	static const unsigned int nr_arguments = 8;
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7,T8), 0> { typedef T1 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7,T8), 1> { typedef T2 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7,T8), 2> { typedef T3 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7,T8), 3> { typedef T4 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7,T8), 4> { typedef T5 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7,T8), 5> { typedef T6 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7,T8), 6> { typedef T7 type; };

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> 
struct function_pointer_argument_list<R (*)(T1,T2,T3,T4,T5,T6,T7,T8), 7> { typedef T8 type; };




		}
	}
}

