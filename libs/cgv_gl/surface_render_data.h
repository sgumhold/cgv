#pragma once

#include "surface_renderer.h"
#include "render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for triangular surface geometry with support for the surface_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class surface_render_data : public render_data_base<surface_renderer, surface_render_style, ColorType> {
private:
	// Base class we're going to use virtual functions from
	typedef render_data_base<surface_renderer, surface_render_style, ColorType> super;
	
	surface_renderer& ref_renderer_singleton(context& ctx, int ref_count_change = 0) override {
		return ref_surface_renderer(ctx, ref_count_change);
	}

protected:
	/// @brief See render_data_base::transfer.
	bool transfer(context& ctx, surface_renderer& r) override {
		if(super::transfer(ctx, r)) {
			CGV_RDB_TRANSFER_ARRAY(normal, normals);
			CGV_RDB_TRANSFER_ARRAY(texcoord, texcoords);
			return true;
		}
		return false;
	}

public:
	/// array of normals
	std::vector<cgv::vec3> normals;
	/// array of texture coordinates
	std::vector<cgv::vec2> texcoords;

	void clear() {
		super::clear();
		normals.clear();
		texcoords.clear();
	}

	void add_normal(const cgv::vec3& normal) {
		normals.push_back(normal);
	}
	
	void add_texcoord(const cgv::vec2& texcoord) {
		texcoords.push_back(texcoord);
	}

	void add_triangle(const vec3& position0, const vec3& position1, const vec3& position2) {
		super::add_position(position0);
		super::add_position(position1);
		super::add_position(position2);
	}

	void add_triangle_color(const ColorType& color) {
		add_color(color);
		add_color(color);
		add_color(color);
	}

	void add_triangle_colors(const ColorType& color0, const ColorType& color1, const ColorType& color2) {
		add_color(color0);
		add_color(color1);
		add_color(color2);
	}

	void add_triangle_normal(const vec3& normal) {
		add_normal(normal);
		add_normal(normal);
		add_normal(normal);
	}

	void add_triangle_normals(const vec3& normal0, const vec3& normal1, const vec3& normal2) {
		add_normal(normal0);
		add_normal(normal1);
		add_normal(normal2);
	}

	void add_triangle_texcoord(const vec2& texcoord) {
		add_texcoord(texcoord);
		add_texcoord(texcoord);
		add_texcoord(texcoord);
	}

	void add_triangle_texcoords(const vec2& texcoord0, const vec2& texcoord1, const vec2& texcoord2) {
		add_texcoord(texcoord0);
		add_texcoord(texcoord1);
		add_texcoord(texcoord2);
	}

	// Explicitly use add from the base class since it is shadowed by the overloaded versions
	using super::add;

	void add(const vec3& position, const cgv::vec3& normal) {
		super::add_position(position);
		add_normal(normal);
	}

	void add(const vec3& position, const ColorType& color, const cgv::vec3& normal) {
		super::add(position, color);
		add_normal(normal);
	}

	void add(const vec3& position, const ColorType& color, const cgv::vec2& texcoord) {
		super::add(position, color);
		add_texcoord(texcoord);
	}

	void fill_normals(const cgv::vec3& normal) {
		super::fill(normals, normal);
	}

	void fill_texcoords(const cgv::vec2& texcoord) {
		super::fill(texcoords, texcoord);
	}
};

}
}
