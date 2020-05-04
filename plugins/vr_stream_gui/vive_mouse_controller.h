
#include <cgv/gui/event_handler.h>

#include "cooldown.h"

namespace trajectory {
namespace util {
	class vive_mouse_controller : public cgv::gui::event_handler {
	  private:
		float throttle_thresh = 0.2f;
		float scroll_thresh = 0.1f;

		bool touching = false;
		bool dragging_left = false;
		bool dragging_middle = false;

		cooldown left_click_cd = {200};

	  public:
		vive_mouse_controller() = default;
		~vive_mouse_controller() = default;

		void stream_help(std::ostream &os);
		bool handle(cgv::gui::event &e);

		// try to fix invalid state, e.g. perma-holding left mouse button
		void try_fix_state();
	};
} // namespace util
} // namespace trajectory
