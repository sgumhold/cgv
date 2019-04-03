#pragma once

#include <cgv/signal/abst_signal.h>
#include <cgv/type/invalid_type.h>
#include <cgv/type/func/make_argument.h>
#include <cgv/signal/bool_combiner.h>
#include <vector>
#include <map>

#include "lib_begin.h"

namespace cgv {
	namespace signal {

template <int i, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class bool_functor;

template <int i, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class bool_signal_impl;

// signal that constructs a boolean return value by using the bool_combiner class
template <typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class bool_signal : public bool_signal_impl<type::count_valid_types<T1, T2, T3, T4, T5, T6, T7, T8>::value, T1, T2, T3, T4, T5, T6, T7, T8>, public tacker
{
public:
	// construct from option string that configure the bool_combiner base class
	bool_signal(const char* opt = "*&") : bool_signal_impl<type::count_valid_types<T1, T2, T3, T4, T5, T6, T7, T8>::value, T1, T2, T3, T4, T5, T6, T7, T8>(opt) {}
};

template <int i, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class bool_function_functor;

template <int i, class X, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class bool_method_functor;

template <int i, class X, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class const_bool_method_functor;

template <int i, typename X, typename T1 = type::invalid_type, typename T2 = type::invalid_type, typename T3 = type::invalid_type, typename T4 = type::invalid_type, typename T5 = type::invalid_type, typename T6 = type::invalid_type, typename T7 = type::invalid_type, typename T8 = type::invalid_type>
class bool_object_functor;



template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_functor<0,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<0,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual bool operator() () const = 0;
};

// implementation of a boolean signal with a signature of length 0
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_signal_impl<0,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<0,T1, T2, T3, T4, T5, T6, T7, T8>, public bool_combiner
{
public:
	typedef bool_functor<0,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;	
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	bool_signal_impl(const char* opt) : bool_combiner(opt) {}
	// emit the bool_signal with the ()-operator by using it like bool function 
	bool operator() () const
	{
		bool result = get_neutral_value();
		for (unsigned i=0; i<(unsigned)functors.size(); ++i)
			if (combine_result((*static_cast<functor_type*>(functors[i]))(),result))
				return result;
		return result;
	}
	void connect(const functor_type& f)    { signal_base::connect(f.clone()); }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_function_functor<0,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<0,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (*function_ptr)();
	function_ptr fp;
	bool_function_functor(function_ptr _fp) : fp(_fp) {}
	bool operator() () const { return fp(); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (void*&)fp; }
	virtual functor_base* clone() const { return new bool_function_functor(*this); }
};

// connect bool_signal to a bool function
extern CGV_API void connect(bool_signal<>& s, bool (*fp)());

// disconnect bool_signal from a bool function
extern void CGV_API disconnect(bool_signal<>& s, bool (*fp)());


template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_method_functor<0,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<0,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)();
	X* ip;
	method_ptr mp;
	bool_method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() () const { return (ip->*mp)(); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X>
void connect(bool_signal<>& s, X* ip, bool (X::*mp)()) { s.connect(bool_method_functor<0,X>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X>
void disconnect(bool_signal<>& s, X* ip, bool (X::*mp)()) { s.disconnect(bool_method_functor<0,X>(ip,mp)); }

template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_bool_method_functor<0,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<0,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)() const;
	const X* ip;
	method_ptr mp;
	const_bool_method_functor(const X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() () const { return (ip->*mp)(); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X>
void connect(bool_signal<>& s, X* ip, bool (X::*mp)() const) { s.connect(const_bool_method_functor<0,X>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X>
void disconnect(bool_signal<>& s, X* ip, bool (X::*mp)() const) { s.disconnect(const_bool_method_functor<0,X>(ip,mp)); }

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_object_functor<0,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<0,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<0,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	bool_object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() () const { return ip(); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(&ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new bool_object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X>
void connect(bool_signal<>& s, X& ip) { s.connect(bool_object_functor<0,X&>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X>
void disconnect(bool_signal<>& s, X& ip) { s.disconnect(bool_object_functor<0,X&>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X>
void connect(bool_signal<>& s, const X& ip) { s.connect(bool_object_functor<0,const X&>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X>
void disconnect(bool_signal<>& s, const X& ip) { s.disconnect(bool_object_functor<0,const X&>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X>
X& connect_copy(bool_signal<>& s, const X& ip) { 
	return static_cast<const bool_object_functor<0,X>&>(s.connect(bool_object_functor<0,X>(ip))).ip; 
}



template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_functor<1,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<1,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual bool operator() (typename S::A1 v1) const = 0;
};

// implementation of a boolean signal with a signature of length 1
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_signal_impl<1,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<1,T1, T2, T3, T4, T5, T6, T7, T8>, public bool_combiner
{
public:
	typedef bool_functor<1,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;	
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	bool_signal_impl(const char* opt) : bool_combiner(opt) {}
	// emit the bool_signal with the ()-operator by using it like bool function 
	bool operator() (typename S::A1 v1) const
	{
		bool result = get_neutral_value();
		for (unsigned i=0; i<(unsigned)functors.size(); ++i)
			if (combine_result((*static_cast<functor_type*>(functors[i]))(v1),result))
				return result;
		return result;
	}
	void connect(const functor_type& f)    { signal_base::connect(f.clone()); }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_function_functor<1,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<1,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (*function_ptr)(T1);
	function_ptr fp;
	bool_function_functor(function_ptr _fp) : fp(_fp) {}
	bool operator() (typename S::A1 v1) const { return fp(v1); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (void*&)fp; }
	virtual functor_base* clone() const { return new bool_function_functor(*this); }
};

// connect bool_signal to a bool function
template <typename T1>
void connect(bool_signal<T1>& s, bool (*fp)(T1)) { s.connect(bool_function_functor<1,T1>(fp)); }


// disconnect bool_signal from a bool function
template <typename T1>
void disconnect(bool_signal<T1>& s, bool (*fp)(T1)) { s.disconnect(bool_function_functor<1,T1>(fp)); }



template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_method_functor<1,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<1,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1);
	X* ip;
	method_ptr mp;
	bool_method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1) const { return (ip->*mp)(v1); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1>
void connect(bool_signal<T1>& s, X* ip, bool (X::*mp)(T1)) { s.connect(bool_method_functor<1,X,T1>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1>
void disconnect(bool_signal<T1>& s, X* ip, bool (X::*mp)(T1)) { s.disconnect(bool_method_functor<1,X,T1>(ip,mp)); }

template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_bool_method_functor<1,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<1,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1) const;
	const X* ip;
	method_ptr mp;
	const_bool_method_functor(const X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1) const { return (ip->*mp)(v1); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1>
void connect(bool_signal<T1>& s, X* ip, bool (X::*mp)(T1) const) { s.connect(const_bool_method_functor<1,X,T1>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1>
void disconnect(bool_signal<T1>& s, X* ip, bool (X::*mp)(T1) const) { s.disconnect(const_bool_method_functor<1,X,T1>(ip,mp)); }

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_object_functor<1,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<1,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<1,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	bool_object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1) const { return ip(v1); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(&ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new bool_object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1>
void connect(bool_signal<T1>& s, X& ip) { s.connect(bool_object_functor<1,X&,T1>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1>
void disconnect(bool_signal<T1>& s, X& ip) { s.disconnect(bool_object_functor<1,X&,T1>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1>
void connect(bool_signal<T1>& s, const X& ip) { s.connect(bool_object_functor<1,const X&,T1>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1>
void disconnect(bool_signal<T1>& s, const X& ip) { s.disconnect(bool_object_functor<1,const X&,T1>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1>
X& connect_copy(bool_signal<T1>& s, const X& ip) { 
	return static_cast<const bool_object_functor<1,X,T1>&>(s.connect(bool_object_functor<1,X,T1>(ip))).ip; 
}



template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_functor<2,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<2,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual bool operator() (typename S::A1 v1, typename S::A2 v2) const = 0;
};

// implementation of a boolean signal with a signature of length 2
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_signal_impl<2,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<2,T1, T2, T3, T4, T5, T6, T7, T8>, public bool_combiner
{
public:
	typedef bool_functor<2,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;	
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	bool_signal_impl(const char* opt) : bool_combiner(opt) {}
	// emit the bool_signal with the ()-operator by using it like bool function 
	bool operator() (typename S::A1 v1, typename S::A2 v2) const
	{
		bool result = get_neutral_value();
		for (unsigned i=0; i<(unsigned)functors.size(); ++i)
			if (combine_result((*static_cast<functor_type*>(functors[i]))(v1, v2),result))
				return result;
		return result;
	}
	void connect(const functor_type& f)    { signal_base::connect(f.clone()); }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_function_functor<2,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<2,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (*function_ptr)(T1, T2);
	function_ptr fp;
	bool_function_functor(function_ptr _fp) : fp(_fp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2) const { return fp(v1,v2); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (void*&)fp; }
	virtual functor_base* clone() const { return new bool_function_functor(*this); }
};

// connect bool_signal to a bool function
template <typename T1, typename T2>
void connect(bool_signal<T1,T2>& s, bool (*fp)(T1, T2)) { s.connect(bool_function_functor<2,T1, T2>(fp)); }


// disconnect bool_signal from a bool function
template <typename T1, typename T2>
void disconnect(bool_signal<T1,T2>& s, bool (*fp)(T1, T2)) { s.disconnect(bool_function_functor<2,T1, T2>(fp)); }



template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_method_functor<2,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<2,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2);
	X* ip;
	method_ptr mp;
	bool_method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2) const { return (ip->*mp)(v1, v2); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2>
void connect(bool_signal<T1, T2>& s, X* ip, bool (X::*mp)(T1, T2)) { s.connect(bool_method_functor<2,X,T1, T2>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2>
void disconnect(bool_signal<T1, T2>& s, X* ip, bool (X::*mp)(T1, T2)) { s.disconnect(bool_method_functor<2,X,T1, T2>(ip,mp)); }

template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_bool_method_functor<2,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<2,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2) const;
	const X* ip;
	method_ptr mp;
	const_bool_method_functor(const X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2) const { return (ip->*mp)(v1, v2); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2>
void connect(bool_signal<T1, T2>& s, X* ip, bool (X::*mp)(T1, T2) const) { s.connect(const_bool_method_functor<2,X,T1, T2>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2>
void disconnect(bool_signal<T1, T2>& s, X* ip, bool (X::*mp)(T1, T2) const) { s.disconnect(const_bool_method_functor<2,X,T1, T2>(ip,mp)); }

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_object_functor<2,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<2,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<2,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	bool_object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2) const { return ip(v1, v2); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(&ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new bool_object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2>
void connect(bool_signal<T1, T2>& s, X& ip) { s.connect(bool_object_functor<2,X&,T1, T2>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2>
void disconnect(bool_signal<T1, T2>& s, X& ip) { s.disconnect(bool_object_functor<2,X&,T1, T2>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2>
void connect(bool_signal<T1, T2>& s, const X& ip) { s.connect(bool_object_functor<2,const X&,T1, T2>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2>
void disconnect(bool_signal<T1, T2>& s, const X& ip) { s.disconnect(bool_object_functor<2,const X&,T1, T2>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2>
X& connect_copy(bool_signal<T1, T2>& s, const X& ip) { 
	return static_cast<const bool_object_functor<2,X,T1, T2>&>(s.connect(bool_object_functor<2,X,T1, T2>(ip))).ip; 
}



template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_functor<3,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<3,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const = 0;
};

// implementation of a boolean signal with a signature of length 3
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_signal_impl<3,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<3,T1, T2, T3, T4, T5, T6, T7, T8>, public bool_combiner
{
public:
	typedef bool_functor<3,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;	
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	bool_signal_impl(const char* opt) : bool_combiner(opt) {}
	// emit the bool_signal with the ()-operator by using it like bool function 
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const
	{
		bool result = get_neutral_value();
		for (unsigned i=0; i<(unsigned)functors.size(); ++i)
			if (combine_result((*static_cast<functor_type*>(functors[i]))(v1, v2, v3),result))
				return result;
		return result;
	}
	void connect(const functor_type& f)    { signal_base::connect(f.clone()); }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_function_functor<3,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<3,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (*function_ptr)(T1, T2, T3);
	function_ptr fp;
	bool_function_functor(function_ptr _fp) : fp(_fp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const { return fp(v1,v2,v3); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (void*&)fp; }
	virtual functor_base* clone() const { return new bool_function_functor(*this); }
};

// connect bool_signal to a bool function
template <typename T1, typename T2, typename T3>
void connect(bool_signal<T1,T2,T3>& s, bool (*fp)(T1, T2, T3)) { s.connect(bool_function_functor<3,T1, T2, T3>(fp)); }


// disconnect bool_signal from a bool function
template <typename T1, typename T2, typename T3>
void disconnect(bool_signal<T1,T2,T3>& s, bool (*fp)(T1, T2, T3)) { s.disconnect(bool_function_functor<3,T1, T2, T3>(fp)); }



template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_method_functor<3,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<3,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3);
	X* ip;
	method_ptr mp;
	bool_method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const { return (ip->*mp)(v1, v2, v3); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3>
void connect(bool_signal<T1, T2, T3>& s, X* ip, bool (X::*mp)(T1, T2, T3)) { s.connect(bool_method_functor<3,X,T1, T2, T3>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3>
void disconnect(bool_signal<T1, T2, T3>& s, X* ip, bool (X::*mp)(T1, T2, T3)) { s.disconnect(bool_method_functor<3,X,T1, T2, T3>(ip,mp)); }

template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_bool_method_functor<3,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<3,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3) const;
	const X* ip;
	method_ptr mp;
	const_bool_method_functor(const X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const { return (ip->*mp)(v1, v2, v3); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3>
void connect(bool_signal<T1, T2, T3>& s, X* ip, bool (X::*mp)(T1, T2, T3) const) { s.connect(const_bool_method_functor<3,X,T1, T2, T3>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3>
void disconnect(bool_signal<T1, T2, T3>& s, X* ip, bool (X::*mp)(T1, T2, T3) const) { s.disconnect(const_bool_method_functor<3,X,T1, T2, T3>(ip,mp)); }

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_object_functor<3,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<3,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<3,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	bool_object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3) const { return ip(v1, v2, v3); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(&ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new bool_object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3>
void connect(bool_signal<T1, T2, T3>& s, X& ip) { s.connect(bool_object_functor<3,X&,T1, T2, T3>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3>
void disconnect(bool_signal<T1, T2, T3>& s, X& ip) { s.disconnect(bool_object_functor<3,X&,T1, T2, T3>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3>
void connect(bool_signal<T1, T2, T3>& s, const X& ip) { s.connect(bool_object_functor<3,const X&,T1, T2, T3>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3>
void disconnect(bool_signal<T1, T2, T3>& s, const X& ip) { s.disconnect(bool_object_functor<3,const X&,T1, T2, T3>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3>
X& connect_copy(bool_signal<T1, T2, T3>& s, const X& ip) { 
	return static_cast<const bool_object_functor<3,X,T1, T2, T3>&>(s.connect(bool_object_functor<3,X,T1, T2, T3>(ip))).ip; 
}



template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_functor<4,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<4,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const = 0;
};

// implementation of a boolean signal with a signature of length 4
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_signal_impl<4,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<4,T1, T2, T3, T4, T5, T6, T7, T8>, public bool_combiner
{
public:
	typedef bool_functor<4,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;	
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	bool_signal_impl(const char* opt) : bool_combiner(opt) {}
	// emit the bool_signal with the ()-operator by using it like bool function 
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const
	{
		bool result = get_neutral_value();
		for (unsigned i=0; i<(unsigned)functors.size(); ++i)
			if (combine_result((*static_cast<functor_type*>(functors[i]))(v1, v2, v3, v4),result))
				return result;
		return result;
	}
	void connect(const functor_type& f)    { signal_base::connect(f.clone()); }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_function_functor<4,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<4,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (*function_ptr)(T1, T2, T3, T4);
	function_ptr fp;
	bool_function_functor(function_ptr _fp) : fp(_fp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const { return fp(v1,v2,v3,v4); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (void*&)fp; }
	virtual functor_base* clone() const { return new bool_function_functor(*this); }
};

// connect bool_signal to a bool function
template <typename T1, typename T2, typename T3, typename T4>
void connect(bool_signal<T1,T2,T3,T4>& s, bool (*fp)(T1, T2, T3, T4)) { s.connect(bool_function_functor<4,T1, T2, T3, T4>(fp)); }


// disconnect bool_signal from a bool function
template <typename T1, typename T2, typename T3, typename T4>
void disconnect(bool_signal<T1,T2,T3,T4>& s, bool (*fp)(T1, T2, T3, T4)) { s.disconnect(bool_function_functor<4,T1, T2, T3, T4>(fp)); }



template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_method_functor<4,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<4,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3, T4);
	X* ip;
	method_ptr mp;
	bool_method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const { return (ip->*mp)(v1, v2, v3, v4); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4>
void connect(bool_signal<T1, T2, T3, T4>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4)) { s.connect(bool_method_functor<4,X,T1, T2, T3, T4>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4>
void disconnect(bool_signal<T1, T2, T3, T4>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4)) { s.disconnect(bool_method_functor<4,X,T1, T2, T3, T4>(ip,mp)); }

template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_bool_method_functor<4,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<4,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3, T4) const;
	const X* ip;
	method_ptr mp;
	const_bool_method_functor(const X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const { return (ip->*mp)(v1, v2, v3, v4); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4>
void connect(bool_signal<T1, T2, T3, T4>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4) const) { s.connect(const_bool_method_functor<4,X,T1, T2, T3, T4>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4>
void disconnect(bool_signal<T1, T2, T3, T4>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4) const) { s.disconnect(const_bool_method_functor<4,X,T1, T2, T3, T4>(ip,mp)); }

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_object_functor<4,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<4,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<4,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	bool_object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4) const { return ip(v1, v2, v3, v4); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(&ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new bool_object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4>
void connect(bool_signal<T1, T2, T3, T4>& s, X& ip) { s.connect(bool_object_functor<4,X&,T1, T2, T3, T4>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4>
void disconnect(bool_signal<T1, T2, T3, T4>& s, X& ip) { s.disconnect(bool_object_functor<4,X&,T1, T2, T3, T4>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4>
void connect(bool_signal<T1, T2, T3, T4>& s, const X& ip) { s.connect(bool_object_functor<4,const X&,T1, T2, T3, T4>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4>
void disconnect(bool_signal<T1, T2, T3, T4>& s, const X& ip) { s.disconnect(bool_object_functor<4,const X&,T1, T2, T3, T4>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3, typename T4>
X& connect_copy(bool_signal<T1, T2, T3, T4>& s, const X& ip) { 
	return static_cast<const bool_object_functor<4,X,T1, T2, T3, T4>&>(s.connect(bool_object_functor<4,X,T1, T2, T3, T4>(ip))).ip; 
}



template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_functor<5,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<5,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const = 0;
};

// implementation of a boolean signal with a signature of length 5
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_signal_impl<5,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<5,T1, T2, T3, T4, T5, T6, T7, T8>, public bool_combiner
{
public:
	typedef bool_functor<5,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;	
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	bool_signal_impl(const char* opt) : bool_combiner(opt) {}
	// emit the bool_signal with the ()-operator by using it like bool function 
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const
	{
		bool result = get_neutral_value();
		for (unsigned i=0; i<(unsigned)functors.size(); ++i)
			if (combine_result((*static_cast<functor_type*>(functors[i]))(v1, v2, v3, v4, v5),result))
				return result;
		return result;
	}
	void connect(const functor_type& f)    { signal_base::connect(f.clone()); }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_function_functor<5,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<5,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (*function_ptr)(T1, T2, T3, T4, T5);
	function_ptr fp;
	bool_function_functor(function_ptr _fp) : fp(_fp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const { return fp(v1,v2,v3,v4,v5); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (void*&)fp; }
	virtual functor_base* clone() const { return new bool_function_functor(*this); }
};

// connect bool_signal to a bool function
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(bool_signal<T1,T2,T3,T4,T5>& s, bool (*fp)(T1, T2, T3, T4, T5)) { s.connect(bool_function_functor<5,T1, T2, T3, T4, T5>(fp)); }


// disconnect bool_signal from a bool function
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(bool_signal<T1,T2,T3,T4,T5>& s, bool (*fp)(T1, T2, T3, T4, T5)) { s.disconnect(bool_function_functor<5,T1, T2, T3, T4, T5>(fp)); }



template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_method_functor<5,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<5,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3, T4, T5);
	X* ip;
	method_ptr mp;
	bool_method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const { return (ip->*mp)(v1, v2, v3, v4, v5); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(bool_signal<T1, T2, T3, T4, T5>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5)) { s.connect(bool_method_functor<5,X,T1, T2, T3, T4, T5>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(bool_signal<T1, T2, T3, T4, T5>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5)) { s.disconnect(bool_method_functor<5,X,T1, T2, T3, T4, T5>(ip,mp)); }

template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_bool_method_functor<5,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<5,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3, T4, T5) const;
	const X* ip;
	method_ptr mp;
	const_bool_method_functor(const X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const { return (ip->*mp)(v1, v2, v3, v4, v5); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(bool_signal<T1, T2, T3, T4, T5>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5) const) { s.connect(const_bool_method_functor<5,X,T1, T2, T3, T4, T5>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(bool_signal<T1, T2, T3, T4, T5>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5) const) { s.disconnect(const_bool_method_functor<5,X,T1, T2, T3, T4, T5>(ip,mp)); }

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_object_functor<5,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<5,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<5,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	bool_object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5) const { return ip(v1, v2, v3, v4, v5); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(&ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new bool_object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(bool_signal<T1, T2, T3, T4, T5>& s, X& ip) { s.connect(bool_object_functor<5,X&,T1, T2, T3, T4, T5>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(bool_signal<T1, T2, T3, T4, T5>& s, X& ip) { s.disconnect(bool_object_functor<5,X&,T1, T2, T3, T4, T5>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void connect(bool_signal<T1, T2, T3, T4, T5>& s, const X& ip) { s.connect(bool_object_functor<5,const X&,T1, T2, T3, T4, T5>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
void disconnect(bool_signal<T1, T2, T3, T4, T5>& s, const X& ip) { s.disconnect(bool_object_functor<5,const X&,T1, T2, T3, T4, T5>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5>
X& connect_copy(bool_signal<T1, T2, T3, T4, T5>& s, const X& ip) { 
	return static_cast<const bool_object_functor<5,X,T1, T2, T3, T4, T5>&>(s.connect(bool_object_functor<5,X,T1, T2, T3, T4, T5>(ip))).ip; 
}



template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_functor<6,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<6,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const = 0;
};

// implementation of a boolean signal with a signature of length 6
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_signal_impl<6,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<6,T1, T2, T3, T4, T5, T6, T7, T8>, public bool_combiner
{
public:
	typedef bool_functor<6,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;	
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	bool_signal_impl(const char* opt) : bool_combiner(opt) {}
	// emit the bool_signal with the ()-operator by using it like bool function 
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const
	{
		bool result = get_neutral_value();
		for (unsigned i=0; i<(unsigned)functors.size(); ++i)
			if (combine_result((*static_cast<functor_type*>(functors[i]))(v1, v2, v3, v4, v5, v6),result))
				return result;
		return result;
	}
	void connect(const functor_type& f)    { signal_base::connect(f.clone()); }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_function_functor<6,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<6,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (*function_ptr)(T1, T2, T3, T4, T5, T6);
	function_ptr fp;
	bool_function_functor(function_ptr _fp) : fp(_fp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const { return fp(v1,v2,v3,v4,v5,v6); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (void*&)fp; }
	virtual functor_base* clone() const { return new bool_function_functor(*this); }
};

// connect bool_signal to a bool function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(bool_signal<T1,T2,T3,T4,T5,T6>& s, bool (*fp)(T1, T2, T3, T4, T5, T6)) { s.connect(bool_function_functor<6,T1, T2, T3, T4, T5, T6>(fp)); }


// disconnect bool_signal from a bool function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(bool_signal<T1,T2,T3,T4,T5,T6>& s, bool (*fp)(T1, T2, T3, T4, T5, T6)) { s.disconnect(bool_function_functor<6,T1, T2, T3, T4, T5, T6>(fp)); }



template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_method_functor<6,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<6,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3, T4, T5, T6);
	X* ip;
	method_ptr mp;
	bool_method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const { return (ip->*mp)(v1, v2, v3, v4, v5, v6); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(bool_signal<T1, T2, T3, T4, T5, T6>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6)) { s.connect(bool_method_functor<6,X,T1, T2, T3, T4, T5, T6>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6)) { s.disconnect(bool_method_functor<6,X,T1, T2, T3, T4, T5, T6>(ip,mp)); }

template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_bool_method_functor<6,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<6,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3, T4, T5, T6) const;
	const X* ip;
	method_ptr mp;
	const_bool_method_functor(const X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const { return (ip->*mp)(v1, v2, v3, v4, v5, v6); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(bool_signal<T1, T2, T3, T4, T5, T6>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6) const) { s.connect(const_bool_method_functor<6,X,T1, T2, T3, T4, T5, T6>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6) const) { s.disconnect(const_bool_method_functor<6,X,T1, T2, T3, T4, T5, T6>(ip,mp)); }

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_object_functor<6,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<6,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<6,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	bool_object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6) const { return ip(v1, v2, v3, v4, v5, v6); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(&ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new bool_object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(bool_signal<T1, T2, T3, T4, T5, T6>& s, X& ip) { s.connect(bool_object_functor<6,X&,T1, T2, T3, T4, T5, T6>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6>& s, X& ip) { s.disconnect(bool_object_functor<6,X&,T1, T2, T3, T4, T5, T6>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void connect(bool_signal<T1, T2, T3, T4, T5, T6>& s, const X& ip) { s.connect(bool_object_functor<6,const X&,T1, T2, T3, T4, T5, T6>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6>& s, const X& ip) { s.disconnect(bool_object_functor<6,const X&,T1, T2, T3, T4, T5, T6>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
X& connect_copy(bool_signal<T1, T2, T3, T4, T5, T6>& s, const X& ip) { 
	return static_cast<const bool_object_functor<6,X,T1, T2, T3, T4, T5, T6>&>(s.connect(bool_object_functor<6,X,T1, T2, T3, T4, T5, T6>(ip))).ip; 
}



template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_functor<7,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<7,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const = 0;
};

// implementation of a boolean signal with a signature of length 7
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_signal_impl<7,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<7,T1, T2, T3, T4, T5, T6, T7, T8>, public bool_combiner
{
public:
	typedef bool_functor<7,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;	
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	bool_signal_impl(const char* opt) : bool_combiner(opt) {}
	// emit the bool_signal with the ()-operator by using it like bool function 
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const
	{
		bool result = get_neutral_value();
		for (unsigned i=0; i<(unsigned)functors.size(); ++i)
			if (combine_result((*static_cast<functor_type*>(functors[i]))(v1, v2, v3, v4, v5, v6, v7),result))
				return result;
		return result;
	}
	void connect(const functor_type& f)    { signal_base::connect(f.clone()); }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_function_functor<7,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<7,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (*function_ptr)(T1, T2, T3, T4, T5, T6, T7);
	function_ptr fp;
	bool_function_functor(function_ptr _fp) : fp(_fp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const { return fp(v1,v2,v3,v4,v5,v6,v7); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (void*&)fp; }
	virtual functor_base* clone() const { return new bool_function_functor(*this); }
};

// connect bool_signal to a bool function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(bool_signal<T1,T2,T3,T4,T5,T6,T7>& s, bool (*fp)(T1, T2, T3, T4, T5, T6, T7)) { s.connect(bool_function_functor<7,T1, T2, T3, T4, T5, T6, T7>(fp)); }


// disconnect bool_signal from a bool function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(bool_signal<T1,T2,T3,T4,T5,T6,T7>& s, bool (*fp)(T1, T2, T3, T4, T5, T6, T7)) { s.disconnect(bool_function_functor<7,T1, T2, T3, T4, T5, T6, T7>(fp)); }



template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_method_functor<7,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<7,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3, T4, T5, T6, T7);
	X* ip;
	method_ptr mp;
	bool_method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const { return (ip->*mp)(v1, v2, v3, v4, v5, v6, v7); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(bool_signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6, T7)) { s.connect(bool_method_functor<7,X,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6, T7)) { s.disconnect(bool_method_functor<7,X,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }

template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_bool_method_functor<7,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<7,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3, T4, T5, T6, T7) const;
	const X* ip;
	method_ptr mp;
	const_bool_method_functor(const X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const { return (ip->*mp)(v1, v2, v3, v4, v5, v6, v7); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(bool_signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6, T7) const) { s.connect(const_bool_method_functor<7,X,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6, T7>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6, T7) const) { s.disconnect(const_bool_method_functor<7,X,T1, T2, T3, T4, T5, T6, T7>(ip,mp)); }

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_object_functor<7,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<7,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<7,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	bool_object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7) const { return ip(v1, v2, v3, v4, v5, v6, v7); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(&ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new bool_object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(bool_signal<T1, T2, T3, T4, T5, T6, T7>& s, X& ip) { s.connect(bool_object_functor<7,X&,T1, T2, T3, T4, T5, T6, T7>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6, T7>& s, X& ip) { s.disconnect(bool_object_functor<7,X&,T1, T2, T3, T4, T5, T6, T7>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void connect(bool_signal<T1, T2, T3, T4, T5, T6, T7>& s, const X& ip) { s.connect(bool_object_functor<7,const X&,T1, T2, T3, T4, T5, T6, T7>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6, T7>& s, const X& ip) { s.disconnect(bool_object_functor<7,const X&,T1, T2, T3, T4, T5, T6, T7>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
X& connect_copy(bool_signal<T1, T2, T3, T4, T5, T6, T7>& s, const X& ip) { 
	return static_cast<const bool_object_functor<7,X,T1, T2, T3, T4, T5, T6, T7>&>(s.connect(bool_object_functor<7,X,T1, T2, T3, T4, T5, T6, T7>(ip))).ip; 
}



template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_functor<8,T1, T2, T3, T4, T5, T6, T7, T8> : public functor_base, public signature<8,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	virtual bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const = 0;
};

// implementation of a boolean signal with a signature of length 8
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_signal_impl<8,T1, T2, T3, T4, T5, T6, T7, T8> : public signal_base, public signature<8,T1, T2, T3, T4, T5, T6, T7, T8>, public bool_combiner
{
public:
	typedef bool_functor<8,T1, T2, T3, T4, T5, T6, T7, T8> functor_type;	
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	bool_signal_impl(const char* opt) : bool_combiner(opt) {}
	// emit the bool_signal with the ()-operator by using it like bool function 
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const
	{
		bool result = get_neutral_value();
		for (unsigned i=0; i<(unsigned)functors.size(); ++i)
			if (combine_result((*static_cast<functor_type*>(functors[i]))(v1, v2, v3, v4, v5, v6, v7, v8),result))
				return result;
		return result;
	}
	void connect(const functor_type& f)    { signal_base::connect(f.clone()); }
	void disconnect(const functor_type& f) { signal_base::disconnect(&f); }
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_function_functor<8,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<8,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (*function_ptr)(T1, T2, T3, T4, T5, T6, T7, T8);
	function_ptr fp;
	bool_function_functor(function_ptr _fp) : fp(_fp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const { return fp(v1,v2,v3,v4,v5,v6,v7,v8); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = 0; p2 = (void*&)fp; }
	virtual functor_base* clone() const { return new bool_function_functor(*this); }
};

// connect bool_signal to a bool function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(bool_signal<T1,T2,T3,T4,T5,T6,T7,T8>& s, bool (*fp)(T1, T2, T3, T4, T5, T6, T7, T8)) { s.connect(bool_function_functor<8,T1, T2, T3, T4, T5, T6, T7, T8>(fp)); }


// disconnect bool_signal from a bool function
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(bool_signal<T1,T2,T3,T4,T5,T6,T7,T8>& s, bool (*fp)(T1, T2, T3, T4, T5, T6, T7, T8)) { s.disconnect(bool_function_functor<8,T1, T2, T3, T4, T5, T6, T7, T8>(fp)); }



template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_method_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<8,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3, T4, T5, T6, T7, T8);
	X* ip;
	method_ptr mp;
	bool_method_functor(X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const { return (ip->*mp)(v1, v2, v3, v4, v5, v6, v7, v8); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(bool_signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8)) { s.connect(bool_method_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8)) { s.disconnect(bool_method_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }

template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class const_bool_method_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<8,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	typedef bool (X::*method_ptr)(T1, T2, T3, T4, T5, T6, T7, T8) const;
	const X* ip;
	method_ptr mp;
	const_bool_method_functor(const X* _ip, method_ptr _mp) : ip(_ip), mp(_mp) {}
	bool operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const { return (ip->*mp)(v1, v2, v3, v4, v5, v6, v7, v8); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = ip; p2 = (void*&)mp; }
	virtual functor_base* clone() const { return new const_bool_method_functor(*this); }
};

// connect bool_signal to a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(bool_signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8) const) { s.connect(const_bool_method_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }

// disconnect bool_signal from a bool method of an object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X* ip, bool (X::*mp)(T1, T2, T3, T4, T5, T6, T7, T8) const) { s.disconnect(const_bool_method_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8>(ip,mp)); }

template <typename X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class bool_object_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8> : public bool_functor<8,T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	typedef signature<8,T1, T2, T3, T4, T5, T6, T7, T8> S;
	mutable X ip;
	bool_object_functor(typename type::func::make_argument<X>::type _ip) : ip(_ip) {}
	void operator() (typename S::A1 v1, typename S::A2 v2, typename S::A3 v3, typename S::A4 v4, typename S::A5 v5, typename S::A6 v6, typename S::A7 v7, typename S::A8 v8) const { return ip(v1, v2, v3, v4, v5, v6, v7, v8); }
	const tacker* get_tacker() const { return static_cast<const tacker*>(&ip); }
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &ip; p2 = 0; }
	virtual functor_base* clone() const { return new bool_object_functor(*this); }
};

// connect signal to the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(bool_signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X& ip) { s.connect(bool_object_functor<8,X&,T1, T2, T3, T4, T5, T6, T7, T8>(ip)); }
// disconnect signal from the ()-operator of a referenced object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, X& ip) { s.disconnect(bool_object_functor<8,X&,T1, T2, T3, T4, T5, T6, T7, T8>(ip)); }
// connect signal to the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void connect(bool_signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, const X& ip) { s.connect(bool_object_functor<8,const X&,T1, T2, T3, T4, T5, T6, T7, T8>(ip)); }
// disconnect signal from the const ()-operator of a referenced const object
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void disconnect(bool_signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, const X& ip) { s.disconnect(bool_object_functor<8,const X&,T1, T2, T3, T4, T5, T6, T7, T8>(ip)); }

// connect signal to the ()-operator of a copy of a temporary object, a reference to the copy is returned for disconnection
template <class X, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
X& connect_copy(bool_signal<T1, T2, T3, T4, T5, T6, T7, T8>& s, const X& ip) { 
	return static_cast<const bool_object_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8>&>(s.connect(bool_object_functor<8,X,T1, T2, T3, T4, T5, T6, T7, T8>(ip))).ip; 
}



	}
}

#include <cgv/config/lib_end.h>