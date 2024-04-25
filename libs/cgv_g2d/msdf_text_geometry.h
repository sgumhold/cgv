#pragma once

#include <cgv/render/context.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/render/vertex_buffer.h>
#include "msdf_font.h" 

#include "lib_begin.h"

namespace cgv {
namespace g2d {

class CGV_API msdf_text_geometry {
public:
	struct text_info {
		std::string str = "";
		int offset = 0;
		vec2 position = vec2(0.0f);
		vec2 size = vec2(0.0f);
		cgv::render::TextAlignment alignment = cgv::render::TextAlignment::TA_NONE;
		float angle = 0.0f;
		rgba color = rgba(0.0f, 0.0f, 0.0f, 1.0f);

		text_info() : text_info("", vec2(0.0f), vec2(1.0f)) {}

		text_info(const std::string& str, const vec2& position, const vec2& size, const cgv::render::TextAlignment alignment = cgv::render::TA_NONE, float angle = 0.0f, rgba color = rgba(0.0f, 0.0f, 0.0f, 1.0f))
			: str(str), position(position), size(size), alignment(alignment), angle(angle), color(color) {}
	};

protected:
	msdf_font::FontFace font_face = msdf_font::FontFace::FF_REGULAR;
	msdf_font* msdf_font_ptr = nullptr;
	msdf_font* custom_msdf_font_ptr = nullptr;

	std::vector<text_info> texts;
	std::vector<vec4> vertices;

	cgv::render::vertex_buffer geometry_buffer;
	bool state_out_of_date = true;

	msdf_font& handle_font_ref(cgv::render::context& ctx, int ref_count_change);

	msdf_font& ref_font() const;

	void update_offsets(size_t begin);

	void create_vertex_data();

public:
	msdf_text_geometry();

	msdf_text_geometry(msdf_font::FontFace font_face);

	~msdf_text_geometry();

	bool init(cgv::render::context& ctx);

	void destruct(cgv::render::context& ctx);

	void clear();

	bool is_created() const { return !state_out_of_date; }

	const msdf_font& get_msdf_font() { return ref_font(); }

	void set_msdf_font(msdf_font* font_ptr, bool update_texts = true);

	void set_text(unsigned i, const std::string& text);

	template<typename T>
	void set_position(unsigned i, const cgv::math::fvec<T, 2>& position) {
		if(i < texts.size())
			texts[i].position = static_cast<vec2>(position);
	}

	void set_alignment(unsigned i, const cgv::render::TextAlignment alignment);

	void set_scale(unsigned i, float scale);

	void set_angle(unsigned i, const float angle);

	void set_color(unsigned i, const rgba color);

	size_t size() const { return texts.size(); }

	const std::vector<text_info>& ref_texts() const { return texts; }

	vec2 get_text_render_size(unsigned i, float font_size, size_t length = std::string::npos) const;

	template<typename T>
	void add_text(const std::string& str, const cgv::math::fvec<T, 2>& position, const cgv::render::TextAlignment alignment = cgv::render::TA_NONE, float scale = 1.0f, float angle = 0.0f, rgba color = rgba(0.0f, 0.0f, 0.0f, 1.0f)) {
		int offset = 0;
		if(texts.size() > 0) {
			const text_info& last_text = texts.back();
			offset = int(last_text.offset + last_text.str.size());
		}

		texts.emplace_back(str, static_cast<vec2>(position), vec2(ref_font().compute_length(str), scale), alignment, angle, color);
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
