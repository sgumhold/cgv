#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/base/find_action.h>
#include <cg_vr/vr_server.h>
#include "vr_scene.h"

#include "lib_begin.h"

namespace vr {

class CGV_API vr_tool : 
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
protected:
	/// interaction parameters
	bool tool_is_active;
	/// store pointer to scene
	vr_scene* scene_ptr;
	/// store pointer to vr kit
	vr::vr_kit* kit_ptr;
	/// store pointer to vr view interactor
	vr_view_interactor* vr_view_ptr;
	/// use on device change to store the kit_ptr
	void on_device_change(void* device_handle, bool connect);
	// traverse the scene graph to find the vr_scene node and in case of several scene nodes use \c scene_idx parameter to select
	vr_scene* find_scene(size_t scene_idx = 0) const;
public:
	/// construct with all pointers set to null and tool being active
	vr_tool(const std::string& _name = "vr_tool");
	/// extract scene and view ptr here
	void init_frame(cgv::render::context& ctx);
//	std::string get_default_options() const;
};

}