#pragma once

#include <cgv/media/color_scale.h>
#include "shader_program.h"

#include "lib_begin.h"

namespace cgv {
	namespace render {

		/// <summary>
		/// convenience function to configure a shader that uses color_scale.glsl with a single color scale
		/// </summary>
		/// <param name="ctx">render context</param>
		/// <param name="prog">shader program using the color scale</param>
		/// <param name="cs">color scale index that can surpass cgv::media::CS_NAMED to index named color scales</param>
		/// <param name="window_zero_position">window_zero_position which is mapped to center of a bipolar color scale</param>
		extern CGV_API void configure_color_scale(
			cgv::render::context& ctx, cgv::render::shader_program& prog,
			cgv::media::ColorScale cs, float window_zero_position = 0.5f);

		/// <summary>
		/// convenience function to configure a shader that uses color_scale.glsl with two color scales
		/// </summary>
		/// <param name="ctx">render context</param>
		/// <param name="prog">shader program using the color scales</param>
		/// <param name="cs">color scale indices that can surpass cgv::media::CS_NAMED to index named color scales</param>
		/// <param name="window_zero_position">window space positions of attribute value zero which is mapped to center of a bipolar color scale</param>
		extern CGV_API void configure_color_scale(
			cgv::render::context& ctx, cgv::render::shader_program& prog,
			cgv::media::ColorScale cs[2], float window_zero_position[2]);

	}
}

#include <cgv/config/lib_end.h>