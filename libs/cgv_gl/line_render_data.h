#pragma once

#include "line_renderer.h"
#include "render_data_base.h"

namespace cgv {
namespace render {

/// @brief Render data for line geometry with support for the line_renderer. See render_data_base.
/// @tparam ColorType The type used to represent colors. Must be cgv::render::rgb or cgv::render::rgba.
template <typename ColorType = rgb>
class line_render_data : public render_data_base<line_renderer, line_render_style, ColorType> {
private:
	// Base class we're going to use virtual functions from
	typedef render_data_base<line_renderer, line_render_style, ColorType> super;

	line_renderer& ref_renderer_singleton(context& ctx, int ref_count_change = 0) override {
		return ref_line_renderer(ctx, ref_count_change);
	}

protected:
	/// @brief See render_data_base::transfer.
	bool transfer(context& ctx, line_renderer& r) override {
		if(super::transfer(ctx, r)) {
			CGV_RDB_TRANSFER_ARRAY(normal, normals);
			CGV_RDB_TRANSFER_ARRAY(line_width, widths);
			return true;
		}
		return false;
	}

public:
	/// array of normals
	std::vector<vec3> normals;
	/// array of widths
	std::vector<float> widths;

	void clear() {
		super::clear();
		normals.clear();
		widths.clear();
	}

	void add_normal(const vec3& normal) {
		normals.push_back(normal);
	}

	void add_width(const float width) {
		widths.push_back(width);
	}

	void add_segment_normal(const vec3& normal) {
		add_normal(normal);
		add_normal(normal);
	}

	void add_segment_width(const float width) {
		add_width(width);
		add_width(width);
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

	void add(const vec3& start_position, const vec3& end_position, const vec3& normal) {
		add(start_position, end_position);
		add_segment_normal(normal);
	}

	void add(const vec3& start_position, const vec3& end_position, const float width) {
		add(start_position, end_position);
		add_segment_width(width);
	}

	void add(const vec3& start_position, const vec3& end_position, const ColorType& color, const float width) {
		add(start_position, end_position);
		add_segment_color(color);
		add_segment_width(width);
	}

	void add(const float start_width, const float end_width) {
		add_width(start_width);
		add_width(end_width);
	}

	void add(const ColorType& start_color, const ColorType& end_color) {
		super::add_color(start_color);
		super::add_color(end_color);
	}
	
	void fill_normals(const vec3& normal) {
		super::fill(normals, normal);
	}

	void fill_widths(const float width) {
		super::fill(widths, width);
	}
};

}
}
