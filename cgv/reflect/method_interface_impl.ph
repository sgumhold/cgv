#pragma once
@exclude <cgv/config/ppp.ppp>
#include <cgv/reflect/reflection_handler.h>
#include <cgv/type/variant.h>

namespace cgv {
	namespace reflect {

template <typename M>
struct method_interface_impl;

@for (i=0; i<=N_ARG; ++i) @{

template <typename X, typename R@[", typename T1"; ""; ", typename T".i]>
struct method_interface_impl<R (X::*)(@["T1"; ","; "T".i])> : public method_interface
{
	typedef R (X::*M)(@["T1"; ","; "T".i]);
	M m;
	method_interface_impl(M _m) : m(_m) {}
	void call_void(void* instance, 
				   const std::vector<abst_reflection_traits*>& param_value_traits,
				   const std::vector<const void*>& param_value_ptrs,
				   const std::vector<std::string>& param_type_names,
				   const abst_reflection_traits* result_traits,
				   void* result_value_ptr,
				   const std::string& result_type_name)
	{
		cgv::type::set_variant((((X*)instance)->*m)(@for(j=1;j<=i;++j)@{
			cgv::type::variant<T@(j)>::get(param_type_names[@(j-1)],param_value_ptrs[@(j-1)])@if(j<i)@{,@}@}
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename R@[", typename T1"; ""; ", typename T".i]>
struct method_interface_impl<R (X::*)(@["T1"; ","; "T".i]) const> : public method_interface
{
	typedef R (X::*M)(@["T1"; ","; "T".i]) const;
	M m;
	method_interface_impl(M _m) : m(_m) {}
	void call_void(void* instance, 
				   const std::vector<abst_reflection_traits*>& param_value_traits,
				   const std::vector<const void*>& param_value_ptrs,
				   const std::vector<std::string>& param_type_names,
				   const abst_reflection_traits* result_traits,
				   void* result_value_ptr,
				   const std::string& result_type_name)
	{
		cgv::type::set_variant((((X*)instance)->*m)(@for (j=1; j<=i; ++j)@{
			cgv::type::variant<T@(j)>::get(param_type_names[@(j-1)],param_value_ptrs[@(j-1)])@if(j<i)@{,@}@}
		), result_type_name, result_value_ptr);
	}
};


template <typename X@[", typename T1"; ""; ", typename T".i]>
struct method_interface_impl<void (X::*)(@["T1"; ","; "T".i])> : public method_interface
{
	typedef void (X::*M)(@["T1"; ","; "T".i]);
	M m;
	method_interface_impl(M _m) : m(_m) {}
	void call_void(void* instance, 
				   const std::vector<abst_reflection_traits*>& param_value_traits,
				   const std::vector<const void*>& param_value_ptrs,
				   const std::vector<std::string>& param_type_names,
				   const abst_reflection_traits* result_traits,
				   void* result_value_ptr,
				   const std::string& result_type_name)
	{
		(((X*)instance)->*m)(@for (j=1; j<=i; ++j)@{
			cgv::type::variant<T@(j)>::get(param_type_names[@(j-1)],param_value_ptrs[@(j-1)])@if(j<i)@{,@}@}
		);
	}
};

template <typename X@[", typename T1"; ""; ", typename T".i]>
struct method_interface_impl<void (X::*)(@["T1"; ","; "T".i]) const> : public method_interface
{
	typedef void (X::*M)(@["T1"; ","; "T".i]) const;
	M m;
	method_interface_impl(M _m) : m(_m) {}
	void call_void(void* instance, 
				   const std::vector<abst_reflection_traits*>& param_value_traits,
				   const std::vector<const void*>& param_value_ptrs,
				   const std::vector<std::string>& param_type_names,
				   const abst_reflection_traits* result_traits,
				   void* result_value_ptr,
				   const std::string& result_type_name)
	{
		(((X*)instance)->*m)(@for (j=1; j<=i; ++j)@{
			cgv::type::variant<T@(j)>::get(param_type_names[@(j-1)],param_value_ptrs[@(j-1)])@if(j<i)@{,@}@}
		);
	}
};

@}

	}
}