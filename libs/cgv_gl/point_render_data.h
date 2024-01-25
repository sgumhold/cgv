#pragma once

#include "point_renderer.h"
#include "render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for point geometry with support for the point_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class point_render_data : public render_data_base<ColorType> {
public:
	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

	/// stores an array of diameters
	std::vector<float> diameters;

protected:
	/// @brief See render_data_base::transfer.
	bool transfer(context& ctx, point_renderer& r) {
		if(super::transfer(ctx, r)) {
			if(diameters.size() == super::size())
				r.set_point_size_array(ctx, diameters);
			return true;
		}
		return false;
	}

public:
	void clear() {
		super::clear();
		diameters.clear();
	}

	void add_diameter(const float diameter) {
		diameters.push_back(diameter);
	}

	// Explicitly use add from the base class since it is shadowed by the overloaded versions
	using super::add;

	void add(const vec3& position, const float diameter) {
		super::add_position(position);
		add_diameter(diameter);
	}

	void add(const vec3& position, const ColorType& color, const float diameter) {
		super::add(position, color);
		add_diameter(diameter);
	}

	void fill_diameters(const float diameter) {
		super::fill(diameters, diameter);
	}

	RDB_BASE_FUNC_DEF(point_renderer, point_render_style);
};

}
}
