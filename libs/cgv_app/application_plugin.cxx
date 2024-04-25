#include "application_plugin.h"

#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>

namespace cgv {
namespace app {

application_plugin_base::application_plugin_base(const std::string& name) : group(name)
{
	view_ptr = nullptr;
	last_blocking_overlay_ptr = nullptr;
	blocking_overlay_ptr = nullptr;
}

bool application_plugin_base::handle(cgv::gui::event& e)
{
	if(e.get_kind() == cgv::gui::EID_MOUSE) {
		// Perform special handling for mouse events.
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
		cgv::gui::MouseAction ma = me.get_action();

		// Drag & Drop events are never handled by overlays.
		if(me.get_flags() & cgv::gui::EF_DND)
			return handle_event(e);

		// Only mouse MA_PRESS, MA_MOVE or MA_WHEEL events can trigger an overlay to capture an event.
		if(ma == cgv::gui::MA_PRESS || ma == cgv::gui::MA_MOVE || ma == cgv::gui::MA_WHEEL) {
			ivec2 mpos(me.get_x(), me.get_y());

			// Test for all visible overlays in reverse registration order if they are hit by the current
			// mouse pointer position and store the first hit overlay as the active blocking overlay.
			blocking_overlay_ptr = nullptr;
			for(auto it = overlays.rbegin(); it != overlays.rend(); ++it) {
				overlay_ptr op = (*it);
				if(op->is_visible() && op->is_hit(mpos)) {
					blocking_overlay_ptr = op;
					break;
				}
			}

			// Test if the mouse pointer enters or leaves the active overlay, which may issue additional event handling.
			if(ma == cgv::gui::MA_MOVE && blocking_overlay_ptr != last_blocking_overlay_ptr) {
				if(last_blocking_overlay_ptr) {
					// The mouse pointer leaves the active overlay. Handle it as an additional MA_LEAVE event.
					me.set_action(cgv::gui::MA_LEAVE);
					last_blocking_overlay_ptr->handle_event(e);
				}
				if(blocking_overlay_ptr) {
					// The mouse pointer enters the active overlay. Handle it as an additional MA_ENTER event.
					me.set_action(cgv::gui::MA_ENTER);
					if(!blocking_overlay_ptr->handle_event(e))
						blocking_overlay_ptr = nullptr;
				}
				// Revert the event action to MA_MOVE.
				me.set_action(cgv::gui::MA_MOVE);
			}
		}

		bool was_handled = false;
		bool was_blocked = false;
		bool result = false;

		// Call handle_event on the active overlay, if found, and record the result and whether the overlay
		// will block the event from further processing.
		if(blocking_overlay_ptr) {
			was_handled = true;
			was_blocked = blocking_overlay_ptr->blocks_events();
			result = blocking_overlay_ptr->handle_event(e);
		}

		// An MA_RELASE action will reset the active overlay.
		if(ma == cgv::gui::MA_RELEASE)
			blocking_overlay_ptr = nullptr;

		// Store the current active overlay for reference to the next event.
		last_blocking_overlay_ptr = blocking_overlay_ptr;

		// Return the result of handle_event from the overlay or true if it was blocked, indicating that the event will not be passed on to the next plugin.
		// If the event was not handled, pass it to the application plugin itself.
		if(was_handled) {
			if(was_blocked)
				return true;
			else
				return result;
		} else {
			return handle_event(e);
		}
	} else {
		// TODO: make the overlay have a handles keys flag?
		// TODO: have a flag that enables blocking the event from further processing when returning true or false?
		// 
		// Handle all non-mouse type events on all visible overlays in reverse registration order.
		for(auto it = overlays.rbegin(); it != overlays.rend(); ++it) {
			overlay_ptr op = (*it);
			if(op->is_visible())
				op->handle_event(e);
		}
		return handle_event(e);
	}
}

application_plugin::application_plugin(const std::string& name) : generic_application_plugin(name)
{
}

} // namespace app
} // namespace cgv
