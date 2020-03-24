#pragma once

#include <cgv/render/render_types.h>
#include <cgv/media/font/font.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cgv/render/texture.h>
#include <cgv/render/frame_buffer.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

struct label
{
protected:
	int width, height;
	friend class label_manager;
public:
	std::string text;
	cgv::render::render_types::rgba background_color;
	int border_x, border_y;
	int get_width() const { return std::abs(width); }
	int get_height() const { return std::abs(height); }
};

enum LabelState
{
	LS_CURRENT = 0,
	LS_NEW_TEXT = 1,
	LS_NEW_SIZE = 2,
	LS_NEW_COLOR = 4
};

class CGV_API label_manager : public cgv::render::render_types
{
public:
	typedef cgv::media::axis_aligned_box<int32_t, 2> ibox2;
protected:
	std::vector<label> labels;
	std::vector<ibox2> tex_ranges;
	int tex_width, tex_height;
	/// this is the final texture with all labels in
	std::shared_ptr<cgv::render::texture> tex;
	cgv::render::frame_buffer fbo;
	/// this is a temporary rotated texture into which the rotated labels are drawn
	cgv::render::texture tmp_tex;
	cgv::render::frame_buffer tmp_fbo;

	std::vector<uint8_t> label_states;
	bool packing_outofdate;
	bool texture_outofdate;
	bool texture_content_outofdate;
	std::vector<uint32_t> not_rotated_labels;
	std::vector<uint32_t> rotated_labels;
	cgv::media::font::font_face_ptr font_face;
	float font_size;
	int safety_extension;
	rgba text_color;
	cgv::render::surface_render_style rrs;
	bool ensure_tex_fbo_combi(cgv::render::context& ctx, cgv::render::texture& tex, cgv::render::frame_buffer& fbo, int width, int height);
	void draw_label_backgrounds(cgv::render::context& ctx, const std::vector<uint32_t>& indices, bool all, bool swap);
	void draw_label_texts(cgv::render::context& ctx, const std::vector<uint32_t>& indices, int height, bool all, bool swap);
public:
	label_manager(cgv::media::font::font_face_ptr _font_face = 0, float _font_size = -1);
	/// set the number of texels by which labels are extended in texture space to avoid texture filtering problems at label boundaries, defaults to 4
	void set_safety_extension(int nr_texels) { safety_extension = nr_texels; packing_outofdate = true; }
	/// return number of texels by which labels are extended in texture space to avoid texture filtering problems
	int get_safety_extension() const { return safety_extension; }
	/// set default font face active at begin of each label
	void set_font_face(cgv::media::font::font_face_ptr _font_face);
	/// set default font size active at begin of each label
	void set_font_size(float _font_size);
	/// set default text color active at begin of each label, defaults to opaque black
	void set_text_color(const rgba& clr);
	/// add a label
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