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
		vec2 position;
		vec2 size;
		cgv::render::TextAlignment alignment;
		float angle;

		text_info() : text_info("", vec2(0.0f), vec2(1.0f)) {}

		text_info(const std::string& str, const vec2& position, const vec2& size, const cgv::render::TextAlignment alignment = cgv::render::TA_NONE, float angle = 0.0f)
			: str(str), position(position), size(size), alignment(alignment), angle(angle) {}
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

	void update_offsets(size_t begin);

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

	template<typename T>
	void set_position(unsigned i, const cgv::math::fvec<T, 2>& position) {
		if(i < texts.size())
			texts[i].position = static_cast<vec2>(position);
	}

	void set_alignment(unsigned i, const cgv::render::TextAlignment alignment);

	void set_scale(unsigned i, float scale);

	void set_angle(unsigned i, const float angle);

	size_t size() const { return texts.size(); }

	const std::vector<text_info>& ref_texts() const { return texts; }

	vec2 get_text_render_size(unsigned i, float font_size) const;

	template<typename T>
	void add_text(const std::string& str, const cgv::math::fvec<T, 2>& position, const cgv::render::TextAlignment alignment = cgv::render::TA_NONE, float scale = 1.0f, float angle = 0.0f) {
		int offset = 0;
		if(texts.size() > 0) {
			const text_info& last_text = texts.back();
			offset = int(last_text.offset + last_text.str.size());
		}

		texts.emplace_back(str, static_cast<vec2>(position), vec2(compute_length(str), scale), alignment, angle);
		texts.back().offset = offset;

		state_out_of_date = true;
	}

	bool create(cgv::render::context& ctx);

	bool enable(cgv::render::context& ctx);

	void disable(cgv::render::context& ctx);
};

}
}

#include <cgv/config/lib_end.h>
