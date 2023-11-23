#include "control_manager.h"
#include <cgv_g2d/utils2d.h>

using namespace cgv::render;

namespace cgv {
namespace g2d {

control_manager::control_manager() {
	
}

bool control_manager::init(context& ctx) {

	ref_msdf_gl_canvas_font_renderer(ctx, 1);
	ref_msdf_font_regular(ctx, 1);

	//shaders.add("rectangle", shaders::rectangle);
	//shaders.add("line", shaders::line, { { "MODE", "0" } });
	//bool success = shaders.load_all(ctx, "control_manager::init()");

	//success &= label_texts.init(ctx);
	//success &= value_texts.init(ctx);

	init_styles();

	return true;// success;
}

void control_manager::destruct(context& ctx) {

	ref_msdf_gl_canvas_font_renderer(ctx, -1);
	ref_msdf_font_regular(ctx, -1);

	shaders.clear(ctx);
}

void control_manager::clear() {

	controls.clear();
}

bool control_manager::handle(cgv::gui::event& e, const ivec2& viewport_size, const irect& container) {

	unsigned et = e.get_kind();

	if(et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = dynamic_cast<cgv::gui::key_event&>(e);
		
		for(auto& control : controls) {
			if(control->handle_key_event(ke))
				return true;
		}
	} else if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = dynamic_cast<cgv::gui::mouse_event&>(e);
		ivec2 mouse_position = get_local_mouse_pos(ivec2(me.get_x(), me.get_y()), viewport_size, container);

		for(auto& control : controls) {
			if(control->handle_mouse_event(me, mouse_position))
				return true;
		}
	}

	return false;
}

void control_manager::draw(context& ctx, cgv::g2d::canvas& cnvs) {

	for(auto& control : controls)
		control->draw(ctx, cnvs, styles);
}

void control_manager::init_styles() {

	auto& ti = cgv::gui::theme_info::instance();

	styles.control_box.use_fill_color = true;
	styles.control_box.fill_color = ti.control();
	styles.control_box.feather_width = 1.0f;
	//styles.control_box.border_radius = 5.0f;
	//styles.control_box.use_blending = true;
	
	styles.text = text2d_style::preset_default(ti.text());
	styles.text.font_size = 12.0f;

	styles.colored_box = styles.control_box;
	styles.colored_box.use_fill_color = false;

	styles.control_color = ti.control();
	styles.background_color = ti.background();
	styles.shadow_color = ti.shadow();
}

void control_manager::handle_theme_change(const cgv::gui::theme_info& theme) {

	init_styles();
}

}
}
