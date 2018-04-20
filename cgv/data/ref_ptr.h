#pragma once

#include "ref_counted.h"

#include <cgv/defines/assert.h>
#include <cgv/type/cond/is_base_of.h>
#include <cgv/type/cond/has_virtual_destructor.h>
#include <assert.h>

#include "lib_begin.h"

namespace cgv {
	/// namespace for data management components
	namespace data {

// extern CGV_API bool validate_delete(const void* ptr);

template <class T, bool is_ref_counted>
class ref_ptr;

/// struct used to make ref pointers to ref_counted friends of ref_counted
class ref_ptr_tag
{
protected:
	/// increment the count of a ref counted object
	void inc_ref_count(const ref_counted* ptr) const
	{
		ptr->set_ref_count(ptr->get_ref_count()+1);
	}
	/// decrement the count of a ref counted object and return whether to delete the object
	bool dec_ref_count(const ref_counted* ptr) const
	{
		int count = ptr->get_ref_count();
		if (count > 0) {
			ptr->set_ref_count(count-1);
			return count == 1;
		}
		// ERROR: zero ref count decremented
		assert(0);
		return false;
	}
};

template <typename T, bool is_ref_counted = false>
class ref_ptr_impl
{
protected:
	friend class ref_ptr_impl<const T, is_ref_counted>;
	/// struct to store the pointer with a count
	struct counter_type 
	{
		counter_type(T* p, int c) : ptr(p), count(c) {}
		T*  ptr;
		int count;
	};
	/// store pointer to counter struct
	mutable counter_type* counter;
	/// decrement the count, delete if it is 0
	void release()
	{
		if (counter) {
			if (--counter->count == 0) {
				delete counter->ptr;
				counter->ptr = 0;
				delete counter;
			}
			counter = 0;
		}
	}
	/// return the pointer itself
	T*	ref () const        { return counter ? counter->ptr : 0; }
	/// construct reference counted pointer
	explicit ref_ptr_impl(T* p = 0) : counter(0) {
		if (p) 
			counter = new counter_type(p, 1);
	}
	/// copy construct from same pointer type and increment count
	ref_ptr_impl(const ref_ptr_impl<T,false>& r) : counter(r.counter)
	{
		if (counter)
			++counter->count;
	}
	/// copy construct from pointer of derived type with virtual destructor
	template <typename S>
	ref_ptr_impl(const ref_ptr_impl<S,false>& s)
	{
		// ref_ptr conversion only valid if T is base of S
		CGV_DEFINES_ASSERT(type::cond::is_base_of<T,S>::value);
		// and T has a virtual destructor
		CGV_DEFINES_ASSERT(type::cond::has_virtual_destructor<T>::value);
		// after validity checks, set pointer 
		counter = reinterpret_cast<counter_type*>(s.counter);
		// and increment reference count
		if (counter)
			++counter->count;
	}
public:
	/// return current count
	int get_count() const  { return counter ? counter->count : 0; }
};

template <typename T>
class ref_ptr_impl<T,true> : public ref_ptr_tag
{
	T* ptr;
protected:
	/// if the pointer had been initialized before, decrement reference count and release pointer, if necessary delete instance
	void release() {
		if (ptr) { 
			if (dec_ref_count(ptr)) {
//				if (validate_delete(ptr))
					delete ptr;
			}
			ptr = 0; 
		}
	}
	/// return the pointer itself
	T*	ref () const         { 
		return ptr; 
	}
	/// construct from pointer and increment reference count
	explicit ref_ptr_impl(T* p) : ptr(p) {
		if (ptr)
			inc_ref_count(ptr);
	}
	/// copy construct from same pointer type and increment count
	ref_ptr_impl(const ref_ptr_impl<T,true>& r) : ptr(r.ref())
	{
		if (ptr)
			inc_ref_count(ptr);
	}
	/// copy construct from pointer of derived type with virtual destructor
	template <typename S>
	ref_ptr_impl(const ref_ptr_impl<S,true>& s)
	{
		// ref_ptr conversion only valid if T is base of S
		CGV_DEFINES_ASSERT(type::cond::is_base_of<T,S>::value);
		// and T has a virtual destructor
		CGV_DEFINES_ASSERT(type::cond::has_virtual_destructor<T>::value);
		// after validity checks, set pointer with a very bad hack!!
		ptr = static_cast<const ref_ptr<S,true>&>(s).operator->();
		// and increment reference count
		if (ptr)
			inc_ref_count(ptr);
	}
public:
	// void kill() { ptr = 0; }
	/// return the reference count
	int get_count() const	{ 
		return ptr ? ptr->get_ref_count() : 0; 
	}
};

/** reference counted pointer, which can work together with types that are derived
    from ref_counted, in which case the reference count of ref_counted is
	used. Otherwise a reference count is allocated and access to the stored instance
	needs to follow two pointers. */
template <class T, bool is_ref_counted = type::cond::is_base_of<ref_counted,T>::value>
class ref_ptr : public ref_ptr_impl<T,is_ref_counted>
{
public:
	/// type of the reference counted pointer
	typedef ref_ptr<T,is_ref_counted> this_type;
	/// type of base class that implements the reference count specific methods
	typedef ref_ptr_impl<T,is_ref_counted> base_type;
public:
	/// construction from pointer or empty if pointer is not given
	ref_ptr(T* p = 0) : base_type(p) {}
	/// copy constructor for reference pointers of the same type
	ref_ptr(const this_type& r) : base_type(r) {}
	/// destruct reference counted pointer
   ~ref_ptr() { 
		this->release();
	}
	/// allow to copy ref_ptr<S> to a ref_ptr<T> if T is a base class of S and if T has a virtual destructor
	template <typename S>
	ref_ptr(const ref_ptr<S,is_ref_counted>& s) : base_type(s) {}
	/// use static cast to convert from T to S if T is a base class of S and has a virtual destructor
	template <typename S>
	ref_ptr<S,is_ref_counted> up_cast() const {
		// ref_ptr conversion only valid if T is base of S
		CGV_DEFINES_ASSERT(type::cond::is_base_of<T,S>::value);
		// and S has a virtual destructor
		CGV_DEFINES_ASSERT(type::cond::has_virtual_destructor<T>::value);
		// after validity checks, return converted pointer
		return ref_ptr<S,is_ref_counted>(
					*reinterpret_cast<const ref_ptr<S,is_ref_counted>*>(this)
				 );
	}
	/// assignment to pointer of same type
	this_type& operator = (const this_type& r) {
		if (this == &r)
			return *this;
		this->release();
		new (this) this_type(r);
		return *this;
	}
	/// assignment to pointer of derived type with virtual destructor
	template <typename S>
	this_type& operator = (const ref_ptr<S,is_ref_counted>& r) {
		if (this == (const this_type*)&r)
			return *this;
		return *this = this_type(r);
	}
	/// access to element
	T& operator*() const { 
		return *this->ref();
	}
	/// access to element pointer
   T* operator->() const {
		return this->ref();
	}
	/// compare by pointer
	bool operator == (const this_type& r) const { 
		return this->ref() == r.ref(); 
	}
	/// compare by pointer
	bool operator < (const this_type& r) const { 
		return this->ref() < r.ref(); 
	}
	/// compare by pointer
	bool operator != (const this_type& r) const { 
		return this->ref() != r.ref(); 
	}
	/// check if this is the only reference
	bool unique() const {
		return this->get_count() <= 1; 
	}
	/// check if pointer is not yet set
	bool empty() const { 
		return this->get_count() == 0; 
	}
	/// conversion to bool results in false if the stored pointer is a null pointer
	operator bool () const {
		return !empty();
	}
	/// set to null pointer
	void clear() { 
		this->release();
	}
};

	}
}

#include <cgv/config/lib_end.h>