#include "theme_info.h"

namespace cgv {
namespace gui {

theme_info& theme_info::instance() {
	static theme_info instance;
	return instance;
}

theme_info::theme_info() {
	theme_idx = -1;

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

void theme_info::set_theme_idx(int idx) {
	if(idx != theme_idx) {
		theme_idx = idx;
		on_change(instance());
	}
}

int theme_info::get_theme_idx() const {
	return theme_idx;
}

}
}
