#pragma once

#include "sphere_renderer.h"
#include "render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for sphere geometry with support for the sphere_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class sphere_render_data : public render_data_base<sphere_renderer, sphere_render_style, ColorType> {
private:
	// Base class we're going to use virtual functions from
	typedef render_data_base<sphere_renderer, sphere_render_style, ColorType> super;
	
	sphere_renderer& ref_renderer_singleton(context& ctx, int ref_count_change = 0) override {
		return ref_sphere_renderer(ctx, ref_count_change);
	}

protected:
	/// @brief See render_data_base::transfer.
	bool transfer(context& ctx, sphere_renderer& r) override {
		if(super::transfer(ctx, r)) {
			CGV_RDB_TRANSFER_ARRAY(radius, radii);
			return true;
		}
		return false;
	}

public:
	/// array of radii
	std::vector<float> radii;

	void clear() {
		super::clear();
		radii.clear();
	}

	void add_radius(const float radius) {
		radii.push_back(radius);
	}

	// Explicitly use add from the base class since it is shadowed by the overloaded versions
	using super::add;

	void add(const vec3& position, const float radius) {
		super::add_position(position);
		add_radius(radius);
	}

	void add(const vec3& position, const ColorType& color, const float radius) {
		super::add(position, color);
		add_radius(radius);
	}

	void fill_radii(const float radius) {
		super::fill(radii, radius);
	}
};

}
}
