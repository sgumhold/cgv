#pragma once

#include <cgv/render/texture.h>
#include <cgv_glutil/color_map.h>
#include <cgv_glutil/frame_buffer_container.h>
#include <cgv_glutil/generic_renderer.h>
#include <cgv_glutil/msdf_gl_font_renderer.h>
#include <cgv_glutil/overlay.h>
#include <cgv_glutil/2d/canvas.h>
#include <cgv_glutil/2d/shape2d_styles.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API color_map_legend : public cgv::glutil::overlay {
protected:
	int last_theme_idx = -1;
	frame_buffer_container fbc;

	canvas _canvas, overlay_canvas;
	shape2d_style container_style, border_style, color_map_style;

	enum OrientationOption {
		OO_HORIZONTAL,
		OO_VERTICAL
	};

	struct layout_attributes {
		int padding;
		int label_space = 12;
		//int band_height;
		int total_height;

		OrientationOption orientation;
		AlignmentOption label_alignment;

		// dependent members
		rect color_map_rect;

		void update(const ivec2& parent_size) {
			color_map_rect.set_pos(padding + label_space);
			color_map_rect.set_size(parent_size - 2 * padding - ivec2(2*label_space, label_space));
		}
	} layout;

	bool update_layout = true;

	//bool texts_out_of_date = false;
	//texture* texture_ptr;
	texture tex;

	vec2 range;
	unsigned num_ticks;

	// text appearance
	float font_size = 12.0f;
	cgv::render::TextAlignment text_align_h, text_align_v;

	shape2d_style text_style;
	msdf_font font;
	msdf_gl_font_renderer font_renderer;
	msdf_text_geometry labels;

	generic_renderer tick_renderer;
	DEFINE_GENERIC_RENDER_DATA_CLASS(tick_geometry, 2, vec2, position, vec2, size);

	tick_geometry ticks;

	//bool background;
	//bool show;
	//bool transparent;

	void init_styles(context& ctx);
	void create_ticks();

public:
	color_map_legend();
	std::string get_type_name() const { return "color_map_legend"; }

	void clear(cgv::render::context& ctx);

	bool self_reflect(cgv::reflect::reflection_handler& _rh);
	void stream_help(std::ostream& os) {}

	bool handle_event(cgv::gui::event& e);
	void on_set(void* member_ptr);

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);

	void create_gui();
	void create_gui(cgv::gui::provider& p);

	void set_color_map(cgv::render::context & ctx, color_map& cm);

	void set_range(vec2 range);

	void set_num_ticks(unsigned n);
};

}
}

#include <cgv/config/lib_end.h>
