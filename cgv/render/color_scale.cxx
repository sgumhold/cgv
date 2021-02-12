#include "color_scale.h"

namespace cgv {
	namespace render {

void configure_color_scale(cgv::render::context& ctx, cgv::render::shader_program& prog, cgv::media::ColorScale cs, float window_zero_position)
{
	bool is_bipolar = false;
	if (cs >= cgv::media::CS_NAMED) {
		const auto& scs = cgv::media::query_named_color_scale(cgv::media::query_color_scale_names()[cs - cgv::media::CS_NAMED], &is_bipolar);
		prog.set_uniform(ctx, "nr_color_scale_samples", (int)scs.size());
		prog.set_uniform_array(ctx, "color_scale_samples", scs);
	}
	prog.set_uniform(ctx, "color_scale_index", (int)std::min(cs, cgv::media::CS_NAMED));
	prog.set_uniform(ctx, "color_scale_is_bipolar", is_bipolar);
	if (is_bipolar)
		prog.set_uniform(ctx, "window_zero_position", window_zero_position);
}

	}
}
