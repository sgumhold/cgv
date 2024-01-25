#pragma once

#include "box_wire_renderer.h"
#include "render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for box geometry with support for the box_wire_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class box_wire_render_data : public render_data_base<ColorType> {
public:
	// Base class we're going to use virtual functions from
	typedef render_data_base<ColorType> super;

	/// stores an array of extents
	std::vector<vec3> extents;
	/// stores an array of translations
	std::vector<vec3> translations;
	/// stores an array of rotations
	std::vector<quat> rotations;

protected:
	/// @brief See render_data_base::transfer.
	bool transfer(context& ctx, box_wire_renderer& r) {
		if(super::transfer(ctx, r)) {
			if(extents.size() == super::size())
				r.set_extent_array(ctx, extents);
			if(translations.size() == super::size())
				r.set_translation_array(ctx, translations);
			if(rotations.size() == super::size())
				r.set_rotation_array(ctx, rotations);
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
	}

	void add_extent(const vec3 extent) {
		extents.push_back(extent);
	}

	void add_translation(const vec3 translation) {
		translations.push_back(translation);
	}

	void add_rotation(const quat& rotation) {
		rotations.push_back(rotation);
	}

	// Explicitly use add from the base class since it is shadowed by the overloaded versions
	using super::add;

	void add(const vec3& position, const vec3& extent) {
		super::add_position(position);
		add_extent(extent);
	}

	void add(const vec3& position, const vec3& extent, const quat& rotation) {
		super::add_position(position);
		add_extent(extent);
		add_rotation(rotation);
	}

	void add(const vec3& position, const vec3& extent, const ColorType& color) {
		add(position, extent);
		super::add_color(color);
	}

	void add(const vec3& translation, const quat& rotation) {
		add_translation(translation);
		add_rotation(rotation);
	}

	void fill_extents(const vec3& extent) {
		super::fill(extents, extent);
	}

	void fill_translations(const vec3& translation) {
		super::fill(translations, translation);
	}

	void fill_rotations(const quat& rotation) {
		super::fill(rotations, rotation);
	}

	RDB_BASE_FUNC_DEF(box_wire_renderer, box_wire_render_style);
};

}
}
