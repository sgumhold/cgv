#pragma once

#include <cgv/render/context.h>
#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API msdf_font;

extern CGV_API msdf_font& ref_msdf_font(cgv::render::context& ctx, int ref_count_change = 0);

class CGV_API msdf_font : public cgv::render::render_types {
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
	float initial_font_size;
	float pixel_range;
	FontFace font_face = FF_REGULAR;

	std::vector<glyph_info> glyphs;
	cgv::render::texture atlas_texture;

	bool load_atlas_texture(cgv::render::context& ctx, const std::string& filename);

	bool load_atlas_metadata(const std::string& filename);

	void compute_derived_glyph_attributes();

public:
	msdf_font();

	void manage_singleton(cgv::render::context& ctx, int& ref_count, int ref_count_change);

	void destruct(cgv::render::context& ctx);

	bool is_initialized() const;

	bool init(cgv::render::context& ctx);

	void set_font_face(FontFace ff) { font_face = ff; }

	const glyph_info& get_glyph_info(unsigned char id) const { return glyphs[id]; }

	float get_initial_font_size() const { return initial_font_size; }

	float get_pixel_range() const { return pixel_range; }

	bool enable(cgv::render::context& ctx);

	bool disable(cgv::render::context& ctx);
};

}
}

#include <cgv/config/lib_end.h>
