#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/base/find_action.h>
#include <cg_vr/vr_server.h>
#include "vr_tool.h"

namespace vr {

void vr_tool::on_device_change(void* device_handle, bool connect)
{
	if (connect)
		kit_ptr = vr::get_vr_kit(device_handle);
	else
		kit_ptr = 0;
}

//
vr_scene* vr_tool::find_scene(size_t scene_idx) const
{
	cgv::base::base* base_ptr = const_cast<cgv::base::base*>(static_cast<const cgv::base::base*>(this));
	std::vector<vr_scene*> scenes;
	cgv::base::find_interface<vr_scene>(base_ptr, scenes);
	if (scenes.empty() || scene_idx > scenes.size())
		return 0;
	return scenes[scene_idx];
}

vr_tool::vr_tool(const std::string& _name) : cgv::base::node(_name)
{
	tool_is_active = true;

	scene_ptr = 0;
	kit_ptr = 0;
	vr_view_ptr = 0;
	
	connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_tool::on_device_change);
}

void vr_tool::init_frame(cgv::render::context& ctx)
{
	if (!scene_ptr)
		scene_ptr = find_scene();
	if (!vr_view_ptr) {
		auto view_ptr = find_view_as_node();
		if (view_ptr)
			vr_view_ptr = dynamic_cast<vr_view_interactor*>(view_ptr);
	}
}

//	std::string vr_tool::get_default_options() const {
//		return "parents=none;views=none";
//	}

}