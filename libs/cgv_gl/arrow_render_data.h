#pragma once

#include "arrow_renderer.h"
#include "render_data_base.h"

namespace cgv {
namespace render {

template <typename ColorType = rgb>
class arrow_render_data : public render_data_base<ColorType> {
public:
	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

	bool direction_is_endpoint = false;
	std::vector<vec3> directions;

protected:
	bool transfer(context& ctx, arrow_renderer& r) {
		if(super::transfer(ctx, r)) {
			if(directions.size() == super::size()) {
				if(direction_is_endpoint)
					r.set_end_point_array(ctx, directions);
				else
					r.set_direction_array(ctx, directions);
			}
			return true;
		}
		return false;
	}

public:
	void clear() {
		super::clear();
		directions.clear();
	}

	void add_direction(const vec3& direction) {
		directions.push_back(direction);
	}

	// Explicitly use add from the base class since it is shadowed by the overloaded versions
	using super::add;

	void add(const vec3& position, const vec3& direction) {
		super::add_position(position);
		add_direction(direction);
	}

	void add(const vec3& position, const ColorType& color, const vec3& direction) {
		super::add(position, color);
		add_direction(direction);
	}

	void fill_directions(const vec3& direction) {
		super::fill(directions, direction);
	}

	RDB_BASE_FUNC_DEF(arrow_renderer, arrow_render_style);
};

}
}
