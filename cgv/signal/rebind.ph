#pragma once
@exclude <cgv/config/ppp.ppp>
#include <cgv/type/cond/is_base_of.h>
#include <cgv/type/func/make_argument.h>
#include <cgv/type/invalid_type.h>

using cgv::type::func::make_argument;
using cgv::type::func::clean;

namespace cgv {
	namespace signal {

/* functors *******************************************************************/

template <typename R, int i, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
struct fct_functor;

template <typename R, int i, class X, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
struct mtd_functor;

template <typename R, int i, class X, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
struct const_mtd_functor;

@for (i=0; i<=N_ARG; i=i+1) @{
template <typename R, @["typename T1"; ", "; "typename T".N_ARG]>
struct fct_functor<R, @(i),@["T1"; ", "; "T".N_ARG]>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (*function_ptr)(@["T1"; ", "; "T".i]);
	function_ptr fp;
	fct_functor(function_ptr _fp) : fp(_fp) {}
	R operator() (@["const typename clean<T1>::type& v1"; ", "; "const typename clean<T".i.">::type& v".i]) const 
	{ 
		return fp(@["const_cast<typename make_argument<T1>::type>(v1)"; ", "; "const_cast<typename make_argument<T".i.">::type>(v".i.")"]);
	}
};

template <typename R, class X, @["typename T1"; ", "; "typename T".N_ARG]>
struct mtd_functor<R,@(i),X,@["T1"; ", "; "T".N_ARG]>
{
	typedef R return_type;
	static const bool is_void = false;
	typedef R (X::*method_ptr)(@["T1"; ", "; "T".i]);
	X* ip;
	method_ptr mp;
	mtd_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}	
	R operator() (@["const typename clean<T1>::type& v1"; ", "; "const typename clean<T".i.">::type& v".i]) const 
	{
		return (ip->*mp)(@["const_cast<typename make_argument<T1>::type>(v1)"; ", "; "const_cast<typename make_argument<T".i.">::type>(v".i.")"]); 
	}
};

template <typename R, class X, @["typename T1"; ", "; "typename T".N_ARG]>
struct const_mtd_functor<R,@(i),X,@["T1"; ", "; "T".N_ARG]>
{
	typedef R (X::*const_method_ptr)(@["T1"; ", "; "T".i]) const;
	const X* ip;
	const_method_ptr mp;
	const_mtd_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	R operator() (@["const typename clean<T1>::type& v1"; ", "; "const typename clean<T".i.">::type& v".i]) const 
	{
		return (ip->*mp)(@["const_cast<typename make_argument<T1>::type>(v1)"; ", "; "const_cast<typename make_argument<T".i.">::type>(v".i.")"]); 
	}
};

@}

/* expressions *******************************************************************/

namespace expression {
	struct tag {};
}

template <typename T>
struct const_expression : public expression::tag
{
	const T v;
	const_expression(const T& _v) : v(_v) {}
	const T operator() () { return v; }@for (i=1; i<=N_ARG; i=i+1) @{
	template <@["typename T1"; ", "; "typename T".i]>
	const T operator() (@["const T1&"; ", "; "const T".i."&"]) { return v; }@}
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
	T& operator() () { return v; }@for (i=1; i<=N_ARG; i=i+1) @{
	template <@["typename T1"; ", "; "typename T".i]>
	T& operator() (@["const T1&"; ", "; "const T".i."&"]) { return v; }@}
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
	const T& operator() () { return v; }@for (i=1; i<=N_ARG; i=i+1) @{
	template <@["typename T1"; ", "; "typename T".i]>
	const T& operator() (@["const T1&"; ", "; "const T".i."&"]) { return v; }@}
};

template <typename T> 
const_ref_expression<T> _cr(const T& v) 
{
	return const_ref_expression<T>(v); 
}

template <int i>
struct placeholder_expression;

@for (i=1; i<=N_ARG; i=i+1) @{
template <>
struct placeholder_expression<@(i)> : public expression::tag
{
	void operator() () {}@for (j=1; j<i; j=j+1) @{
	template <@["typename T1"; ", "; "typename T".j]>
	void operator() (@["T1&"; ", "; "T".j."&"]) { }@}@for (j=i; j<=N_ARG; j=j+1) @{
	template <@["typename T1"; ", "; "typename T".j]>
	const T@(i)& operator() (@["const T1& v1"; ", "; "const T".j."& v".j]) { return v@(i); }@}
};

static placeholder_expression<@(i)> _@(i);
@}

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

@for (i=0; i<=N_ARG; i=i+1) @{
template <class F, typename R@[", typename E1";"";", typename E".i]>
struct rebind@(i)
{
	F  functor; @for (j=1; j<=i; j=j+1) @{
	E@(j) e@(j);@}
	rebind@(i)(const F& _functor@[", const E1& _e1"; ""; ", const E".i."& _e".i]) : 
		functor(_functor)@[", e1(_e1)"; ""; ", e".i."(_e".i.")"] {	}
	R operator () () { return functor(@["e1()"; ", "; "e".i."()"]); }
@for (j=1; j<=N_ARG; j=j+1) @{
	template <@["typename T1"; ", "; "typename T".j]>
	R operator () (@["const T1& v1"; ", "; "const T".j."& v".j]) 
	{ 
		return functor(@for (k=1; k<=i; ++k) @{
	               e@(k)(@["v1"; ", "; "v".j])@if(k<i)@{,@}@}
		);
	}
@}
};

/*  \brief rebind the arguments of a function to expressions.
 *  \param inst_ptr pointer to an instance of type X
 *  \param mtd_ptr pointer to a method with return type R and arguments T1, ..., Tn
 *  \param A1, ..., An arguments that define the expressions to which the method parameters are bound
 *  \return returns the result of method evaluation of type R
 *
 * No long description yet.
 */
template <typename R, class X@[", typename T1, typename A1"; ""; ", typename T".i.", typename A".i]>
rebind@(i)<mtd_functor<R, @(i), X@[", T1";"";", T".i]>, R
           @[", typename make_expression_type<A1>::type"; ""; ", typename make_expression_type<A".i.">::type"]> 
	rebind(X* x, R (X::*m_ptr)(@["T1"; ", "; "T".i])
	@[", const A1& a1"; ""; ", const A".i."& a".i])
{
	return rebind@(i)<mtd_functor<R, @(i), X@[", T1";"";", T".i]>, R
           @[", typename make_expression_type<A1>::type"; ""; ", typename make_expression_type<A".i.">::type"]>(
		       mtd_functor<R, @(i), X@[", T1";"";", T".i]>(x, m_ptr)
		       @[", make_expression(a1)";""; ", make_expression(a".i.")"]);
}

template <typename R, class X@[", typename T1, typename A1"; ""; ", typename T".i.", typename A".i]>
rebind@(i)<const_mtd_functor<R, @(i), X@[", T1";"";", T".i]>, R
           @[", typename make_expression_type<A1>::type"; ""; ", typename make_expression_type<A".i.">::type"]> 
	rebind(const X* x, R (X::*m_ptr)(@["T1"; ", "; "T".i]) const
	@[", const A1& a1"; ""; ", const A".i."& a".i])
{
	return rebind@(i)<const_mtd_functor<R, @(i), X@[", T1";"";", T".i]>, R
           @[", typename make_expression_type<A1>::type"; ""; ", typename make_expression_type<A".i.">::type"]>(
		       const_mtd_functor<R, @(i), X@[", T1";"";", T".i]>(x, m_ptr)
		       @[", make_expression(a1)";""; ", make_expression(a".i.")"]);
}

/*  \brief rebind the arguments of a function to expressions.
 *  \param fct_ptr pointer to a function with return type R and arguments @["T1";",";"T".i]
 *  \param @["A1";",";"A".i] arguments that define the expressions to which the function parameters are bound
 *  \return returns the result of function evaluation of type R
 */
template <typename R@[", typename T1, typename A1"; ""; ", typename T".i.", typename A".i]>
rebind@(i)<fct_functor<R, @(i)@[", T1";"";", T".i]>, R
           @[", typename make_expression_type<A1>::type"; ""; ", typename make_expression_type<A".i.">::type"]> 
	rebind(R (*f_ptr)(@["T1"; ", "; "T".i])
	@[", const A1& a1"; ""; ", const A".i."& a".i])
{
	return rebind@(i)<fct_functor<R, @(i)@[", T1";"";", T".i]>, R
           @[", typename make_expression_type<A1>::type"; ""; ", typename make_expression_type<A".i.">::type"]>(
		       fct_functor<R, @(i)@[", T1";"";", T".i]>(f_ptr)
		       @[", make_expression(a1)";""; ", make_expression(a".i.")"]);
}

@}

	}
}