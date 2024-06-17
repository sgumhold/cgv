#pragma once

#include "box_renderer.h"
#include "box_render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for box geometry with support for the box_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class box_render_data : public box_render_data_base<box_renderer, box_render_style, ColorType> {
private:
	box_renderer& ref_renderer_singleton(context& ctx, int ref_count_change = 0) override {
		return ref_box_renderer(ctx, ref_count_change);
	}
};

}
}
