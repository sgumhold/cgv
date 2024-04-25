#include <cgv/gui/subprovider.h>

namespace cgv {
	namespace gui {

void subprovider::update_member(void* member_ptr) {

	if(provider_ptr)
		provider_ptr->update_member(member_ptr);
}

void subprovider::create_gui(provider* p) {

	provider_ptr = p;
	cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
	if(b && p)
		create_gui_impl(b, p);
}

void subprovider::create_gui_tree_node(provider* p, const std::string& title, bool initial_visibility, const std::string& options) {

	provider_ptr = p;
	cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
	if(b && p) {
		if(p->begin_tree_node(title, tree_node_handle, initial_visibility, options)) {
			p->align("\a");
			create_gui_impl(b, p);
			p->align("\b");
			p->end_tree_node(tree_node_handle);
		}
	}
}

	}
}
