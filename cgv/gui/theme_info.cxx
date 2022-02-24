#include "theme_info.h"

namespace cgv {
namespace gui {

theme_info& theme_info::instance() {
	static theme_info instance;
	return instance;
}

theme_info::theme_info() {
	background_col = rgb(0.0f);
	group_col = rgb(0.0f);
	control_col = rgb(0.0f);
	border_col = rgb(0.0f);
	selection_col = rgb(0.0f);
	highlight_col = rgb(0.0f);
	warning_col = rgb(0.0f);
}

}
}
