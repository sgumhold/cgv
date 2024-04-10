#pragma once

#include "surfel_renderer.h"
#include "render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for surfel geometry with support for the surfel_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class surfel_render_data : public render_data_base<ColorType> {
public:
	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

	/// stores an array of normals
	std::vector<vec3> normals;
	/// stores an array of diameters
	std::vector<float> diameters;
	
protected:
	/// @brief See render_data_base::transfer.
	bool transfer(context& ctx, surfel_renderer& r) {
		if(super::transfer(ctx, r)) {
			if(normals.size() == super::size())
				r.set_normal_array(ctx, normals);
			if(diameters.size() == super::size())
				r.set_point_size_array(ctx, diameters);
			return true;
		}
		return false;
	}

public:
	void clear() {
		super::clear();
		normals.clear();
		diameters.clear();
	}

	void add_normal(const vec3& normal) {
		normals.push_back(normal);
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

	void add(const vec3& position, const vec3& normal) {
		super::add_position(position);
		add_normal(normal);
	}

	void add(const vec3& position, const vec3& normal, const float diameter) {
		super::add_position(position);
		add_normal(normal);
		add_diameter(diameter);
	}

	void add(const vec3& position, const ColorType& color, const float diameter) {
		super::add(position, color);
		add_diameter(diameter);
	}

	void fill_normals(const vec3& normal) {
		super::fill(normals, normal);
	}

	void fill_diameters(const float diameter) {
		super::fill(diameters, diameter);
	}

	RDB_BASE_FUNC_DEF(surfel_renderer, surfel_render_style);
};

}
}
