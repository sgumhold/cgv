#include "theme_info.h"

namespace cgv {
namespace gui {

theme_info& theme_info::instance() {
	static theme_info instance;
	return instance;
}

theme_info::theme_info() {
	background(240, 240, 240);
	group(240, 240, 240);
	control(225, 225, 225);
	border(98, 98, 98);
	text(0, 0, 0);
	text_background(255, 255, 255);
	selection(0, 120, 215);
	highlight(0, 120, 215);
	warning(255, 0, 0);
}

bool theme_info::is_dark() const {
	return index_ > 1;
}

void theme_info::notify_observers() const {
	on_change(instance());
}

}
}
