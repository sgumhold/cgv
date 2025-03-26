#pragma once

#include "rectangle_renderer.h"
#include "render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for rectangle geometry with support for the rectangle_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class rectangle_render_data : public render_data_base<rectangle_renderer, rectangle_render_style, ColorType> {
private:
	// Base class we're going to use virtual functions from
	typedef render_data_base<rectangle_renderer, rectangle_render_style, ColorType> super;

	rectangle_renderer& ref_renderer_singleton(context& ctx, int ref_count_change = 0) override {
		return ref_rectangle_renderer(ctx, ref_count_change);
	}

protected:
	/// @brief See render_data_base::transfer.
	bool transfer(context& ctx, rectangle_renderer& r) override {
		if(super::transfer(ctx, r)) {
			CGV_RDB_TRANSFER_ARRAY(secondary_color, secondary_colors);
			CGV_RDB_TRANSFER_ARRAY(border_color, border_colors);
			CGV_RDB_TRANSFER_ARRAY(extent, extents);
			CGV_RDB_TRANSFER_ARRAY(translation, translations);
			CGV_RDB_TRANSFER_ARRAY(rotation, rotations);
			CGV_RDB_TRANSFER_ARRAY(texcoord, texcoords);
			return true;
		}
		return false;
	}

	/// @brief See render_data_base::set_const_attributes.
	void set_const_attributes(context& ctx, rectangle_renderer& r) override {
		super::set_const_attributes(ctx, r);
		if(secondary_colors.empty() && const_secondary_color)
			r.set_secondary_color(ctx, const_secondary_color.value());
		if(border_colors.empty() && const_border_color)
			r.set_border_color(ctx, const_border_color.value());
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
	/// array of secondary colors
	std::vector<ColorType> secondary_colors;
	/// array of border colors
	std::vector<ColorType> border_colors;
	/// array of extents
	std::vector<vec2> extents;
	/// array of translations
	std::vector<vec3> translations;
	/// array of rotations
	std::vector<quat> rotations;
	/// array of texcoords
	std::vector<vec4> texcoords;

	/// optional constant secondary color used for all elements
	cgv::data::optional<ColorType> const_secondary_color;
	/// optional constant border color used for all elements
	cgv::data::optional<ColorType> const_border_color;
	/// optional constant extent used for all elements
	cgv::data::optional<vec2> const_extent;
	/// optional constant translation used for all elements
	cgv::data::optional<vec3> const_translation;
	/// optional constant rotation used for all elements
	cgv::data::optional<quat> const_rotation;
	/// optional constant texcoord used for all elements
	cgv::data::optional<vec4> const_texcoord;

	void clear() {
		super::clear();
		secondary_colors.clear();
		border_colors.clear();
		extents.clear();
		translations.clear();
		rotations.clear();
		texcoords.clear();
	}

	void add_secondary_color(const ColorType& color) {
		secondary_colors.push_back(color);
	}

	void add_border_color(const ColorType& color) {
		border_colors.push_back(color);
	}

	void add_extent(const vec2& extent) {
		extents.push_back(extent);
	}

	void add_translation(const vec3& translation) {
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

	void fill_secondary_colors(const ColorType& color) {
		super::fill(secondary_colors, color);
	}

	void fill_border_colors(const ColorType& color) {
		super::fill(border_colors, color);
	}

	void fill_extents(const vec2& extent) {
		super::fill(extents, extent);
	}

	void fill_translations(const vec3& translation) {
		super::fill(translations, translation);
	}

	void fill_rotations(const quat& rotation) {
		super::fill(rotations, rotation);
	}

	void fill_texcoords(const vec4& texcoord) {
		super::fill(texcoords, texcoord);
	}
};

}
}
