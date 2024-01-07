#include "msdf_text_geometry.h"

namespace cgv {
namespace g2d {

msdf_text_geometry::msdf_text_geometry() {
	geometry_buffer = cgv::render::vertex_buffer(cgv::render::VBT_STORAGE);
};

msdf_text_geometry::msdf_text_geometry(msdf_font::FontFace font_face) : msdf_text_geometry() {
	this->font_face = font_face;
}

msdf_text_geometry::~msdf_text_geometry() {
	clear();
};

bool msdf_text_geometry::init(cgv::render::context& ctx) {
	msdf_font_ptr = &handle_font_ref(ctx, 1);
	return msdf_font_ptr->is_initialized();
}

void msdf_text_geometry::destruct(cgv::render::context& ctx) {
	handle_font_ref(ctx, -1);
	msdf_font_ptr = nullptr;
	custom_msdf_font_ptr = nullptr;
	geometry_buffer.destruct(ctx);
}

void msdf_text_geometry::clear() {
	state_out_of_date = true;

	texts.clear();
	vertices.clear();
}

void msdf_text_geometry::set_msdf_font(msdf_font* font_ptr, bool update_texts) {
	custom_msdf_font_ptr = font_ptr;

	if(custom_msdf_font_ptr && update_texts) {
		for(unsigned i = 0; i < texts.size(); ++i)
			set_text(i, texts[i].str);
	}
}

void msdf_text_geometry::set_text(unsigned i, const std::string& text) {
	if(i < texts.size()) {
		texts[i].str = text;
		texts[i].size.x() = ref_font().compute_length(text);
		
		update_offsets(i);

		state_out_of_date = true;
	}
}

void msdf_text_geometry::set_alignment(unsigned i, const cgv::render::TextAlignment alignment) {
	if(i < texts.size())
		texts[i].alignment = alignment;
}

void msdf_text_geometry::set_scale(unsigned i, float scale) {
	if(i < texts.size())
		texts[i].size.y() = scale;
}

void msdf_text_geometry::set_angle(unsigned i, const float angle) {
	if(i < texts.size())
		texts[i].angle = angle;
}

void msdf_text_geometry::set_color(unsigned i, const rgba color) {
	if(i < texts.size())
		texts[i].color = color;
}

vec2 msdf_text_geometry::get_text_render_size(unsigned i, float font_size, size_t length) const {

	if(i < texts.size()) {
		const text_info& text = texts[i];

		float size_x = text.size.x();

		if(length != std::string::npos && length < text.str.length()) {
			std::string str = text.str.substr(0, length);
			size_x = ref_font().compute_length(str);
		}

		return font_size * vec2(size_x * text.size.y(), text.size.y());
	}

	return vec2(0.0f);
}

bool msdf_text_geometry::create(cgv::render::context& ctx) {
	create_vertex_data();

	if(vertices.empty()) {
		geometry_buffer.destruct(ctx);
		state_out_of_date = false;
	} else {
		state_out_of_date = !geometry_buffer.create_or_resize(ctx, vertices);
	}
	
	return !state_out_of_date;
}

bool msdf_text_geometry::enable(cgv::render::context& ctx) {
	if(state_out_of_date)
		create(ctx);

	geometry_buffer.bind(ctx, 0);
	return ref_font().enable(ctx);
}

void msdf_text_geometry::disable(cgv::render::context& ctx) {
	ref_font().disable(ctx);
	geometry_buffer.unbind(ctx, 0);
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

void msdf_text_geometry::update_offsets(size_t begin) {
	if(begin < texts.size()) {
		int offset = texts[begin].offset + static_cast<int>(texts[begin].str.size());

		for(size_t i = begin + 1; i < texts.size(); ++i) {
			text_info& text = texts[i];
			text.offset = offset;
			offset += int(text.str.size());
		}
	}
}

void msdf_text_geometry::create_vertex_data() {
	vertices.clear();

	size_t glyph_count = 0;
	for(text_info& text : texts)
		glyph_count += text.str.length();

	vertices.reserve(2 * glyph_count);

	for(text_info& text : texts) {
		auto vertex_data = ref_font().create_vertex_data(text.str);
		vertices.insert(vertices.end(), vertex_data.begin(), vertex_data.end());
	}
}

}
}
