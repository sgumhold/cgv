#include "focusable.h"
#include "dispatcher.h"

namespace cgv {
	namespace nui {

		std::string to_string(refocus_policy rp, char sep)
		{
			std::string res;
			consider_flag(rp.deattach, "deattach", res, sep);
			consider_flag(rp.transfer, "transfer", res, sep);
			consider_flag(rp.grab, "grab", res, sep);
			consider_flag(rp.handle, "handle", res, sep);
			consider_flag(rp.spatial, "spatial", res, sep);
			return res;
		}
		std::string to_string(dispatch_policy dp, char sep)
		{
			std::string res;
			consider_flag(dp.focus_recursive, "focus_recursive", res, sep);
			consider_flag(dp.structural, "structural", res, sep);
			consider_flag(dp.current_root_first, "current_root_first", res, sep);
			consider_flag(dp.spatial, "spatial", res, sep);
			consider_flag(dp.event_handler, "event_handler", res, sep);			
			return res;

		}
		std::string to_string(spatial_analysis sa, char sep)
		{
			std::string res;
			consider_flag(sa.only_focus, "only_focus", res, sep);
			consider_flag(sa.proximity, "proximity", res, sep);
			consider_flag(sa.pointing, "pointing", res, sep);
			return res;
		}
		std::string to_string(focus_level fl)
		{
			const char* names[] = { "none", "hid", "kit", "category", "all" };
			return names[int(fl)];
		}
		std::ostream& operator << (std::ostream& os, const focus_configuration& foc_cfg)
		{
			return os << "dispatch=(" << to_string(foc_cfg.dispatch)
				<< "), refocus=(" << to_string(foc_cfg.refocus)
				<< "), spatial=(" << to_string(foc_cfg.spatial)
				<< ")";
		}
		std::ostream& operator << (std::ostream& os, const focus_attachment& foc_att)
		{
			os << to_string(foc_att.level);
			switch (foc_att.level) {
			case focus_level::hid: os << "[" << foc_att.hid_id << "]"; break;
			case focus_level::kit: os << "[" << foc_att.kit_id << "]"; break;
			case focus_level::category: os << "[" << to_string(foc_att.selection) << "]"; break;
			}
			return os;
		}

		focus_attachment::focus_attachment(focus_level _level)
		{
			level = _level;
			hid_id = { hid_category::keyboard };
		}
		focus_attachment::focus_attachment(const hid_identifier& _hid_id)
		{
			level = focus_level::hid;
			hid_id = _hid_id;
		}
		focus_attachment::focus_attachment(const kit_identifier& _kit_id)
		{
			level = focus_level::kit;
			kit_id = _kit_id;
		}
		focus_attachment::focus_attachment(hid_selection _selection)
		{
			level = focus_level::category;
			selection = _selection;
		}
		void focusable::set_dispatcher_ptr(dispatcher* _disp_ptr)
		{
			disp_ptr = _disp_ptr;
		}

		void focusable::drag_begin(focus_request& request, bool pointing_not_grabbing, focus_configuration& last_focus_config) const
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
		void focusable::drag_end(focus_request& request, focus_configuration& last_focus_config) const
		{
			request.request = cgv::nui::focus_change_action::reconfigure;
			request.demand.config = last_focus_config;
		}
		bool focusable::drag_end(const focus_attachment& foc_att, cgv::base::base_ptr obj_ptr, focus_configuration& focus_config) const
		{
			if (!disp_ptr)
				return false;
			return disp_ptr->reconfigure(foc_att, obj_ptr, focus_config);
		}
		focusable::focusable()
		{
		}
		bool focusable::wants_to_grab_focus(const cgv::gui::event& e, const hid_identifier& hid_id, focus_demand& demand)
		{
			return false;
		}
		bool focusable::focus_change(focus_change_action action, refocus_action rfa, const focus_demand& demand, const cgv::gui::event& e, const dispatch_info& dis_info)
		{
			return true;
		}
	}
}
