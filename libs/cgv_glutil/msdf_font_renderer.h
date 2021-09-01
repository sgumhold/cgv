#pragma once

#include <cgv/base/import.h>
#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv/utils/tokenizer.h>
#include <cgv_gl/gl/gl.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class msdf_font : public cgv::render::render_types {
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

	bool load_atlas_texture(cgv::render::context& ctx, const std::string& filename) {
		// load the font atlas used for rendering as a texture
		if(atlas_texture.is_created())
			atlas_texture.destruct(ctx);

		cgv::data::data_format format;
		cgv::data::data_view data;
		return atlas_texture.create_from_image(format, data, ctx, filename, (unsigned char*)0, 0);
	}

	bool load_atlas_metadata(const std::string& filename) {
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

				// TODO: can be removed
				if(!line.empty()) {
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
		}

		compute_derived_glyph_attributes();

		return true;
	}

	void compute_derived_glyph_attributes() {
		vec2 atlas_dimensions(atlas_texture.get_width(), atlas_texture.get_height());
		vec4 inv_dimensions = vec4(atlas_dimensions.x(), atlas_dimensions.y(), atlas_dimensions.x(), atlas_dimensions.y());

		for(size_t i = 0; i < glyphs.size(); ++i) {
			glyph_info& glyph = glyphs[i];

			vec2 pa(glyph.plane_bounds.x(), glyph.plane_bounds.y());
			vec2 pb(glyph.plane_bounds.z(), glyph.plane_bounds.w());

			glyph.position = pa;
			glyph.size = pb - pa;
			glyph.texcoords = glyph.atlas_bounds * inv_dimensions;
		}
	}

public:
	msdf_font() {
		initial_font_size = 0;
		pixel_range = 0;
	}

	bool init(cgv::render::context& ctx) {
		bool success = true;
		success &= load_atlas_texture(ctx, "res://segoeui_atlas.png");
		success &= load_atlas_metadata("res://segoeui_meta.png");
		return success;
	}

	const glyph_info& get_glyph_info(unsigned char id) const {
		return glyphs[id];
	}

	float get_initial_font_size() const { return initial_font_size; }
	
	float get_pixel_range() const { return pixel_range; }

	bool enable(cgv::render::context& ctx) {
		if(atlas_texture.is_created())
			return atlas_texture.enable(ctx, 0);
		return false;
	}

	bool disable(cgv::render::context& ctx) {
		return atlas_texture.disable(ctx);
	}
};

class msdf_text_geometry : public cgv::render::render_types {
protected:
	struct text_info {
		int offset;
		int count;
		ivec2 position;
		vec2 size;
		cgv::render::TextAlignment alignment;
	};

	struct vertex_type {
		vec4 position_size;
		vec4 texcoords;
	};

	// TODO: use a ref_ptr
	msdf_font* msdf_font_ptr;

	GLuint ssbo;
	bool state_out_of_date;

	std::vector<text_info> texts;
	std::vector<vertex_type> vertices;

	float render_font_size;

	void add_vertex(const vec4& pos, const vec4& txc) {
		vertices.push_back({ pos, txc });
	}

	void end_text(const ivec2& position, const vec2& size, const cgv::render::TextAlignment alignment) {
		text_info text;

		if(texts.size() > 0) {
			const text_info& last_text = *texts.rbegin();
			text.offset = last_text.offset + last_text.count;
			text.count = vertices.size() - text.offset;
		} else {
			text.offset = 0;
			text.count = vertices.size();
		}

		text.position = position;
		text.size = size;
		text.alignment = alignment;
		texts.push_back(text);
	}

public:
	msdf_text_geometry() {
		msdf_font_ptr = nullptr;
		state_out_of_date = true;

		render_font_size = 32.0f;
	};

	~msdf_text_geometry() {
		clear();
	};

	void clear() {
		state_out_of_date = true;

		texts.clear();
		vertices.clear();

		if(ssbo != 0) {
			glDeleteBuffers(1, &ssbo);
			ssbo = 0;
		}
	}

	bool is_created() const {
		return !state_out_of_date;
	}

	const msdf_font* get_msdf_font() { return msdf_font_ptr; }

	float get_font_size() { return render_font_size; }

	void set_msdf_font(msdf_font* ptr) {
		msdf_font_ptr = ptr;
	}

	void set_font_size(float size) {
		// TODO: make size change not require a rebuild of the vertex data
		render_font_size = size;
		state_out_of_date = true;
	}

	const std::vector<text_info>& ref_texts() const { return texts; }

	void add_text(const std::string& text, const ivec2& position, const cgv::render::TextAlignment alignment) {
		if(!msdf_font_ptr)
			return;
		
		float acc_advance = 0.0f;
		
		vec2 text_size(0.0f, render_font_size);

		for(char c : text) {
			const msdf_font::glyph_info& g = msdf_font_ptr->get_glyph_info(static_cast<unsigned char>(c));
			
			vec2 position = g.position * render_font_size + vec2(acc_advance, 0.0f);
			vec2 size = g.size * render_font_size;

			text_size.x() = acc_advance + size.x();
			acc_advance += g.advance * render_font_size;

			add_vertex(vec4(position.x(), position.y(), size.x(), size.y()), g.texcoords);
		}
		
		end_text(position, text_size, alignment);

		state_out_of_date = true;
	}

	bool create(cgv::render::context& ctx) {
		if(ssbo != 0) {
			glDeleteBuffers(1, &ssbo);
			ssbo = 0;
		}

		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, vertices.size() * sizeof(vertex_type), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		state_out_of_date = false;
		return true;
	}

	bool enable(cgv::render::context& ctx) {
		if(state_out_of_date) {
			create(ctx);
		}

		if(!msdf_font_ptr) {
			std::cerr << "msdf_text_geometry::enable: msdf_font is not specified" << std::endl;
			return false;
		}

		bool success = msdf_font_ptr->enable(ctx);
		if(success)
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
		return success;
	}

	void disable(cgv::render::context& ctx) {
		msdf_font_ptr->disable(ctx);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	/*void render(cgv::render::context& ctx, cgv::render::shader_program& prog) {
		if(out_of_date) {
			create(ctx);
		}

		prog.enable(ctx);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

		for(const auto& text : texts) {
			vec2 position(text.position);
			vec2 size = text.size;

			position -= 0.5f * size;

			if(text.alignment & cgv::render::TA_LEFT)
				position.x() += 0.5f * size.x();
			else if(text.alignment & cgv::render::TA_RIGHT)
				position.x() -= 0.5f * size.x();

			if(text.alignment & cgv::render::TA_TOP)
				position.y() -= 0.5f * size.y();
			else if(text.alignment & cgv::render::TA_BOTTOM)
				position.y() += 0.5f * size.y();

			prog.set_uniform(ctx, "position", ivec2(round(position)));
			glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP, 0, 4, text.count, text.offset);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		prog.disable(ctx);
	}*/
};



class msdf_font_renderer : public cgv::render::render_types {
protected:
	cgv::render::shader_program prog;

	bool build_shader_program(const cgv::render::context& ctx) {
		return prog.build_program(ctx, "sdf_font2d.glpr", true);// , defines);
	}

public:
	bool init(cgv::render::context& ctx) {
		return build_shader_program(ctx);
	}

	bool enable(cgv::render::context& ctx) {
		return prog.enable(ctx);
	}

	bool disable(cgv::render::context& ctx) {
		return prog.disable(ctx);
	}

	void draw(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg) {
		if(!tg.enable(ctx))
			return;

		for(const auto& text : tg.ref_texts()) {
			vec2 position(text.position);
			vec2 size = text.size;

			position -= 0.5f * size;

			if(text.alignment & cgv::render::TA_LEFT)
				position.x() += 0.5f * size.x();
			else if(text.alignment & cgv::render::TA_RIGHT)
				position.x() -= 0.5f * size.x();

			if(text.alignment & cgv::render::TA_TOP)
				position.y() -= 0.5f * size.y();
			else if(text.alignment & cgv::render::TA_BOTTOM)
				position.y() += 0.5f * size.y();

			const msdf_font* mf = tg.get_msdf_font();

			prog.set_uniform(ctx, "resolution", viewport_resolution);
			prog.set_uniform(ctx, "tex", 0);
			prog.set_uniform(ctx, "position", ivec2(round(position)));
			//prog.set_uniform(ctx, "src_size", mf->get_initial_font_size());
			//prog.set_uniform(ctx, "pixel_range", mf->get_pixel_range());
			//prog.set_uniform(ctx, "font_size", tg.get_font_size());
			glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP, (GLint)0, (GLsizei)4, (GLsizei)text.count, (GLuint)text.offset);
		}

		tg.disable(ctx);
	}

	bool render(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg) {
		if(!enable(ctx))
			return false;
		draw(ctx, viewport_resolution, tg);
		return disable(ctx);
	}
};

}
}

#include <cgv/config/lib_end.h>
