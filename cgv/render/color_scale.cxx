#include "color_scale.h"

namespace cgv {
	namespace render {

void configure_color_scale(cgv::render::context& ctx, cgv::render::shader_program& prog, cgv::media::ColorScale cs)
{
	prog.set_uniform(ctx, "color_scale_index", (int)std::min(cs, cgv::media::CS_NAMED));
	if (cs >= cgv::media::CS_NAMED) {
		const auto& scs = cgv::media::query_named_color_scale(cgv::media::query_color_scale_names()[cs - cgv::media::CS_NAMED]);
		prog.set_uniform(ctx, "nr_color_scale_samples", (int)scs.size());
		prog.set_uniform_array(ctx, "color_scale_samples", scs);
	}
}

	}
}
