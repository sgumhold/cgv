#pragma once

#include <cgv/defines/assert.h>
#include <cgv/type/cond/is_base_of.h>
#include <cgv/type/cond/has_virtual_destructor.h>

namespace cgv {
	/// namespace for data management components
	namespace data {

template <typename T>
class ref_arr
{
private:
	/// struct to store the array pointer with a count
	struct counter_type 
	{
		counter_type(T* a, int c) : arr(a), count(c) {}
		T*  arr;
		int count;
	};
	/// store pointer to counter struct
	counter_type* counter;
	/// store pointer to array
	T* arr;
	/// decrement the count, delete if it is 0
	void release()
	{
		if (counter) {
			if (--counter->count == 0) {
				delete [] counter->arr;
				delete counter;
			}
			counter = 0;
		}
		arr = 0;
	}
public:
	/// construct reference counted pointer
	explicit ref_arr(T* a = 0, counter_type* c = 0) : counter(c), arr(a) {
		if (!c && a)
			counter = new counter_type(a, 1);
	}
	/// copy construct from same pointer type and increment count
	ref_arr(const ref_arr<T>& ra) : counter(ra.counter), arr(ra.arr)
	{
		if (counter)
			++counter->count;
	}
	/// destruct reference counted pointer
   ~ref_arr() { 
		release(); 
	}
	/// assignment to pointer of same type
	ref_arr<T>& operator = (const ref_arr<T>& ra) {
		if (this == &ra)
			return *this;
		release();
		new (this) ref_arr<T>(ra);
		return *this;
	}
	/// return array pointer
	T* ptr() const {
		return arr;
	}
	/// access to element
	T& operator*() const { 
		return *arr;
	}
	/// access to element pointer
   T* operator->() const {
		return arr;
	}
	/// access to i-th element
	T& operator [] (int i) {
		return arr[i];
	}
	/// return an array pointer pointing to the i-th element
	ref_arr<T> operator () (int i) const {
		return ref_arr<T>(arr+i,counter);
	}
	/// compare by pointer
	bool operator == (const ref_arr<T>& ra) const { 
		return arr == ra.arr; 
	}
	/// return current count
	int get_count() const  { 
		return counter ? counter->count : 0; 
	}
	/// check if this is the only reference
	bool unique() const {
		return get_count() <= 1; 
	}
	/// check if pointer is not yet set
	bool empty() const { 
		return get_count() == 0; 
	}
	/// conversion to bool results in false if the stored pointer is a null pointer
	operator bool () const {
		return !empty();
	}
	/// clear the array pointer
	void clear() { 
		release();
	}
};

	}
}