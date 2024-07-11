#include "depth_masking.h"

#include <random>

namespace cgv {
namespace post {

void depth_masking::destruct(cgv::render::context& ctx) {

	fbc_depth_info.destruct(ctx);
	fbc_blurred_depth_ping.destruct(ctx);
	fbc_blurred_depth_pong.destruct(ctx);

	post_process_effect::destruct(ctx);
}

bool depth_masking::init(cgv::render::context& ctx) {

	fbc_draw.add_attachment("depth", "[D]");
	fbc_draw.add_attachment("color", "flt32[R,G,B,A]");

	fbc_depth_info.add_attachment("linear_depth", "flt32[R]");
	fbc_depth_info.add_attachment("mask", "int8[R]");

	fbc_blurred_depth_ping.add_attachment("blurred_depth", "flt32[R,G]");
	fbc_blurred_depth_pong.add_attachment("blurred_depth", "flt32[R,G]");

	shaders.add("depth_info_extraction", "depth_info_extraction.glpr");
	shaders.add(
		"box_blur_x",
		"weighted_box_blur_separable_x.glpr"
	);
	shaders.add(
		"box_blur_y",
		"weighted_box_blur_separable_y.glpr"
	);
	shaders.add("depth_masking", "depth_masking.glpr");

	return post_process_effect::init(ctx);
}

bool depth_masking::ensure(cgv::render::context& ctx) {

	fbc_depth_info.ensure(ctx);
	fbc_blurred_depth_ping.ensure(ctx);
	fbc_blurred_depth_pong.ensure(ctx);

	return post_process_effect::ensure(ctx);
}

void depth_masking::begin(cgv::render::context& ctx) {

	assert_init();

	if(!enable)
		return;

	fbc_draw.enable(ctx);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void depth_masking::end(cgv::render::context& ctx) {

	assert_init();

	if(!enable || !view)
		return;

	fbc_draw.disable(ctx);

	// extract linear depth and depth mask
	fbc_depth_info.enable(ctx);

	fbc_draw.enable_attachment(ctx, "depth", 0);
	
	auto& depth_info_extraction_prog = shaders.get("depth_info_extraction");
	depth_info_extraction_prog.enable(ctx);
	depth_info_extraction_prog.set_uniform(ctx, "z_near", static_cast<float>(view->get_z_near()));
	depth_info_extraction_prog.set_uniform(ctx, "z_far", static_cast<float>(view->get_z_far()));
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	depth_info_extraction_prog.disable(ctx);

	fbc_draw.disable_attachment(ctx, "depth");

	fbc_depth_info.disable(ctx);

	// blur depth values to create depth mask
	const auto gauss_boxes = [](unsigned n, unsigned r) {
		float rf = static_cast<float>(r);
		float nf = static_cast<float>(3);

		float w_ideal = std::sqrt((12.0f * rf * rf / nf) + 1.0f);
		int wl = static_cast<int>(std::floor(w_ideal));

		if(wl % 2 == 0)
			--wl;
		if(wl < 0)
			wl = 0;

		int wu = wl + 2;

		float m_ideal = (12.0f * rf * rf - nf * wl * wl - 4.0f * nf * wl - 3.0f * nf) / (-4.0f * wl - 4.0f);
		unsigned m = static_cast<int>(std::round(m_ideal));

		std::vector<unsigned> radii(n);
		for(unsigned i = 0; i < n; ++i) {
			int size = i < m ? wl : wu;
			radii[i] = static_cast<unsigned>((size - 1) / 2);
		}

		return radii;
	};

	float viewport_width = static_cast<float>(ctx.get_width());
	float viewport_height = static_cast<float>(ctx.get_height());
	float kernel_size_percentage = radius_percentage;
	float kernel_size = kernel_size_percentage * std::sqrt(viewport_width * viewport_width + viewport_height * viewport_height);
	int blur_radius = static_cast<int>(std::ceil(kernel_size / 2.0f));

	const unsigned iteration_count = cgv::math::clamp(blur_iterations, 1u, 5u);
	std::vector<unsigned> box_radii = gauss_boxes(iteration_count, blur_radius);

	if(blur_iterations == 1)
		box_radii.front() = blur_radius;

	auto& box_blur_x_prog = shaders.get("box_blur_x");
	auto& box_blur_y_prog = shaders.get("box_blur_y");

	fbc_depth_info.enable_attachment(ctx, "mask", 1);

	for(unsigned i = 0; i < iteration_count; ++i) {
		int radius = static_cast<int>(box_radii[i]);

		// filter along x-axis
		fbc_blurred_depth_ping.enable(ctx);

		if(i == 0)
			// use given input texture
			fbc_depth_info.enable_attachment(ctx, "linear_depth", 0);
		else
			// use output texture from previous iteration as input
			fbc_blurred_depth_pong.enable_attachment(ctx, "blurred_depth", 0);

		box_blur_x_prog.enable(ctx);
		box_blur_x_prog.set_uniform(ctx, "radius", radius);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		box_blur_x_prog.disable(ctx);

		if(i == 0)
			fbc_depth_info.disable_attachment(ctx, "linear_depth");
		else
			fbc_blurred_depth_pong.disable_attachment(ctx, "blurred_depth");

		fbc_blurred_depth_ping.disable(ctx);

		// filter along y-axis
		fbc_blurred_depth_pong.enable(ctx);

		fbc_blurred_depth_ping.enable_attachment(ctx, "blurred_depth", 0);

		box_blur_y_prog.enable(ctx);
		box_blur_y_prog.set_uniform(ctx, "radius", static_cast<int>(box_radii[0]));

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		box_blur_y_prog.disable(ctx);

		fbc_blurred_depth_ping.disable_attachment(ctx, "blurred_depth");

		fbc_blurred_depth_pong.disable(ctx);
	}

	fbc_depth_info.disable_attachment(ctx, "mask");

	// combine to final image
	fbc_draw.enable_attachment(ctx, "color", 0);
	fbc_draw.enable_attachment(ctx, "depth", 1);
	fbc_depth_info.enable_attachment(ctx, "linear_depth", 2);
	fbc_blurred_depth_pong.enable_attachment(ctx, "blurred_depth", 3);

	auto& depth_masking_prog = shaders.get("depth_masking");
	depth_masking_prog.enable(ctx);
	
	depth_masking_prog.set_uniform(ctx, "mode", static_cast<int>(mode));
	depth_masking_prog.set_uniform(ctx, "strength_scale", strength);
	depth_masking_prog.set_uniform(ctx, "inside_color", inside_color);
	depth_masking_prog.set_uniform(ctx, "outside_color", outside_color);
	depth_masking_prog.set_uniform(ctx, "tint_colors", tint_colors);
	depth_masking_prog.set_uniform(ctx, "clamp_output", clamp_output);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	depth_masking_prog.disable(ctx);

	fbc_draw.disable_attachment(ctx, "color");
	fbc_draw.disable_attachment(ctx, "depth");
	fbc_depth_info.disable_attachment(ctx, "linear_depth");
	fbc_blurred_depth_pong.disable_attachment(ctx, "blurred_depth");
}

void depth_masking::create_gui_impl(cgv::base::base* b, cgv::gui::provider* p) {

	post_process_effect::create_gui_impl(b, p);
	p->add_member_control(b, "Mode", mode, "dropdown", "enums='Inside,Outside,Center,In- & Outside'");
	p->add_member_control(b, "Strength", strength, "value_slider", "min=-2;step=0.001;max=2");
	p->add_member_control(b, "Radius %", radius_percentage, "value_slider", "min=0;max=0.05;step=0.001");
	p->add_member_control(b, "Quality", blur_iterations, "value_slider", "min=1;max=5;step=1");
	p->add_member_control(b, "Tint", tint_colors, "check");
	p->add_member_control(b, "Inside Color", inside_color);
	p->add_member_control(b, "Outside Color", outside_color);
	p->add_member_control(b, "Clamp Output", clamp_output, "check");
}

}
}
