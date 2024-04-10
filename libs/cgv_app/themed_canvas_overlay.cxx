#include "themed_canvas_overlay.h"

namespace cgv {
namespace app {

themed_canvas_overlay::themed_canvas_overlay() : canvas_overlay() {
	const auto& theme = cgv::gui::theme_info::instance();
	init_container_style(theme);
	set_margin(cgv::ivec2(-theme.spacing()));
}

bool themed_canvas_overlay::init(cgv::render::context& ctx) {
	register_shader("rectangle", cgv::g2d::shaders::rectangle);
	return canvas_overlay::init(ctx);
}

void themed_canvas_overlay::begin_content(cgv::render::context& ctx, bool clear_frame_buffer) {
	canvas_overlay::begin_content(ctx, clear_frame_buffer);
	
	if(background_visible_) {
		content_canvas.enable_shader(ctx, "rectangle");
		content_canvas.set_style(ctx, container_style_);
		content_canvas.draw_shape(ctx, get_local_rectangle());
		content_canvas.disable_current_shader(ctx);
	}
}

void themed_canvas_overlay::handle_theme_change(const cgv::gui::theme_info& theme) {
	init_container_style(theme);
	init_styles();
	post_damage();
}

void themed_canvas_overlay::init_container_style(const cgv::gui::theme_info& theme) {
	// configure style for the container background
	container_style_.fill_color = theme.group();
	container_style_.border_color = theme.background();
	container_style_.border_width = static_cast<float>(theme.spacing());
	container_style_.feather_width = 0.0f;
}

cgv::g2d::irect themed_canvas_overlay::get_content_rectangle() const {
	auto rectangle = get_local_rectangle();
	rectangle.scale(-padding_);
	return rectangle;
}

void themed_canvas_overlay::set_background_visible(bool flag) {
	background_visible_ = flag;
	on_set(&background_visible_);
}

}
}
