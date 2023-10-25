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

	/// stores an array of extents
	std::vector<vec2> extents;
	/// stores an array of translations
	std::vector<vec3> translations;
	/// stores an array of rotations
	std::vector<quat> rotations;
	/// stores an array of texcoords
	std::vector<vec4> texcoords;
	/// stores an optional constant extent used for all elements
	cgv::data::optional<vec2> const_extent;
	/// stores an optional constant translation used for all elements
	cgv::data::optional<vec3> const_translation;
	/// stores an optional constant rotation used for all elements
	cgv::data::optional<quat> const_rotation;
	/// stores an optional constant texcoord used for all elements
	cgv::data::optional<vec4> const_texcoord;

protected:
	/// @brief See render_data_base::transfer.
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

	/// @brief See render_data_base::set_const_attributes.
	void set_const_attributes(context& ctx, rectangle_renderer& r) {
		super::set_const_attributes(ctx, r);
		if(extents.empty() && const_extent)
			r.set_extent(ctx, const_extent.value());
		if(translations.empty() && const_translation)
			r.set_translation(ctx, const_translation.value());
		if(rotations.empty() && const_rotation)
			r.set_rotation(ctx, const_rotation.value());
		if(texcoords.empty() && const_texcoord)
			r.set_texcoord(ctx, const_texcoord.value());
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
