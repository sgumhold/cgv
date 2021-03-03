#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/base/find_action.h>
#include <cg_vr/vr_server.h>
#include "vr_scene.h"

#include "lib_begin.h"

namespace vr {

class CGV_API vr_tool : public cgv::signal::tacker
{
private:
	/// store pointer to scene
	mutable vr_scene* scene_ptr;
	/// store pointer to vr kit
	mutable vr::vr_kit* kit_ptr;
	/// flag to remember that search of view yielded a no vr_view_interactor
	mutable bool no_vr_view;
	/// store pointer to vr view interactor
	mutable vr_view_interactor* view_ptr;
	/// use on device change to store the kit_ptr
	void on_device_change(void* device_handle, bool connect);
	// traverse the scene graph to find the vr_scene node and in case of several scene nodes use \c scene_idx parameter to select
	vr_scene* find_scene(size_t scene_idx = 0) const;
protected:
	/// interaction parameters
	bool tool_is_active;
	/// overload to be notified after a new kit has been connected
	virtual void after_connect_kit();
	/// overload to be notified before a kit is disconnected
	virtual void before_disconnect_kit();
	/// access to vr scene object, which can return nullptr in case scene is registered after tool
	vr_scene* get_scene_ptr() const;
	/// access to vr kit, which can return nullptr if no kit is attached
	vr_kit* get_kit_ptr() const;
	/// access to vr view, which can return nullptr in case no vr view is available or vr view is registered after tool
	vr_view_interactor* get_view_ptr() const;
public:
	/// construct with all pointers set to null and tool being active
	vr_tool();
//	std::string get_default_options() const;
};

}

#include <cgv/config/lib_end.h>
