#include "canvas_overlay.h"

using namespace cgv::render;

namespace cgv {
namespace app {

canvas_overlay::canvas_overlay() : overlay() {

	blit_style_.use_blending = true;
	blit_style_.use_texture = true;
	blit_style_.feather_width = 0.0f;
}

bool canvas_overlay::init(context& ctx) {
	
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

void canvas_overlay::clear(context& ctx) {

	content_canvas.destruct(ctx);
	overlay_canvas.destruct(ctx);
	frame_buffer_.destruct(ctx);
}

void canvas_overlay::on_set(void* member_ptr) {

	handle_member_change(cgv::utils::pointer_test(member_ptr));
	update_member(member_ptr);
	post_damage();
}

void canvas_overlay::after_finish(context& ctx) {

	if(is_visible())
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

bool canvas_overlay::ensure_layout(context& ctx) {

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

void canvas_overlay::begin_content(context& ctx, bool clear_frame_buffer) {

	frame_buffer_.enable(ctx);
	if(clear_frame_buffer) {
		ctx.push_bg_color();
		ctx.set_bg_color({ 0.0f, 0.0f, 0.0f, 1.0f });
		ctx.clear_background(true, false);
		ctx.pop_bg_color();
	}
}

void canvas_overlay::end_content(context& ctx, bool keep_damage) {

	frame_buffer_.disable(ctx);
	has_damage_ = keep_damage;
}

void canvas_overlay::draw_impl(context& ctx) {

	ctx.push_depth_test_state();
	ctx.disable_depth_test();

	ctx.push_blend_state();
	const context::BlendState blend_state = {
		true,
		BF_SRC_ALPHA,
		BF_ONE_MINUS_SRC_ALPHA,
		BF_ZERO,
		BF_ONE_MINUS_SRC_ALPHA
	};
	ctx.set_blend_state(blend_state);

	if(has_damage_)
		draw_content(ctx);

	// draw frame buffer texture to screen
	ctx.set_blend_func(BF_ONE, BF_SRC_ALPHA);

	overlay_canvas.enable_shader(ctx, "rectangle");
	overlay_canvas.set_style(ctx, blit_style_);
	frame_buffer_.enable_attachment(ctx, "color", 0);
	overlay_canvas.draw_shape(ctx, get_rectangle());
	frame_buffer_.disable_attachment(ctx, "color");
	overlay_canvas.disable_current_shader(ctx);

	ctx.pop_blend_state();
	ctx.pop_depth_test_state();
}

}
}
