#pragma once

#include "box_wire_renderer.h"
#include "box_render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for box geometry with support for the box_wire_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class box_wire_render_data : public box_render_data_base<box_wire_renderer, box_wire_render_style, ColorType> {
private:
	box_wire_renderer& ref_renderer_singleton(context& ctx, int ref_count_change = 0) override {
		return ref_box_wire_renderer(ctx, ref_count_change);
	}
};

}
}
