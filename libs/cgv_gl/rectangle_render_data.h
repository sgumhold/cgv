#pragma once

#include "rectangle_renderer.h"
#include "render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for rectangle geometry with support for the rectangle_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class rectangle_render_data : public render_data_base<ColorType> {
public:
	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

	std::vector<vec2> extents;
	std::vector<vec3> translations;
	std::vector<quat> rotations;
	std::vector<vec4> texcoords;

protected:
	bool transfer(context& ctx, rectangle_renderer& r) {
		if(super::transfer(ctx, r)) {
			if(extents.size() == super::size())
				r.set_extent_array(ctx, extents);
			if(translations.size() == super::size())
				r.set_translation_array(ctx, translations);
			if(rotations.size() == super::size())
				r.set_rotation_array(ctx, rotations);
			if(texcoords.size() == super::size())
				r.set_texcoord_array(ctx, texcoords);
			return true;
		}
		return false;
	}

public:
	void clear() {
		super::clear();
		extents.clear();
		translations.clear();
		rotations.clear();
		texcoords.clear();
	}

	void add_extent(const vec2 extent) {
		extents.push_back(extent);
	}

	void add_translation(const vec3 translation) {
		translations.push_back(translation);
	}

	void add_rotation(const quat& rotation) {
		rotations.push_back(rotation);
	}

	void add_texcoord(const vec4& texcoord) {
		texcoords.push_back(texcoord);
	}

	// Explicitly use add from the base class since it is shadowed by the overloaded versions
	using super::add;

	void add(const vec3& position, const vec2& extent) {
		super::add_position(position);
		add_extent(extent);
	}

	void add(const vec3& position, const vec4& texcoord) {
		super::add_position(position);
		add_texcoord(texcoord);
	}

	void add(const vec3& position, const ColorType& color, const vec2& extent) {
		super::add(position, color);
		add_extent(extent);
	}

	void add(const vec3& position, const vec2& extent, const quat& rotation) {
		super::add_position(position);
		add_extent(extent);
		add_rotation(rotation);
	}

	void add(const vec3& translation, const quat& rotation) {
		add_translation(translation);
		add_rotation(rotation);
	}

	void fill_extents(const vec2& extent) {
		super::fill(extents, extent);
	}

	void fill_rotations(const quat& rotation) {
		super::fill(rotations, rotation);
	}

	void fill_texcoords(const vec4& texcoord) {
		super::fill(texcoords, texcoord);
	}

	RDB_BASE_FUNC_DEF(rectangle_renderer, rectangle_render_style);
};

}
}
