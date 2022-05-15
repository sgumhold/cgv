#pragma once

#include <cgv/render/render_types.h>
#include <cgv/render/clipped_view.h>

namespace stream_vis {

	/// store information of joint offsets
	struct view_info : public cgv::render::render_types
	{
		/// name of offset
		std::string name;
		vec2 stretch = vec2(1.0f);
		vec2 offset = vec2(0.0f);
		char toggle = 0;
		/// dimension of view
		int dim;
		cgv::render::clipped_view current_view;
		cgv::render::view default_view;
	};
}
