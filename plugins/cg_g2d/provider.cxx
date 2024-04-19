#include "provider.h"

#include <cgv_g2d/msdf_font.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>

using namespace cgv::render;

namespace cg {
namespace g2d {

bool provider::init(context& ctx) {

	// TODO: ensure the canvas has the needed shaders

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);
	cgv::g2d::ref_msdf_font_regular(ctx, 1);

	//shaders.add("rectangle", shaders::rectangle);
	//shaders.add("line", shaders::line, { { "MODE", "0" } });
	//bool success = shaders.load_all(ctx, "control_manager::init()");

	init_styles();

	return true;// success;
}

void provider::destruct(context& ctx) {

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);
	cgv::g2d::ref_msdf_font_regular(ctx, -1);
}

void provider::clear() {

	buttons.clear();

	controls.clear();
	control_widgets.clear();
}

bool provider::handle(cgv::gui::event& e, const cgv::ivec2& viewport_size, const cgv::g2d::irect& container) {

	unsigned et = e.get_kind();

	if(et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = dynamic_cast<cgv::gui::key_event&>(e);
		
		for(auto& control_widget : control_widgets) {
			if(control_widget->handle_key_event(ke))
				return true;
		}
	} else if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = dynamic_cast<cgv::gui::mouse_event&>(e);
		cgv::ivec2 mouse_position = get_local_mouse_pos(cgv::ivec2(me.get_x(), me.get_y()), viewport_size, container);

		for(auto& button : buttons) {
			if(button->handle_mouse_event(me, mouse_position))
				return true;
		}

		for(auto& control_widget : control_widgets) {
			if(control_widget->handle_mouse_event(me, mouse_position))
				return true;
		}
	}

	return false;
}

void provider::draw(context& ctx, cgv::g2d::canvas& cnvs) {

	for(auto& button : buttons)
		button->draw(ctx, cnvs, style);

	for(auto& control_widget : control_widgets)
		control_widget->draw(ctx, cnvs, style);
}

void provider::init_styles() {

	auto& theme = cgv::gui::theme_info::instance();

	style.background_color = theme.background();
	style.group_color = theme.group();
	style.control_color = theme.control();
	style.shadow_color = theme.shadow();
	style.text_color = theme.text();
	style.selection_color = theme.selection();
	style.highlight_color = theme.highlight();

	style.text.fill_color = theme.text();
	style.text.font_size = 12.0f;

	style.flat_box.use_fill_color = false;
	style.flat_box.feather_width = 0.0f;

	style.rounded_box = style.flat_box;
	style.rounded_box.feather_width = 1.0f;
	style.rounded_box.border_radius = 10.0f;
	style.rounded_box.use_blending = true;
}

void provider::handle_theme_change(const cgv::gui::theme_info& theme) {

	init_styles();
}

}
}
