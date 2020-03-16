#pragma once

#include <cgv/render/render_types.h>
#include <cgv/media/font/font.h>
#include <cgv/render/texture.h>
#include <cgv/render/frame_buffer.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

struct label
{
	std::string text;
	cgv::render::render_types::rgba background_color;
	int border_x, border_y;
	int width, height;
};

enum LabelState
{
	LS_CURRENT = 0,
	LS_NEW_TEXT = 1,
	LS_NEW_SIZE = 2
};

class CGV_API label_manager : public cgv::render::render_types
{
protected:
	std::vector<label> labels;
	std::vector<ivec4> tex_ranges;
	int tex_width, tex_height;
	std::shared_ptr<cgv::render::texture> tex;
	cgv::render::frame_buffer fbo;
	std::vector<uint8_t> label_states;
	bool packing_outofdate;
	bool texture_outofdate;
	cgv::media::font::font_face_ptr font_face;
	float font_size;
public:
	label_manager(cgv::media::font::font_face_ptr _font_face = 0, float _font_size = -1);
	void set_font_face(cgv::media::font::font_face_ptr _font_face);
	void set_font_size(float _font_size);
	void add_label(const std::string& text,
		const rgba& bg_clr, int _border_x = 4, int _border_y = 4,
		int _width = -1, int height = -1);
	void compute_label_sizes();
	void pack_labels();
	vec4 get_texcoord_range(uint32_t label_index);
	uint32_t get_nr_labels() const { return labels.size(); }
	const label& get_label(uint32_t i) const { return labels[i]; }
	void update_label_text(uint32_t i, const std::string& new_text);
	void update_label_size(uint32_t i, int w, int h);
	void set_texture_outofdate() { texture_outofdate = true; }
	void init(cgv::render::context& ctx);
	void draw_labels(cgv::render::context& ctx, bool all);
	void destruct(cgv::render::context& ctx);
	void ensure_texture_uptodate(cgv::render::context& ctx);
	std::shared_ptr<cgv::render::texture> get_texture() const { return tex; }
};

	}
}

#include <cgv/config/lib_end.h>