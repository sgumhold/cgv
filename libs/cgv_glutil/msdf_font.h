#pragma once

#include <cgv/render/context.h>
#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API msdf_font : public cgv::render::render_types {
public:
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

	std::vector<glyph_info> glyphs;
	cgv::render::texture atlas_texture;

	bool load_atlas_texture(cgv::render::context& ctx, const std::string& filename);

	bool load_atlas_metadata(const std::string& filename);

	void compute_derived_glyph_attributes();

public:
	msdf_font();

	void destruct(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);

	const glyph_info& get_glyph_info(unsigned char id) const { return glyphs[id]; }

	float get_initial_font_size() const { return initial_font_size; }

	float get_pixel_range() const { return pixel_range; }

	bool enable(cgv::render::context& ctx);

	bool disable(cgv::render::context& ctx);
};

}
}

#include <cgv/config/lib_end.h>
