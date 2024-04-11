#include "temporal_anti_aliasing.h"

#include <cgv/render/view.h>

namespace cgv {
namespace post {

void temporal_anti_aliasing::destruct(cgv::render::context& ctx) {

	fbc_post.destruct(ctx);
	fbc_hist.destruct(ctx);
	fbc_resolve.destruct(ctx);

	view_ptr = nullptr;

	post_process_effect::destruct(ctx);
}

bool temporal_anti_aliasing::init(cgv::render::context& ctx) {

	const std::string color_format = "flt32[R,G,B,A]";

	fbc_draw.add_attachment("depth", "[D]");
	fbc_draw.add_attachment("color", color_format, cgv::render::TF_LINEAR);

	fbc_post.add_attachment("color", color_format);

	fbc_hist.add_attachment("color", color_format, cgv::render::TF_LINEAR);

	fbc_resolve.add_attachment("color", color_format);
	
	shaders.add("screen", "screen_texture.glpr");
	shaders.add("resolve", "taa_resolve.glpr");
	shaders.add("fxaa", "fxaa3.glpr");

	post_process_effect::init(ctx);

	if(is_initialized) {
		auto& screen_prog = shaders.get("screen");
		screen_prog.enable(ctx);
		screen_prog.set_uniform(ctx, "color_tex", 0);
		screen_prog.set_uniform(ctx, "depth_tex", 1);
		screen_prog.disable(ctx);
	}

	return is_initialized;
}

bool temporal_anti_aliasing::ensure(cgv::render::context& ctx) {

	fbc_post.ensure(ctx);
	fbc_hist.ensure(ctx);
	fbc_resolve.ensure(ctx);

	if(post_process_effect::ensure(ctx) || jitter_sample_count != jitter_offsets.size()) {
		generate_jitter_offsets();
		return true;
	}

	return false;
}

void temporal_anti_aliasing::reset() {
	accumulate = false;
	accumulate_count = 0;
	static_frame_count = 0;
}

void temporal_anti_aliasing::reset_static_frame_count() {
	static_frame_count = 0;
}

void temporal_anti_aliasing::begin(cgv::render::context& ctx) {

	assert_init();
	
	if(!(enable || enable_fxaa) || !view_ptr)
		return;

	current_view.eye_pos = view_ptr->get_eye();
	current_view.view_dir = view_ptr->get_view_dir();
	current_view.view_up_dir = view_ptr->get_view_up_dir();

	// Get the projection matrix before applying jitter and get the view matrix.
	// Without any model transformations the modelview matrix contains the view matrix only.
	// This assumes no changes have been made to the modelview matrix before this call.
	current_view.view_projection_matrix = ctx.get_projection_matrix() * ctx.get_modelview_matrix();

	ctx.push_projection_matrix();

	if(enable && accumulate) {
		dmat4 m;
		m.identity();

		vec2 jitter_offset = get_current_jitter_offset();
		m(0, 2) = jitter_offset.x();
		m(1, 2) = jitter_offset.y();

		ctx.mul_projection_matrix(m);
	}

	fbc_draw.enable(ctx);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void temporal_anti_aliasing::end(cgv::render::context& ctx) {

	assert_init();

	if(!(enable || enable_fxaa) || !view_ptr)
		return;

	fbc_draw.disable(ctx);

	ctx.pop_projection_matrix();

	if(enable_fxaa) {
		fbc_post.enable(ctx);

		auto& fxaa_prog = shaders.get("fxaa");
		fxaa_prog.enable(ctx);
		fxaa_prog.set_uniform(ctx, "inverse_viewport_size", vec2(1.0f) / viewport_size);
		fxaa_prog.set_uniform(ctx, "mix_factor", fxaa_mix_factor);

		fbc_draw.enable_attachment(ctx, "color", 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		fbc_draw.disable_attachment(ctx, "color");

		fxaa_prog.disable(ctx);

		fbc_post.disable(ctx);
	}

	fbc_resolve.enable(ctx);
	glDepthFunc(GL_ALWAYS);

	bool first = !accumulate;
	if(accumulate) {
		bool is_static = true;
		if(!is_static_view()) {
			is_static = false;
			static_frame_count = 0;
		}

		mat4 curr_clip_to_prev_clip_matrix = previous_view.view_projection_matrix * inv(current_view.view_projection_matrix);

		auto& resolve_prog = shaders.get("resolve");
		resolve_prog.enable(ctx);

		resolve_prog.set_uniform(ctx, "viewport_size", viewport_size);
		resolve_prog.set_uniform(ctx, "curr_clip_to_prev_clip_matrix", curr_clip_to_prev_clip_matrix);
		resolve_prog.set_uniform(ctx, "is_static", is_static);
		resolve_prog.set_uniform(ctx, "curr_mix_factor", mix_factor);
		resolve_prog.set_uniform(ctx, "use_velocity", use_velocity);

		auto& color_src_fbc = enable_fxaa ? fbc_post : fbc_draw;

		color_src_fbc.enable_attachment(ctx, "color", 0);
		fbc_hist.enable_attachment(ctx, "color", 1);
		fbc_draw.enable_attachment(ctx, "depth", 2);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		color_src_fbc.disable_attachment(ctx, "color");
		fbc_hist.disable_attachment(ctx, "color");
		fbc_draw.disable_attachment(ctx, "depth");

		resolve_prog.disable(ctx);

		++accumulate_count;
		if(accumulate_count > jitter_sample_count - 1)
			accumulate_count = 0;

		if(static_frame_count < jitter_sample_count) {
			++static_frame_count;
			ctx.post_redraw();
		}
	} else {
		accumulate = enable;
		accumulate_count = 0;
		if(accumulate)
			ctx.post_redraw();
	}

	fbc_resolve.disable(ctx);

	auto& color_src_fbc = first ? (enable_fxaa ? fbc_post : fbc_draw) : fbc_resolve;

	auto& screen_prog = shaders.get("screen");
	screen_prog.enable(ctx);
	
	color_src_fbc.enable_attachment(ctx, "color", 0);
	fbc_draw.enable_attachment(ctx, "depth", 1);

	fbc_hist.enable(ctx);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	fbc_hist.disable(ctx);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	color_src_fbc.disable_attachment(ctx, "color");
	fbc_draw.disable_attachment(ctx, "depth");

	screen_prog.disable(ctx);

	glDepthFunc(GL_LESS);

	previous_view = current_view;
	return;
}

void temporal_anti_aliasing::create_gui_impl(cgv::base::base* b, cgv::gui::provider* p) {
	
	post_process_effect::create_gui_impl(b, p);
	
	p->add_member_control(b, "Jitter Samples", jitter_sample_count, "value_slider", "min=1;max=32;step=1");
	p->add_member_control(b, "Jitter Scale", jitter_scale, "value_slider", "min=0;max=2;step=0.0001");
	p->add_member_control(b, "Mix Factor", mix_factor, "value_slider", "min=0;max=1;step=0.0001");
	p->add_member_control(b, "Use Velocity", use_velocity, "check");

	p->add_member_control(b, "FXAA", enable_fxaa, "toggle");
	p->add_member_control(b, "Mix Factor", fxaa_mix_factor, "value_slider", "min=0;max=1;step=0.0001");
}

float temporal_anti_aliasing::van_der_corput(int n, int base) const {
	float vdc = 0.0f;
	int denominator = 1;

	while(n > 0) {
		denominator *= base;
		int remainder = n % base;
		n /= base;
		vdc += remainder / static_cast<float>(denominator);
	}

	return vdc;
}

vec2 temporal_anti_aliasing::sample_halton_2d(unsigned k, int base1, int base2) const {
	return vec2(van_der_corput(k, base1), van_der_corput(k, base2));
}

void temporal_anti_aliasing::generate_jitter_offsets() {
	jitter_offsets.clear();

	if(jitter_sample_count < 1)
		jitter_sample_count = 1;
	else if(jitter_sample_count > 128)
		jitter_sample_count = 128;

	for(size_t i = 0; i < jitter_sample_count; ++i) {
		vec2 sample = sample_halton_2d(static_cast<unsigned>(i + 1), 2, 3);
		vec2 offset = (2.0f * sample - 1.0f) / viewport_size;
		jitter_offsets.push_back(offset);
	}
}

vec2 temporal_anti_aliasing::get_current_jitter_offset() const {
	return jitter_scale * jitter_offsets[accumulate_count];
}

bool temporal_anti_aliasing::is_static_view() const {
	return
		previous_view.eye_pos == current_view.eye_pos &&
		previous_view.view_dir == current_view.view_dir &&
		previous_view.view_up_dir == current_view.view_up_dir;
}

}
}
