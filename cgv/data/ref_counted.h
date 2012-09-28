#pragma once

namespace cgv {
	namespace data {

class ref_ptr_tag;

/** if you derive your class from this class, a ref_ptr will do reference counting in the inhereted 
    ref_count member. */
class ref_counted
{
private:
	/// keep a mutable reference count to allow ref counted points to const instances
	mutable int ref_count;
protected:
	friend class ref_ptr_tag;
	/// constructor initializes the count to 0
	inline ref_counted() : ref_count(0) {}
	/// write access is also const to allow ref counted pointers to const instances
	inline void set_ref_count(int c) const { ref_count = c; }
public:
	/// read access to current count
	inline int get_ref_count() const { return ref_count; }
};

	}
}
