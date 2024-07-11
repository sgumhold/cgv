#include "supersampling_anti_aliasing.h"

namespace cgv {
namespace post {

void supersampling_anti_aliasing::destruct(cgv::render::context& ctx) {

	post_process_effect::destruct(ctx);
}

bool supersampling_anti_aliasing::init(cgv::render::context& ctx) {

	fbc_draw.add_attachment("depth", "[D]");
	fbc_draw.add_attachment("color", "flt32[R,G,B,A]");

	shaders.add("ssaa", "ssaa.glpr");

	return post_process_effect::init(ctx);
}

bool supersampling_anti_aliasing::ensure(cgv::render::context& ctx) {

	multiplier = cgv::math::clamp(multiplier, 1, 4);
	ensure_fbc_size(ctx);
	return post_process_effect::ensure(ctx);
}

void supersampling_anti_aliasing::begin(cgv::render::context& ctx) {

	assert_init();

	if(!enable)
		return;

	fbc_draw.enable(ctx);
	ctx.clear_background(true, true);
}

void supersampling_anti_aliasing::end(cgv::render::context& ctx) {

	assert_init();

	if(!enable)
		return;

	fbc_draw.disable(ctx);

	fbc_draw.enable_attachment(ctx, "color", 0);
	fbc_draw.enable_attachment(ctx, "depth", 1);
	
	auto& ssaa_prog = shaders.get("ssaa");
	ssaa_prog.enable(ctx);
	
	ssaa_prog.set_uniform(ctx, "viewport_size", viewport_size);
	ssaa_prog.set_uniform(ctx, "multiplier", multiplier);
	
	ctx.push_depth_test_state();
	ctx.set_depth_func(cgv::render::CompareFunction::CF_ALWAYS);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	ctx.pop_depth_test_state();

	ssaa_prog.disable(ctx);

	fbc_draw.disable_attachment(ctx, "color");
	fbc_draw.disable_attachment(ctx, "depth");
}

void supersampling_anti_aliasing::create_gui_impl(cgv::base::base* b, cgv::gui::provider* p) {

	post_process_effect::create_gui_impl(b, p);
	p->add_member_control(b, "Resolution Multiplier", multiplier, "value_slider", "min=1;step=1;max=4");
}

void supersampling_anti_aliasing::ensure_fbc_size(cgv::render::context& ctx) {

	cgv::ivec2 screen_size(
		static_cast<int>(ctx.get_width()),
		static_cast<int>(ctx.get_height())
	);

	screen_size *= multiplier;

	if(screen_size != last_screen_size) {
		fbc_draw.set_size(screen_size);
		last_screen_size = screen_size;
	}
}

}
}
