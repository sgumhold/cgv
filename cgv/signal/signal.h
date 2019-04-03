#pragma once

#include <cgv/signal/abst_signal.h>
#include <cgv/type/invalid_type.h>
#include <cgv/type/cond/is_base_of.h>
#include <vector>
#include <map>

#include "lib_begin.h"

namespace cgv {
	namespace signal {

template <int i, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class functor;

template <int i, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class signal_impl;

// signal that sends callbacks to connected functions, methods or functors when executing the ()-oeprator of the signal
template <typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class signal : public signal_impl<type::count_valid_types<T1, T2, T3, T4, T5, T6, T7, T8>::value, T1, T2, T3, T4, T5, T6, T7, T8>, public tacker
{
};

template <int i, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class function_functor;

template <int i, class X, class Y, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class method_functor;

template <int i, typename X, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class object_functor;

template <int i, class X, class Y, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class const_method_functor;

//template <int i, typename X, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
//class const_object_functor;



template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class functor<0,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<0,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual void operator() () const = 0;
};

// implementation of a boolean signal with a signature of length 0
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class signal_impl<0,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<0,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef functor<0,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;
	// emit the signal with the ()-operator by using it like bool function 
	void operator ()() const
	{
		unsigned n = (unsigned) functors.size();
		for (unsigned i=0; i<n; ++i)
			(*static_cast<functor_type*>(functors[i]))();
	}
	functor_type& connect(const functor_type& _f)    { functor_type *f = static_cast<functor_type*>(_f.clone()); signal_base::connect(f); return *f; }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class function_functor<0,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<0,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (*function_ptr)();
	function_ptr fp;
	function_functor(function_ptr _fp) : fp(_fp) {}
	void operator() () const { fp(); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (const void*)fp; }
	virtual functor_base* clone() const { return new function_functor(*this); }
};

// connect signal to a void function
extern void CGV_API connect(signal<>& s, void (*fp)());

// disconnect signal from a void function
extern void CGV_API disconnect(signal<>& s, void (*fp)());


template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class method_functor<0,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<0,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*method_ptr)();
	X* ip;
	method_ptr mp;
	method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() () const { (ip->*mp)(); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<const Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new method_functor(*this); }
};

/// connect signal to a void method of an object
template <class X>
void connect(signal<>& s, X* ip, void (X::*mp)()) { s.connect(method_functor<0,X,X>(ip,mp)); }

//! connect signal to a void method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X>
void connect(signal<>& s, X* ip, void (X::*mp)()) { s.connect(method_functor<0,X,Y>(ip,mp)); }

/// disconnect signal from a void method of an object
template <class X>
void disconnect(signal<>& s, X* ip, void (X::*mp)()) { s.disconnect(method_functor<0,X,X>(ip,mp)); }

//! disconnect signal from a void method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X>
void disconnect(signal<>& s, X* ip, void (X::*mp)()) { s.disconnect(method_functor<0,X,Y>(ip,mp)); }

template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_method_functor<0,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<0,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*const_method_ptr)() const;
	const X* ip;
	const_method_ptr mp;
	const_method_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() () const { (ip->*mp)(); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_method_functor(*this); }
};

/// connect signal to a void const method of an object
template <class X>
void connect(signal<>& s, X* ip, void (X::*mp)() const) { s.connect(const_method_functor<0,X,X>(ip,mp)); }

/// disconnect signal from a void const method of an object
template <class X>
void disconnect(signal<>& s, X* ip, void (X::*mp)() const) { s.disconnect(const_method_functor<0,X,X>(ip,mp)); }

//! connect signal to a void const method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X>
void connect(signal<>& s, X* ip, void (X::*mp)() const) { s.connect(const_method_functor<0,X,Y>(ip,mp)); }

//! disconnect signal from a void const method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X>
void disconnect(signal<>& s, X* ip, void (X::*mp)() const) { s.disconnect(const_method_functor<0,X,Y>(ip,mp)); }


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class object_functor<0,X,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<0,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() () const { ip(); }
	const tacker* get_tacker() const { return type::cond::is_base_of<tacker,X>::value?reinterpret_cast<const tacker*>(&ip):0; }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X>
void connect(signal<>& s, X& ip) { s.connect(object_functor<0,X&>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X>
void disconnect(signal<>& s, X& ip) { s.disconnect(object_functor<0,X&>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X>
void connect(signal<>& s, const X& ip) { s.connect(object_functor<0,const X&>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X>
void disconnect(signal<>& s, const X& ip) { s.disconnect(object_functor<0,const X&>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X>
X& connect_copy(signal<>& s, const X& ip) { 
	return static_cast<const object_functor<0,X>&>(s.connect(object_functor<0,X>(ip))).ip; 
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class functor<1,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<1,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual void operator() (typename S::A1 v1) const = 0;
};

// implementation of a boolean signal with a signature of length 1
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class signal_impl<1,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<1,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef functor<1,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;
	// emit the signal with the ()-operator by using it like bool function 
	void operator ()(typename S::A1 v1) const
	{
		unsigned n = (unsigned) functors.size();
		for (unsigned i=0; i<n; ++i)
			(*static_cast<functor_type*>(functors[i]))(v1);
	}
	functor_type& connect(const functor_type& _f)    { functor_type *f = static_cast<functor_type*>(_f.clone()); signal_base::connect(f); return *f; }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class function_functor<1,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<1,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (*function_ptr)(T1);
	function_ptr fp;
	function_functor(function_ptr _fp) : fp(_fp) {}
	void operator() (typename S::A1 v1) const { fp(v1); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (const void*)fp; }
	virtual functor_base* clone() const { return new function_functor(*this); }
};

// connect signal to a void function
template <typename T1>
void connect(signal<T1>& s, void (*fp)(T1)) { s.connect(function_functor<1,T1>(fp)); }


// disconnect signal from a void function
template <typename T1>
void disconnect(signal<T1>& s, void (*fp)(T1)) { s.disconnect(function_functor<1,T1>(fp)); }



template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class method_functor<1,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<1,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*method_ptr)(T1);
	X* ip;
	method_ptr mp;
	method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1) const { (ip->*mp)(v1); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<const Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new method_functor(*this); }
};

/// connect signal to a void method of an object
template <class X, typename T1>
void connect(signal<T1>& s, X* ip, void (X::*mp)(T1)) { s.connect(method_functor<1,X,X,T1>(ip,mp)); }

//! connect signal to a void method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1>
void connect(signal<T1>& s, X* ip, void (X::*mp)(T1)) { s.connect(method_functor<1,X,Y,T1>(ip,mp)); }

/// disconnect signal from a void method of an object
template <class X, typename T1>
void disconnect(signal<T1>& s, X* ip, void (X::*mp)(T1)) { s.disconnect(method_functor<1,X,X,T1>(ip,mp)); }

//! disconnect signal from a void method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1>
void disconnect(signal<T1>& s, X* ip, void (X::*mp)(T1)) { s.disconnect(method_functor<1,X,Y,T1>(ip,mp)); }

template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_method_functor<1,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<1,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*const_method_ptr)(T1) const;
	const X* ip;
	const_method_ptr mp;
	const_method_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1) const { (ip->*mp)(v1); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_method_functor(*this); }
};

/// connect signal to a void const method of an object
template <class X, typename T1>
void connect(signal<T1>& s, X* ip, void (X::*mp)(T1) const) { s.connect(const_method_functor<1,X,X,T1>(ip,mp)); }

/// disconnect signal from a void const method of an object
template <class X, typename T1>
void disconnect(signal<T1>& s, X* ip, void (X::*mp)(T1) const) { s.disconnect(const_method_functor<1,X,X,T1>(ip,mp)); }

//! connect signal to a void const method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1>
void connect(signal<T1>& s, X* ip, void (X::*mp)(T1) const) { s.connect(const_method_functor<1,X,Y,T1>(ip,mp)); }

//! disconnect signal from a void const method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1>
void disconnect(signal<T1>& s, X* ip, void (X::*mp)(T1) const) { s.disconnect(const_method_functor<1,X,Y,T1>(ip,mp)); }


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class object_functor<1,X,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<1,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1) const { ip(v1); }
	const tacker* get_tacker() const { return type::cond::is_base_of<tacker,X>::value?reinterpret_cast<const tacker*>(&ip):0; }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1>
void connect(signal<T1>& s, X& ip) { s.connect(object_functor<1,X&,T1>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1>
void disconnect(signal<T1>& s, X& ip) { s.disconnect(object_functor<1,X&,T1>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1>
void connect(signal<T1>& s, const X& ip) { s.connect(object_functor<1,const X&,T1>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1>
void disconnect(signal<T1>& s, const X& ip) { s.disconnect(object_functor<1,const X&,T1>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1>
X& connect_copy(signal<T1>& s, const X& ip) { 
	return static_cast<const object_functor<1,X,T1>&>(s.connect(object_functor<1,X,T1>(ip))).ip; 
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class functor<2,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<2,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual void operator() (typename S::A1 v1, typename S::A2 v2) const = 0;
};

// implementation of a boolean signal with a signature of length 2
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class signal_impl<2,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<2,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef functor<2,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;
	// emit the signal with the ()-operator by using it like bool function 
	void operator ()(typename S::A1 v1, typename S::A2 v2) const
	{
		unsigned n = (unsigned) functors.size();
		for (unsigned i=0; i<n; ++i)
			(*static_cast<functor_type*>(functors[i]))(v1, v2);
	}
	functor_type& connect(const functor_type& _f)    { functor_type *f = static_cast<functor_type*>(_f.clone()); signal_base::connect(f); return *f; }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class function_functor<2,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<2,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (*function_ptr)(T1, T2);
	function_ptr fp;
	function_functor(function_ptr _fp) : fp(_fp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2) const { fp(v1,v2); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (const void*)fp; }
	virtual functor_base* clone() const { return new function_functor(*this); }
};

// connect signal to a void function
template <typename T1, typename T2>
void connect(signal<T1,T2>& s, void (*fp)(T1, T2)) { s.connect(function_functor<2,T1, T2>(fp)); }


// disconnect signal from a void function
template <typename T1, typename T2>
void disconnect(signal<T1,T2>& s, void (*fp)(T1, T2)) { s.disconnect(function_functor<2,T1, T2>(fp)); }



template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class method_functor<2,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<2,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*method_ptr)(T1, T2);
	X* ip;
	method_ptr mp;
	method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2) const { (ip->*mp)(v1, v2); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<const Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new method_functor(*this); }
};

/// connect signal to a void method of an object
template <class X, typename T1, typename T2>
void connect(signal<T1, T2>& s, X* ip, void (X::*mp)(T1, T2)) { s.connect(method_functor<2,X,X,T1, T2>(ip,mp)); }

//! connect signal to a void method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2>
void connect(signal<T1, T2>& s, X* ip, void (X::*mp)(T1, T2)) { s.connect(method_functor<2,X,Y,T1, T2>(ip,mp)); }

/// disconnect signal from a void method of an object
template <class X, typename T1, typename T2>
void disconnect(signal<T1, T2>& s, X* ip, void (X::*mp)(T1, T2)) { s.disconnect(method_functor<2,X,X,T1, T2>(ip,mp)); }

//! disconnect signal from a void method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2>
void disconnect(signal<T1, T2>& s, X* ip, void (X::*mp)(T1, T2)) { s.disconnect(method_functor<2,X,Y,T1, T2>(ip,mp)); }

template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_method_functor<2,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<2,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*const_method_ptr)(T1, T2) const;
	const X* ip;
	const_method_ptr mp;
	const_method_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2) const { (ip->*mp)(v1, v2); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_method_functor(*this); }
};

/// connect signal to a void const method of an object
template <class X, typename T1, typename T2>
void connect(signal<T1, T2>& s, X* ip, void (X::*mp)(T1, T2) const) { s.connect(const_method_functor<2,X,X,T1, T2>(ip,mp)); }

/// disconnect signal from a void const method of an object
template <class X, typename T1, typename T2>
void disconnect(signal<T1, T2>& s, X* ip, void (X::*mp)(T1, T2) const) { s.disconnect(const_method_functor<2,X,X,T1, T2>(ip,mp)); }

//! connect signal to a void const method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2>
void connect(signal<T1, T2>& s, X* ip, void (X::*mp)(T1, T2) const) { s.connect(const_method_functor<2,X,Y,T1, T2>(ip,mp)); }

//! disconnect signal from a void const method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2>
void disconnect(signal<T1, T2>& s, X* ip, void (X::*mp)(T1, T2) const) { s.disconnect(const_method_functor<2,X,Y,T1, T2>(ip,mp)); }


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class object_functor<2,X,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<2,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2) const { ip(v1, v2); }
	const tacker* get_tacker() const { return type::cond::is_base_of<tacker,X>::value?reinterpret_cast<const tacker*>(&ip):0; }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2>
void connect(signal<T1, T2>& s, X& ip) { s.connect(object_functor<2,X&,T1, T2>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2>
void disconnect(signal<T1, T2>& s, X& ip) { s.disconnect(object_functor<2,X&,T1, T2>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2>
void connect(signal<T1, T2>& s, const X& ip) { s.connect(object_functor<2,const X&,T1, T2>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2>
void disconnect(signal<T1, T2>& s, const X& ip) { s.disconnect(object_functor<2,const X&,T1, T2>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2>
X& connect_copy(signal<T1, T2>& s, const X& ip) { 
	return static_cast<const object_functor<2,X,T1, T2>&>(s.connect(object_functor<2,X,T1, T2>(ip))).ip; 
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class functor<3,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<3,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const = 0;
};

// implementation of a boolean signal with a signature of length 3
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class signal_impl<3,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<3,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef functor<3,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;
	// emit the signal with the ()-operator by using it like bool function 
	void operator ()(typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const
	{
		unsigned n = (unsigned) functors.size();
		for (unsigned i=0; i<n; ++i)
			(*static_cast<functor_type*>(functors[i]))(v1, v2, v3);
	}
	functor_type& connect(const functor_type& _f)    { functor_type *f = static_cast<functor_type*>(_f.clone()); signal_base::connect(f); return *f; }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class function_functor<3,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<3,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (*function_ptr)(T1, T2, T3);
	function_ptr fp;
	function_functor(function_ptr _fp) : fp(_fp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const { fp(v1,v2,v3); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (const void*)fp; }
	virtual functor_base* clone() const { return new function_functor(*this); }
};

// connect signal to a void function
template <typename T1, typename T2, typename T3>
void connect(signal<T1,T2,T3>& s, void (*fp)(T1, T2, T3)) { s.connect(function_functor<3,T1, T2, T3>(fp)); }


// disconnect signal from a void function
template <typename T1, typename T2, typename T3>
void disconnect(signal<T1,T2,T3>& s, void (*fp)(T1, T2, T3)) { s.disconnect(function_functor<3,T1, T2, T3>(fp)); }



template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class method_functor<3,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<3,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*method_ptr)(T1, T2, T3);
	X* ip;
	method_ptr mp;
	method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const { (ip->*mp)(v1, v2, v3); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<const Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new method_functor(*this); }
};

/// connect signal to a void method of an object
template <class X, typename T1, typename T2, typename T3>
void connect(signal<T1, T2, T3>& s, X* ip, void (X::*mp)(T1, T2, T3)) { s.connect(method_functor<3,X,X,T1, T2, T3>(ip,mp)); }

//! connect signal to a void method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3>
void connect(signal<T1, T2, T3>& s, X* ip, void (X::*mp)(T1, T2, T3)) { s.connect(method_functor<3,X,Y,T1, T2, T3>(ip,mp)); }

/// disconnect signal from a void method of an object
template <class X, typename T1, typename T2, typename T3>
void disconnect(signal<T1, T2, T3>& s, X* ip, void (X::*mp)(T1, T2, T3)) { s.disconnect(method_functor<3,X,X,T1, T2, T3>(ip,mp)); }

//! disconnect signal from a void method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3>
void disconnect(signal<T1, T2, T3>& s, X* ip, void (X::*mp)(T1, T2, T3)) { s.disconnect(method_functor<3,X,Y,T1, T2, T3>(ip,mp)); }

template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_method_functor<3,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<3,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*const_method_ptr)(T1, T2, T3) const;
	const X* ip;
	const_method_ptr mp;
	const_method_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const { (ip->*mp)(v1, v2, v3); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_method_functor(*this); }
};

/// connect signal to a void const method of an object
template <class X, typename T1, typename T2, typename T3>
void connect(signal<T1, T2, T3>& s, X* ip, void (X::*mp)(T1, T2, T3) const) { s.connect(const_method_functor<3,X,X,T1, T2, T3>(ip,mp)); }

/// disconnect signal from a void const method of an object
template <class X, typename T1, typename T2, typename T3>
void disconnect(signal<T1, T2, T3>& s, X* ip, void (X::*mp)(T1, T2, T3) const) { s.disconnect(const_method_functor<3,X,X,T1, T2, T3>(ip,mp)); }

//! connect signal to a void const method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3>
void connect(signal<T1, T2, T3>& s, X* ip, void (X::*mp)(T1, T2, T3) const) { s.connect(const_method_functor<3,X,Y,T1, T2, T3>(ip,mp)); }

//! disconnect signal from a void const method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3>
void disconnect(signal<T1, T2, T3>& s, X* ip, void (X::*mp)(T1, T2, T3) const) { s.disconnect(const_method_functor<3,X,Y,T1, T2, T3>(ip,mp)); }


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class object_functor<3,X,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<3,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const { ip(v1, v2, v3); }
	const tacker* get_tacker() const { return type::cond::is_base_of<tacker,X>::value?reinterpret_cast<const tacker*>(&ip):0; }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3>
void connect(signal<T1, T2, T3>& s, X& ip) { s.connect(object_functor<3,X&,T1, T2, T3>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3>
void disconnect(signal<T1, T2, T3>& s, X& ip) { s.disconnect(object_functor<3,X&,T1, T2, T3>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3>
void connect(signal<T1, T2, T3>& s, const X& ip) { s.connect(object_functor<3,const X&,T1, T2, T3>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3>
void disconnect(signal<T1, T2, T3>& s, const X& ip) { s.disconnect(object_functor<3,const X&,T1, T2, T3>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3>
X& connect_copy(signal<T1, T2, T3>& s, const X& ip) { 
	return static_cast<const object_functor<3,X,T1, T2, T3>&>(s.connect(object_functor<3,X,T1, T2, T3>(ip))).ip; 
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class functor<4,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<4,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const = 0;
};

// implementation of a boolean signal with a signature of length 4
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class signal_impl<4,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<4,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef functor<4,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;
	// emit the signal with the ()-operator by using it like bool function 
	void operator ()(typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const
	{
		unsigned n = (unsigned) functors.size();
		for (unsigned i=0; i<n; ++i)
			(*static_cast<functor_type*>(functors[i]))(v1, v2, v3, v4);
	}
	functor_type& connect(const functor_type& _f)    { functor_type *f = static_cast<functor_type*>(_f.clone()); signal_base::connect(f); return *f; }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class function_functor<4,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<4,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (*function_ptr)(T1, T2, T3, T4);
	function_ptr fp;
	function_functor(function_ptr _fp) : fp(_fp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const { fp(v1,v2,v3,v4); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (const void*)fp; }
	virtual functor_base* clone() const { return new function_functor(*this); }
};

// connect signal to a void function
template <typename T1, typename T2, typename T3, typename T4>
void connect(signal<T1,T2,T3,T4>& s, void (*fp)(T1, T2, T3, T4)) { s.connect(function_functor<4,T1, T2, T3, T4>(fp)); }


// disconnect signal from a void function
template <typename T1, typename T2, typename T3, typename T4>
void disconnect(signal<T1,T2,T3,T4>& s, void (*fp)(T1, T2, T3, T4)) { s.disconnect(function_functor<4,T1, T2, T3, T4>(fp)); }



template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class method_functor<4,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<4,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*method_ptr)(T1, T2, T3, T4);
	X* ip;
	method_ptr mp;
	method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const { (ip->*mp)(v1, v2, v3, v4); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<const Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new method_functor(*this); }
};

/// connect signal to a void method of an object
template <class X, typename T1, typename T2, typename T3, typename T4>
void connect(signal<T1, T2, T3, T4>& s, X* ip, void (X::*mp)(T1, T2, T3, T4)) { s.connect(method_functor<4,X,X,T1, T2, T3, T4>(ip,mp)); }

//! connect signal to a void method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4>
void connect(signal<T1, T2, T3, T4>& s, X* ip, void (X::*mp)(T1, T2, T3, T4)) { s.connect(method_functor<4,X,Y,T1, T2, T3, T4>(ip,mp)); }

/// disconnect signal from a void method of an object
template <class X, typename T1, typename T2, typename T3, typename T4>
void disconnect(signal<T1, T2, T3, T4>& s, X* ip, void (X::*mp)(T1, T2, T3, T4)) { s.disconnect(method_functor<4,X,X,T1, T2, T3, T4>(ip,mp)); }

//! disconnect signal from a void method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4>
void disconnect(signal<T1, T2, T3, T4>& s, X* ip, void (X::*mp)(T1, T2, T3, T4)) { s.disconnect(method_functor<4,X,Y,T1, T2, T3, T4>(ip,mp)); }

template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_method_functor<4,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<4,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*const_method_ptr)(T1, T2, T3, T4) const;
	const X* ip;
	const_method_ptr mp;
	const_method_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const { (ip->*mp)(v1, v2, v3, v4); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_method_functor(*this); }
};

/// connect signal to a void const method of an object
template <class X, typename T1, typename T2, typename T3, typename T4>
void connect(signal<T1, T2, T3, T4>& s, X* ip, void (X::*mp)(T1, T2, T3, T4) const) { s.connect(const_method_functor<4,X,X,T1, T2, T3, T4>(ip,mp)); }

/// disconnect signal from a void const method of an object
template <class X, typename T1, typename T2, typename T3, typename T4>
void disconnect(signal<T1, T2, T3, T4>& s, X* ip, void (X::*mp)(T1, T2, T3, T4) const) { s.disconnect(const_method_functor<4,X,X,T1, T2, T3, T4>(ip,mp)); }

//! connect signal to a void const method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4>
void connect(signal<T1, T2, T3, T4>& s, X* ip, void (X::*mp)(T1, T2, T3, T4) const) { s.connect(const_method_functor<4,X,Y,T1, T2, T3, T4>(ip,mp)); }

//! disconnect signal from a void const method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4>
void disconnect(signal<T1, T2, T3, T4>& s, X* ip, void (X::*mp)(T1, T2, T3, T4) const) { s.disconnect(const_method_functor<4,X,Y,T1, T2, T3, T4>(ip,mp)); }


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class object_functor<4,X,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<4,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const { ip(v1, v2, v3, v4); }
	const tacker* get_tacker() const { return type::cond::is_base_of<tacker,X>::value?reinterpret_cast<const tacker*>(&ip):0; }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4>
void connect(signal<T1, T2, T3, T4>& s, X& ip) { s.connect(object_functor<4,X&,T1, T2, T3, T4>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4>
void disconnect(signal<T1, T2, T3, T4>& s, X& ip) { s.disconnect(object_functor<4,X&,T1, T2, T3, T4>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4>
void connect(signal<T1, T2, T3, T4>& s, const X& ip) { s.connect(object_functor<4,const X&,T1, T2, T3, T4>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4>
void disconnect(signal<T1, T2, T3, T4>& s, const X& ip) { s.disconnect(object_functor<4,const X&,T1, T2, T3, T4>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3, typename T4>
X& connect_copy(signal<T1, T2, T3, T4>& s, const X& ip) { 
	return static_cast<const object_functor<4,X,T1, T2, T3, T4>&>(s.connect(object_functor<4,X,T1, T2, T3, T4>(ip))).ip; 
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class functor<5,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<5,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const = 0;
};

// implementation of a boolean signal with a signature of length 5
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class signal_impl<5,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<5,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef functor<5,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;
	// emit the signal with the ()-operator by using it like bool function 
	void operator ()(typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const
	{
		unsigned n = (unsigned) functors.size();
		for (unsigned i=0; i<n; ++i)
			(*static_cast<functor_type*>(functors[i]))(v1, v2, v3, v4, v5);
	}
	functor_type& connect(const functor_type& _f)    { functor_type *f = static_cast<functor_type*>(_f.clone()); signal_base::connect(f); return *f; }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class function_functor<5,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<5,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (*function_ptr)(T1, T2, T3, T4, T5);
	function_ptr fp;
	function_functor(function_ptr _fp) : fp(_fp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const { fp(v1,v2,v3,v4,v5); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (const void*)fp; }
	virtual functor_base* clone() const { return new function_functor(*this); }
};

// connect signal to a void function
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(signal<T1,T2,T3,T4,T5>& s, void (*fp)(T1, T2, T3, T4, T5)) { s.connect(function_functor<5,T1, T2, T3, T4, T5>(fp)); }


// disconnect signal from a void function
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(signal<T1,T2,T3,T4,T5>& s, void (*fp)(T1, T2, T3, T4, T5)) { s.disconnect(function_functor<5,T1, T2, T3, T4, T5>(fp)); }



template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class method_functor<5,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<5,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*method_ptr)(T1, T2, T3, T4, T5);
	X* ip;
	method_ptr mp;
	method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const { (ip->*mp)(v1, v2, v3, v4, v5); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<const Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new method_functor(*this); }
};

/// connect signal to a void method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(signal<T1, T2, T3, T4, T5>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5)) { s.connect(method_functor<5,X,X,T1, T2, T3, T4, T5>(ip,mp)); }

//! connect signal to a void method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(signal<T1, T2, T3, T4, T5>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5)) { s.connect(method_functor<5,X,Y,T1, T2, T3, T4, T5>(ip,mp)); }

/// disconnect signal from a void method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(signal<T1, T2, T3, T4, T5>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5)) { s.disconnect(method_functor<5,X,X,T1, T2, T3, T4, T5>(ip,mp)); }

//! disconnect signal from a void method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(signal<T1, T2, T3, T4, T5>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5)) { s.disconnect(method_functor<5,X,Y,T1, T2, T3, T4, T5>(ip,mp)); }

template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_method_functor<5,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<5,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*const_method_ptr)(T1, T2, T3, T4, T5) const;
	const X* ip;
	const_method_ptr mp;
	const_method_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const { (ip->*mp)(v1, v2, v3, v4, v5); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_method_functor(*this); }
};

/// connect signal to a void const method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(signal<T1, T2, T3, T4, T5>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5) const) { s.connect(const_method_functor<5,X,X,T1, T2, T3, T4, T5>(ip,mp)); }

/// disconnect signal from a void const method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(signal<T1, T2, T3, T4, T5>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5) const) { s.disconnect(const_method_functor<5,X,X,T1, T2, T3, T4, T5>(ip,mp)); }

//! connect signal to a void const method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(signal<T1, T2, T3, T4, T5>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5) const) { s.connect(const_method_functor<5,X,Y,T1, T2, T3, T4, T5>(ip,mp)); }

//! disconnect signal from a void const method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(signal<T1, T2, T3, T4, T5>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5) const) { s.disconnect(const_method_functor<5,X,Y,T1, T2, T3, T4, T5>(ip,mp)); }


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class object_functor<5,X,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<5,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const { ip(v1, v2, v3, v4, v5); }
	const tacker* get_tacker() const { return type::cond::is_base_of<tacker,X>::value?reinterpret_cast<const tacker*>(&ip):0; }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(signal<T1, T2, T3, T4, T5>& s, X& ip) { s.connect(object_functor<5,X&,T1, T2, T3, T4, T5>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(signal<T1, T2, T3, T4, T5>& s, X& ip) { s.disconnect(object_functor<5,X&,T1, T2, T3, T4, T5>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(signal<T1, T2, T3, T4, T5>& s, const X& ip) { s.connect(object_functor<5,const X&,T1, T2, T3, T4, T5>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(signal<T1, T2, T3, T4, T5>& s, const X& ip) { s.disconnect(object_functor<5,const X&,T1, T2, T3, T4, T5>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
X& connect_copy(signal<T1, T2, T3, T4, T5>& s, const X& ip) { 
	return static_cast<const object_functor<5,X,T1, T2, T3, T4, T5>&>(s.connect(object_functor<5,X,T1, T2, T3, T4, T5>(ip))).ip; 
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class functor<6,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<6,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const = 0;
};

// implementation of a boolean signal with a signature of length 6
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class signal_impl<6,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<6,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef functor<6,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;
	// emit the signal with the ()-operator by using it like bool function 
	void operator ()(typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const
	{
		unsigned n = (unsigned) functors.size();
		for (unsigned i=0; i<n; ++i)
			(*static_cast<functor_type*>(functors[i]))(v1, v2, v3, v4, v5, v6);
	}
	functor_type& connect(const functor_type& _f)    { functor_type *f = static_cast<functor_type*>(_f.clone()); signal_base::connect(f); return *f; }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class function_functor<6,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<6,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (*function_ptr)(T1, T2, T3, T4, T5, T6);
	function_ptr fp;
	function_functor(function_ptr _fp) : fp(_fp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const { fp(v1,v2,v3,v4,v5,v6); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (const void*)fp; }
	virtual functor_base* clone() const { return new function_functor(*this); }
};

// connect signal to a void function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(signal<T1,T2,T3,T4,T5,T6>& s, void (*fp)(T1, T2, T3, T4, T5, T6)) { s.connect(function_functor<6,T1, T2, T3, T4, T5, T6>(fp)); }


// disconnect signal from a void function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(signal<T1,T2,T3,T4,T5,T6>& s, void (*fp)(T1, T2, T3, T4, T5, T6)) { s.disconnect(function_functor<6,T1, T2, T3, T4, T5, T6>(fp)); }



template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class method_functor<6,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<6,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*method_ptr)(T1, T2, T3, T4, T5, T6);
	X* ip;
	method_ptr mp;
	method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const { (ip->*mp)(v1, v2, v3, v4, v5, v6); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<const Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new method_functor(*this); }
};

/// connect signal to a void method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(signal<T1, T2, T3, T4, T5, T6>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6)) { s.connect(method_functor<6,X,X,T1, T2, T3, T4, T5, T6>(ip,mp)); }

//! connect signal to a void method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(signal<T1, T2, T3, T4, T5, T6>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6)) { s.connect(method_functor<6,X,Y,T1, T2, T3, T4, T5, T6>(ip,mp)); }

/// disconnect signal from a void method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(signal<T1, T2, T3, T4, T5, T6>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6)) { s.disconnect(method_functor<6,X,X,T1, T2, T3, T4, T5, T6>(ip,mp)); }

//! disconnect signal from a void method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(signal<T1, T2, T3, T4, T5, T6>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6)) { s.disconnect(method_functor<6,X,Y,T1, T2, T3, T4, T5, T6>(ip,mp)); }

template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_method_functor<6,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<6,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*const_method_ptr)(T1, T2, T3, T4, T5, T6) const;
	const X* ip;
	const_method_ptr mp;
	const_method_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const { (ip->*mp)(v1, v2, v3, v4, v5, v6); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_method_functor(*this); }
};

/// connect signal to a void const method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(signal<T1, T2, T3, T4, T5, T6>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6) const) { s.connect(const_method_functor<6,X,X,T1, T2, T3, T4, T5, T6>(ip,mp)); }

/// disconnect signal from a void const method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(signal<T1, T2, T3, T4, T5, T6>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6) const) { s.disconnect(const_method_functor<6,X,X,T1, T2, T3, T4, T5, T6>(ip,mp)); }

//! connect signal to a void const method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(signal<T1, T2, T3, T4, T5, T6>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6) const) { s.connect(const_method_functor<6,X,Y,T1, T2, T3, T4, T5, T6>(ip,mp)); }

//! disconnect signal from a void const method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(signal<T1, T2, T3, T4, T5, T6>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6) const) { s.disconnect(const_method_functor<6,X,Y,T1, T2, T3, T4, T5, T6>(ip,mp)); }


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class object_functor<6,X,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<6,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const { ip(v1, v2, v3, v4, v5, v6); }
	const tacker* get_tacker() const { return type::cond::is_base_of<tacker,X>::value?reinterpret_cast<const tacker*>(&ip):0; }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(signal<T1, T2, T3, T4, T5, T6>& s, X& ip) { s.connect(object_functor<6,X&,T1, T2, T3, T4, T5, T6>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(signal<T1, T2, T3, T4, T5, T6>& s, X& ip) { s.disconnect(object_functor<6,X&,T1, T2, T3, T4, T5, T6>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(signal<T1, T2, T3, T4, T5, T6>& s, const X& ip) { s.connect(object_functor<6,const X&,T1, T2, T3, T4, T5, T6>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(signal<T1, T2, T3, T4, T5, T6>& s, const X& ip) { s.disconnect(object_functor<6,const X&,T1, T2, T3, T4, T5, T6>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
X& connect_copy(signal<T1, T2, T3, T4, T5, T6>& s, const X& ip) { 
	return static_cast<const object_functor<6,X,T1, T2, T3, T4, T5, T6>&>(s.connect(object_functor<6,X,T1, T2, T3, T4, T5, T6>(ip))).ip; 
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class functor<7,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<7,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const = 0;
};

// implementation of a boolean signal with a signature of length 7
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class signal_impl<7,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<7,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef functor<7,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;
	// emit the signal with the ()-operator by using it like bool function 
	void operator ()(typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const
	{
		unsigned n = (unsigned) functors.size();
		for (unsigned i=0; i<n; ++i)
			(*static_cast<functor_type*>(functors[i]))(v1, v2, v3, v4, v5, v6, v7);
	}
	functor_type& connect(const functor_type& _f)    { functor_type *f = static_cast<functor_type*>(_f.clone()); signal_base::connect(f); return *f; }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class function_functor<7,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<7,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (*function_ptr)(T1, T2, T3, T4, T5, T6, T7);
	function_ptr fp;
	function_functor(function_ptr _fp) : fp(_fp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const { fp(v1,v2,v3,v4,v5,v6,v7); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (const void*)fp; }
	virtual functor_base* clone() const { return new function_functor(*this); }
};

// connect signal to a void function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(signal<T1,T2,T3,T4,T5,T6,T7>& s, void (*fp)(T1, T2, T3, T4, T5, T6, T7)) { s.connect(function_functor<7,T1, T2, T3, T4, T5, T6, T7>(fp)); }


// disconnect signal from a void function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(signal<T1,T2,T3,T4,T5,T6,T7>& s, void (*fp)(T1, T2, T3, T4, T5, T6, T7)) { s.disconnect(function_functor<7,T1, T2, T3, T4, T5, T6, T7>(fp)); }



template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class method_functor<7,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<7,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*method_ptr)(T1, T2, T3, T4, T5, T6, T7);
	X* ip;
	method_ptr mp;
	method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const { (ip->*mp)(v1, v2, v3, v4, v5, v6, v7); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<const Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new method_functor(*this); }
};

/// connect signal to a void method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7)) { s.connect(method_functor<7,X,X,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }

//! connect signal to a void method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7)) { s.connect(method_functor<7,X,Y,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }

/// disconnect signal from a void method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7)) { s.disconnect(method_functor<7,X,X,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }

//! disconnect signal from a void method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7)) { s.disconnect(method_functor<7,X,Y,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }

template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_method_functor<7,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<7,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*const_method_ptr)(T1, T2, T3, T4, T5, T6, T7) const;
	const X* ip;
	const_method_ptr mp;
	const_method_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const { (ip->*mp)(v1, v2, v3, v4, v5, v6, v7); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_method_functor(*this); }
};

/// connect signal to a void const method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7) const) { s.connect(const_method_functor<7,X,X,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }

/// disconnect signal from a void const method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7) const) { s.disconnect(const_method_functor<7,X,X,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }

//! connect signal to a void const method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7) const) { s.connect(const_method_functor<7,X,Y,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }

//! disconnect signal from a void const method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7) const) { s.disconnect(const_method_functor<7,X,Y,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class object_functor<7,X,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<7,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const { ip(v1, v2, v3, v4, v5, v6, v7); }
	const tacker* get_tacker() const { return type::cond::is_base_of<tacker,X>::value?reinterpret_cast<const tacker*>(&ip):0; }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(signal<T1, T2, T3, T4, T5, T6, T7>& s, X& ip) { s.connect(object_functor<7,X&,T1, T2, T3, T4, T5, T6, T7>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7>& s, X& ip) { s.disconnect(object_functor<7,X&,T1, T2, T3, T4, T5, T6, T7>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(signal<T1, T2, T3, T4, T5, T6, T7>& s, const X& ip) { s.connect(object_functor<7,const X&,T1, T2, T3, T4, T5, T6, T7>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7>& s, const X& ip) { s.disconnect(object_functor<7,const X&,T1, T2, T3, T4, T5, T6, T7>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
X& connect_copy(signal<T1, T2, T3, T4, T5, T6, T7>& s, const X& ip) { 
	return static_cast<const object_functor<7,X,T1, T2, T3, T4, T5, T6, T7>&>(s.connect(object_functor<7,X,T1, T2, T3, T4, T5, T6, T7>(ip))).ip; 
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class functor<8,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<8,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const = 0;
};

// implementation of a boolean signal with a signature of length 8
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class signal_impl<8,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<8,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef functor<8,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;
	// emit the signal with the ()-operator by using it like bool function 
	void operator ()(typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const
	{
		unsigned n = (unsigned) functors.size();
		for (unsigned i=0; i<n; ++i)
			(*static_cast<functor_type*>(functors[i]))(v1, v2, v3, v4, v5, v6, v7, v8);
	}
	functor_type& connect(const functor_type& _f)    { functor_type *f = static_cast<functor_type*>(_f.clone()); signal_base::connect(f); return *f; }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class function_functor<8,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<8,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (*function_ptr)(T1, T2, T3, T4, T5, T6, T7, T8);
	function_ptr fp;
	function_functor(function_ptr _fp) : fp(_fp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const { fp(v1,v2,v3,v4,v5,v6,v7,v8); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (const void*)fp; }
	virtual functor_base* clone() const { return new function_functor(*this); }
};

// connect signal to a void function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(signal<T1,T2,T3,T4,T5,T6,T7,T8>& s, void (*fp)(T1, T2, T3, T4, T5, T6, T7, T8)) { s.connect(function_functor<8,T1, T2, T3, T4, T5, T6, T7, T8>(fp)); }


// disconnect signal from a void function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(signal<T1,T2,T3,T4,T5,T6,T7,T8>& s, void (*fp)(T1, T2, T3, T4, T5, T6, T7, T8)) { s.disconnect(function_functor<8,T1, T2, T3, T4, T5, T6, T7, T8>(fp)); }



template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class method_functor<8,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<8,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*method_ptr)(T1, T2, T3, T4, T5, T6, T7, T8);
	X* ip;
	method_ptr mp;
	method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const { (ip->*mp)(v1, v2, v3, v4, v5, v6, v7, v8); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<const Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new method_functor(*this); }
};

/// connect signal to a void method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8)) { s.connect(method_functor<8,X,X,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }

//! connect signal to a void method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8)) { s.connect(method_functor<8,X,Y,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }

/// disconnect signal from a void method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8)) { s.disconnect(method_functor<8,X,X,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }

//! disconnect signal from a void method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8)) { s.disconnect(method_functor<8,X,Y,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }

template <class X, class Y, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_method_functor<8,X,Y,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<8,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef void (X::*const_method_ptr)(T1, T2, T3, T4, T5, T6, T7, T8) const;
	const X* ip;
	const_method_ptr mp;
	const_method_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const { (ip->*mp)(v1, v2, v3, v4, v5, v6, v7, v8); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_method_functor(*this); }
};

/// connect signal to a void const method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8) const) { s.connect(const_method_functor<8,X,X,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }

/// disconnect signal from a void const method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8) const) { s.disconnect(const_method_functor<8,X,X,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }

//! connect signal to a void const method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8) const) { s.connect(const_method_functor<8,X,Y,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }

//! disconnect signal from a void const method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, void (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8) const) { s.disconnect(const_method_functor<8,X,Y,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }


template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class object_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8> : public functor<8,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const { ip(v1, v2, v3, v4, v5, v6, v7, v8); }
	const tacker* get_tacker() const { return type::cond::is_base_of<tacker,X>::value?reinterpret_cast<const tacker*>(&ip):0; }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X& ip) { s.connect(object_functor<8,X&,T1, T2, T3, T4, T5, T6, T7, T8>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X& ip) { s.disconnect(object_functor<8,X&,T1, T2, T3, T4, T5, T6, T7, T8>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, const X& ip) { s.connect(object_functor<8,const X&,T1, T2, T3, T4, T5, T6, T7, T8>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, const X& ip) { s.disconnect(object_functor<8,const X&,T1, T2, T3, T4, T5, T6, T7, T8>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
X& connect_copy(signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, const X& ip) { 
	return static_cast<const object_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8>&>(s.connect(object_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8>(ip))).ip; 
}


	}
}

#include <cgv/config/lib_end.h>
