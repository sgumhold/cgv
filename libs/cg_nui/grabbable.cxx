#include "grabbable.h"
#include <cg_vr/vr_events.h>

namespace cgv {
	namespace nui {
		void proximity_dispatch_info::copy(const dispatch_info& dis_info)
		{
			*this = static_cast<const proximity_dispatch_info&>(dis_info);
		}
		const hit_info* proximity_dispatch_info::get_hit_info() const
		{
			return this; 
		}
		dispatch_info* proximity_dispatch_info::clone() const
		{
			return new proximity_dispatch_info(*this);
		}
		grabbable::grabbable()
		{
		}
		bool grabbable::is_grab_change(const cgv::gui::event& e, bool& grabbed) const
		{
			if (e.get_kind() != cgv::gui::EID_KEY)
				return false;
			if ((e.get_flags() & cgv::gui::EF_VR) == 0)
				return false;
			const auto& vrke = reinterpret_cast<const cgv::gui::vr_key_event&>(e);
			if (vrke.get_key() != vr::VR_GRIP)
				return false;
			grabbed = vrke.get_action() == cgv::gui::KA_PRESS;
			return true;
		}
		bool grabbable::is_grabbing(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info) const
		{
			if (dis_info.mode != cgv::nui::dispatch_mode::proximity)
				return false;
			if (e.get_kind() == cgv::gui::EID_POSE)
				return true;
			return false;
		}
		const proximity_info& grabbable::get_proximity_info(const dispatch_info& dis_info) const
		{
			return reinterpret_cast<const proximity_dispatch_info&>(dis_info);
		}
	}
}
