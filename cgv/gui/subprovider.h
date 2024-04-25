#pragma once

#include "provider.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// derive from this class to use a provider from another viewer
class CGV_API subprovider
{
private:
	// used in the optional tree node to store its visibility state
	int tree_node_handle = 0;

protected:
	/// pointer to the main provider
	provider* provider_ptr = nullptr;

	/// call this to update all views and controls of a member
	void update_member(void* member_ptr);

	/// call this to update the value and all views and controls of a member
	template<typename T>
	void set_and_update_member(T& member, const T& value) {

		member = value;
		update_member(&member);
	}

	/// implement this to add gui controls using the supplied base and provider
	virtual void create_gui_impl(cgv::base::base* b, provider* p) = 0;

public:
	/// Create the gui of the subprovider using the implementation given in create_gui_impl().
	/// Call this in your provider::create_gui() method.
	void create_gui(provider* p);

	/// Create the gui of the subprovider using the implementation given in create_gui_impl() and
	/// additionally create a tree node around the actual gui.
	/// Call this in your provider::create_gui() method.
	void create_gui_tree_node(provider* p, const std::string& title, bool initial_visibility, const std::string& options = "");
};

	}
}

#include <cgv/config/lib_end.h>