#pragma once

#include <cgv/render/context.h>
#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace g2d {

class CGV_API msdf_font {
public:
	enum FontFace {
		FF_LIGHT = 0,
		FF_REGULAR = 1,
		FF_BOLD = 2,
	};

	struct glyph_info {
		float advance;
		vec4 plane_bounds;
		vec4 atlas_bounds;
		vec4 texcoords;
		vec2 position;
		vec2 size;
	};

protected:
	float initial_font_size = 0.0f;
	float pixel_range = 0.0f;
	float em_size = 0.0f;
	float line_height = 0.0f;
	float top = 0.0f;
	float bottom = 0.0f;
	float ascender = 0.0f;
	float descender = 0.0f;
	float cap_height = 0.0f;
	float x_height = 0.0f;
	float underline_y = 0.0f;
	float underline_thickness = 0.0f;
	FontFace font_face = FF_REGULAR;

	std::vector<glyph_info> glyphs;
	cgv::render::texture atlas_texture;

	bool load_atlas_texture(cgv::render::context& ctx, const std::string& filename);

	bool load_atlas_metadata(const std::string& filename);

	void read_metric(const std::string& name, float value);

	void compute_derived_glyph_attributes();

public:
	msdf_font();

	void manage_singleton(cgv::render::context& ctx, const std::string& class_name, int& ref_count, int ref_count_change);

	void destruct(cgv::render::context& ctx);

	bool is_initialized() const;

	bool init(cgv::render::context& ctx);

	void set_font_face(FontFace ff) { font_face = ff; }

	const glyph_info& get_glyph_info(unsigned char id) const { return glyphs[id]; }

	float get_initial_font_size() const { return initial_font_size; }

	float get_pixel_range() const { return pixel_range; }

	float get_em_size() const { return em_size; }

	float get_line_height() const { return line_height; }

	float get_top() const { return top; }

	float get_bottom() const { return bottom; }

	float get_ascender() const { return ascender; }

	float get_descender() const { return descender; }

	float get_cap_height() const { return cap_height; }

	float get_x_height() const { return x_height; }

	float get_underline_y() const { return underline_y; }

	float get_underline_thickness() const { return underline_thickness; }

	float compute_normalized_length(const std::string& str, size_t end = std::string::npos) const;

	vec2 compute_render_size(const std::string& str, float scale) const;

	vec2 compute_render_size(const std::string& str, size_t end, float scale) const;

	void generate_vertex_data(const std::string& str, std::vector<vec4>& quads, std::vector<vec4>& texcoords) const;

	bool enable(cgv::render::context& ctx);

	bool disable(cgv::render::context& ctx);
};

class CGV_API msdf_font_regular : public msdf_font {
public:
	msdf_font_regular() {
		font_face = FontFace::FF_REGULAR;
	}

	void set_font_face(FontFace ff) = delete;
};

extern CGV_API msdf_font_regular& ref_msdf_font_regular(cgv::render::context& ctx, int ref_count_change = 0);

class CGV_API msdf_font_light : public msdf_font {
public:
	msdf_font_light() {
		font_face = FontFace::FF_LIGHT;
	}

	void set_font_face(FontFace ff) = delete;
};

extern CGV_API msdf_font_light& ref_msdf_font_light(cgv::render::context& ctx, int ref_count_change = 0);

class CGV_API msdf_font_bold : public msdf_font {
public:
	msdf_font_bold() {
		font_face = FontFace::FF_BOLD;
	}

	void set_font_face(FontFace ff) = delete;
};

extern CGV_API msdf_font_bold& ref_msdf_font_bold(cgv::render::context& ctx, int ref_count_change = 0);

}
}

#include <cgv/config/lib_end.h>
