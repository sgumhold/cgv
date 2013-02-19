#pragma once
@exclude <cgv/config/ppp.ppp>
#include <cgv/signal/abst_signal.h>
#include <cgv/type/invalid_type.h>
#include <cgv/type/cond/is_base_of.h>
#include <vector>
#include <map>

namespace cgv {
	namespace signal {

template <int i, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class functor;

template <int i, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class signal_impl;

// signal that sends callbacks to connected functions, methods or functors when executing the ()-oeprator of the signal
template <@["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class signal : public signal_impl<type::count_valid_types<@["T1"; ", "; "T".N_ARG]>::value, @["T1"; ", "; "T".N_ARG]>, public tacker
{
};

template <int i, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class function_functor;

template <int i, class X, class Y, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class method_functor;

template <int i, typename X, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class object_functor;

template <int i, class X, class Y, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class const_method_functor;

//template <int i, typename X, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
//class const_object_functor;

@for (i=1; i<=N_ARG; i=i+1) @{

template <@["typename T1"; ", "; "typename T".N_ARG]>
class functor<@(i),@["T1"; ", "; "T".N_ARG]> : public functor_base, public signature<@(i),@["T1"; ", "; "T".N_ARG]>
{
public:
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	virtual void operator() (@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const = 0;
};

// implementation of a boolean signal with a signature of length @(i)
template <@["typename T1"; ", "; "typename T".N_ARG]>
class signal_impl<@(i),@["T1"; ", "; "T".N_ARG]> : public signal_base, public signature<@(i),@["T1"; ", "; "T".N_ARG]>
{
public:
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	typedef functor<@(i),@["T1"; ", "; "T".N_ARG]> functor_type;
	// emit the signal with the ()-operator by using it like bool function 
	void operator ()(@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const
	{
		unsigned int n = functors.size();
		for (unsigned int i=0; i<n; ++i)
			(*static_cast<functor_type*>(functors[i]))(@["v1"; ", "; "v".i]);
	}
	functor_type& connect(const functor_type& _f)    { functor_type *f = static_cast<functor_type*>(_f.clone()); signal_base::connect(f); return *f; }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <@["typename T1"; ", "; "typename T".N_ARG]>
class function_functor<@(i),@["T1"; ", "; "T".N_ARG]> : public functor<@(i),@["T1"; ", "; "T".N_ARG]>
{
public:
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	typedef void (*function_ptr)(@["T1"; ", "; "T".i]);
	function_ptr fp;
	function_functor(function_ptr _fp) : fp(_fp) {}
	void operator() (@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const { fp(@["v1";",";"v".i]); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (const void*)fp; }
	virtual functor_base* clone() const { return new function_functor(*this); }
};

// connect signal to a void function
template <@["typename T1"; ", "; "typename T".i]>
void connect(signal<@["T1"; ","; "T".i]>& s, void (*fp)(@["T1"; ", "; "T".i])) { s.connect(function_functor<@(i),@["T1"; ", "; "T".i]>(fp)); }

// disconnect signal from a void function
template <@["typename T1"; ", "; "typename T".i]>
void disconnect(signal<@["T1"; ","; "T".i]>& s, void (*fp)(@["T1"; ", "; "T".i])) { s.disconnect(function_functor<@(i),@["T1"; ", "; "T".i]>(fp)); }


template <class X, class Y, @["typename T1"; ", "; "typename T".N_ARG]>
class method_functor<@(i),X,Y,@["T1"; ", "; "T".N_ARG]> : public functor<@(i),@["T1"; ", "; "T".N_ARG]>
{
public:
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	typedef void (X::*method_ptr)(@["T1"; ", "; "T".i]);
	X* ip;
	method_ptr mp;
	method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const { (ip->*mp)(@["v1"; ", "; "v".i]); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<const Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new method_functor(*this); }
};

/// connect signal to a void method of an object
template <class X, @["typename T1"; ", "; "typename T".i]>
void connect(signal<@["T1"; ", "; "T".i]>& s, X* ip, void (X::*mp)(@["T1"; ", "; "T".i])) { s.connect(method_functor<@(i),X,X,@["T1"; ", "; "T".i]>(ip,mp)); }

//! connect signal to a void method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, @["typename T1"; ", "; "typename T".i]>
void connect(signal<@["T1"; ", "; "T".i]>& s, X* ip, void (X::*mp)(@["T1"; ", "; "T".i])) { s.connect(method_functor<@(i),X,Y,@["T1"; ", "; "T".i]>(ip,mp)); }

/// disconnect signal from a void method of an object
template <class X, @["typename T1"; ", "; "typename T".i]>
void disconnect(signal<@["T1"; ", "; "T".i]>& s, X* ip, void (X::*mp)(@["T1"; ", "; "T".i])) { s.disconnect(method_functor<@(i),X,X,@["T1"; ", "; "T".i]>(ip,mp)); }

//! disconnect signal from a void method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, @["typename T1"; ", "; "typename T".i]>
void disconnect(signal<@["T1"; ", "; "T".i]>& s, X* ip, void (X::*mp)(@["T1"; ", "; "T".i])) { s.disconnect(method_functor<@(i),X,Y,@["T1"; ", "; "T".i]>(ip,mp)); }

template <class X, class Y, @["typename T1"; ", "; "typename T".N_ARG]>
class const_method_functor<@(i),X,Y,@["T1"; ", "; "T".N_ARG]> : public functor<@(i),@["T1"; ", "; "T".N_ARG]>
{
public:
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	typedef void (X::*const_method_ptr)(@["T1"; ", "; "T".i]) const;
	const X* ip;
	const_method_ptr mp;
	const_method_functor(const X* _ip, const_method_ptr _mp) : ip(_ip), mp(_mp) {}
	void operator() (@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const { (ip->*mp)(@["v1"; ", "; "v".i]); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(static_cast<Y*>(ip)); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_method_functor(*this); }
};

/// connect signal to a void const method of an object
template <class X, @["typename T1"; ", "; "typename T".i]>
void connect(signal<@["T1"; ", "; "T".i]>& s, X* ip, void (X::*mp)(@["T1"; ", "; "T".i]) const) { s.connect(const_method_functor<@(i),X,X,@["T1"; ", "; "T".i]>(ip,mp)); }

/// disconnect signal from a void const method of an object
template <class X, @["typename T1"; ", "; "typename T".i]>
void disconnect(signal<@["T1"; ", "; "T".i]>& s, X* ip, void (X::*mp)(@["T1"; ", "; "T".i]) const) { s.disconnect(const_method_functor<@(i),X,X,@["T1"; ", "; "T".i]>(ip,mp)); }

//! connect signal to a void const method, where an additional template argument can be used to specify a base class
/*! This version of connect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to connect an
	instance x of type X via connect<Y>(sig,x,&X::callback) if the tacker of Y is to be used and connect<Z>(sig,x,&X::callback)
	if the tacker of Z is to be used. In most cases it does not matter which tacker is used. */
template <class Y, class X, @["typename T1"; ", "; "typename T".i]>
void connect(signal<@["T1"; ", "; "T".i]>& s, X* ip, void (X::*mp)(@["T1"; ", "; "T".i]) const) { s.connect(const_method_functor<@(i),X,Y,@["T1"; ", "; "T".i]>(ip,mp)); }

//! disconnect signal from a void const method of an object, where an additional template argument can be used to specify a base class
/*! This version of disconnect is necessary for class types X derived via multiple inheritance from cgv::signal::tacker 
    via several base classes, such that the conversion static_cast<cgv::signal::tacker> becomes ambigeous. For an 
	example suppose X inherits Y and Z and both Y and Z inherit cgv::signal::tacker. Then once needs to disconnect an
	instance x of type X via disconnect<Y>(sig,x,&X::callback) if the tacker of Y has been connected with connect<Y>
	and connect<Z>(sig,x,&X::callback) if the tacker of Z had been used. For disconnect the same tacker has to be used
	as in the corresponding connect call!!! */
template <class Y, class X, @["typename T1"; ", "; "typename T".i]>
void disconnect(signal<@["T1"; ", "; "T".i]>& s, X* ip, void (X::*mp)(@["T1"; ", "; "T".i]) const) { s.disconnect(const_method_functor<@(i),X,Y,@["T1"; ", "; "T".i]>(ip,mp)); }


template <typename X, @["typename T1"; ", "; "typename T".N_ARG]>
class object_functor<@(i),X,@["T1"; ", "; "T".N_ARG]> : public functor<@(i),@["T1"; ", "; "T".N_ARG]>
{
public:
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	mutable X ip;
	object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const { ip(@["v1"; ", "; "v".i]); }
	const tacker* get_tacker() const { return type::cond::is_base_of<tacker,X>::value?reinterpret_cast<const tacker*>(&ip):0; }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, @["typename T1"; ", "; "typename T".i]>
void connect(signal<@["T1"; ", "; "T".i]>& s, X& ip) { s.connect(object_functor<@(i),X&,@["T1"; ", "; "T".i]>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, @["typename T1"; ", "; "typename T".i]>
void disconnect(signal<@["T1"; ", "; "T".i]>& s, X& ip) { s.disconnect(object_functor<@(i),X&,@["T1"; ", "; "T".i]>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, @["typename T1"; ", "; "typename T".i]>
void connect(signal<@["T1"; ", "; "T".i]>& s, const X& ip) { s.connect(object_functor<@(i),const X&,@["T1"; ", "; "T".i]>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, @["typename T1"; ", "; "typename T".i]>
void disconnect(signal<@["T1"; ", "; "T".i]>& s, const X& ip) { s.disconnect(object_functor<@(i),const X&,@["T1"; ", "; "T".i]>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, @["typename T1"; ", "; "typename T".i]>
X& connect_copy(signal<@["T1"; ", "; "T".i]>& s, const X& ip) { 
	return static_cast<const object_functor<@(i),X,@["T1"; ", "; "T".i]>&>(s.connect(object_functor<@(i),X,@["T1"; ", "; "T".i]>(ip))).ip; 
}
@}

	}
}