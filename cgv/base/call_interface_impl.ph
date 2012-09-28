#pragma once
@exclude <cgv/config/ppp.ppp>
#include <vector>
#include "base.h"

namespace cgv {
	namespace base {

@for (i=0; i<=N_ARG; ++i) @{

@if(i==0)@{
/// call a void method without parameters
bool exec(base_ptr bp, const std::string& method)
{
	return bp->call_void(method,std::vector<std::string>(), std::vector<const void*>());
}
@}
@else@{
/// call a void method with one parameter of type T
template <@["typename T1"; ", "; "typename T".i]>
bool exec(base_ptr bp, const std::string& method@[", const T1& v1";""; ", const T".i."& v".i])
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;
@for(j=1;j<=i;++j)@{
	type_names.push_back(cgv::type::info::type_name<T@(j)>::get_name());
	value_ptrs.push_back(const_cast<T@(j)*>(&v@(j)));
@}
	return bp->call_void(method, type_names, value_ptrs);
}
@}

/// call a void method with one parameter of type T
template <typename R@[", typename T1"; ""; ", typename T".i]>
bool eval(base_ptr bp, const std::string& method@[", const T1& v1";""; ", const T".i."& v".i], R& r)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;
@for(j=1;j<=i;++j)@{
	type_names.push_back(cgv::type::info::type_name<T@(j)>::get_name());
	value_ptrs.push_back(const_cast<T@(j)*>(&v@(j)));
@}
	return bp->call_void(method, type_names, value_ptrs, cgv::type::info::type_name<R>::get_name(), &r);
}

@}

	}
}