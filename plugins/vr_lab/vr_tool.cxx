#include "vr_tool.h"
#include <cgv/base/find_action.h>

namespace vr {

void vr_tool::on_device_change(void* device_handle, bool connect)
{
	if (connect) {
		kit_ptr = vr::get_vr_kit(device_handle);
		after_connect_kit();
	}
	else {
		before_disconnect_kit();
		kit_ptr = 0;
	}
}

//
vr_scene* vr_tool::find_scene(size_t scene_idx) const
{
	cgv::base::base* base_ptr = const_cast<cgv::base::base*>(dynamic_cast<const cgv::base::base*>(this));
	std::vector<vr_scene*> scenes;
	cgv::base::find_interface<vr_scene>(base_ptr, scenes);
	if (scenes.empty() || scene_idx > scenes.size())
		return 0;
	return scenes[scene_idx];
}

/// overload to be notified after a new kit has been connected
void vr_tool::after_connect_kit()
{
}
/// overload to be notified before a kit is disconnected
void vr_tool::before_disconnect_kit()
{
}
/// access to vr scene object, which can return nullptr in case scene is registered after tool
vr_scene* vr_tool::get_scene_ptr() const
{
	if (!scene_ptr)
		scene_ptr = find_scene();
	return scene_ptr;
}
/// access to vr kit, which can return nullptr if no kit is attached
vr_kit* vr_tool::get_kit_ptr() const
{
	return kit_ptr;
}

/// access to vr view, which can return nullptr in case no vr view is available or vr view is registered after tool
vr_view_interactor* vr_tool::get_view_ptr() const
{
	if (!view_ptr && !no_vr_view) {
		cgv::base::base* base_ptr = const_cast<cgv::base::base*>(dynamic_cast<const cgv::base::base*>(this));
		std::vector<vr_view_interactor*> views;
		cgv::base::find_interface<vr_view_interactor>(base_ptr, views);
		if (views.empty()) {
			no_vr_view = true;
			std::cerr << "WARNING: found view which cannot be cast into vr_view_interactor!\nmake sure to have crg_vr_view in your addProjectDeps list." << std::endl;
		}
		else {
			view_ptr = views[0];
		}
	}
	return view_ptr;
}

vr_tool::vr_tool() 
{
	tool_is_active = true;

	scene_ptr = 0;
	kit_ptr = 0;
	view_ptr = 0;
	
	no_vr_view = false;

	connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_tool::on_device_change);
}

//	std::string vr_tool::get_default_options() const {
//		return "parents=none;views=none";
//	}

}