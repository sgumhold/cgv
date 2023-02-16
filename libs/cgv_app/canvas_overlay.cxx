#include "canvas_overlay.h"

namespace cgv {
namespace app {

canvas_overlay::canvas_overlay() : overlay() {}

void canvas_overlay::clear(cgv::render::context& ctx) {

	content_canvas.destruct(ctx);
	overlay_canvas.destruct(ctx);
	fbc.clear(ctx);
}

bool canvas_overlay::init(cgv::render::context& ctx) {
	
	bool success = true;

	fbc.add_attachment("color", "uint8[R,G,B,A]");
	fbc.set_size(get_overlay_size());
	success &= fbc.ensure(ctx);

	content_canvas.set_apply_gamma(false);
	success &= content_canvas.init(ctx);

	overlay_canvas.register_shader("rectangle", cgv::g2d::canvas::shaders_2d::rectangle);
	success &= overlay_canvas.init(ctx);

	if(success)
		init_overlay_style(ctx);

	return success;
}

void canvas_overlay::draw(cgv::render::context& ctx) {

	if(!show)
		return;

	GLboolean depth_test_was_enabled = false;
	glGetBooleanv(GL_DEPTH_TEST, &depth_test_was_enabled);
	if(depth_test_was_enabled)
		glDisable(GL_DEPTH_TEST);

	if(blend_overlay)
		enable_blending();

	if(has_damage)
		draw_content(ctx);

	// draw frame buffer texture to screen
	auto& overlay_prog = overlay_canvas.enable_shader(ctx, "rectangle");
	fbc.enable_attachment(ctx, "color", 0);
	overlay_canvas.draw_shape(ctx, get_overlay_position(), get_overlay_size());
	fbc.disable_attachment(ctx, "color");
	overlay_canvas.disable_current_shader(ctx);

	if(depth_test_was_enabled)
		glEnable(GL_DEPTH_TEST);

	if(blend_overlay)
		disable_blending();
}

void canvas_overlay::register_shader(const std::string& name, const std::string& filename) {

	content_canvas.register_shader(name, filename);
}

void canvas_overlay::init_overlay_style(cgv::render::context& ctx) {

	// configure style for final blending of overlay into main frame buffer
	cgv::g2d::shape2d_style overlay_style;
	overlay_style.use_texture = true;
	overlay_style.use_blending = blend_overlay;
	overlay_style.feather_width = 0.0f;

	auto& overlay_prog = overlay_canvas.enable_shader(ctx, "rectangle");
	overlay_style.apply(ctx, overlay_prog);
	overlay_canvas.disable_current_shader(ctx);
}

bool canvas_overlay::ensure_layout(cgv::render::context& ctx) {

	if(ensure_overlay_layout(ctx) || recreate_layout) {
		recreate_layout = false;
		has_damage = true;

		fbc.set_size(get_overlay_size());
		fbc.ensure(ctx);

		content_canvas.set_resolution(ctx, get_overlay_size());
		overlay_canvas.set_resolution(ctx, get_viewport_size());
		return true;
	}
	return false;
}

void canvas_overlay::post_recreate_layout() {
	recreate_layout = true;
}

void canvas_overlay::post_damage(bool redraw) {
	has_damage = true;
	if(redraw)
		post_redraw();
}

void canvas_overlay::clear_damage() {
	has_damage = false;
}

bool canvas_overlay::is_damaged() const {
	return has_damage;
}

bool canvas_overlay::ensure_theme() {
	
	auto& ti = cgv::gui::theme_info::instance();
	int theme_idx = ti.get_theme_idx();
	if(last_theme_idx != theme_idx) {
		last_theme_idx = theme_idx;
		has_damage = true;
		return true;
	}
	return false;
}

void canvas_overlay::begin_content(cgv::render::context& ctx, bool clear_frame_buffer) {

	fbc.enable(ctx);
	if(clear_frame_buffer) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

void canvas_overlay::end_content(cgv::render::context& ctx, bool keep_damage) {

	fbc.disable(ctx);
	has_damage = keep_damage;
}

void canvas_overlay::enable_blending() {

	glGetBooleanv(GL_BLEND, &blending_was_enabled);
	if(!blending_was_enabled)
		glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void canvas_overlay::disable_blending() {

	if(!blending_was_enabled)
		glDisable(GL_BLEND);
}

}
}
