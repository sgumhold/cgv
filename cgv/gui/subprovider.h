#pragma once

#include "provider.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// derive from this class to use a provider from another viewer
class CGV_API subprovider
{
protected:
	/// pointer to the main provider
	cgv::gui::provider* provider_ptr = nullptr;

	/// call this to update all views and controls of a member
	void update_member(void* member_ptr);

	/// call this to update the value and all views and controls of a member
	template<typename T>
	void set_and_update_member(T& member, const T& value) {

		member = value;
		update_member(&member);
	}

	/// implement this to add gui controls using the supplied base and provider
	virtual void create_gui_impl(cgv::base::base* b, cgv::gui::provider* p) = 0;

public:
	/// call this in your viewer create_gui() method to create the gui of this subprovider as specified in create_gui_impl()
	void create_gui(cgv::gui::provider* p);
};

	}
}

#include <cgv/config/lib_end.h>