#include "pointable.h"
#include <cgv/gui/mouse_event.h>
#include <cg_vr/vr_events.h>

namespace cgv {
	namespace nui {

		void intersection_dispatch_info::copy(const dispatch_info& dis_info)
		{
			*this = static_cast<const intersection_dispatch_info&>(dis_info);
		}
		const hit_info* intersection_dispatch_info::get_hit_info() const
		{
			return this;
		}
		dispatch_info* intersection_dispatch_info::clone() const
		{
			return new intersection_dispatch_info(*this);
		}
		pointable::pointable()
		{
		}
		bool pointable::is_trigger_change(const cgv::gui::event& e, bool& triggered) const
		{
			if (e.get_kind() == cgv::gui::EID_KEY && ((e.get_flags() & cgv::gui::EF_VR) != 0)) {
				const auto& vrke = reinterpret_cast<const cgv::gui::vr_key_event&>(e);
				if (vrke.get_key() != vr::VR_INPUT1)
					return false;
				triggered = vrke.get_action() == cgv::gui::KA_PRESS;
				return true;
			}
			if (e.get_kind() == cgv::gui::EID_MOUSE) {
				const auto& me = reinterpret_cast<const cgv::gui::mouse_event&>(e);
				if (me.get_button() != cgv::gui::MB_LEFT_BUTTON)
					return false;
				if (me.get_action() == cgv::gui::MA_PRESS) {
					triggered = true;
					return true;
				}
				if (me.get_action() == cgv::gui::MA_RELEASE) {
					triggered = false;
					return true;
				}
			}
			return false;
		}
		bool pointable::is_pointing(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info) const
		{
			if (dis_info.mode != cgv::nui::dispatch_mode::pointing)
				return false;
			if (e.get_kind() == cgv::gui::EID_POSE)
				return true;
			if (e.get_kind() == cgv::gui::EID_MOUSE) {
				const auto& me = reinterpret_cast<const cgv::gui::mouse_event&>(e);
				switch (me.get_action()) {
				case cgv::gui::MA_ENTER:
				case cgv::gui::MA_MOVE:
				case cgv::gui::MA_DRAG:
					return true;
				}
			}
			return false;
		}
		const intersection_info& pointable::get_intersection_info(const dispatch_info& dis_info) const
		{
			return reinterpret_cast<const intersection_dispatch_info&>(dis_info);
		}
	}
}
