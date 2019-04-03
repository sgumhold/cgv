#pragma once

#include <cgv/type/cond/is_base_of.h>
#include <cgv/type/func/make_argument.h>
#include <cgv/type/invalid_type.h>

using cgv::type::func::make_argument;
using cgv::type::func::clean;

namespace cgv {
	namespace signal {

/* functors *******************************************************************/

template <typename R, int i, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
struct fct_functor;

template <typename R, int i, class X, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
struct mtd_functor;

template <typename R, int i, class X, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
struct const_mtd_functor;


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct fct_functor<R, 0,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (*function_ptr)();
	function_ptr fp;
	fct_functor(function_ptr _fp) : fp(_fp) {}
	R operator() () const 
	{ 
		return fp();
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct mtd_functor<R,0,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (X::*method_ptr)();
	X* ip;
	method_ptr mp;
	mtd_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}	
	R operator() () const 
	{
		return (ip->*mp)(); 
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct const_mtd_functor<R,0,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R (X::*const_method_ptr)() const;
	const X* ip;
	const_method_ptr mp;
	const_mtd_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	R operator() () const 
	{
		return (ip->*mp)(); 
	}
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct fct_functor<R, 1,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (*function_ptr)(T1);
	function_ptr fp;
	fct_functor(function_ptr _fp) : fp(_fp) {}
	R operator() (const typename clean<T1>::type& v1) const 
	{ 
		return fp(const_cast<typename make_argument<T1>::type>(v1));
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct mtd_functor<R,1,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (X::*method_ptr)(T1);
	X* ip;
	method_ptr mp;
	mtd_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}	
	R operator() (const typename clean<T1>::type& v1) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1)); 
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct const_mtd_functor<R,1,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R (X::*const_method_ptr)(T1) const;
	const X* ip;
	const_method_ptr mp;
	const_mtd_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	R operator() (const typename clean<T1>::type& v1) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1)); 
	}
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct fct_functor<R, 2,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (*function_ptr)(T1, T2);
	function_ptr fp;
	fct_functor(function_ptr _fp) : fp(_fp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2) const 
	{ 
		return fp(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2));
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct mtd_functor<R,2,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (X::*method_ptr)(T1, T2);
	X* ip;
	method_ptr mp;
	mtd_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}	
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2)); 
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct const_mtd_functor<R,2,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R (X::*const_method_ptr)(T1, T2) const;
	const X* ip;
	const_method_ptr mp;
	const_mtd_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2)); 
	}
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct fct_functor<R, 3,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (*function_ptr)(T1, T2, T3);
	function_ptr fp;
	fct_functor(function_ptr _fp) : fp(_fp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3) const 
	{ 
		return fp(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3));
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct mtd_functor<R,3,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (X::*method_ptr)(T1, T2, T3);
	X* ip;
	method_ptr mp;
	mtd_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}	
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3)); 
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct const_mtd_functor<R,3,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R (X::*const_method_ptr)(T1, T2, T3) const;
	const X* ip;
	const_method_ptr mp;
	const_mtd_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3)); 
	}
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct fct_functor<R, 4,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (*function_ptr)(T1, T2, T3, T4);
	function_ptr fp;
	fct_functor(function_ptr _fp) : fp(_fp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4) const 
	{ 
		return fp(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4));
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct mtd_functor<R,4,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (X::*method_ptr)(T1, T2, T3, T4);
	X* ip;
	method_ptr mp;
	mtd_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}	
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4)); 
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct const_mtd_functor<R,4,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R (X::*const_method_ptr)(T1, T2, T3, T4) const;
	const X* ip;
	const_method_ptr mp;
	const_mtd_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4)); 
	}
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct fct_functor<R, 5,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (*function_ptr)(T1, T2, T3, T4, T5);
	function_ptr fp;
	fct_functor(function_ptr _fp) : fp(_fp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5) const 
	{ 
		return fp(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5));
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct mtd_functor<R,5,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (X::*method_ptr)(T1, T2, T3, T4, T5);
	X* ip;
	method_ptr mp;
	mtd_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}	
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5)); 
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct const_mtd_functor<R,5,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R (X::*const_method_ptr)(T1, T2, T3, T4, T5) const;
	const X* ip;
	const_method_ptr mp;
	const_mtd_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5)); 
	}
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct fct_functor<R, 6,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (*function_ptr)(T1, T2, T3, T4, T5, T6);
	function_ptr fp;
	fct_functor(function_ptr _fp) : fp(_fp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5, const typename clean<T6>::type& v6) const 
	{ 
		return fp(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5), const_cast<typename make_argument<T6>::type>(v6));
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct mtd_functor<R,6,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (X::*method_ptr)(T1, T2, T3, T4, T5, T6);
	X* ip;
	method_ptr mp;
	mtd_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}	
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5, const typename clean<T6>::type& v6) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5), const_cast<typename make_argument<T6>::type>(v6)); 
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct const_mtd_functor<R,6,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R (X::*const_method_ptr)(T1, T2, T3, T4, T5, T6) const;
	const X* ip;
	const_method_ptr mp;
	const_mtd_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5, const typename clean<T6>::type& v6) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5), const_cast<typename make_argument<T6>::type>(v6)); 
	}
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct fct_functor<R, 7,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (*function_ptr)(T1, T2, T3, T4, T5, T6, T7);
	function_ptr fp;
	fct_functor(function_ptr _fp) : fp(_fp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5, const typename clean<T6>::type& v6, const typename clean<T7>::type& v7) const 
	{ 
		return fp(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5), const_cast<typename make_argument<T6>::type>(v6), const_cast<typename make_argument<T7>::type>(v7));
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct mtd_functor<R,7,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (X::*method_ptr)(T1, T2, T3, T4, T5, T6, T7);
	X* ip;
	method_ptr mp;
	mtd_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}	
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5, const typename clean<T6>::type& v6, const typename clean<T7>::type& v7) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5), const_cast<typename make_argument<T6>::type>(v6), const_cast<typename make_argument<T7>::type>(v7)); 
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct const_mtd_functor<R,7,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R (X::*const_method_ptr)(T1, T2, T3, T4, T5, T6, T7) const;
	const X* ip;
	const_method_ptr mp;
	const_mtd_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5, const typename clean<T6>::type& v6, const typename clean<T7>::type& v7) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5), const_cast<typename make_argument<T6>::type>(v6), const_cast<typename make_argument<T7>::type>(v7)); 
	}
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct fct_functor<R, 8,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (*function_ptr)(T1, T2, T3, T4, T5, T6, T7, T8);
	function_ptr fp;
	fct_functor(function_ptr _fp) : fp(_fp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5, const typename clean<T6>::type& v6, const typename clean<T7>::type& v7, const typename clean<T8>::type& v8) const 
	{ 
		return fp(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5), const_cast<typename make_argument<T6>::type>(v6), const_cast<typename make_argument<T7>::type>(v7), const_cast<typename make_argument<T8>::type>(v8));
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct mtd_functor<R,8,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (X::*method_ptr)(T1, T2, T3, T4, T5, T6, T7, T8);
	X* ip;
	method_ptr mp;
	mtd_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}	
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5, const typename clean<T6>::type& v6, const typename clean<T7>::type& v7, const typename clean<T8>::type& v8) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5), const_cast<typename make_argument<T6>::type>(v6), const_cast<typename make_argument<T7>::type>(v7), const_cast<typename make_argument<T8>::type>(v8)); 
	}
};

template <typename R, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct const_mtd_functor<R,8,X,T1, T2, T3, T4, T5, T6, T7, T8>
{
	typedef R (X::*const_method_ptr)(T1, T2, T3, T4, T5, T6, T7, T8) const;
	const X* ip;
	const_method_ptr mp;
	const_mtd_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	R operator() (const typename clean<T1>::type& v1, const typename clean<T2>::type& v2, const typename clean<T3>::type& v3, const typename clean<T4>::type& v4, const typename clean<T5>::type& v5, const typename clean<T6>::type& v6, const typename clean<T7>::type& v7, const typename clean<T8>::type& v8) const 
	{
		return (ip->*mp)(const_cast<typename make_argument<T1>::type>(v1), const_cast<typename make_argument<T2>::type>(v2), const_cast<typename make_argument<T3>::type>(v3), const_cast<typename make_argument<T4>::type>(v4), const_cast<typename make_argument<T5>::type>(v5), const_cast<typename make_argument<T6>::type>(v6), const_cast<typename make_argument<T7>::type>(v7), const_cast<typename make_argument<T8>::type>(v8)); 
	}
};



/* expressions *******************************************************************/

namespace expression {
	struct tag {};
}

template <typename T>
struct const_expression : public expression::tag
{
	const T v;
	const_expression(const T& _v) : v(_v) {}
	const T operator() () { return v; }
	template <typename T1>
	const T operator() (const T1&) { return v; }
	template <typename T1, typename T2>
	const T operator() (const T1&, const T2&) { return v; }
	template <typename T1, typename T2, typename T3>
	const T operator() (const T1&, const T2&, const T3&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4>
	const T operator() (const T1&, const T2&, const T3&, const T4&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	const T operator() (const T1&, const T2&, const T3&, const T4&, const T5&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	const T operator() (const T1&, const T2&, const T3&, const T4&, const T5&, const T6&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	const T operator() (const T1&, const T2&, const T3&, const T4&, const T5&, const T6&, const T7&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	const T operator() (const T1&, const T2&, const T3&, const T4&, const T5&, const T6&, const T7&, const T8&) { return v; }
};

template <typename T> 
const_expression<T> _c(const T& v) 
{
	return const_expression<T>(v); 
}

template <typename T>
struct ref_expression : public expression::tag
{
	T& v;
	ref_expression(T& _v) : v(_v) {}
	T& operator() () { return v; }
	template <typename T1>
	T& operator() (const T1&) { return v; }
	template <typename T1, typename T2>
	T& operator() (const T1&, const T2&) { return v; }
	template <typename T1, typename T2, typename T3>
	T& operator() (const T1&, const T2&, const T3&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4>
	T& operator() (const T1&, const T2&, const T3&, const T4&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	T& operator() (const T1&, const T2&, const T3&, const T4&, const T5&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	T& operator() (const T1&, const T2&, const T3&, const T4&, const T5&, const T6&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	T& operator() (const T1&, const T2&, const T3&, const T4&, const T5&, const T6&, const T7&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	T& operator() (const T1&, const T2&, const T3&, const T4&, const T5&, const T6&, const T7&, const T8&) { return v; }
};

template <typename T> 
ref_expression<T> _r(T& v) 
{
	return ref_expression<T>(v); 
}

template <typename T>
struct const_ref_expression : public expression::tag
{
	const T& v;
	const_ref_expression(const T& _v) : v(_v) {}
	const T& operator() () { return v; }
	template <typename T1>
	const T& operator() (const T1&) { return v; }
	template <typename T1, typename T2>
	const T& operator() (const T1&, const T2&) { return v; }
	template <typename T1, typename T2, typename T3>
	const T& operator() (const T1&, const T2&, const T3&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4>
	const T& operator() (const T1&, const T2&, const T3&, const T4&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	const T& operator() (const T1&, const T2&, const T3&, const T4&, const T5&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	const T& operator() (const T1&, const T2&, const T3&, const T4&, const T5&, const T6&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	const T& operator() (const T1&, const T2&, const T3&, const T4&, const T5&, const T6&, const T7&) { return v; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	const T& operator() (const T1&, const T2&, const T3&, const T4&, const T5&, const T6&, const T7&, const T8&) { return v; }
};

template <typename T> 
const_ref_expression<T> _cr(const T& v) 
{
	return const_ref_expression<T>(v); 
}

template <int i>
struct placeholder_expression;


template <>
struct placeholder_expression<1> : public expression::tag
{
	void operator() () {}
	template <typename T1>
	const T1& operator() (const T1& v1) { return v1; }
	template <typename T1, typename T2>
	const T1& operator() (const T1& v1, const T2& v2) { return v1; }
	template <typename T1, typename T2, typename T3>
	const T1& operator() (const T1& v1, const T2& v2, const T3& v3) { return v1; }
	template <typename T1, typename T2, typename T3, typename T4>
	const T1& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4) { return v1; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	const T1& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) { return v1; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	const T1& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) { return v1; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	const T1& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) { return v1; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	const T1& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) { return v1; }
};

static placeholder_expression<1> _1;

template <>
struct placeholder_expression<2> : public expression::tag
{
	void operator() () {}
	template <typename T1>
	void operator() (T1&) { }
	template <typename T1, typename T2>
	const T2& operator() (const T1& v1, const T2& v2) { return v2; }
	template <typename T1, typename T2, typename T3>
	const T2& operator() (const T1& v1, const T2& v2, const T3& v3) { return v2; }
	template <typename T1, typename T2, typename T3, typename T4>
	const T2& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4) { return v2; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	const T2& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) { return v2; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	const T2& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) { return v2; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	const T2& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) { return v2; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	const T2& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) { return v2; }
};

static placeholder_expression<2> _2;

template <>
struct placeholder_expression<3> : public expression::tag
{
	void operator() () {}
	template <typename T1>
	void operator() (T1&) { }
	template <typename T1, typename T2>
	void operator() (T1&, T2&) { }
	template <typename T1, typename T2, typename T3>
	const T3& operator() (const T1& v1, const T2& v2, const T3& v3) { return v3; }
	template <typename T1, typename T2, typename T3, typename T4>
	const T3& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4) { return v3; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	const T3& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) { return v3; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	const T3& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) { return v3; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	const T3& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) { return v3; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	const T3& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) { return v3; }
};

static placeholder_expression<3> _3;

template <>
struct placeholder_expression<4> : public expression::tag
{
	void operator() () {}
	template <typename T1>
	void operator() (T1&) { }
	template <typename T1, typename T2>
	void operator() (T1&, T2&) { }
	template <typename T1, typename T2, typename T3>
	void operator() (T1&, T2&, T3&) { }
	template <typename T1, typename T2, typename T3, typename T4>
	const T4& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4) { return v4; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	const T4& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) { return v4; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	const T4& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) { return v4; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	const T4& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) { return v4; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	const T4& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) { return v4; }
};

static placeholder_expression<4> _4;

template <>
struct placeholder_expression<5> : public expression::tag
{
	void operator() () {}
	template <typename T1>
	void operator() (T1&) { }
	template <typename T1, typename T2>
	void operator() (T1&, T2&) { }
	template <typename T1, typename T2, typename T3>
	void operator() (T1&, T2&, T3&) { }
	template <typename T1, typename T2, typename T3, typename T4>
	void operator() (T1&, T2&, T3&, T4&) { }
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	const T5& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) { return v5; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	const T5& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) { return v5; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	const T5& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) { return v5; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	const T5& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) { return v5; }
};

static placeholder_expression<5> _5;

template <>
struct placeholder_expression<6> : public expression::tag
{
	void operator() () {}
	template <typename T1>
	void operator() (T1&) { }
	template <typename T1, typename T2>
	void operator() (T1&, T2&) { }
	template <typename T1, typename T2, typename T3>
	void operator() (T1&, T2&, T3&) { }
	template <typename T1, typename T2, typename T3, typename T4>
	void operator() (T1&, T2&, T3&, T4&) { }
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	void operator() (T1&, T2&, T3&, T4&, T5&) { }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	const T6& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) { return v6; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	const T6& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) { return v6; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	const T6& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) { return v6; }
};

static placeholder_expression<6> _6;

template <>
struct placeholder_expression<7> : public expression::tag
{
	void operator() () {}
	template <typename T1>
	void operator() (T1&) { }
	template <typename T1, typename T2>
	void operator() (T1&, T2&) { }
	template <typename T1, typename T2, typename T3>
	void operator() (T1&, T2&, T3&) { }
	template <typename T1, typename T2, typename T3, typename T4>
	void operator() (T1&, T2&, T3&, T4&) { }
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	void operator() (T1&, T2&, T3&, T4&, T5&) { }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	void operator() (T1&, T2&, T3&, T4&, T5&, T6&) { }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	const T7& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) { return v7; }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	const T7& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) { return v7; }
};

static placeholder_expression<7> _7;

template <>
struct placeholder_expression<8> : public expression::tag
{
	void operator() () {}
	template <typename T1>
	void operator() (T1&) { }
	template <typename T1, typename T2>
	void operator() (T1&, T2&) { }
	template <typename T1, typename T2, typename T3>
	void operator() (T1&, T2&, T3&) { }
	template <typename T1, typename T2, typename T3, typename T4>
	void operator() (T1&, T2&, T3&, T4&) { }
	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	void operator() (T1&, T2&, T3&, T4&, T5&) { }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	void operator() (T1&, T2&, T3&, T4&, T5&, T6&) { }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	void operator() (T1&, T2&, T3&, T4&, T5&, T6&, T7&) { }
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	const T8& operator() (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) { return v8; }
};

static placeholder_expression<8> _8;


template <class X>
struct is_expression
{
	static const bool value = cgv::type::cond::is_base_of<expression::tag,X>::value;
};

template <typename T, bool is_expr = false>
struct make_expression_type_impl 
{
	typedef const_expression<T> type;
};

template <typename T>
struct make_expression_type_impl<T,true>
{
	typedef T type;
};

template <typename T>
struct make_expression_type 
{
	typedef typename make_expression_type_impl<T, is_expression<T>::value>::type type;
};

template <typename T>
typename make_expression_type<T>::type make_expression(const T& v) 
{
	return (typename make_expression_type<T>::type)(v);
}

// rebinding structs *******************************************************************


template <class F, typename R>
struct rebind0
{
	F  functor; 
	rebind0(const F& _functor) : 
		functor(_functor) {	}
	R operator () () { return functor(); }

	template <typename T1>
	R operator () (const T1& v1) 
	{ 
		return functor(
		);
	}

	template <typename T1, typename T2>
	R operator () (const T1& v1, const T2& v2) 
	{ 
		return functor(
		);
	}

	template <typename T1, typename T2, typename T3>
	R operator () (const T1& v1, const T2& v2, const T3& v3) 
	{ 
		return functor(
		);
	}

	template <typename T1, typename T2, typename T3, typename T4>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4) 
	{ 
		return functor(
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) 
	{ 
		return functor(
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) 
	{ 
		return functor(
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) 
	{ 
		return functor(
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) 
	{ 
		return functor(
		);
	}

};

/*  \brief rebind the arguments of a function to expressions.
 *  \param inst_ptr pointer to an instance of type X
 *  \param mtd_ptr pointer to a method with return type R and arguments T1, ..., Tn
 *  \param A1, ..., An arguments that define the expressions to which the method parameters are bound
 *  \return returns the result of method evaluation of type R
 *
 * No long description yet.
 */
template <typename R, class X>
rebind0<mtd_functor<R, 0, X>, R
           > 
	rebind(X* x, R (X::*m_ptr)()
	)
{
	return rebind0<mtd_functor<R, 0, X>, R
           >(
		       mtd_functor<R, 0, X>(x, m_ptr)
		       );
}

template <typename R, class X>
rebind0<const_mtd_functor<R, 0, X>, R
           > 
	rebind(const X* x, R (X::*m_ptr)() const
	)
{
	return rebind0<const_mtd_functor<R, 0, X>, R
           >(
		       const_mtd_functor<R, 0, X>(x, m_ptr)
		       );
}

/*  \brief rebind the arguments of a function to expressions.
 *  \param fct_ptr pointer to a function with return type R and arguments 
 *  \param  arguments that define the expressions to which the function parameters are bound
 *  \return returns the result of function evaluation of type R
 */
template <typename R>
rebind0<fct_functor<R, 0>, R
           > 
	rebind(R (*f_ptr)()
	)
{
	return rebind0<fct_functor<R, 0>, R
           >(
		       fct_functor<R, 0>(f_ptr)
		       );
}


template <class F, typename R, typename E1>
struct rebind1
{
	F  functor; 
	E1 e1;
	rebind1(const F& _functor, const E1& _e1) : 
		functor(_functor), e1(_e1) {	}
	R operator () () { return functor(e1()); }

	template <typename T1>
	R operator () (const T1& v1) 
	{ 
		return functor(
	               e1(v1)
		);
	}

	template <typename T1, typename T2>
	R operator () (const T1& v1, const T2& v2) 
	{ 
		return functor(
	               e1(v1, v2)
		);
	}

	template <typename T1, typename T2, typename T3>
	R operator () (const T1& v1, const T2& v2, const T3& v3) 
	{ 
		return functor(
	               e1(v1, v2, v3)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7, v8)
		);
	}

};

/*  \brief rebind the arguments of a function to expressions.
 *  \param inst_ptr pointer to an instance of type X
 *  \param mtd_ptr pointer to a method with return type R and arguments T1, ..., Tn
 *  \param A1, ..., An arguments that define the expressions to which the method parameters are bound
 *  \return returns the result of method evaluation of type R
 *
 * No long description yet.
 */
template <typename R, class X, typename T1, typename A1>
rebind1<mtd_functor<R, 1, X, T1>, R
           , typename make_expression_type<A1>::type> 
	rebind(X* x, R (X::*m_ptr)(T1)
	, const A1& a1)
{
	return rebind1<mtd_functor<R, 1, X, T1>, R
           , typename make_expression_type<A1>::type>(
		       mtd_functor<R, 1, X, T1>(x, m_ptr)
		       , make_expression(a1));
}

template <typename R, class X, typename T1, typename A1>
rebind1<const_mtd_functor<R, 1, X, T1>, R
           , typename make_expression_type<A1>::type> 
	rebind(const X* x, R (X::*m_ptr)(T1) const
	, const A1& a1)
{
	return rebind1<const_mtd_functor<R, 1, X, T1>, R
           , typename make_expression_type<A1>::type>(
		       const_mtd_functor<R, 1, X, T1>(x, m_ptr)
		       , make_expression(a1));
}

/*  \brief rebind the arguments of a function to expressions.
 *  \param fct_ptr pointer to a function with return type R and arguments T1
 *  \param A1 arguments that define the expressions to which the function parameters are bound
 *  \return returns the result of function evaluation of type R
 */
template <typename R, typename T1, typename A1>
rebind1<fct_functor<R, 1, T1>, R
           , typename make_expression_type<A1>::type> 
	rebind(R (*f_ptr)(T1)
	, const A1& a1)
{
	return rebind1<fct_functor<R, 1, T1>, R
           , typename make_expression_type<A1>::type>(
		       fct_functor<R, 1, T1>(f_ptr)
		       , make_expression(a1));
}


template <class F, typename R, typename E1, typename E2>
struct rebind2
{
	F  functor; 
	E1 e1;
	E2 e2;
	rebind2(const F& _functor, const E1& _e1, const E2& _e2) : 
		functor(_functor), e1(_e1), e2(_e2) {	}
	R operator () () { return functor(e1(), e2()); }

	template <typename T1>
	R operator () (const T1& v1) 
	{ 
		return functor(
	               e1(v1),
	               e2(v1)
		);
	}

	template <typename T1, typename T2>
	R operator () (const T1& v1, const T2& v2) 
	{ 
		return functor(
	               e1(v1, v2),
	               e2(v1, v2)
		);
	}

	template <typename T1, typename T2, typename T3>
	R operator () (const T1& v1, const T2& v2, const T3& v3) 
	{ 
		return functor(
	               e1(v1, v2, v3),
	               e2(v1, v2, v3)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4),
	               e2(v1, v2, v3, v4)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5),
	               e2(v1, v2, v3, v4, v5)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6),
	               e2(v1, v2, v3, v4, v5, v6)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7),
	               e2(v1, v2, v3, v4, v5, v6, v7)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7, v8),
	               e2(v1, v2, v3, v4, v5, v6, v7, v8)
		);
	}

};

/*  \brief rebind the arguments of a function to expressions.
 *  \param inst_ptr pointer to an instance of type X
 *  \param mtd_ptr pointer to a method with return type R and arguments T1, ..., Tn
 *  \param A1, ..., An arguments that define the expressions to which the method parameters are bound
 *  \return returns the result of method evaluation of type R
 *
 * No long description yet.
 */
template <typename R, class X, typename T1, typename A1, typename T2, typename A2>
rebind2<mtd_functor<R, 2, X, T1, T2>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type> 
	rebind(X* x, R (X::*m_ptr)(T1, T2)
	, const A1& a1, const A2& a2)
{
	return rebind2<mtd_functor<R, 2, X, T1, T2>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type>(
		       mtd_functor<R, 2, X, T1, T2>(x, m_ptr)
		       , make_expression(a1), make_expression(a2));
}

template <typename R, class X, typename T1, typename A1, typename T2, typename A2>
rebind2<const_mtd_functor<R, 2, X, T1, T2>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type> 
	rebind(const X* x, R (X::*m_ptr)(T1, T2) const
	, const A1& a1, const A2& a2)
{
	return rebind2<const_mtd_functor<R, 2, X, T1, T2>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type>(
		       const_mtd_functor<R, 2, X, T1, T2>(x, m_ptr)
		       , make_expression(a1), make_expression(a2));
}

/*  \brief rebind the arguments of a function to expressions.
 *  \param fct_ptr pointer to a function with return type R and arguments T1,T2
 *  \param A1,A2 arguments that define the expressions to which the function parameters are bound
 *  \return returns the result of function evaluation of type R
 */
template <typename R, typename T1, typename A1, typename T2, typename A2>
rebind2<fct_functor<R, 2, T1, T2>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type> 
	rebind(R (*f_ptr)(T1, T2)
	, const A1& a1, const A2& a2)
{
	return rebind2<fct_functor<R, 2, T1, T2>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type>(
		       fct_functor<R, 2, T1, T2>(f_ptr)
		       , make_expression(a1), make_expression(a2));
}


template <class F, typename R, typename E1, typename E2, typename E3>
struct rebind3
{
	F  functor; 
	E1 e1;
	E2 e2;
	E3 e3;
	rebind3(const F& _functor, const E1& _e1, const E2& _e2, const E3& _e3) : 
		functor(_functor), e1(_e1), e2(_e2), e3(_e3) {	}
	R operator () () { return functor(e1(), e2(), e3()); }

	template <typename T1>
	R operator () (const T1& v1) 
	{ 
		return functor(
	               e1(v1),
	               e2(v1),
	               e3(v1)
		);
	}

	template <typename T1, typename T2>
	R operator () (const T1& v1, const T2& v2) 
	{ 
		return functor(
	               e1(v1, v2),
	               e2(v1, v2),
	               e3(v1, v2)
		);
	}

	template <typename T1, typename T2, typename T3>
	R operator () (const T1& v1, const T2& v2, const T3& v3) 
	{ 
		return functor(
	               e1(v1, v2, v3),
	               e2(v1, v2, v3),
	               e3(v1, v2, v3)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4),
	               e2(v1, v2, v3, v4),
	               e3(v1, v2, v3, v4)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5),
	               e2(v1, v2, v3, v4, v5),
	               e3(v1, v2, v3, v4, v5)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6),
	               e2(v1, v2, v3, v4, v5, v6),
	               e3(v1, v2, v3, v4, v5, v6)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7),
	               e2(v1, v2, v3, v4, v5, v6, v7),
	               e3(v1, v2, v3, v4, v5, v6, v7)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7, v8),
	               e2(v1, v2, v3, v4, v5, v6, v7, v8),
	               e3(v1, v2, v3, v4, v5, v6, v7, v8)
		);
	}

};

/*  \brief rebind the arguments of a function to expressions.
 *  \param inst_ptr pointer to an instance of type X
 *  \param mtd_ptr pointer to a method with return type R and arguments T1, ..., Tn
 *  \param A1, ..., An arguments that define the expressions to which the method parameters are bound
 *  \return returns the result of method evaluation of type R
 *
 * No long description yet.
 */
template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3>
rebind3<mtd_functor<R, 3, X, T1, T2, T3>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type> 
	rebind(X* x, R (X::*m_ptr)(T1, T2, T3)
	, const A1& a1, const A2& a2, const A3& a3)
{
	return rebind3<mtd_functor<R, 3, X, T1, T2, T3>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type>(
		       mtd_functor<R, 3, X, T1, T2, T3>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3));
}

template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3>
rebind3<const_mtd_functor<R, 3, X, T1, T2, T3>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type> 
	rebind(const X* x, R (X::*m_ptr)(T1, T2, T3) const
	, const A1& a1, const A2& a2, const A3& a3)
{
	return rebind3<const_mtd_functor<R, 3, X, T1, T2, T3>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type>(
		       const_mtd_functor<R, 3, X, T1, T2, T3>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3));
}

/*  \brief rebind the arguments of a function to expressions.
 *  \param fct_ptr pointer to a function with return type R and arguments T1,T2,T3
 *  \param A1,A2,A3 arguments that define the expressions to which the function parameters are bound
 *  \return returns the result of function evaluation of type R
 */
template <typename R, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3>
rebind3<fct_functor<R, 3, T1, T2, T3>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type> 
	rebind(R (*f_ptr)(T1, T2, T3)
	, const A1& a1, const A2& a2, const A3& a3)
{
	return rebind3<fct_functor<R, 3, T1, T2, T3>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type>(
		       fct_functor<R, 3, T1, T2, T3>(f_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3));
}


template <class F, typename R, typename E1, typename E2, typename E3, typename E4>
struct rebind4
{
	F  functor; 
	E1 e1;
	E2 e2;
	E3 e3;
	E4 e4;
	rebind4(const F& _functor, const E1& _e1, const E2& _e2, const E3& _e3, const E4& _e4) : 
		functor(_functor), e1(_e1), e2(_e2), e3(_e3), e4(_e4) {	}
	R operator () () { return functor(e1(), e2(), e3(), e4()); }

	template <typename T1>
	R operator () (const T1& v1) 
	{ 
		return functor(
	               e1(v1),
	               e2(v1),
	               e3(v1),
	               e4(v1)
		);
	}

	template <typename T1, typename T2>
	R operator () (const T1& v1, const T2& v2) 
	{ 
		return functor(
	               e1(v1, v2),
	               e2(v1, v2),
	               e3(v1, v2),
	               e4(v1, v2)
		);
	}

	template <typename T1, typename T2, typename T3>
	R operator () (const T1& v1, const T2& v2, const T3& v3) 
	{ 
		return functor(
	               e1(v1, v2, v3),
	               e2(v1, v2, v3),
	               e3(v1, v2, v3),
	               e4(v1, v2, v3)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4),
	               e2(v1, v2, v3, v4),
	               e3(v1, v2, v3, v4),
	               e4(v1, v2, v3, v4)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5),
	               e2(v1, v2, v3, v4, v5),
	               e3(v1, v2, v3, v4, v5),
	               e4(v1, v2, v3, v4, v5)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6),
	               e2(v1, v2, v3, v4, v5, v6),
	               e3(v1, v2, v3, v4, v5, v6),
	               e4(v1, v2, v3, v4, v5, v6)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7),
	               e2(v1, v2, v3, v4, v5, v6, v7),
	               e3(v1, v2, v3, v4, v5, v6, v7),
	               e4(v1, v2, v3, v4, v5, v6, v7)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7, v8),
	               e2(v1, v2, v3, v4, v5, v6, v7, v8),
	               e3(v1, v2, v3, v4, v5, v6, v7, v8),
	               e4(v1, v2, v3, v4, v5, v6, v7, v8)
		);
	}

};

/*  \brief rebind the arguments of a function to expressions.
 *  \param inst_ptr pointer to an instance of type X
 *  \param mtd_ptr pointer to a method with return type R and arguments T1, ..., Tn
 *  \param A1, ..., An arguments that define the expressions to which the method parameters are bound
 *  \return returns the result of method evaluation of type R
 *
 * No long description yet.
 */
template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4>
rebind4<mtd_functor<R, 4, X, T1, T2, T3, T4>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type> 
	rebind(X* x, R (X::*m_ptr)(T1, T2, T3, T4)
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
{
	return rebind4<mtd_functor<R, 4, X, T1, T2, T3, T4>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type>(
		       mtd_functor<R, 4, X, T1, T2, T3, T4>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4));
}

template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4>
rebind4<const_mtd_functor<R, 4, X, T1, T2, T3, T4>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type> 
	rebind(const X* x, R (X::*m_ptr)(T1, T2, T3, T4) const
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
{
	return rebind4<const_mtd_functor<R, 4, X, T1, T2, T3, T4>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type>(
		       const_mtd_functor<R, 4, X, T1, T2, T3, T4>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4));
}

/*  \brief rebind the arguments of a function to expressions.
 *  \param fct_ptr pointer to a function with return type R and arguments T1,T2,T3,T4
 *  \param A1,A2,A3,A4 arguments that define the expressions to which the function parameters are bound
 *  \return returns the result of function evaluation of type R
 */
template <typename R, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4>
rebind4<fct_functor<R, 4, T1, T2, T3, T4>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type> 
	rebind(R (*f_ptr)(T1, T2, T3, T4)
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
{
	return rebind4<fct_functor<R, 4, T1, T2, T3, T4>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type>(
		       fct_functor<R, 4, T1, T2, T3, T4>(f_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4));
}


template <class F, typename R, typename E1, typename E2, typename E3, typename E4, typename E5>
struct rebind5
{
	F  functor; 
	E1 e1;
	E2 e2;
	E3 e3;
	E4 e4;
	E5 e5;
	rebind5(const F& _functor, const E1& _e1, const E2& _e2, const E3& _e3, const E4& _e4, const E5& _e5) : 
		functor(_functor), e1(_e1), e2(_e2), e3(_e3), e4(_e4), e5(_e5) {	}
	R operator () () { return functor(e1(), e2(), e3(), e4(), e5()); }

	template <typename T1>
	R operator () (const T1& v1) 
	{ 
		return functor(
	               e1(v1),
	               e2(v1),
	               e3(v1),
	               e4(v1),
	               e5(v1)
		);
	}

	template <typename T1, typename T2>
	R operator () (const T1& v1, const T2& v2) 
	{ 
		return functor(
	               e1(v1, v2),
	               e2(v1, v2),
	               e3(v1, v2),
	               e4(v1, v2),
	               e5(v1, v2)
		);
	}

	template <typename T1, typename T2, typename T3>
	R operator () (const T1& v1, const T2& v2, const T3& v3) 
	{ 
		return functor(
	               e1(v1, v2, v3),
	               e2(v1, v2, v3),
	               e3(v1, v2, v3),
	               e4(v1, v2, v3),
	               e5(v1, v2, v3)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4),
	               e2(v1, v2, v3, v4),
	               e3(v1, v2, v3, v4),
	               e4(v1, v2, v3, v4),
	               e5(v1, v2, v3, v4)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5),
	               e2(v1, v2, v3, v4, v5),
	               e3(v1, v2, v3, v4, v5),
	               e4(v1, v2, v3, v4, v5),
	               e5(v1, v2, v3, v4, v5)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6),
	               e2(v1, v2, v3, v4, v5, v6),
	               e3(v1, v2, v3, v4, v5, v6),
	               e4(v1, v2, v3, v4, v5, v6),
	               e5(v1, v2, v3, v4, v5, v6)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7),
	               e2(v1, v2, v3, v4, v5, v6, v7),
	               e3(v1, v2, v3, v4, v5, v6, v7),
	               e4(v1, v2, v3, v4, v5, v6, v7),
	               e5(v1, v2, v3, v4, v5, v6, v7)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7, v8),
	               e2(v1, v2, v3, v4, v5, v6, v7, v8),
	               e3(v1, v2, v3, v4, v5, v6, v7, v8),
	               e4(v1, v2, v3, v4, v5, v6, v7, v8),
	               e5(v1, v2, v3, v4, v5, v6, v7, v8)
		);
	}

};

/*  \brief rebind the arguments of a function to expressions.
 *  \param inst_ptr pointer to an instance of type X
 *  \param mtd_ptr pointer to a method with return type R and arguments T1, ..., Tn
 *  \param A1, ..., An arguments that define the expressions to which the method parameters are bound
 *  \return returns the result of method evaluation of type R
 *
 * No long description yet.
 */
template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5>
rebind5<mtd_functor<R, 5, X, T1, T2, T3, T4, T5>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type> 
	rebind(X* x, R (X::*m_ptr)(T1, T2, T3, T4, T5)
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
{
	return rebind5<mtd_functor<R, 5, X, T1, T2, T3, T4, T5>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type>(
		       mtd_functor<R, 5, X, T1, T2, T3, T4, T5>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5));
}

template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5>
rebind5<const_mtd_functor<R, 5, X, T1, T2, T3, T4, T5>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type> 
	rebind(const X* x, R (X::*m_ptr)(T1, T2, T3, T4, T5) const
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
{
	return rebind5<const_mtd_functor<R, 5, X, T1, T2, T3, T4, T5>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type>(
		       const_mtd_functor<R, 5, X, T1, T2, T3, T4, T5>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5));
}

/*  \brief rebind the arguments of a function to expressions.
 *  \param fct_ptr pointer to a function with return type R and arguments T1,T2,T3,T4,T5
 *  \param A1,A2,A3,A4,A5 arguments that define the expressions to which the function parameters are bound
 *  \return returns the result of function evaluation of type R
 */
template <typename R, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5>
rebind5<fct_functor<R, 5, T1, T2, T3, T4, T5>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type> 
	rebind(R (*f_ptr)(T1, T2, T3, T4, T5)
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
{
	return rebind5<fct_functor<R, 5, T1, T2, T3, T4, T5>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type>(
		       fct_functor<R, 5, T1, T2, T3, T4, T5>(f_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5));
}


template <class F, typename R, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6>
struct rebind6
{
	F  functor; 
	E1 e1;
	E2 e2;
	E3 e3;
	E4 e4;
	E5 e5;
	E6 e6;
	rebind6(const F& _functor, const E1& _e1, const E2& _e2, const E3& _e3, const E4& _e4, const E5& _e5, const E6& _e6) : 
		functor(_functor), e1(_e1), e2(_e2), e3(_e3), e4(_e4), e5(_e5), e6(_e6) {	}
	R operator () () { return functor(e1(), e2(), e3(), e4(), e5(), e6()); }

	template <typename T1>
	R operator () (const T1& v1) 
	{ 
		return functor(
	               e1(v1),
	               e2(v1),
	               e3(v1),
	               e4(v1),
	               e5(v1),
	               e6(v1)
		);
	}

	template <typename T1, typename T2>
	R operator () (const T1& v1, const T2& v2) 
	{ 
		return functor(
	               e1(v1, v2),
	               e2(v1, v2),
	               e3(v1, v2),
	               e4(v1, v2),
	               e5(v1, v2),
	               e6(v1, v2)
		);
	}

	template <typename T1, typename T2, typename T3>
	R operator () (const T1& v1, const T2& v2, const T3& v3) 
	{ 
		return functor(
	               e1(v1, v2, v3),
	               e2(v1, v2, v3),
	               e3(v1, v2, v3),
	               e4(v1, v2, v3),
	               e5(v1, v2, v3),
	               e6(v1, v2, v3)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4),
	               e2(v1, v2, v3, v4),
	               e3(v1, v2, v3, v4),
	               e4(v1, v2, v3, v4),
	               e5(v1, v2, v3, v4),
	               e6(v1, v2, v3, v4)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5),
	               e2(v1, v2, v3, v4, v5),
	               e3(v1, v2, v3, v4, v5),
	               e4(v1, v2, v3, v4, v5),
	               e5(v1, v2, v3, v4, v5),
	               e6(v1, v2, v3, v4, v5)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6),
	               e2(v1, v2, v3, v4, v5, v6),
	               e3(v1, v2, v3, v4, v5, v6),
	               e4(v1, v2, v3, v4, v5, v6),
	               e5(v1, v2, v3, v4, v5, v6),
	               e6(v1, v2, v3, v4, v5, v6)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7),
	               e2(v1, v2, v3, v4, v5, v6, v7),
	               e3(v1, v2, v3, v4, v5, v6, v7),
	               e4(v1, v2, v3, v4, v5, v6, v7),
	               e5(v1, v2, v3, v4, v5, v6, v7),
	               e6(v1, v2, v3, v4, v5, v6, v7)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7, v8),
	               e2(v1, v2, v3, v4, v5, v6, v7, v8),
	               e3(v1, v2, v3, v4, v5, v6, v7, v8),
	               e4(v1, v2, v3, v4, v5, v6, v7, v8),
	               e5(v1, v2, v3, v4, v5, v6, v7, v8),
	               e6(v1, v2, v3, v4, v5, v6, v7, v8)
		);
	}

};

/*  \brief rebind the arguments of a function to expressions.
 *  \param inst_ptr pointer to an instance of type X
 *  \param mtd_ptr pointer to a method with return type R and arguments T1, ..., Tn
 *  \param A1, ..., An arguments that define the expressions to which the method parameters are bound
 *  \return returns the result of method evaluation of type R
 *
 * No long description yet.
 */
template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5, typename T6, typename A6>
rebind6<mtd_functor<R, 6, X, T1, T2, T3, T4, T5, T6>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type> 
	rebind(X* x, R (X::*m_ptr)(T1, T2, T3, T4, T5, T6)
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
{
	return rebind6<mtd_functor<R, 6, X, T1, T2, T3, T4, T5, T6>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type>(
		       mtd_functor<R, 6, X, T1, T2, T3, T4, T5, T6>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5), make_expression(a6));
}

template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5, typename T6, typename A6>
rebind6<const_mtd_functor<R, 6, X, T1, T2, T3, T4, T5, T6>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type> 
	rebind(const X* x, R (X::*m_ptr)(T1, T2, T3, T4, T5, T6) const
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
{
	return rebind6<const_mtd_functor<R, 6, X, T1, T2, T3, T4, T5, T6>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type>(
		       const_mtd_functor<R, 6, X, T1, T2, T3, T4, T5, T6>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5), make_expression(a6));
}

/*  \brief rebind the arguments of a function to expressions.
 *  \param fct_ptr pointer to a function with return type R and arguments T1,T2,T3,T4,T5,T6
 *  \param A1,A2,A3,A4,A5,A6 arguments that define the expressions to which the function parameters are bound
 *  \return returns the result of function evaluation of type R
 */
template <typename R, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5, typename T6, typename A6>
rebind6<fct_functor<R, 6, T1, T2, T3, T4, T5, T6>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type> 
	rebind(R (*f_ptr)(T1, T2, T3, T4, T5, T6)
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
{
	return rebind6<fct_functor<R, 6, T1, T2, T3, T4, T5, T6>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type>(
		       fct_functor<R, 6, T1, T2, T3, T4, T5, T6>(f_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5), make_expression(a6));
}


template <class F, typename R, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7>
struct rebind7
{
	F  functor; 
	E1 e1;
	E2 e2;
	E3 e3;
	E4 e4;
	E5 e5;
	E6 e6;
	E7 e7;
	rebind7(const F& _functor, const E1& _e1, const E2& _e2, const E3& _e3, const E4& _e4, const E5& _e5, const E6& _e6, const E7& _e7) : 
		functor(_functor), e1(_e1), e2(_e2), e3(_e3), e4(_e4), e5(_e5), e6(_e6), e7(_e7) {	}
	R operator () () { return functor(e1(), e2(), e3(), e4(), e5(), e6(), e7()); }

	template <typename T1>
	R operator () (const T1& v1) 
	{ 
		return functor(
	               e1(v1),
	               e2(v1),
	               e3(v1),
	               e4(v1),
	               e5(v1),
	               e6(v1),
	               e7(v1)
		);
	}

	template <typename T1, typename T2>
	R operator () (const T1& v1, const T2& v2) 
	{ 
		return functor(
	               e1(v1, v2),
	               e2(v1, v2),
	               e3(v1, v2),
	               e4(v1, v2),
	               e5(v1, v2),
	               e6(v1, v2),
	               e7(v1, v2)
		);
	}

	template <typename T1, typename T2, typename T3>
	R operator () (const T1& v1, const T2& v2, const T3& v3) 
	{ 
		return functor(
	               e1(v1, v2, v3),
	               e2(v1, v2, v3),
	               e3(v1, v2, v3),
	               e4(v1, v2, v3),
	               e5(v1, v2, v3),
	               e6(v1, v2, v3),
	               e7(v1, v2, v3)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4),
	               e2(v1, v2, v3, v4),
	               e3(v1, v2, v3, v4),
	               e4(v1, v2, v3, v4),
	               e5(v1, v2, v3, v4),
	               e6(v1, v2, v3, v4),
	               e7(v1, v2, v3, v4)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5),
	               e2(v1, v2, v3, v4, v5),
	               e3(v1, v2, v3, v4, v5),
	               e4(v1, v2, v3, v4, v5),
	               e5(v1, v2, v3, v4, v5),
	               e6(v1, v2, v3, v4, v5),
	               e7(v1, v2, v3, v4, v5)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6),
	               e2(v1, v2, v3, v4, v5, v6),
	               e3(v1, v2, v3, v4, v5, v6),
	               e4(v1, v2, v3, v4, v5, v6),
	               e5(v1, v2, v3, v4, v5, v6),
	               e6(v1, v2, v3, v4, v5, v6),
	               e7(v1, v2, v3, v4, v5, v6)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7),
	               e2(v1, v2, v3, v4, v5, v6, v7),
	               e3(v1, v2, v3, v4, v5, v6, v7),
	               e4(v1, v2, v3, v4, v5, v6, v7),
	               e5(v1, v2, v3, v4, v5, v6, v7),
	               e6(v1, v2, v3, v4, v5, v6, v7),
	               e7(v1, v2, v3, v4, v5, v6, v7)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7, v8),
	               e2(v1, v2, v3, v4, v5, v6, v7, v8),
	               e3(v1, v2, v3, v4, v5, v6, v7, v8),
	               e4(v1, v2, v3, v4, v5, v6, v7, v8),
	               e5(v1, v2, v3, v4, v5, v6, v7, v8),
	               e6(v1, v2, v3, v4, v5, v6, v7, v8),
	               e7(v1, v2, v3, v4, v5, v6, v7, v8)
		);
	}

};

/*  \brief rebind the arguments of a function to expressions.
 *  \param inst_ptr pointer to an instance of type X
 *  \param mtd_ptr pointer to a method with return type R and arguments T1, ..., Tn
 *  \param A1, ..., An arguments that define the expressions to which the method parameters are bound
 *  \return returns the result of method evaluation of type R
 *
 * No long description yet.
 */
template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5, typename T6, typename A6, typename T7, typename A7>
rebind7<mtd_functor<R, 7, X, T1, T2, T3, T4, T5, T6, T7>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type> 
	rebind(X* x, R (X::*m_ptr)(T1, T2, T3, T4, T5, T6, T7)
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
{
	return rebind7<mtd_functor<R, 7, X, T1, T2, T3, T4, T5, T6, T7>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type>(
		       mtd_functor<R, 7, X, T1, T2, T3, T4, T5, T6, T7>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5), make_expression(a6), make_expression(a7));
}

template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5, typename T6, typename A6, typename T7, typename A7>
rebind7<const_mtd_functor<R, 7, X, T1, T2, T3, T4, T5, T6, T7>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type> 
	rebind(const X* x, R (X::*m_ptr)(T1, T2, T3, T4, T5, T6, T7) const
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
{
	return rebind7<const_mtd_functor<R, 7, X, T1, T2, T3, T4, T5, T6, T7>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type>(
		       const_mtd_functor<R, 7, X, T1, T2, T3, T4, T5, T6, T7>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5), make_expression(a6), make_expression(a7));
}

/*  \brief rebind the arguments of a function to expressions.
 *  \param fct_ptr pointer to a function with return type R and arguments T1,T2,T3,T4,T5,T6,T7
 *  \param A1,A2,A3,A4,A5,A6,A7 arguments that define the expressions to which the function parameters are bound
 *  \return returns the result of function evaluation of type R
 */
template <typename R, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5, typename T6, typename A6, typename T7, typename A7>
rebind7<fct_functor<R, 7, T1, T2, T3, T4, T5, T6, T7>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type> 
	rebind(R (*f_ptr)(T1, T2, T3, T4, T5, T6, T7)
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
{
	return rebind7<fct_functor<R, 7, T1, T2, T3, T4, T5, T6, T7>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type>(
		       fct_functor<R, 7, T1, T2, T3, T4, T5, T6, T7>(f_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5), make_expression(a6), make_expression(a7));
}


template <class F, typename R, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8>
struct rebind8
{
	F  functor; 
	E1 e1;
	E2 e2;
	E3 e3;
	E4 e4;
	E5 e5;
	E6 e6;
	E7 e7;
	E8 e8;
	rebind8(const F& _functor, const E1& _e1, const E2& _e2, const E3& _e3, const E4& _e4, const E5& _e5, const E6& _e6, const E7& _e7, const E8& _e8) : 
		functor(_functor), e1(_e1), e2(_e2), e3(_e3), e4(_e4), e5(_e5), e6(_e6), e7(_e7), e8(_e8) {	}
	R operator () () { return functor(e1(), e2(), e3(), e4(), e5(), e6(), e7(), e8()); }

	template <typename T1>
	R operator () (const T1& v1) 
	{ 
		return functor(
	               e1(v1),
	               e2(v1),
	               e3(v1),
	               e4(v1),
	               e5(v1),
	               e6(v1),
	               e7(v1),
	               e8(v1)
		);
	}

	template <typename T1, typename T2>
	R operator () (const T1& v1, const T2& v2) 
	{ 
		return functor(
	               e1(v1, v2),
	               e2(v1, v2),
	               e3(v1, v2),
	               e4(v1, v2),
	               e5(v1, v2),
	               e6(v1, v2),
	               e7(v1, v2),
	               e8(v1, v2)
		);
	}

	template <typename T1, typename T2, typename T3>
	R operator () (const T1& v1, const T2& v2, const T3& v3) 
	{ 
		return functor(
	               e1(v1, v2, v3),
	               e2(v1, v2, v3),
	               e3(v1, v2, v3),
	               e4(v1, v2, v3),
	               e5(v1, v2, v3),
	               e6(v1, v2, v3),
	               e7(v1, v2, v3),
	               e8(v1, v2, v3)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4),
	               e2(v1, v2, v3, v4),
	               e3(v1, v2, v3, v4),
	               e4(v1, v2, v3, v4),
	               e5(v1, v2, v3, v4),
	               e6(v1, v2, v3, v4),
	               e7(v1, v2, v3, v4),
	               e8(v1, v2, v3, v4)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5),
	               e2(v1, v2, v3, v4, v5),
	               e3(v1, v2, v3, v4, v5),
	               e4(v1, v2, v3, v4, v5),
	               e5(v1, v2, v3, v4, v5),
	               e6(v1, v2, v3, v4, v5),
	               e7(v1, v2, v3, v4, v5),
	               e8(v1, v2, v3, v4, v5)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6),
	               e2(v1, v2, v3, v4, v5, v6),
	               e3(v1, v2, v3, v4, v5, v6),
	               e4(v1, v2, v3, v4, v5, v6),
	               e5(v1, v2, v3, v4, v5, v6),
	               e6(v1, v2, v3, v4, v5, v6),
	               e7(v1, v2, v3, v4, v5, v6),
	               e8(v1, v2, v3, v4, v5, v6)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7),
	               e2(v1, v2, v3, v4, v5, v6, v7),
	               e3(v1, v2, v3, v4, v5, v6, v7),
	               e4(v1, v2, v3, v4, v5, v6, v7),
	               e5(v1, v2, v3, v4, v5, v6, v7),
	               e6(v1, v2, v3, v4, v5, v6, v7),
	               e7(v1, v2, v3, v4, v5, v6, v7),
	               e8(v1, v2, v3, v4, v5, v6, v7)
		);
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	R operator () (const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8) 
	{ 
		return functor(
	               e1(v1, v2, v3, v4, v5, v6, v7, v8),
	               e2(v1, v2, v3, v4, v5, v6, v7, v8),
	               e3(v1, v2, v3, v4, v5, v6, v7, v8),
	               e4(v1, v2, v3, v4, v5, v6, v7, v8),
	               e5(v1, v2, v3, v4, v5, v6, v7, v8),
	               e6(v1, v2, v3, v4, v5, v6, v7, v8),
	               e7(v1, v2, v3, v4, v5, v6, v7, v8),
	               e8(v1, v2, v3, v4, v5, v6, v7, v8)
		);
	}

};

/*  \brief rebind the arguments of a function to expressions.
 *  \param inst_ptr pointer to an instance of type X
 *  \param mtd_ptr pointer to a method with return type R and arguments T1, ..., Tn
 *  \param A1, ..., An arguments that define the expressions to which the method parameters are bound
 *  \return returns the result of method evaluation of type R
 *
 * No long description yet.
 */
template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5, typename T6, typename A6, typename T7, typename A7, typename T8, typename A8>
rebind8<mtd_functor<R, 8, X, T1, T2, T3, T4, T5, T6, T7, T8>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type, typename make_expression_type<A8>::type> 
	rebind(X* x, R (X::*m_ptr)(T1, T2, T3, T4, T5, T6, T7, T8)
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
{
	return rebind8<mtd_functor<R, 8, X, T1, T2, T3, T4, T5, T6, T7, T8>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type, typename make_expression_type<A8>::type>(
		       mtd_functor<R, 8, X, T1, T2, T3, T4, T5, T6, T7, T8>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5), make_expression(a6), make_expression(a7), make_expression(a8));
}

template <typename R, class X, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5, typename T6, typename A6, typename T7, typename A7, typename T8, typename A8>
rebind8<const_mtd_functor<R, 8, X, T1, T2, T3, T4, T5, T6, T7, T8>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type, typename make_expression_type<A8>::type> 
	rebind(const X* x, R (X::*m_ptr)(T1, T2, T3, T4, T5, T6, T7, T8) const
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
{
	return rebind8<const_mtd_functor<R, 8, X, T1, T2, T3, T4, T5, T6, T7, T8>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type, typename make_expression_type<A8>::type>(
		       const_mtd_functor<R, 8, X, T1, T2, T3, T4, T5, T6, T7, T8>(x, m_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5), make_expression(a6), make_expression(a7), make_expression(a8));
}

/*  \brief rebind the arguments of a function to expressions.
 *  \param fct_ptr pointer to a function with return type R and arguments T1,T2,T3,T4,T5,T6,T7,T8
 *  \param A1,A2,A3,A4,A5,A6,A7,A8 arguments that define the expressions to which the function parameters are bound
 *  \return returns the result of function evaluation of type R
 */
template <typename R, typename T1, typename A1, typename T2, typename A2, typename T3, typename A3, typename T4, typename A4, typename T5, typename A5, typename T6, typename A6, typename T7, typename A7, typename T8, typename A8>
rebind8<fct_functor<R, 8, T1, T2, T3, T4, T5, T6, T7, T8>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type, typename make_expression_type<A8>::type> 
	rebind(R (*f_ptr)(T1, T2, T3, T4, T5, T6, T7, T8)
	, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
{
	return rebind8<fct_functor<R, 8, T1, T2, T3, T4, T5, T6, T7, T8>, R
           , typename make_expression_type<A1>::type, typename make_expression_type<A2>::type, typename make_expression_type<A3>::type, typename make_expression_type<A4>::type, typename make_expression_type<A5>::type, typename make_expression_type<A6>::type, typename make_expression_type<A7>::type, typename make_expression_type<A8>::type>(
		       fct_functor<R, 8, T1, T2, T3, T4, T5, T6, T7, T8>(f_ptr)
		       , make_expression(a1), make_expression(a2), make_expression(a3), make_expression(a4), make_expression(a5), make_expression(a6), make_expression(a7), make_expression(a8));
}



	}
}