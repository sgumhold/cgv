#include "color_scale.h"

namespace cgv {
	namespace render {

void configure_color_scale(cgv::render::context& ctx, cgv::render::shader_program& prog, cgv::media::ColorScale cs, float window_zero_position)
{
	bool is_bipolar = false;
	if (cs >= cgv::media::CS_NAMED) {
		const auto& scs = cgv::media::query_named_color_scale(cgv::media::query_color_scale_names()[cs - cgv::media::CS_NAMED], &is_bipolar);
		prog.set_uniform(ctx, "nr_color_scale_samples[0]", (int)scs.size());
		prog.set_uniform_array(ctx, "color_scale_samples", scs);
	}
	prog.set_uniform(ctx, "color_scale_index[0]", (int)std::min(cs, cgv::media::CS_NAMED));
	prog.set_uniform(ctx, "color_scale_is_bipolar[0]", is_bipolar);
	if (is_bipolar)
		prog.set_uniform(ctx, "window_zero_position[0]", window_zero_position);
}

void configure_color_scale(
	cgv::render::context& ctx, cgv::render::shader_program& prog,
	cgv::media::ColorScale cs[2], float window_zero_position[2])
{
	const std::vector<rgb>* scs_ptrs[2] = { 0, 0 };
	int nr_color_scale_samples[2] = { 0, 0 };
	int color_scale_index[2];
	int color_scale_is_bipolar[2];
	for (unsigned idx = 0; idx < 2; ++idx) {
		color_scale_is_bipolar[idx] = 0;
		color_scale_index[idx] = (int)std::min(cs[idx], cgv::media::CS_NAMED);
		if (cs[idx] >= cgv::media::CS_NAMED) {
			bool is_bipolar;
			scs_ptrs[idx] = &cgv::media::query_named_color_scale(cgv::media::query_color_scale_names()[cs[idx] - cgv::media::CS_NAMED], &is_bipolar);
			color_scale_is_bipolar[idx] = is_bipolar ? 1 : 0;
			nr_color_scale_samples[idx] = (int)scs_ptrs[idx]->size();
		}
	}
	prog.set_uniform_array(ctx, "nr_color_scale_samples", nr_color_scale_samples, 2);
	std::vector<rgb> colors(32 + nr_color_scale_samples[1]);
	if (scs_ptrs[0])
		std::copy(scs_ptrs[0]->begin(), scs_ptrs[0]->end(), colors.begin());
	if (scs_ptrs[1])
		std::copy(scs_ptrs[1]->begin(), scs_ptrs[1]->end(), colors.begin()+32);
	prog.set_uniform_array(ctx, "color_scale_samples", colors);
	prog.set_uniform_array(ctx, "color_scale_index", color_scale_index, 2);
	prog.set_uniform_array(ctx, "color_scale_is_bipolar", color_scale_is_bipolar, 2);
	prog.set_uniform_array(ctx, "window_zero_position", window_zero_position, 2);
}


	}
}
