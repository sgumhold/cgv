#include "application_plugin.h"

#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>

namespace cgv {
namespace app {

application_plugin::application_plugin(const std::string& name) : group(name) 
{
}

bool application_plugin::handle(cgv::gui::event& e) 
{
	if(e.get_kind() == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
		cgv::gui::MouseAction ma = me.get_action();

		if(me.get_flags() & cgv::gui::EF_DND)
			return handle_event(e);

		if(ma == cgv::gui::MA_PRESS || ma == cgv::gui::MA_MOVE || ma == cgv::gui::MA_WHEEL) {
			ivec2 mpos(me.get_x(), me.get_y());

			blocking_overlay_ptr = nullptr;
			for(auto it = overlays.rbegin(); it != overlays.rend(); ++it) {
				overlay_ptr op = (*it);
				if(op->is_visible() && op->is_hit(mpos)) {
					blocking_overlay_ptr = op;
					break;
				}
			}

			if(ma == cgv::gui::MA_MOVE && blocking_overlay_ptr != last_blocking_overlay_ptr) {
				if(last_blocking_overlay_ptr) {
					me.set_action(cgv::gui::MA_LEAVE);
					last_blocking_overlay_ptr->handle_event(e);
				}
				if(blocking_overlay_ptr) {
					me.set_action(cgv::gui::MA_ENTER);
					if (!blocking_overlay_ptr->handle_event(e))
						blocking_overlay_ptr = nullptr;
				}
				me.set_action(cgv::gui::MA_MOVE);
			}
		}

		bool was_handled = false;
		bool was_blocked = false;
		bool result = false;

		if(blocking_overlay_ptr) {
			result = blocking_overlay_ptr->handle_event(e);
			was_handled = true;
			was_blocked = blocking_overlay_ptr->blocks_events();
		}

		if(ma == cgv::gui::MA_RELEASE) {
			blocking_overlay_ptr = nullptr;
		}

		last_blocking_overlay_ptr = blocking_overlay_ptr;

		if(was_handled)
			if(was_blocked)
				return true;
			else
				return result;
		else
			return handle_event(e);
	} else {
		// TODO: make the overlay have a handles keys flag?
		// TODO: have a flag that enables blocking the event from further processing when returning true or false?
		for(auto it = overlays.rbegin(); it != overlays.rend(); ++it) {
			overlay_ptr op = (*it);
			if(op->is_visible())
				op->handle_event(e);
		}
		return handle_event(e);
	}
}

void application_plugin::on_set(void* member_ptr)
{
	on_set(on_set_evaluator(member_ptr));
	update_member(member_ptr);
	post_redraw();
}

bool application_plugin::initialize_view_ptr()
{
	return !view_ptr && (view_ptr = find_view_as_node());
}

}
}
