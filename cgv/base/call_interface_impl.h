#pragma once

#include <vector>
#include "base.h"

namespace cgv {
	namespace base {




/// call a void method without parameters
bool exec(base_ptr bp, const std::string& method)
{
	return bp->call_void(method,std::vector<std::string>(), std::vector<const void*>());
}


/// call a void method with one parameter of type T
template <typename R>
bool eval(base_ptr bp, const std::string& method, R& r)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	return bp->call_void(method, type_names, value_ptrs, cgv::type::info::type_name<R>::get_name(), &r);
}




/// call a void method with one parameter of type T
template <typename T1>
bool exec(base_ptr bp, const std::string& method, const T1& v1)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	return bp->call_void(method, type_names, value_ptrs);
}


/// call a void method with one parameter of type T
template <typename R, typename T1>
bool eval(base_ptr bp, const std::string& method, const T1& v1, R& r)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	return bp->call_void(method, type_names, value_ptrs, cgv::type::info::type_name<R>::get_name(), &r);
}




/// call a void method with one parameter of type T
template <typename T1, typename T2>
bool exec(base_ptr bp, const std::string& method, const T1& v1, const T2& v2)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	return bp->call_void(method, type_names, value_ptrs);
}


/// call a void method with one parameter of type T
template <typename R, typename T1, typename T2>
bool eval(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, R& r)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	return bp->call_void(method, type_names, value_ptrs, cgv::type::info::type_name<R>::get_name(), &r);
}




/// call a void method with one parameter of type T
template <typename T1, typename T2, typename T3>
bool exec(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	return bp->call_void(method, type_names, value_ptrs);
}


/// call a void method with one parameter of type T
template <typename R, typename T1, typename T2, typename T3>
bool eval(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3, R& r)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	return bp->call_void(method, type_names, value_ptrs, cgv::type::info::type_name<R>::get_name(), &r);
}




/// call a void method with one parameter of type T
template <typename T1, typename T2, typename T3, typename T4>
bool exec(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3, const T4& v4)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	type_names.push_back(cgv::type::info::type_name<T4>::get_name());
	value_ptrs.push_back(const_cast<T4*>(&v4));

	return bp->call_void(method, type_names, value_ptrs);
}


/// call a void method with one parameter of type T
template <typename R, typename T1, typename T2, typename T3, typename T4>
bool eval(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3, const T4& v4, R& r)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	type_names.push_back(cgv::type::info::type_name<T4>::get_name());
	value_ptrs.push_back(const_cast<T4*>(&v4));

	return bp->call_void(method, type_names, value_ptrs, cgv::type::info::type_name<R>::get_name(), &r);
}




/// call a void method with one parameter of type T
template <typename T1, typename T2, typename T3, typename T4, typename T5>
bool exec(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	type_names.push_back(cgv::type::info::type_name<T4>::get_name());
	value_ptrs.push_back(const_cast<T4*>(&v4));

	type_names.push_back(cgv::type::info::type_name<T5>::get_name());
	value_ptrs.push_back(const_cast<T5*>(&v5));

	return bp->call_void(method, type_names, value_ptrs);
}


/// call a void method with one parameter of type T
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
bool eval(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, R& r)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	type_names.push_back(cgv::type::info::type_name<T4>::get_name());
	value_ptrs.push_back(const_cast<T4*>(&v4));

	type_names.push_back(cgv::type::info::type_name<T5>::get_name());
	value_ptrs.push_back(const_cast<T5*>(&v5));

	return bp->call_void(method, type_names, value_ptrs, cgv::type::info::type_name<R>::get_name(), &r);
}




/// call a void method with one parameter of type T
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool exec(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	type_names.push_back(cgv::type::info::type_name<T4>::get_name());
	value_ptrs.push_back(const_cast<T4*>(&v4));

	type_names.push_back(cgv::type::info::type_name<T5>::get_name());
	value_ptrs.push_back(const_cast<T5*>(&v5));

	type_names.push_back(cgv::type::info::type_name<T6>::get_name());
	value_ptrs.push_back(const_cast<T6*>(&v6));

	return bp->call_void(method, type_names, value_ptrs);
}


/// call a void method with one parameter of type T
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool eval(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, R& r)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	type_names.push_back(cgv::type::info::type_name<T4>::get_name());
	value_ptrs.push_back(const_cast<T4*>(&v4));

	type_names.push_back(cgv::type::info::type_name<T5>::get_name());
	value_ptrs.push_back(const_cast<T5*>(&v5));

	type_names.push_back(cgv::type::info::type_name<T6>::get_name());
	value_ptrs.push_back(const_cast<T6*>(&v6));

	return bp->call_void(method, type_names, value_ptrs, cgv::type::info::type_name<R>::get_name(), &r);
}




/// call a void method with one parameter of type T
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool exec(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	type_names.push_back(cgv::type::info::type_name<T4>::get_name());
	value_ptrs.push_back(const_cast<T4*>(&v4));

	type_names.push_back(cgv::type::info::type_name<T5>::get_name());
	value_ptrs.push_back(const_cast<T5*>(&v5));

	type_names.push_back(cgv::type::info::type_name<T6>::get_name());
	value_ptrs.push_back(const_cast<T6*>(&v6));

	type_names.push_back(cgv::type::info::type_name<T7>::get_name());
	value_ptrs.push_back(const_cast<T7*>(&v7));

	return bp->call_void(method, type_names, value_ptrs);
}


/// call a void method with one parameter of type T
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool eval(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, R& r)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	type_names.push_back(cgv::type::info::type_name<T4>::get_name());
	value_ptrs.push_back(const_cast<T4*>(&v4));

	type_names.push_back(cgv::type::info::type_name<T5>::get_name());
	value_ptrs.push_back(const_cast<T5*>(&v5));

	type_names.push_back(cgv::type::info::type_name<T6>::get_name());
	value_ptrs.push_back(const_cast<T6*>(&v6));

	type_names.push_back(cgv::type::info::type_name<T7>::get_name());
	value_ptrs.push_back(const_cast<T7*>(&v7));

	return bp->call_void(method, type_names, value_ptrs, cgv::type::info::type_name<R>::get_name(), &r);
}




/// call a void method with one parameter of type T
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool exec(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	type_names.push_back(cgv::type::info::type_name<T4>::get_name());
	value_ptrs.push_back(const_cast<T4*>(&v4));

	type_names.push_back(cgv::type::info::type_name<T5>::get_name());
	value_ptrs.push_back(const_cast<T5*>(&v5));

	type_names.push_back(cgv::type::info::type_name<T6>::get_name());
	value_ptrs.push_back(const_cast<T6*>(&v6));

	type_names.push_back(cgv::type::info::type_name<T7>::get_name());
	value_ptrs.push_back(const_cast<T7*>(&v7));

	type_names.push_back(cgv::type::info::type_name<T8>::get_name());
	value_ptrs.push_back(const_cast<T8*>(&v8));

	return bp->call_void(method, type_names, value_ptrs);
}


/// call a void method with one parameter of type T
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool eval(base_ptr bp, const std::string& method, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8, R& r)
{
	std::vector<std::string> type_names;
	std::vector<const void*> value_ptrs;

	type_names.push_back(cgv::type::info::type_name<T1>::get_name());
	value_ptrs.push_back(const_cast<T1*>(&v1));

	type_names.push_back(cgv::type::info::type_name<T2>::get_name());
	value_ptrs.push_back(const_cast<T2*>(&v2));

	type_names.push_back(cgv::type::info::type_name<T3>::get_name());
	value_ptrs.push_back(const_cast<T3*>(&v3));

	type_names.push_back(cgv::type::info::type_name<T4>::get_name());
	value_ptrs.push_back(const_cast<T4*>(&v4));

	type_names.push_back(cgv::type::info::type_name<T5>::get_name());
	value_ptrs.push_back(const_cast<T5*>(&v5));

	type_names.push_back(cgv::type::info::type_name<T6>::get_name());
	value_ptrs.push_back(const_cast<T6*>(&v6));

	type_names.push_back(cgv::type::info::type_name<T7>::get_name());
	value_ptrs.push_back(const_cast<T7*>(&v7));

	type_names.push_back(cgv::type::info::type_name<T8>::get_name());
	value_ptrs.push_back(const_cast<T8*>(&v8));

	return bp->call_void(method, type_names, value_ptrs, cgv::type::info::type_name<R>::get_name(), &r);
}



	}
}