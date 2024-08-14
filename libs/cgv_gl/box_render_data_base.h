#pragma once

#include "render_data_base.h"

namespace cgv {
namespace render {

/// @brief A base class for storing render data usable with the box_renderer and box_wire_renderer. See render_data_base.
/// @tparam RendererType The type of the used renderer. Must be derived from cgv::render::renderer.
/// @tparam renderStyleType The type of the used render style. Must be supported by RendererType.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <class RendererType, class RenderStyleType, typename ColorType = rgb>
class box_render_data_base : public render_data_base<RendererType, RenderStyleType, ColorType> {
private:
	// Base class we're going to use virtual functions from
	typedef render_data_base<RendererType, RenderStyleType, ColorType> super;

protected:
	/// @brief See render_data_base::transfer.
	bool transfer(context& ctx, RendererType& r) override {
		if(super::transfer(ctx, r)) {
			CGV_RDB_TRANSFER_ARRAY(extent, extents);
			CGV_RDB_TRANSFER_ARRAY(translation, translations);
			CGV_RDB_TRANSFER_ARRAY(rotation, rotations);
			return true;
		}
		return false;
	}

public:
	/// array of extents
	std::vector<vec3> extents;
	/// array of translations
	std::vector<vec3> translations;
	/// array of rotations
	std::vector<quat> rotations;

	void clear() {
		super::clear();
		extents.clear();
		translations.clear();
		rotations.clear();
	}

	void add_extent(const vec3& extent) {
		extents.push_back(extent);
	}

	void add_translation(const vec3& translation) {
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
};

}
}
