#pragma once
@exclude <cgv/config/ppp.ppp>
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

@for (i=0; i<=N_ARG; i=i+1) @{
/// specialize function_pointer template for @(i) argument@if(i!=1)@{s@}
template <typename R@[", typename T1"; ""; ", typename T".i]>
struct function_pointer<R (*)(@["T1"; ","; "T".i])>
{
	static const bool is_function_pointer = true;
	typedef R return_type;
	static const unsigned int nr_arguments = @(i);
};

@for(j=1; j<=i; ++j)@{
template <typename R@[", typename T1"; ""; ", typename T".i]> 
struct function_pointer_argument_list<R (*)(@["T1"; ","; "T".i]), @(j-1)> { typedef T@(j) type; };
@}

@}

		}
	}
}

