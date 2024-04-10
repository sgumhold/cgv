#pragma once

#include <cgv/render/clipped_view.h>

namespace stream_vis {

	/// store information of joint offsets
	struct view_info
	{
		/// name of offset
		std::string name;
		cgv::vec2 stretch = cgv::vec2(1.0f);
		cgv::vec2 offset = cgv::vec2(0.0f);
		char toggle = 0;
		/// dimension of view
		int dim;
		cgv::render::clipped_view current_view;
		cgv::render::view default_view;
	};
}
