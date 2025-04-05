#pragma once

#include "ellipsoid_renderer.h"
#include "render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for ellipsoid geometry with support for the ellipsoid_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class ellipsoid_render_data : public render_data_base<ellipsoid_renderer, ellipsoid_render_style, ColorType> {
private:
	// Base class we're going to use virtual functions from
	typedef render_data_base<ellipsoid_renderer, ellipsoid_render_style, ColorType> super;
	
	ellipsoid_renderer& ref_renderer_singleton(context& ctx, int ref_count_change = 0) override {
		return ref_ellipsoid_renderer(ctx, ref_count_change);
	}

protected:
	/// @brief See render_data_base::transfer.
	bool transfer(context& ctx, ellipsoid_renderer& r) override {
		if(super::transfer(ctx, r)) {
			CGV_RDB_TRANSFER_ARRAY(size, sizes);
			CGV_RDB_TRANSFER_ARRAY(orientation, orientations);
			return true;
		}
		return false;
	}

public:
	/// array of sizes
	std::vector<cgv::vec3> sizes;
	/// array of orientations
	std::vector<cgv::quat> orientations;

	void clear() {
		super::clear();
		sizes.clear();
		orientations.clear();
	}

	void add_size(const cgv::vec3& size) {
		sizes.push_back(size);
	}

	void add_orientation(const cgv::quat& orientation) {
		orientations.push_back(orientation);
	}

	// Explicitly use add from the base class since it is shadowed by the overloaded versions
	using super::add;

	void add(const vec3& position, const cgv::vec3& size) {
		super::add_position(position);
		add_size(size);
	}

	void add(const vec3& position, const ColorType& color, const cgv::vec3& size) {
		super::add(position, color);
		add_size(size);
	}

	void add(const vec3& position, const cgv::vec3& size, const quat& rotation) {
		add(position, size);
		add_size(size);
		add_orientation(orientation);
	}

	void add(const vec3& position, const ColorType& color, const cgv::vec3& size, const quat& rotation) {
		super::add(position, color);
		add_size(size);
		add_orientation(orientation);
	}

	void fill_sizes(const cgv::vec3& size) {
		super::fill(sizes, size);
	}
};

}
}
