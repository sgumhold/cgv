#include "msdf_text_geometry.h"

namespace cgv {
namespace g2d {

bool msdf_text_geometry::init(cgv::render::context& ctx) {
	bool res = attribute_arrays.init(ctx);
	msdf_font_ptr = &handle_font_ref(ctx, 1);
	res &= msdf_font_ptr->is_initialized();
	return res;
}

void msdf_text_geometry::destruct(cgv::render::context& ctx) {
	attribute_arrays.destruct(ctx);
	handle_font_ref(ctx, -1);
	msdf_font_ptr = nullptr;
	custom_msdf_font_ptr = nullptr;
}

void msdf_text_geometry::clear() {
	text_infos.clear();
	positions.clear();
	colors.clear();
	scales.clear();
	rotations.clear();
	alignments.clear();
}

void msdf_text_geometry::set_msdf_font(const cgv::render::context& ctx, msdf_font* font_ptr) {
	custom_msdf_font_ptr = font_ptr;

	if(custom_msdf_font_ptr) {
		std::vector<std::string> texts;
		texts.reserve(text_infos.size());
		std::transform(text_infos.begin(), text_infos.end(), std::back_inserter(texts), [](const auto& info) { return info.str; });
		set_text_array(ctx, texts);
	}
}

void msdf_text_geometry::set_text_array(const cgv::render::context& ctx, const std::vector<std::string>& texts) {
	const int quad_attrib_index = 0;
	const int texcoord_attrib_index = 1;

	text_infos.clear();
	text_infos.reserve(texts.size());

	size_t total_glyph_count = 0;
	for(const auto& text : texts)
		total_glyph_count += text.size();

	if(total_glyph_count == 0) {
		attribute_arrays.remove_attribute_array(ctx, quad_attrib_index);
		attribute_arrays.remove_attribute_array(ctx, texcoord_attrib_index);
		return;
	}

	std::vector<cgv::vec4> quads;
	std::vector<cgv::vec4> texcoords;

	quads.reserve(total_glyph_count);
	texcoords.reserve(total_glyph_count);

	int last_offset = 0;
	for(const auto& text : texts) {
		text_info info;
		info.str = text;
		info.offset = last_offset;
		info.normalized_width = ref_font().compute_normalized_length(text);
		text_infos.push_back(info);

		last_offset += text.length();

		ref_font().generate_vertex_data(text, quads, texcoords);
	}

	attribute_arrays.set_attribute_array(ctx, quad_attrib_index, quads);
	attribute_arrays.set_attribute_array(ctx, texcoord_attrib_index, texcoords);
}

vec2 msdf_text_geometry::compute_text_render_size(size_t index, float font_size) const {
	const text_info& info = text_infos[index];

	float width = info.normalized_width;

	//if(length != std::string::npos && length < text.str.length()) {
		//std::string str = text.str.substr(0, length);
		//size_x = ref_font().compute_normalized_length(str);
	//}

	return get_scale(index) * font_size * vec2(width, 1.0f);
}

bool msdf_text_geometry::enable(cgv::render::context& ctx) {
	bool res = attribute_arrays.enable(ctx);
	// advance vertex attributes once per instance
	// TODO: Ideally the divisors are set only once in the set_text_array() method when the vertex arrays are populated.
	// However, this functionality is not yet supported by the context and we need to do it right here.
	glVertexAttribDivisor(0, 1);
	glVertexAttribDivisor(1, 1);
	return res;
}

bool msdf_text_geometry::disable(cgv::render::context& ctx) {
	return attribute_arrays.disable(ctx);
}

msdf_font& msdf_text_geometry::handle_font_ref(cgv::render::context& ctx, int ref_count_change) {
	switch(font_face) {
	case msdf_font::FontFace::FF_LIGHT:
		return ref_msdf_font_light(ctx, ref_count_change);
	case msdf_font::FontFace::FF_BOLD:
		return ref_msdf_font_bold(ctx, ref_count_change);
	case msdf_font::FontFace::FF_REGULAR:
	default:
		return ref_msdf_font_regular(ctx, ref_count_change);
	}
}

msdf_font& msdf_text_geometry::ref_font() const {
	if(custom_msdf_font_ptr)
		return *custom_msdf_font_ptr;
	else
		return *msdf_font_ptr;
}

}
}
