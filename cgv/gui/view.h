#pragma once

#include <cgv/base/node.h>
#include <cgv/signal/bool_signal.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// type independent &base class of all views
class CGV_API abst_view : public cgv::base::node
{
protected:
	abst_view* next_in_list;
	abst_view* prev_in_list;
	const void* ptr;
public:
	/// pass name on to node, careful the implementation of this is in the source file control.cxx to avoid name clashes with view.cxx
	abst_view(const std::string& name);
	/// ensures detachment of view
	~abst_view();
	/// return whether the view edits the value pointed to by ptr
	virtual bool shows(const void* ptr) const = 0;
	/// links all views to a reference into a doubly linked list in order to allow controls of the reference to update all attached views
	void attach_to_reference(const void* ptr);
	/// detach view again
	void detach_from_reference();
	/// calls the update method of all other attached views
	void update_views();
};

extern CGV_API void update_views(void* member_ptr);

/// ref counted pointer to abst view
typedef data::ref_ptr<abst_view> view_ptr;

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<abst_view>;
#endif

/// class for %gui elements that %view values of the %type specified in the template argument
template <typename T>
class view : public abst_view
{
protected:
	const T* value_ptr;
public:
	/// construct abstract element from reference to value
	view(const std::string& _name, const T& _value) : abst_view(_name), value_ptr(&_value) {
		attach_to_reference(value_ptr);
	}
	/// return the current value
	const T& get_value() const { return *value_ptr; }
	/// check whether the value viewed by this element is pointing to the given pointer
	bool shows(const void* ptr) const { return value_ptr == ptr; }
};

	}
}

#include <cgv/config/lib_end.h>
