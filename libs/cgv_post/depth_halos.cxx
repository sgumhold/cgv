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

	shaders.add("depth_halo", "depth_halo.glpr", get_shader_defines());

	generate_noise_texture(ctx);

	return post_process_effect::init(ctx);
}

bool depth_halos::ensure(cgv::render::context& ctx) {

	if(do_reload_shader) {
		do_reload_shader = false;
		shaders.reload(ctx, "depth_halo", get_shader_defines());
	}

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
	depth_halo_prog.set_uniform(ctx, "depth_scale", depth_scale);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	depth_halo_prog.disable(ctx);

	fbc_draw.disable_attachment(ctx, "depth");
	fbc_draw.disable_attachment(ctx, "color");
	noise_tex.disable(ctx);
}

void depth_halos::create_gui_impl(cgv::base::base* b, cgv::gui::provider* p) {

	post_process_effect::create_gui_impl(b, p);
	connect_copy(
		p->add_member_control(b, "Mode", mode, "dropdown", "enums='Inside,Outside;Center'")->value_change,
		cgv::signal::rebind(this, &depth_halos::on_change_mode));
	p->add_member_control(b, "Strength", strength, "value_slider", "min=0;step=0.001;max=5");
	p->add_member_control(b, "Radius", radius, "value_slider", "min=0;step=0.001;max=20");
	p->add_member_control(b, "Threshold", threshold, "value_slider", "min=0;step=0.001;max=1");
	p->add_member_control(b, "Depth Scale", depth_scale, "value_slider", "min=0.1;step=0.001;max=100;log=true;ticks=true");
}

cgv::render::shader_define_map depth_halos::get_shader_defines() {

	cgv::render::shader_define_map defines;
	cgv::render::shader_code::set_define(defines, "MODE", mode, Mode::Outside);
	return defines;
}

void depth_halos::on_change_mode() {

	do_reload_shader = true;
}

void depth_halos::generate_noise_texture(cgv::render::context& ctx) {

	std::default_random_engine rng;
	std::uniform_real_distribution<float> distr(-0.5, 0.5);

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
