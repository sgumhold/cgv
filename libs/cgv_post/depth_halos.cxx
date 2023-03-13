#pragma once

#include "depth_halos.h"

#include <random>

namespace cgv {
namespace post {

void depth_halos::destruct(cgv::render::context& ctx) {

	post_process_effect::destruct(ctx);
}

bool depth_halos::init(cgv::render::context& ctx) {

	fbc_draw.add_attachment("depth", "[D]");
	fbc_draw.add_attachment("color", "flt32[R,G,B,A]");

	shaders.add("depth_halo", "depth_halo.glpr");
	
	generate_noise_texture(ctx);

	return post_process_effect::init(ctx);
}

bool depth_halos::ensure(cgv::render::context& ctx) {

	return post_process_effect::ensure(ctx);
}

void depth_halos::begin(cgv::render::context& ctx) {

	assert_init();

	if(!enable)
		return;

	fbc_draw.enable(ctx);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void depth_halos::end(cgv::render::context& ctx) {

	assert_init();

	if(!enable)
		return;

	fbc_draw.disable(ctx);

	fbc_draw.enable_attachment(ctx, "depth", 0);
	fbc_draw.enable_attachment(ctx, "color", 1);
	noise_tex.enable(ctx, 2);

	auto& depth_halo_prog = shaders.get("depth_halo");
	depth_halo_prog.enable(ctx);
	depth_halo_prog.set_uniform(ctx, "strength", strength);
	depth_halo_prog.set_uniform(ctx, "radius", radius);
	depth_halo_prog.set_uniform(ctx, "threshold", threshold);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	depth_halo_prog.disable(ctx);

	fbc_draw.disable_attachment(ctx, "depth");
	fbc_draw.disable_attachment(ctx, "color");
	noise_tex.disable(ctx);
}

void depth_halos::create_gui(cgv::gui::provider* p) {
	cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

	post_process_effect::create_gui(p);
	p->add_member_control(b, "Strength", strength, "value_slider", "min=0;step=0.001;max=5");
	p->add_member_control(b, "Radius", radius, "value_slider", "min=0;step=0.001;max=20");
	p->add_member_control(b, "Threshold", threshold, "value_slider", "min=0;step=0.001;max=1");
}

void depth_halos::generate_noise_texture(cgv::render::context& ctx) {

	std::default_random_engine rng;
	std::uniform_real_distribution<float> distr(-1.0, 1.0);

	std::vector<vec3> offsets;
	for(unsigned i = 0; i < 64; ++i) {
		offsets.push_back(vec3(
			distr(rng),
			distr(rng),
			0.0f
		));
	}

	cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(8, 8, TI_FLT32, cgv::data::CF_RGB), offsets.data());
	noise_tex.create(ctx, dv, 0);
	noise_tex.set_min_filter(cgv::render::TF_NEAREST);
	noise_tex.set_mag_filter(cgv::render::TF_NEAREST);
	noise_tex.set_wrap_s(cgv::render::TW_REPEAT);
	noise_tex.set_wrap_t(cgv::render::TW_REPEAT);
}

}
}
