#pragma once

#include "group.h"
#include <string>
#include <cgv/type/cond/is_base_of.h>

#include "lib_begin.h"

namespace cgv {
	namespace base {

class CGV_API traverse_policy;

/** The action class is used in tree traversals together with the traverser. It implements the interface needed
    to perform an action on an arbitrary object. */
class CGV_API action
{
protected:
	bool default_result_begin;
	bool default_result_end;
public:
	/// construct with default return values for the begin and end method
	action(bool _default_result_begin, bool _default_result_end);
	/// sets the value that is returned, whenever no result is obtained from the traversed methods
	void set_default_results(bool _default_result);
	/// sets the value that is returned for on_begin events
	void set_default_result_begin(bool _default_result_begin);
	/// sets the value that is returned for on_end events
	void set_default_result_end(bool _default_result_end);
	/// make the passed object current
	virtual void select(base_ptr p);
	/// check if the current object implements the interface needed for this action
	virtual bool implements_action() const;
	/// return the traverse_policy of the current object if available or 0 otherwise
	virtual traverse_policy* get_policy() const;
	/// perform the enter part of the action on the current object
	virtual bool begin();
	/// perform the leave part of the action on the current object
	virtual bool end();
	/// check whether the action has registered a single begin method or both begin and end methods
	virtual bool has_begin_only() const;
};

template <bool is_derived, class X>
struct extract_policy_struct
{
	static void extract_policy(X*, traverse_policy* &tp) { tp = 0; }
};

template <class X>
struct extract_policy_struct<true,X>
{
	static void extract_policy(X* x, traverse_policy* &tp) { tp = static_cast<traverse_policy*>(x); }
};

/** base class for all actions that use methods of class X*/
template <class X>
class base_method_action : public action
{
protected:
	traverse_policy* tp;
	X* x;
public:
	/// construct from default return values that are passed on to the base class
	base_method_action(bool _default_result_begin, bool _default_result_end) 
		: action(_default_result_begin, _default_result_end), tp(0), x(0) {}

	/// implement the select method and store pointers of type X* and traverse_policy*
	void select(base_ptr p) {
		x  = p->get_interface<X>();
		extract_policy_struct<type::cond::is_base_of<traverse_policy, X>::value,X>::extract_policy(x, tp);
	}
	/// simply return whether the stored pointer of type X* is not 0
	bool implements_action() const {
		return x != 0;
	}
	/// simply return the stored pointer of type traverse_policy*
	traverse_policy* get_policy() const {
		return tp;
	}
};


/** base class for all method actions that take a single argument of type T1 in their signature*/
template <class X, typename T1>
class method_action : public base_method_action<X>
{
protected:
	T1 v1;
public:
	/// construct action from signature and default return values
	method_action(T1 _v1, bool _default_result_begin, bool _default_result_end) 
		: base_method_action<X>(_default_result_begin, _default_result_end), v1(_v1) {}
	/// set a new signature with which the methods of the traversed nodes are called
	void set_signature(T1 _v1) { v1 = _v1; }
	/// call a void method given a default return value
	bool call_method(void (X::*mp)(T1), bool default_result) { 
		if (this->x && mp) 
			(this->x->*mp)(v1); 
		return default_result; 
	}
	/// call a bool method given a default return value
	bool call_method(bool (X::*mp)(T1), bool default_result) { 
		if (this->x && mp) 
			return (this->x->*mp)(v1); 
		return default_result; 
	}
};

/** complete implementation of method actions that only call one method when entering a node*/
template <class X, typename R, typename T1>
class single_method_action : public method_action<X,T1>
{
protected:
	R (X::*on_begin)(T1);
public:
	/// construct from signature, method pointer and default result values
	single_method_action(T1 _v1, R (X::*_on_begin)(T1), 
		bool _default_result_begin = false, bool _default_result_end = false) 
		: method_action<X,T1>(_v1, _default_result_begin, _default_result_end), on_begin(_on_begin) {}
	/// uses call_method of base class method_action to call the method refered to by the stored method pointer
	bool begin() { return this->call_method(on_begin, this->default_result_begin); }
	/// check whether the action has registered a single begin method or both begin and end methods
	bool has_begin_only() const { return true; }
};

/** complete implementation of method actions that call a begin and end method when entering and leaving a node*/
template <class X, typename R1, typename R2, typename T1>
class matched_method_action : public method_action<X,T1>
{
protected:
	R1 (X::*on_begin)(T1);
	R2 (X::*on_end)(T1);
public:
	/// construct from signature, method pointers and default result values
	matched_method_action(T1 _v1, R1 (X::*_on_begin)(T1), R2 (X::*_on_end)(T1), bool _default_result_begin, bool _default_result_end) 
		: method_action<X,T1>(_v1, _default_result_begin, _default_result_end), on_begin(_on_begin), on_end(_on_end) {}
	/// uses call_method of base class method_action to call the method refered to by the stored method pointer
	bool begin() { return this->call_method(on_begin, this->default_result_begin); }
	/// uses call_method of base class method_action to call the method refered to by the stored method pointer
	bool end()   { return this->call_method(on_end, this->default_result_end); }
};

/// helper function to construct an action from a signature and one method that is called when a node is entered
template <typename T1, class X, typename R>
single_method_action<X,R,T1> make_action(T1 _v1, R (X::*_on_begin)(T1), bool _default_result_begin = false, bool _default_result_end = false) {
	return single_method_action<X,R,T1>(_v1, _on_begin, _default_result_begin, _default_result_end);
}

/// helper function to construct an action from a signature and two methods that are called when a node is entered and when it is left again
template <typename T1, class X, typename R1, typename R2>
matched_method_action<X,R1,R2,T1> make_action(T1 _v1, R1 (X::*_on_begin)(T1), R2 (X::*_on_end)(T1), bool _default_result_begin = false, bool _default_result_end = false) {
	return matched_method_action<X,R1,R2,T1>(_v1, _on_begin, _on_end, _default_result_begin, _default_result_end);
}


/** base class for all method actions that take a single argument of type T1 in their signature*/
template <class X, typename T1, typename T2>
class method_action_2 : public base_method_action<X>
{
protected:
	T1 v1;
	T2 v2;
public:
	/// construct action from signature and default return values
	method_action_2(T1 _v1, T2 _v2, bool _default_result_begin, bool _default_result_end) 
		: base_method_action<X>(_default_result_begin, _default_result_end), v1(_v1), v2(_v2) {}
	/// set a new signature with which the methods of the traversed nodes are called
	void set_signature(T1 _v1, T2 _v2) { v1 = _v1; v2 = _v2; }
	/// call a void method given a default return value
	bool call_method(void (X::*mp)(T1, T2), bool default_result) { 
		if (this->x && mp) 
			(this->x->*mp)(v1,v2); 
		return default_result; 
	}
	/// call a bool method given a default return value
	bool call_method(bool (X::*mp)(T1, T2), bool default_result) { 
		if (this->x && mp) 
			return (this->x->*mp)(v1, v2); 
		return default_result; 
	}
};

/** complete implementation of method actions that only call one method when entering a node*/
template <class X, typename R, typename T1, typename T2>
class single_method_action_2 : public method_action_2<X,T1,T2>
{
protected:
	R (X::*on_begin)(T1,T2);
public:
	/// construct from signature, method pointer and default result values
	single_method_action_2(T1 _v1, T2 _v2, R (X::*_on_begin)(T1,T2), 
		bool _default_result_begin = false, bool _default_result_end = false) 
		: method_action_2<X,T1,T2>(_v1, _v2, _default_result_begin, _default_result_end), on_begin(_on_begin) {}
	/// uses call_method of base class method_action to call the method refered to by the stored method pointer
	bool begin() { return this->call_method(on_begin, this->default_result_begin); }
	/// check whether the action has registered a single begin method or both begin and end methods
	bool has_begin_only() const { return true; }
};

/// helper function to construct an action from a signature and one method that is called when a node is entered
template <typename T1, typename T2, class X, typename R>
single_method_action_2<X,R,T1,T2> make_action_2(T1 _v1, T2 _v2, R (X::*_on_begin)(T1,T2), bool _default_result_begin = false, bool _default_result_end = false) {
	return single_method_action_2<X,R,T1,T2>(_v1, _v2, _on_begin, _default_result_begin, _default_result_end);
}

	}
}

#include <cgv/config/lib_end.h>