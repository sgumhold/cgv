#pragma once
@exclude <cgv/config/ppp.ppp>
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

@for (i=0; i<=N_ARG; i=i+1) @{
/// specialize method_pointer template for non const method with @(i) argument@if(i!=1)@{s@}
template <typename X, typename R@[", typename T1"; ""; ", typename T".i]>
struct method_pointer<R (X::*)(@["T1"; ","; "T".i])>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = false;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = @(i);
};
@for(j=1; j<=i; ++j) @{
template <typename X, typename R@[", typename T1"; ""; ", typename T".i]>
struct method_pointer_argument_list<R (X::*)(@["T1"; ","; "T".i]),@(j-1)> { typedef T@(j) type; };
@}

/// specialize method_pointer template for const method with @(i) argument@if(i!=1)@{s@}
template <typename X, typename R@[", typename T1"; ""; ", typename T".i]>
struct method_pointer<R (X::*)(@["T1"; ","; "T".i]) const>
{
	static const bool is_method_pointer = true;
	static const bool is_const_method_pointer = true;
	typedef X this_type;
	typedef R return_type;
	static const unsigned int nr_arguments = @(i);
};
@for(j=1; j<=i; ++j) @{
template <typename X, typename R@[", typename T1"; ""; ", typename T".i]>
struct method_pointer_argument_list<R (X::*)(@["T1"; ","; "T".i]) const, @(j-1)> { typedef T@(j) type; };
@}

@}
		}
	}
}
