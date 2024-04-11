#include "msdf_font.h"

#include <cgv/base/import.h>
#include <cgv/utils/tokenizer.h>

namespace cgv {
namespace g2d {

msdf_font::msdf_font() {
	initial_font_size = 0;
	pixel_range = 0;
	cap_height = 0.0f;
}

void msdf_font::manage_singleton(cgv::render::context& ctx, const std::string& class_name, int& ref_count, int ref_count_change) {
	switch(ref_count_change) {
	case 1:
		if(ref_count == 0) {
			if(!init(ctx))
				ctx.error(std::string("unable to initialize " + class_name + " singleton"));
		}
		++ref_count;
		break;
	case 0:
		break;
	case -1:
		if(ref_count == 0)
			ctx.error(std::string("attempt to decrease reference count of " + class_name + " singleton below 0"));
		else {
			if(--ref_count == 0)
				destruct(ctx);
		}
		break;
	default:
		ctx.error(std::string("invalid change reference count outside {-1,0,1} for " + class_name + " singleton"));
	}
}

void msdf_font::destruct(cgv::render::context& ctx) {
	atlas_texture.destruct(ctx);
}

bool msdf_font::is_initialized() const
{
	return atlas_texture.is_created();
}

bool msdf_font::init(cgv::render::context& ctx) {
	bool success = true;

	std::string font_name = "segoeui";
	switch(font_face) {
	case cgv::g2d::msdf_font::FF_LIGHT: font_name += "l"; break;
	case cgv::g2d::msdf_font::FF_BOLD: font_name += "b"; break;
	default: break;
	}

	success &= load_atlas_texture(ctx, "res://" + font_name + "_atlas.png");
	success &= load_atlas_metadata("res://" + font_name + "_meta.png");
	return success;
}

float msdf_font::compute_length(const std::string& str, size_t end) const {
	float length = 0.0f;
	float acc_advance = 0.0f;

	if(end == std::string::npos || end > str.length())
		end = str.length();

	for(size_t i = 0; i < end; ++i) {
		char c = str[i];
		const auto& glyph = get_glyph_info(static_cast<unsigned char>(c));
		length = acc_advance + glyph.size.x();
		acc_advance += glyph.advance;

		// if the last character is a space add its advance to the length
		if(i == end - 1 && c == ' ')
			length += glyph.advance;
	}

	return length;
}

std::vector<vec4> msdf_font::create_vertex_data(const std::string& str) const {
	std::vector<vec4> vertices;
	vertices.reserve(2 * str.size());

	float accumulated_advance = 0.0f;

	for(char c : str) {
		const auto& glyph = get_glyph_info(static_cast<unsigned char>(c));

		vec2 position = glyph.position + vec2(accumulated_advance, 0.0f);
		vec2 size = glyph.size;
		accumulated_advance += glyph.advance;

		vertices.emplace_back(position.x(), position.y(), size.x(), size.y());
		vertices.emplace_back(glyph.texcoords);
	}

	return vertices;
}

bool msdf_font::enable(cgv::render::context& ctx) {
	if(atlas_texture.is_created())
		return atlas_texture.enable(ctx, 0);
	return false;
}

bool msdf_font::disable(cgv::render::context& ctx) {
	return atlas_texture.disable(ctx);
}

bool msdf_font::load_atlas_texture(cgv::render::context& ctx, const std::string& filename) {
	// load the font atlas used for rendering as a texture
	if(atlas_texture.is_created())
		atlas_texture.destruct(ctx);

	cgv::data::data_format format;
	cgv::data::data_view data;
	return atlas_texture.create_from_image(format, data, ctx, filename, (unsigned char*)0, 0);
}

bool msdf_font::load_atlas_metadata(const std::string& filename) {
	glyphs.resize(256);

	std::string content;
	if(!cgv::base::read_data_file(filename, content, false))
		return false;

	if(content.length() > 0) {
		bool read_lines = true;
		size_t split_pos = content.find_first_of('\n');
		size_t line_offset = 0;

		while(read_lines) {
			std::string line = "";

			if(split_pos == std::string::npos) {
				read_lines = false;
				line = content.substr(line_offset, std::string::npos);
			} else {
				size_t next_line_offset = split_pos;
				line = content.substr(line_offset, next_line_offset - line_offset);
				line_offset = next_line_offset + 1;
				split_pos = content.find_first_of('\n', line_offset);
			}

			std::vector<cgv::utils::token> tokens;

			cgv::utils::tokenizer tknzr(line);
			tknzr.set_ws(",");
			tknzr.bite_all(tokens);

			if(tokens.size() == 3) {
				// This should be the first line. It contains the file id "MSDF_FONT_GLYPH_METADATA",
				// initial font size used to generate the msdf and the pixel range of the signed
				// distance field.
				initial_font_size = static_cast<float>(std::strtol(to_string(tokens[1]).c_str(), 0, 10));
				pixel_range = static_cast<float>(std::strtol(to_string(tokens[2]).c_str(), 0, 10));
			}

			if(tokens.size() == 10) {
				int id = std::strtol(to_string(tokens[0]).c_str(), 0, 10);

				if(id > 0 && id < 256) {
					auto& g = glyphs[id];
					g.advance = std::strtof(to_string(tokens[1]).c_str(), 0);

					g.plane_bounds.x() = std::strtof(to_string(tokens[2]).c_str(), 0);
					g.plane_bounds.y() = std::strtof(to_string(tokens[3]).c_str(), 0);
					g.plane_bounds.z() = std::strtof(to_string(tokens[4]).c_str(), 0);
					g.plane_bounds.w() = std::strtof(to_string(tokens[5]).c_str(), 0);

					g.atlas_bounds.x() = std::strtof(to_string(tokens[6]).c_str(), 0);
					g.atlas_bounds.y() = std::strtof(to_string(tokens[7]).c_str(), 0);
					g.atlas_bounds.z() = std::strtof(to_string(tokens[8]).c_str(), 0);
					g.atlas_bounds.w() = std::strtof(to_string(tokens[9]).c_str(), 0);
				}
			}
		}
	}

	compute_derived_glyph_attributes();

	return true;
}

void msdf_font::compute_derived_glyph_attributes() {
	vec2 atlas_dimensions(float(atlas_texture.get_width()), float(atlas_texture.get_height()));
	vec4 inv_dimensions = vec4(1.0f) / vec4(atlas_dimensions.x(), atlas_dimensions.y(), atlas_dimensions.x(), atlas_dimensions.y());

	for(size_t i = 0; i < glyphs.size(); ++i) {
		glyph_info& glyph = glyphs[i];

		vec2 pa(glyph.plane_bounds.x(), glyph.plane_bounds.y());
		vec2 pb(glyph.plane_bounds.z(), glyph.plane_bounds.w());

		glyph.position = pa;
		glyph.size = pb - pa;
		glyph.texcoords = glyph.atlas_bounds * inv_dimensions;
	}

	const glyph_info& glyph_m = get_glyph_info('M');
	float baseline_offset = glyph_m.position.y();

	for(glyph_info& glyph : glyphs)
		glyph.position.y() -= baseline_offset;

	cap_height = glyph_m.size.y();
}

msdf_font_regular& ref_msdf_font_regular(cgv::render::context& ctx, int ref_count_change) {
	static int ref_count = 0;
	static msdf_font_regular f;
	f.manage_singleton(ctx, "msdf_font_regular", ref_count, ref_count_change);
	return f;
}

msdf_font_light& ref_msdf_font_light(cgv::render::context& ctx, int ref_count_change) {
	static int ref_count = 0;
	static msdf_font_light f;
	f.manage_singleton(ctx, "msdf_font_light", ref_count, ref_count_change);
	return f;
}

msdf_font_bold& ref_msdf_font_bold(cgv::render::context& ctx, int ref_count_change) {
	static int ref_count = 0;
	static msdf_font_bold f;
	f.manage_singleton(ctx, "msdf_font_bold", ref_count, ref_count_change);
	return f;
}

}
}
