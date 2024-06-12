#pragma once

#include <cgv_gl/attribute_array_manager.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>

#include "msdf_font.h" 

#include "lib_begin.h"

namespace cgv {
namespace g2d {

class CGV_API msdf_text_geometry {
public:
	struct text_info {
		std::string str = "";
		unsigned offset = 0;
		float normalized_width = 0.0f;
	};

private:
	msdf_font::FontFace font_face = msdf_font::FontFace::FF_REGULAR;
	msdf_font* msdf_font_ptr = nullptr;
	msdf_font* custom_msdf_font_ptr = nullptr;

	std::vector<text_info> text_infos;

	/// the attribute array manager storing the data in GL buffers
	cgv::render::attribute_array_manager attribute_arrays;

	msdf_font& handle_font_ref(cgv::render::context& ctx, int ref_count_change);

	msdf_font& ref_font() const;

public:
	std::vector<vec3> positions;
	std::vector<rgba> colors;
	std::vector<float> scales;
	std::vector<quat> rotations;
	std::vector<cgv::render::TextAlignment> alignments;
	
	vec3 position = { 0.0f };
	rgba color = rgba(0.0f, 0.0f, 0.0f, 1.0f);
	float scale = 1.0f;
	quat rotation;
	cgv::render::TextAlignment alignment = cgv::render::TextAlignment::TA_NONE;

	msdf_text_geometry() {}

	msdf_text_geometry(msdf_font::FontFace font_face) : font_face(font_face) {}

	bool init(cgv::render::context& ctx);

	void destruct(cgv::render::context& ctx);

	void clear();

	size_t size() const { return text_infos.size(); }

	bool empty() const { return text_infos.empty(); }

	msdf_font& ref_msdf_font() { return ref_font(); }

	void set_msdf_font(const cgv::render::context& ctx, msdf_font* font_ptr);

	void set_text_array(const cgv::render::context& ctx, const std::vector<std::string>& texts);

	const std::vector<text_info>& ref_text_infos() const {
		return text_infos;
	}

	const text_info& get_text_info(size_t index) const {
		return text_infos[index];
	}

	vec3 get_position(size_t index) const {
		return index < positions.size() ? positions[index] : position;
	}

	rgba get_color(size_t index) const {
		return index < colors.size() ? colors[index] : color;
	}

	float get_scale(size_t index) const {
		return index < scales.size() ? scales[index] : scale;
	}

	quat get_rotation(size_t index) const {
		return index < rotations.size() ? rotations[index] : rotation;
	}

	cgv::render::TextAlignment get_alignment(size_t index) const {
		return index < alignments.size() ? alignments[index] : alignment;
	}

	vec2 compute_text_render_size(size_t index, float font_size) const;

	bool enable(cgv::render::context& ctx);

	bool disable(cgv::render::context& ctx);
};

}
}

#include <cgv/config/lib_end.h>
