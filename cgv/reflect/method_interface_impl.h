#pragma once

#include "reflection_handler.h"

namespace cgv {
	namespace reflect {

template <typename M>
struct method_interface_impl;



template <typename X, typename R>
struct method_interface_impl<R (X::*)()> : public method_interface
{
	typedef R (X::*M)();
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
		cgv::type::set_variant((((X*)instance)->*m)(
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename R>
struct method_interface_impl<R (X::*)() const> : public method_interface
{
	typedef R (X::*M)() const;
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
		cgv::type::set_variant((((X*)instance)->*m)(
		), result_type_name, result_value_ptr);
	}
};


template <typename X>
struct method_interface_impl<void (X::*)()> : public method_interface
{
	typedef void (X::*M)();
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
		(((X*)instance)->*m)(
		);
	}
};

template <typename X>
struct method_interface_impl<void (X::*)() const> : public method_interface
{
	typedef void (X::*M)() const;
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
		(((X*)instance)->*m)(
		);
	}
};



template <typename X, typename R, typename T1>
struct method_interface_impl<R (X::*)(T1)> : public method_interface
{
	typedef R (X::*M)(T1);
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename R, typename T1>
struct method_interface_impl<R (X::*)(T1) const> : public method_interface
{
	typedef R (X::*M)(T1) const;
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename T1>
struct method_interface_impl<void (X::*)(T1)> : public method_interface
{
	typedef void (X::*M)(T1);
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0])
		);
	}
};

template <typename X, typename T1>
struct method_interface_impl<void (X::*)(T1) const> : public method_interface
{
	typedef void (X::*M)(T1) const;
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0])
		);
	}
};



template <typename X, typename R, typename T1, typename T2>
struct method_interface_impl<R (X::*)(T1,T2)> : public method_interface
{
	typedef R (X::*M)(T1,T2);
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename R, typename T1, typename T2>
struct method_interface_impl<R (X::*)(T1,T2) const> : public method_interface
{
	typedef R (X::*M)(T1,T2) const;
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename T1, typename T2>
struct method_interface_impl<void (X::*)(T1,T2)> : public method_interface
{
	typedef void (X::*M)(T1,T2);
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1])
		);
	}
};

template <typename X, typename T1, typename T2>
struct method_interface_impl<void (X::*)(T1,T2) const> : public method_interface
{
	typedef void (X::*M)(T1,T2) const;
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1])
		);
	}
};



template <typename X, typename R, typename T1, typename T2, typename T3>
struct method_interface_impl<R (X::*)(T1,T2,T3)> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3);
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename R, typename T1, typename T2, typename T3>
struct method_interface_impl<R (X::*)(T1,T2,T3) const> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3) const;
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename T1, typename T2, typename T3>
struct method_interface_impl<void (X::*)(T1,T2,T3)> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3);
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2])
		);
	}
};

template <typename X, typename T1, typename T2, typename T3>
struct method_interface_impl<void (X::*)(T1,T2,T3) const> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3) const;
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2])
		);
	}
};



template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_interface_impl<R (X::*)(T1,T2,T3,T4)> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3,T4);
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename R, typename T1, typename T2, typename T3, typename T4>
struct method_interface_impl<R (X::*)(T1,T2,T3,T4) const> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3,T4) const;
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename T1, typename T2, typename T3, typename T4>
struct method_interface_impl<void (X::*)(T1,T2,T3,T4)> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3,T4);
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3])
		);
	}
};

template <typename X, typename T1, typename T2, typename T3, typename T4>
struct method_interface_impl<void (X::*)(T1,T2,T3,T4) const> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3,T4) const;
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3])
		);
	}
};



template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_interface_impl<R (X::*)(T1,T2,T3,T4,T5)> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3,T4,T5);
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_interface_impl<R (X::*)(T1,T2,T3,T4,T5) const> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3,T4,T5) const;
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_interface_impl<void (X::*)(T1,T2,T3,T4,T5)> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3,T4,T5);
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4])
		);
	}
};

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5>
struct method_interface_impl<void (X::*)(T1,T2,T3,T4,T5) const> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3,T4,T5) const;
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4])
		);
	}
};



template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_interface_impl<R (X::*)(T1,T2,T3,T4,T5,T6)> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3,T4,T5,T6);
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_interface_impl<R (X::*)(T1,T2,T3,T4,T5,T6) const> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3,T4,T5,T6) const;
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_interface_impl<void (X::*)(T1,T2,T3,T4,T5,T6)> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3,T4,T5,T6);
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5])
		);
	}
};

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct method_interface_impl<void (X::*)(T1,T2,T3,T4,T5,T6) const> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3,T4,T5,T6) const;
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5])
		);
	}
};



template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_interface_impl<R (X::*)(T1,T2,T3,T4,T5,T6,T7)> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3,T4,T5,T6,T7);
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5]),
			cgv::type::variant<T7>::get(param_type_names[6],param_value_ptrs[6])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_interface_impl<R (X::*)(T1,T2,T3,T4,T5,T6,T7) const> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3,T4,T5,T6,T7) const;
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5]),
			cgv::type::variant<T7>::get(param_type_names[6],param_value_ptrs[6])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_interface_impl<void (X::*)(T1,T2,T3,T4,T5,T6,T7)> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3,T4,T5,T6,T7);
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5]),
			cgv::type::variant<T7>::get(param_type_names[6],param_value_ptrs[6])
		);
	}
};

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct method_interface_impl<void (X::*)(T1,T2,T3,T4,T5,T6,T7) const> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3,T4,T5,T6,T7) const;
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5]),
			cgv::type::variant<T7>::get(param_type_names[6],param_value_ptrs[6])
		);
	}
};



template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_interface_impl<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8)> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3,T4,T5,T6,T7,T8);
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5]),
			cgv::type::variant<T7>::get(param_type_names[6],param_value_ptrs[6]),
			cgv::type::variant<T8>::get(param_type_names[7],param_value_ptrs[7])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_interface_impl<R (X::*)(T1,T2,T3,T4,T5,T6,T7,T8) const> : public method_interface
{
	typedef R (X::*M)(T1,T2,T3,T4,T5,T6,T7,T8) const;
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
		cgv::type::set_variant((((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5]),
			cgv::type::variant<T7>::get(param_type_names[6],param_value_ptrs[6]),
			cgv::type::variant<T8>::get(param_type_names[7],param_value_ptrs[7])
		), result_type_name, result_value_ptr);
	}
};


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_interface_impl<void (X::*)(T1,T2,T3,T4,T5,T6,T7,T8)> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3,T4,T5,T6,T7,T8);
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5]),
			cgv::type::variant<T7>::get(param_type_names[6],param_value_ptrs[6]),
			cgv::type::variant<T8>::get(param_type_names[7],param_value_ptrs[7])
		);
	}
};

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct method_interface_impl<void (X::*)(T1,T2,T3,T4,T5,T6,T7,T8) const> : public method_interface
{
	typedef void (X::*M)(T1,T2,T3,T4,T5,T6,T7,T8) const;
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
		(((X*)instance)->*m)(
			cgv::type::variant<T1>::get(param_type_names[0],param_value_ptrs[0]),
			cgv::type::variant<T2>::get(param_type_names[1],param_value_ptrs[1]),
			cgv::type::variant<T3>::get(param_type_names[2],param_value_ptrs[2]),
			cgv::type::variant<T4>::get(param_type_names[3],param_value_ptrs[3]),
			cgv::type::variant<T5>::get(param_type_names[4],param_value_ptrs[4]),
			cgv::type::variant<T6>::get(param_type_names[5],param_value_ptrs[5]),
			cgv::type::variant<T7>::get(param_type_names[6],param_value_ptrs[6]),
			cgv::type::variant<T8>::get(param_type_names[7],param_value_ptrs[7])
		);
	}
};



	}
}