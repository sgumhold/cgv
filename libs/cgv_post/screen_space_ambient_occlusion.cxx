#include "screen_space_ambient_occlusion.h"

#include <random>

namespace cgv {
namespace post {

void screen_space_ambient_occlusion::destruct(cgv::render::context& ctx) {

	fbc_post.destruct(ctx);
	fbc_blur.destruct(ctx);
	noise_tex.destruct(ctx);

	post_process_effect::destruct(ctx);
}

bool screen_space_ambient_occlusion::init(cgv::render::context& ctx) {

	fbc_draw.add_attachment("depth", "[D]");
	fbc_draw.add_attachment("color", "flt32[R,G,B,A]");
	fbc_draw.add_attachment("position", "flt32[R,G,B,A]");
	fbc_draw.add_attachment("normal", "flt32[R,G,B]");

	fbc_post.add_attachment("occlusion", "flt32[R]");

	fbc_blur.add_attachment("occlusion", "flt32[R]");

	shaders.add("ssao", "ssao.glpr");
	shaders.add("ssao_resolve", "ssao_resolve.glpr");
	shaders.add("blur", "box_blur.glpr", { { "CHANNELS", "1" }, { "RADIUS", "2" } });

	generate_samples_and_noise_texture(ctx);
	
	return post_process_effect::init(ctx);
}

bool screen_space_ambient_occlusion::ensure(cgv::render::context& ctx) {

	fbc_post.ensure(ctx);
	fbc_blur.ensure(ctx);
	return post_process_effect::ensure(ctx);
}

void screen_space_ambient_occlusion::begin(cgv::render::context& ctx) {

	assert_init();

	if(!enable)
		return;

	fbc_draw.enable(ctx);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void screen_space_ambient_occlusion::end(cgv::render::context& ctx) {

	assert_init();

	if(!enable)
		return;

	fbc_draw.disable(ctx);

	// use position, normal and depth information to compute the ambient occlussion term in screen space
	fbc_post.enable(ctx);

	fbc_draw.enable_attachment(ctx, "depth", 3);
	fbc_draw.enable_attachment(ctx, "position", 0);
	fbc_draw.enable_attachment(ctx, "normal", 1);
	noise_tex.enable(ctx, 2);

	auto& ssao_prog = shaders.get("ssao");
	ssao_prog.enable(ctx);
	ssao_prog.set_uniform(ctx, "viewport_size", vec2(viewport_size));
	ssao_prog.set_uniform(ctx, "radius", radius);
	ssao_prog.set_uniform(ctx, "bias", bias);
	ssao_prog.set_uniform_array(ctx, "samples", sample_offsets);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	ssao_prog.disable(ctx);

	fbc_draw.disable_attachment(ctx, "position");
	fbc_draw.disable_attachment(ctx, "normal");
	noise_tex.disable(ctx);
	fbc_draw.disable_attachment(ctx, "depth");

	fbc_post.disable(ctx);

	// blur the occlusion term
	fbc_blur.enable(ctx);

	fbc_post.enable_attachment(ctx, "occlusion", 0);

	auto& blur_prog = shaders.get("blur");
	blur_prog.enable(ctx);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	blur_prog.disable(ctx);

	fbc_post.disable_attachment(ctx, "occlusion");

	fbc_blur.disable(ctx);

	// composit the original color with the occlusion term
	fbc_draw.enable_attachment(ctx, "depth", 0);
	fbc_draw.enable_attachment(ctx, "color", 1);
	fbc_blur.enable_attachment(ctx, "occlusion", 2);

	auto& resolve_prog = shaders.get("ssao_resolve");
	resolve_prog.enable(ctx);
	resolve_prog.set_uniform(ctx, "strength", strength);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	resolve_prog.disable(ctx);

	fbc_draw.disable_attachment(ctx, "depth");
	fbc_draw.disable_attachment(ctx, "color");
	fbc_blur.disable_attachment(ctx, "occlusion");
}

void screen_space_ambient_occlusion::create_gui_impl(cgv::base::base* b, cgv::gui::provider* p) {

	post_process_effect::create_gui_impl(b, p);
	p->add_member_control(b, "Strength", strength, "value_slider", "min=0;step=0.001;max=5");
	p->add_member_control(b, "Radius", radius, "value_slider", "min=0;step=0.001;max=2");
	p->add_member_control(b, "Bias", bias, "value_slider", "min=0;step=0.001;max=0.1;log=true;ticks=true");
}

void screen_space_ambient_occlusion::generate_samples_and_noise_texture(cgv::render::context& ctx) {

	std::default_random_engine rng;
	std::uniform_real_distribution<float> distr(0.0, 1.0);

	for(unsigned i = 0; i < 64; ++i) {
		vec3 sample(
			distr(rng) * 2.0f - 1.0f,
			distr(rng) * 2.0f - 1.0f,
			distr(rng)
		);

		sample.normalize();
		sample *= distr(rng);

		float scale = static_cast<float>(i) / 64.0f;
		scale = cgv::math::lerp(0.1f, 1.0f, scale*scale);
		sample *= scale;

		sample_offsets.push_back(sample);
	}

	std::vector<vec3> random_noise;
	for(unsigned i = 0; i < 16; ++i) {
		vec3 noise(
			distr(rng) * 2.0f - 1.0f,
			distr(rng) * 2.0f - 1.0f,
			0.0f
		);
		random_noise.push_back(noise);
	}

	cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(4, 4, TI_FLT32, cgv::data::CF_RGB), random_noise.data());
	noise_tex.create(ctx, dv, 0);
	noise_tex.set_min_filter(cgv::render::TF_NEAREST);
	noise_tex.set_mag_filter(cgv::render::TF_NEAREST);
	noise_tex.set_wrap_s(cgv::render::TW_REPEAT);
	noise_tex.set_wrap_t(cgv::render::TW_REPEAT);
}

}
}
