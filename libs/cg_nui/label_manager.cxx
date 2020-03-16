#include "label_manager.h"
#include <rect_pack/rect_pack.h>
#include <cgv_gl/gl/gl.h>

namespace cgv {
	namespace nui {

label_manager::label_manager(cgv::media::font::font_face_ptr _font_face, float _font_size)
{
	font_face = _font_face;
	font_size = _font_size;
	tex_width = -1;
	tex_height = -1;
	std::vector<uint8_t> label_states;
	packing_outofdate = true;
	texture_outofdate = true;
	tex = std::make_shared<cgv::render::texture>("[R,G,B,A]");
}

void label_manager::set_font_face(cgv::media::font::font_face_ptr _font_face)
{
	font_face = _font_face;
	packing_outofdate = true;
	texture_outofdate = true;
}

void label_manager::set_font_size(float _font_size)
{
	font_size = _font_size;
	packing_outofdate = true;
	texture_outofdate = true;
}

void label_manager::add_label(const std::string& _text,
	const rgba& bg_clr, int _border_x, int _border_y,
	int _width, int _height)
{
	label l;
	l.text = _text;
	l.background_color = bg_clr;
	l.border_x = _border_x;
	l.border_y = _border_y;
	l.width = _width;
	l.height = _height;
	labels.push_back(l);
	label_states.push_back(LS_NEW_SIZE + LS_NEW_TEXT);
	packing_outofdate = true;
	texture_outofdate = true;
}

void label_manager::compute_label_sizes()
{
	for (auto& l : labels) {
		if (l.width == -1) {
			int w = (int)ceil(font_face->measure_text_width(l.text, font_size));
			l.width = -(w + l.border_x);
		}
		if (l.height == -1) {
			int h = (int)ceil(1.2f * font_size);
			l.height = -(l.border_y + h);
		}
	}
}
void label_manager::pack_labels()
{
	std::vector<rect_pack::rectangle> rectangles;
	std::vector<rect_pack::rectangle_size> rectangle_sizes;
	rect_pack::rectangle_size S;
	for (const auto& l : labels) {
		S.width = std::abs(l.width);
		S.height = std::abs(l.height);
		rectangle_sizes.push_back(S);
	}
	unsigned width_out, height_out;
	rect_pack::pack_rectangles_iteratively(rectangle_sizes,
		width_out, height_out, rectangles);
	tex_width = width_out;
	tex_height = height_out;
	tex_ranges.clear();
	for (const auto& R : rectangles)
		tex_ranges.push_back(ivec4(R.x, R.y, R.width, R.height));
	packing_outofdate = false;
}
label_manager::vec4 label_manager::get_texcoord_range(uint32_t label_index)
{
	const auto& R = tex_ranges[label_index];
	return vec4(
		(float)R[0] / tex_width,
		(float)R[1] / tex_height,
		(float)(R[0] + R[2]) / tex_width,
		(float)(R[1] + R[3]) / tex_height);
}

void label_manager::update_label_text(uint32_t i, const std::string& new_text)
{
	labels[i].text = new_text;
	label_states[i] |= LS_NEW_TEXT;
	texture_outofdate = true;
	if (labels[i].width < 0 || labels[i].height < 0) {
		packing_outofdate = true;
		label_states[i] |= LS_NEW_SIZE;
	}
}

void label_manager::update_label_size(uint32_t i, int w, int h)
{
	labels[i].width = w;
	labels[i].height = h;
	label_states[i] |= LS_NEW_SIZE;
}

void label_manager::init(cgv::render::context& ctx)
{
	
}
void label_manager::draw_labels(cgv::render::context& ctx, bool all)
{
	if (!(tex->is_created() && tex->get_width() == tex_width &&
		tex->get_height() == tex_height)) {
		if (tex->is_created()) {
			fbo.destruct(ctx);
			tex->destruct(ctx);
		}
		tex->create(ctx, cgv::render::TT_2D, tex_width, tex_height);
	}
	if (!fbo.is_created()) {
		fbo.create(ctx, tex_width, tex_height);
		fbo.attach(ctx, *tex, 0);
	}
	if (!fbo.is_complete(ctx)) {
		std::cerr << "label_manager::draw_labels ... fbo not complete" << std::endl;
		abort();
	}

	fbo.enable(ctx, 0);
	ctx.push_window_transformation_array();
	ctx.set_viewport(ivec4(0, 0, tex_width, tex_height));
	ctx.push_pixel_coords();
	cgv::media::font::font_face_ptr old_font_face = 
		ctx.get_current_font_face();
	float old_font_size = ctx.get_current_font_size();

	ctx.enable_font_face(font_face, font_size);
	for (uint32_t i = 0; i < labels.size(); ++i) {
		if (!all && ((label_states[i] & LS_NEW_TEXT) == 0))
			continue;
		ctx.set_cursor(tex_ranges[i][0] + labels[i].border_x,
			tex_ranges[i][1] + labels[i].border_y);
		glScissor(tex_ranges[i][0], tex_ranges[i][1], tex_ranges[i][2], tex_ranges[i][3]);		
		ctx.output_stream() << labels[i].text;
		ctx.output_stream().flush();
	}
	ctx.enable_font_face(old_font_face, old_font_size);

	ctx.pop_pixel_coords();
	ctx.pop_window_transformation_array();
	fbo.disable(ctx);
	texture_outofdate = false;
}

void label_manager::destruct(cgv::render::context& ctx)
{
	tex->destruct(ctx);
	tex = 0;
	fbo.destruct(ctx);
}

void label_manager::ensure_texture_uptodate(cgv::render::context& ctx)
{
	if (packing_outofdate) {
		compute_label_sizes();
		pack_labels();
	}
	if (texture_outofdate)
		draw_labels(ctx, packing_outofdate);
}

	}
}