#include "focusable.h"

namespace cgv {
	namespace nui {


		focus_attachment::focus_attachment()
		{
			level = focus_level::hid;
			hid_id = { hid_category::keyboard };
		}

		void focusable::reconfigure_focus(focus_request& request, bool pointing_not_grabbing, focus_configuration& last_focus_config) const
		{
			last_focus_config = request.demand.config;
			request.request = cgv::nui::focus_change_action::reconfigure;
			request.demand.config.refocus.spatial = false;
			request.demand.config.dispatch.structural = false;
			request.demand.config.dispatch.focus_recursive = false;
			if (pointing_not_grabbing)
				request.demand.config.spatial.proximity = false;
			else
				request.demand.config.spatial.pointing = false;
		}
		void focusable::recover_focus(focus_request& request, focus_configuration& last_focus_config) const
		{
			request.request = cgv::nui::focus_change_action::reconfigure;
			request.demand.config = last_focus_config;
		}
		focusable::focusable()
		{
		}
		bool focusable::wants_to_grab_focus(const cgv::gui::event& e, const hid_identifier& hid_id, focus_demand& demand)
		{
			return false;
		}
		void focusable::focus_change(focus_change_action action, refocus_action rfa, const focus_demand& demand, const cgv::gui::event& e, const dispatch_info& dis_info, cgv::base::base_ptr other_object_ptr)
		{
		}
	}
}
