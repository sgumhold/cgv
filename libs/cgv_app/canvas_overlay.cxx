#include "canvas_overlay.h"

namespace cgv {
namespace app {

canvas_overlay::canvas_overlay() : overlay() {

	blit_style_.use_blending = true;
	blit_style_.use_texture = true;
	blit_style_.feather_width = 0.0f;
}

bool canvas_overlay::init(cgv::render::context& ctx) {
	
	bool success = true;

	frame_buffer_.add_attachment("color", "uint8[R,G,B,A]");
	frame_buffer_.set_size(get_rectangle().size);
	success &= frame_buffer_.ensure(ctx);

	content_canvas.set_apply_gamma(false);
	success &= content_canvas.init(ctx);

	overlay_canvas.register_shader("rectangle", cgv::g2d::shaders::rectangle);
	success &= overlay_canvas.init(ctx);

	init_styles();

	return success;
}

void canvas_overlay::clear(cgv::render::context& ctx) {

	content_canvas.destruct(ctx);
	overlay_canvas.destruct(ctx);
	frame_buffer_.destruct(ctx);
}

void canvas_overlay::on_set(void* member_ptr) {

	handle_member_change(cgv::utils::pointer_test(member_ptr));
	update_member(member_ptr);
	post_damage();
}

void canvas_overlay::draw(cgv::render::context& ctx) {

	if(is_visible() && !draw_in_finish_frame)
		draw_impl(ctx);
}

void canvas_overlay::finish_frame(cgv::render::context& ctx) {

	if(is_visible() && draw_in_finish_frame)
		draw_impl(ctx);
}

void canvas_overlay::register_shader(const std::string& name, const std::string& filename) {

	content_canvas.register_shader(name, filename);
}

void canvas_overlay::post_damage(bool redraw) {
	has_damage_ = true;
	if(redraw)
		post_redraw();
}

bool canvas_overlay::ensure_layout(cgv::render::context& ctx) {

	if(overlay::ensure_layout(ctx) || recreate_layout_requested_) {
		recreate_layout_requested_ = false;
		has_damage_ = true;

		frame_buffer_.set_size(get_rectangle().size);
		frame_buffer_.ensure(ctx);

		content_canvas.set_resolution(ctx, get_rectangle().size);
		overlay_canvas.set_resolution(ctx, get_viewport_size());
		return true;
	}
	return false;
}

void canvas_overlay::post_recreate_layout() {
	recreate_layout_requested_ = true;
}

void canvas_overlay::clear_damage() {
	has_damage_ = false;
}

bool canvas_overlay::is_damaged() const {
	return has_damage_;
}

void canvas_overlay::begin_content(cgv::render::context& ctx, bool clear_frame_buffer) {

	frame_buffer_.enable(ctx);
	if(clear_frame_buffer) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

void canvas_overlay::end_content(cgv::render::context& ctx, bool keep_damage) {

	frame_buffer_.disable(ctx);
	has_damage_ = keep_damage;
}

void canvas_overlay::draw_impl(cgv::render::context& ctx) {

	GLboolean depth_test_was_enabled = false;
	glGetBooleanv(GL_DEPTH_TEST, &depth_test_was_enabled);
	if(depth_test_was_enabled)
		glDisable(GL_DEPTH_TEST);

	GLboolean blending_was_enabled = false;
	glGetBooleanv(GL_BLEND, reinterpret_cast<GLboolean*>(&blending_was_enabled));
	if(!blending_was_enabled)
		glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

	if(has_damage_)
		draw_content(ctx);

	// draw frame buffer texture to screen
	glBlendFunc(GL_ONE, GL_SRC_ALPHA);

	overlay_canvas.enable_shader(ctx, "rectangle");
	overlay_canvas.set_style(ctx, blit_style_);
	frame_buffer_.enable_attachment(ctx, "color", 0);
	overlay_canvas.draw_shape(ctx, get_rectangle());
	frame_buffer_.disable_attachment(ctx, "color");
	overlay_canvas.disable_current_shader(ctx);

	if(depth_test_was_enabled)
		glEnable(GL_DEPTH_TEST);

	if(!blending_was_enabled)
		glDisable(GL_BLEND);
}

}
}
