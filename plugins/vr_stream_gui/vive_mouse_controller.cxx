
#include "vive_mouse_controller.h"

#include <cg_vr/vr_events.h>
#include <cg_vr/vr_server.h>

#include "mouse_win.h"

namespace trajectory {
namespace util {

	void vive_mouse_controller::stream_help(std::ostream &os)
	{
		os << "vive mouse handler: no shortcuts defined" << std::endl;
	}

	bool vive_mouse_controller::handle(cgv::gui::event &e)
	{
		if ((e.get_flags() & cgv::gui::EF_VR) == 0) 
			return false;

		switch (e.get_kind()) {
		case cgv::gui::EID_KEY: {
			cgv::gui::vr_key_event &vrke = static_cast<cgv::gui::vr_key_event &>(e);

			if (vrke.get_action() == cgv::gui::KA_PRESS) {

				//std::cout << vrke.get_key();

				switch (vrke.get_key()) {
				case vr::VR_RIGHT_STICK_DOWN: {
					if (dragging_middle) return false;
					util::mouse::hold_left();
					dragging_left = true;
					return true;
				}
				case vr::VR_RIGHT_STICK_RIGHT: {
					util::mouse::right_click();
					return true;
				}
				case vr::VR_RIGHT_STICK: // fallthrough
				case vr::VR_RIGHT_STICK_LEFT: {
					if (dragging_left) return false;
					util::mouse::hold_middle();
					dragging_middle = true;
					return true;
				}
				}
			}

			if (vrke.get_action() == cgv::gui::KA_RELEASE) {

				switch (vrke.get_key()) {
				case vr::VR_RIGHT_STICK_DOWN: {
					if (dragging_middle) return false;
					util::mouse::release_left();
					dragging_left = false;
					return true;
				}
				case vr::VR_RIGHT_STICK_LEFT: {
					if (dragging_left) return false;
					util::mouse::release_middle();
					dragging_middle = false;
					return true;
				}
				}
			}

			return false;
		}
		case cgv::gui::EID_THROTTLE: {
			cgv::gui::vr_throttle_event &vrte =
			    static_cast<cgv::gui::vr_throttle_event &>(e);
			if (vrte.get_controller_index() == 1) {
				if (vrte.get_value() - vrte.get_last_value() > throttle_thresh) {
					if (left_click_cd.is_done_and_reset()) util::mouse::left_click();
				}
			}
			return true;
		}
		case cgv::gui::EID_STICK: {
			cgv::gui::vr_stick_event &vrse = static_cast<cgv::gui::vr_stick_event &>(e);
			switch (vrse.get_action()) {
			case cgv::gui::SA_TOUCH:
			case cgv::gui::SA_PRESS:
			case cgv::gui::SA_UNPRESS:
			case cgv::gui::SA_RELEASE: {
				if (vrse.get_controller_index() == 1) {
					if (vrse.get_action() == cgv::gui::StickAction::SA_TOUCH) {
						touching = true;
					}
					else if (vrse.get_action() == cgv::gui::StickAction::SA_RELEASE) {
						touching = false;
					}
				}
				return true;
			}
			case cgv::gui::SA_MOVE:
			case cgv::gui::SA_DRAG:
				if (touching && vrse.get_controller_index() == 1) {
					auto dir = vrse.get_y() - vrse.get_last_y();
					if (std::abs(dir) > scroll_thresh) {
						if (dir < 0.0f)
							util::mouse::scroll_down(1);
						else if (dir > 0.0f)
							util::mouse::scroll_up(1);
					}
				}
				return true;
			}
			return false;
		}
		case cgv::gui::EID_POSE:
			break;
		}
		return false;
	}
	void vive_mouse_controller::try_fix_state()
	{
		if (dragging_left) {
			dragging_left = false;
			util::mouse::release_left();
		}
		if (dragging_middle) {
			dragging_middle = false;
			util::mouse::release_middle();
		}
		if (touching) touching = false;
	}
} // namespace util
} // namespace trajectory