#include "control_manager.h"

//#include <cgv_g2d/utils2d.h>

#include <cgv_g2d/msdf_font.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>

using namespace cgv::render;

namespace cg {
namespace g2d {

bool control_manager::init(context& ctx) {

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);
	cgv::g2d::ref_msdf_font_regular(ctx, 1);

	//shaders.add("rectangle", shaders::rectangle);
	//shaders.add("line", shaders::line, { { "MODE", "0" } });
	//bool success = shaders.load_all(ctx, "control_manager::init()");

	//success &= label_texts.init(ctx);
	//success &= value_texts.init(ctx);

	init_styles();

	return true;// success;
}

void control_manager::destruct(context& ctx) {

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);
	cgv::g2d::ref_msdf_font_regular(ctx, -1);

	//shaders.clear(ctx);
}

void control_manager::clear() {

	controls.clear();
	g2d_controls.clear();
}

bool control_manager::handle(cgv::gui::event& e, const ivec2& viewport_size, const cgv::g2d::irect& container) {

	unsigned et = e.get_kind();

	if(et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = dynamic_cast<cgv::gui::key_event&>(e);
		
		for(auto& control : g2d_controls) {
			if(control->handle_key_event(ke))
				return true;
		}
	} else if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = dynamic_cast<cgv::gui::mouse_event&>(e);
		ivec2 mouse_position = get_local_mouse_pos(ivec2(me.get_x(), me.get_y()), viewport_size, container);

		for(auto& control : g2d_controls) {
			if(control->handle_mouse_event(me, mouse_position))
				return true;
		}
	}

	return false;
}

void control_manager::draw(context& ctx, cgv::g2d::canvas& cnvs) {

	for(auto& control : g2d_controls)
		control->draw(ctx, cnvs, style);
}

void control_manager::init_styles() {

	auto& ti = cgv::gui::theme_info::instance();

	style.control_box.use_fill_color = true;
	style.control_box.fill_color = ti.control();
	style.control_box.feather_width = 1.0f;
	//style.control_box.border_radius = 5.0f;
	//style.control_box.use_blending = true;
	
	style.text = cgv::g2d::text2d_style::preset_default(ti.text());
	style.text.font_size = 12.0f;

	style.colored_box = style.control_box;
	style.colored_box.use_fill_color = false;

	style.control_color = ti.control();
	style.background_color = ti.background();
	style.shadow_color = ti.shadow();
}

void control_manager::handle_theme_change(const cgv::gui::theme_info& theme) {

	init_styles();
}

}
}
