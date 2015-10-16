#pragma once
@exclude <cgv/config/ppp.ppp>
#include <cgv/signal/abst_signal.h>
#include <cgv/type/invalid_type.h>
#include <cgv/type/func/make_argument.h>
#include <cgv/signal/bool_combiner.h>
#include <vector>
#include <map>

#include <cgv/signal/lib_begin.h>

namespace cgv {
	namespace signal {

template <int i, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class bool_functor;

template <int i, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class bool_signal_impl;

// signal that constructs a boolean return value by using the bool_combiner class
template <@["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class bool_signal : public bool_signal_impl<type::count_valid_types<@["T1"; ", "; "T".N_ARG]>::value, @["T1"; ", "; "T".N_ARG]>, public tacker
{
public:
	// construct from option string that configure the bool_combiner base class
	bool_signal(const char* opt = "*&") : bool_signal_impl<type::count_valid_types<@["T1"; ", "; "T".N_ARG]>::value, @["T1"; ", "; "T".N_ARG]>(opt) {}
};

template <int i, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class bool_function_functor;

template <int i, class X, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class bool_method_functor;

template <int i, class X, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class const_bool_method_functor;

template <int i, typename X, @["typename T1 = type::invalid_type"; ", "; "typename T".N_ARG." = type::invalid_type"]>
class bool_object_functor;

@for (i=1; i<=N_ARG; i=i+1) @{

template <@["typename T1"; ", "; "typename T".N_ARG]>
class bool_functor<@(i),@["T1"; ", "; "T".N_ARG]> : public functor_base, public signature<@(i),@["T1"; ", "; "T".N_ARG]>
{
public:
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	virtual bool operator() (@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const = 0;
};

// implementation of a boolean signal with a signature of length @(i)
template <@["typename T1"; ", "; "typename T".N_ARG]>
class bool_signal_impl<@(i),@["T1"; ", "; "T".N_ARG]> : public signal_base, public signature<@(i),@["T1"; ", "; "T".N_ARG]>, public bool_combiner
{
public:
	typedef bool_functor<@(i),@["T1"; ", "; "T".N_ARG]> functor_type;	
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	bool_signal_impl(const char* opt) : bool_combiner(opt) {}
	// emit the bool_signal with the ()-operator by using it like bool function 
	bool operator() (@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const
	{
		bool result = get_neutral_value();
		for (unsigned i=0; i<(unsigned)functors.size(); ++i)
			if (combine_result((*static_cast<functor_type*>(functors[i]))(@["v1"; ", "; "v".i]),result))
				return result;
		return result;
	}
	void connect(const functor_type& f)    { signal_base::connect(f.clone()); }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <@["typename T1"; ", "; "typename T".N_ARG]>
class bool_function_functor<@(i),@["T1"; ", "; "T".N_ARG]> : public bool_functor<@(i),@["T1"; ", "; "T".N_ARG]>
{
public:
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	typedef bool (*function_ptr)(@["T1"; ", "; "T".i]);
	function_ptr fp;
	bool_function_functor(function_ptr _fp) : fp(_fp) {}
	bool operator() (@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const { return fp(@["v1";",";"v".i]); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = fp; }
	virtual functor_base* clone() const { return new bool_function_functor(*this); }
};

// connect bool_signal to a bool function
template <@["typename T1"; ", "; "typename T".i]>
void connect(bool_signal<@["T1"; ","; "T".i]>& s, bool (*fp)(@["T1"; ", "; "T".i])) { s.connect(bool_function_functor<@(i),@["T1"; ", "; "T".i]>(fp)); }

// disconnect bool_signal from a bool function
template <@["typename T1"; ", "; "typename T".i]>
void disconnect(bool_signal<@["T1"; ","; "T".i]>& s, bool (*fp)(@["T1"; ", "; "T".i])) { s.disconnect(bool_function_functor<@(i),@["T1"; ", "; "T".i]>(fp)); }


template <class X, @["typename T1"; ", "; "typename T".N_ARG]>
class bool_method_functor<@(i),X,@["T1"; ", "; "T".N_ARG]> : public bool_functor<@(i),@["T1"; ", "; "T".N_ARG]>
{
public:
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	typedef bool (X::*method_ptr)(@["T1"; ", "; "T".i]);
	X* ip;
	method_ptr mp;
	bool_method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const { return (ip->*mp)(@["v1"; ", "; "v".i]); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, @["typename T1"; ", "; "typename T".i]>
void connect(bool_signal<@["T1"; ", "; "T".i]>& s, X* ip, bool (X::*mp)(@["T1"; ", "; "T".i])) { s.connect(bool_method_functor<@(i),X,@["T1"; ", "; "T".i]>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, @["typename T1"; ", "; "typename T".i]>
void disconnect(bool_signal<@["T1"; ", "; "T".i]>& s, X* ip, bool (X::*mp)(@["T1"; ", "; "T".i])) { s.disconnect(bool_method_functor<@(i),X,@["T1"; ", "; "T".i]>(ip,mp)); }

template <class X, @["typename T1"; ", "; "typename T".N_ARG]>
class const_bool_method_functor<@(i),X,@["T1"; ", "; "T".N_ARG]> : public bool_functor<@(i),@["T1"; ", "; "T".N_ARG]>
{
public:
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	typedef bool (X::*method_ptr)(@["T1"; ", "; "T".i]) const;
	const X* ip;
	method_ptr mp;
	const_bool_method_functor(const X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const { return (ip->*mp)(@["v1"; ", "; "v".i]); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, @["typename T1"; ", "; "typename T".i]>
void connect(bool_signal<@["T1"; ", "; "T".i]>& s, X* ip, bool (X::*mp)(@["T1"; ", "; "T".i]) const) { s.connect(const_bool_method_functor<@(i),X,@["T1"; ", "; "T".i]>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, @["typename T1"; ", "; "typename T".i]>
void disconnect(bool_signal<@["T1"; ", "; "T".i]>& s, X* ip, bool (X::*mp)(@["T1"; ", "; "T".i]) const) { s.disconnect(const_bool_method_functor<@(i),X,@["T1"; ", "; "T".i]>(ip,mp)); }

template <typename X, @["typename T1"; ", "; "typename T".N_ARG]>
class bool_object_functor<@(i),X,@["T1"; ", "; "T".N_ARG]> : public bool_functor<@(i),@["T1"; ", "; "T".N_ARG]>
{
public:
	typedef signature<@(i),@["T1"; ", "; "T".N_ARG]> S;
	mutable X ip;
	bool_object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (@["typename S::A1 v1"; ", "; "typename S::A".i." v".i]) const { return ip(@["v1"; ", "; "v".i]); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(&ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new bool_object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, @["typename T1"; ", "; "typename T".i]>
void connect(bool_signal<@["T1"; ", "; "T".i]>& s, X& ip) { s.connect(bool_object_functor<@(i),X&,@["T1"; ", "; "T".i]>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, @["typename T1"; ", "; "typename T".i]>
void disconnect(bool_signal<@["T1"; ", "; "T".i]>& s, X& ip) { s.disconnect(bool_object_functor<@(i),X&,@["T1"; ", "; "T".i]>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, @["typename T1"; ", "; "typename T".i]>
void connect(bool_signal<@["T1"; ", "; "T".i]>& s, const X& ip) { s.connect(bool_object_functor<@(i),const X&,@["T1"; ", "; "T".i]>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, @["typename T1"; ", "; "typename T".i]>
void disconnect(bool_signal<@["T1"; ", "; "T".i]>& s, const X& ip) { s.disconnect(bool_object_functor<@(i),const X&,@["T1"; ", "; "T".i]>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, @["typename T1"; ", "; "typename T".i]>
X& connect_copy(bool_signal<@["T1"; ", "; "T".i]>& s, const X& ip) { 
	return static_cast<const bool_object_functor<@(i),X,@["T1"; ", "; "T".i]>&>(s.connect(bool_object_functor<@(i),X,@["T1"; ", "; "T".i]>(ip))).ip; 
}

@}

	}
}

#include <cgv/config/lib_end.h>