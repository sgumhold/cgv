#pragma once

#include <cgv/render/context.h>
#include <cgv_gl/gl/gl.h>
#include "msdf_font.h" 

#include "lib_begin.h"

namespace cgv {
namespace g2d {

class CGV_API msdf_text_geometry : public cgv::render::render_types {
protected:
	struct text_info {
		std::string str;
		int offset;
		ivec2 position;
		vec2 size;
		cgv::render::TextAlignment alignment;
		float angle;
	};

	struct vertex_type {
		vec4 position_size;
		vec4 texcoords;
	};

	// TODO: use a ref_ptr?
	msdf_font* msdf_font_ptr;

	GLuint ssbo;
	bool state_out_of_date;

	std::vector<text_info> texts;
	std::vector<vertex_type> vertices;

	float compute_length(const std::string& str) const;

	void end_text(text_info text);

	void update_offsets_and_counts();

	void add_vertex(const vec4& pos, const vec4& txc);

	void create_vertex_data();

public:
	msdf_text_geometry();

	~msdf_text_geometry();

	void clear();

	bool is_created() const { return !state_out_of_date; }

	const msdf_font* get_msdf_font() { return msdf_font_ptr; }

	void set_msdf_font(msdf_font* ptr, bool update_texts = true);

	void set_text(unsigned i, const std::string& text);

	void set_position(unsigned i, const ivec2& position);

	void set_alignment(unsigned i, const cgv::render::TextAlignment alignment);

	void set_angle(unsigned i, const float angle);

	size_t size() const { return texts.size(); }

	const std::vector<text_info>& ref_texts() const { return texts; }

	vec2 get_text_render_size(unsigned i, float font_size) const;

	void add_text(const std::string& str, const ivec2& position = ivec2(0), const cgv::render::TextAlignment alignment = cgv::render::TA_NONE, float angle = 0.0f);

	bool create(cgv::render::context& ctx);

	bool enable(cgv::render::context& ctx);

	void disable(cgv::render::context& ctx);
};

}
}

#include <cgv/config/lib_end.h>
