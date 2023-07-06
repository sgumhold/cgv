#include <cgv/gui/subprovider.h>

namespace cgv {
	namespace gui {

void subprovider::update_member(void* member_ptr) {

	if(provider_ptr)
		provider_ptr->update_member(member_ptr);
}

void subprovider::create_gui(cgv::gui::provider* p) {

	provider_ptr = p;
	cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
	if(b && p)
		create_gui_impl(b, p);
}

	}
}
