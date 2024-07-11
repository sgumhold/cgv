#pragma once

#include "cone_renderer.h"
#include "render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for cone geometry with support for the cone_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class cone_render_data : public render_data_base<cone_renderer, cone_render_style, ColorType> {
private:
	// Base class we're going to use virtual functions from
	typedef render_data_base<cone_renderer, cone_render_style, ColorType> super;

	cone_renderer& ref_renderer_singleton(context& ctx, int ref_count_change = 0) override {
		return ref_cone_renderer(ctx, ref_count_change);
	}

protected:
	/// @brief See render_data_base::transfer.
	bool transfer(context& ctx, cone_renderer& r) override {
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

	void add_segment_radius(const float radius) {
		add_radius(radius);
		add_radius(radius);
	}

	void add_segment_color(const ColorType& color) {
		super::add_color(color);
		super::add_color(color);
	}

	// Explicitly use add from the base class since it is shadowed by the overloaded versions
	using super::add;

	void add(const vec3& start_position, const vec3& end_position) {
		super::add_position(start_position);
		super::add_position(end_position);
	}

	void add(const vec3& start_position, const vec3& end_position, const ColorType& color) {
		add(start_position, end_position);
		add_segment_color(color);
	}

	void add(const vec3& start_position, const vec3& end_position, const float radius) {
		add(start_position, end_position);
		add_segment_radius(radius);
	}

	void add(const vec3& start_position, const vec3& end_position, const ColorType& color, const float radius) {
		add(start_position, end_position);
		add_segment_color(color);
		add_segment_radius(radius);
	}

	void add(const float start_radius, const float end_radius) {
		add_radius(start_radius);
		add_radius(end_radius);
	}

	void add(const ColorType& start_color, const ColorType& end_color) {
		super::add_color(start_color);
		super::add_color(end_color);
	}

	void fill_radii(const float radius) {
		super::fill(radii, radius);
	}
};

}
}
