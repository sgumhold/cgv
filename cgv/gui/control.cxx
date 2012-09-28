#include "view.h"
#include "control.h"
#include <map>
namespace cgv {
	namespace gui {

/// construct from name
abst_view::abst_view(const std::string& name) : cgv::base::node(name)
{
	next_in_list = 0;
	prev_in_list = 0;
	ptr = 0;
}

/// construct from name
abst_control::abst_control(const std::string& name) : abst_view(name)
{
}

std::map<const void*, abst_view*>& ref_view_map()
{
	static std::map<const void*, abst_view*> view_map;
	return view_map;
}

/// add default implementation passing the query to the controls() method
bool abst_control::shows(const void* ptr) const
{
	return controls(ptr);
}

/// ensures detachment of view
abst_view::~abst_view()
{
	if (ptr)
		detach_from_reference();
}

/// links all views to a reference into a doubly linked list in order to allow controls of the reference to update all attached views
void abst_view::attach_to_reference(const void* ptr)
{
	if (this->ptr)
		detach_from_reference();
	std::map<const void*, abst_view*>& vm = ref_view_map();
	if (vm.find(ptr) == vm.end()) {
		vm[ptr] = this;
	}
	else {
		abst_view* v = vm[ptr];
		next_in_list = v->next_in_list;
		prev_in_list = v;
		v->next_in_list = this;
		if (next_in_list)
			next_in_list->prev_in_list = this;
	}
	this->ptr = ptr;
}

/// links all views to a reference into a doubly linked list in order to allow controls of the reference to update all attached views
void abst_view::detach_from_reference()
{
	std::map<const void*, abst_view*>& vm = ref_view_map();
	if (vm.find(ptr) == vm.end())
		return;
	if (vm[ptr] == this) {
		if (next_in_list == 0)
			vm.erase(ptr);
		else {
			vm[ptr] = next_in_list;
			next_in_list->prev_in_list = 0;
		}
	}
	else {
		if (prev_in_list == 0) {
			if (next_in_list == 0)
				return;
			std::cerr << "ups, abst_view::detach_from_reference called for view that is not representative and has no previous element in list" << std::endl;
			next_in_list = 0;
			return;
		}
		prev_in_list->next_in_list = next_in_list;
		if (next_in_list)
			next_in_list->prev_in_list = prev_in_list;
	}

	next_in_list = prev_in_list = 0;
	return;
}

void update_views(void* member_ptr)
{
	std::map<const void*, abst_view*>& vm = ref_view_map();
	if (vm.find(member_ptr) != vm.end()) {
		abst_view* v = vm[member_ptr];
		v->update();
		v->update_views();
	}
}


/// calls the update method of all other attached views
void abst_view::update_views()
{
	abst_view* v = next_in_list;
	while (v != 0) {
		v->update();
		v = v->next_in_list;
	}
	v = prev_in_list;
	while (v != 0) {
		v->update();
		v = v->prev_in_list;
	}
}

	}
}
