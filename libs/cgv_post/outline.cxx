#include "outline.h"

#include <random>

namespace cgv {
namespace post {

void outline::destruct(cgv::render::context& ctx) {

	post_process_effect::destruct(ctx);
}

bool outline::init(cgv::render::context& ctx) {

	fbc_draw.add_attachment("depth", "[D]");
	fbc_draw.add_attachment("color", "flt32[R,G,B,A]");

	shaders.add("outline", "outline.glpr");

	return post_process_effect::init(ctx);
}

bool outline::ensure(cgv::render::context& ctx) {

	return post_process_effect::ensure(ctx);
}

void outline::begin(cgv::render::context& ctx) {

	assert_init();

	if(!enable)
		return;

	fbc_draw.enable(ctx);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void outline::end(cgv::render::context& ctx) {

	assert_init();

	if(!enable)
		return;

	fbc_draw.disable(ctx);

	fbc_draw.enable_attachment(ctx, "depth", 0);
	fbc_draw.enable_attachment(ctx, "color", 1);

	auto& depth_halo_prog = shaders.get("outline");
	depth_halo_prog.enable(ctx);
	depth_halo_prog.set_uniform(ctx, "strength", strength);
	depth_halo_prog.set_uniform(ctx, "threshold", threshold);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	depth_halo_prog.disable(ctx);

	fbc_draw.disable_attachment(ctx, "depth");
	fbc_draw.disable_attachment(ctx, "color");
}

void outline::create_gui_impl(cgv::base::base* b, cgv::gui::provider* p) {

	post_process_effect::create_gui_impl(b, p);
	p->add_member_control(b, "Strength", strength, "value_slider", "min=0;step=0.001;max=5");
	p->add_member_control(b, "Threshold", threshold, "value_slider", "min=0;step=0.001;max=1");
}

}
}
