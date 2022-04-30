#pragma once

#include <cgv/render/texture.h>
#include <cgv_glutil/frame_buffer_container.h>
#include <cgv_glutil/overlay.h>
#include <cgv_glutil/color_map.h>
#include <cgv_glutil/2d/canvas.h>
#include <cgv_glutil/2d/shape2d_styles.h>
#include <cgv_glutil/msdf_gl_font_renderer.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API color_map_legend : public cgv::glutil::overlay {
protected:
	int last_theme_idx = -1;
	cgv::glutil::frame_buffer_container fbc;

	cgv::glutil::canvas canvas, overlay_canvas;
	cgv::glutil::shape2d_style container_style, border_style, color_map_style;

	struct layout_attributes {
		int padding;
		int band_height;
		int total_height;

		// dependent members
		cgv::glutil::rect color_map_rect;

		void update(const ivec2& parent_size) {
			color_map_rect.set_pos(ivec2(padding));
			color_map_rect.set_size(parent_size - 2 * padding);
		}
	} layout;

	//bool texts_out_of_date = false;
	texture* texture_ptr;

	vec2 range;
	unsigned num_ticks;
	std::vector<unsigned> ticks;

	// text appearance
	float font_size = 14.0f;
	cgv::render::TextAlignment text_align_h, text_align_v;

	cgv::glutil::shape2d_style text_style;
	cgv::glutil::msdf_font msdf_font;
	cgv::glutil::msdf_text_geometry texts;
	cgv::glutil::msdf_gl_font_renderer font_renderer;

	//vec2 padding;
	//uvec2 size;

	//bool background;
	//bool show;
	//bool transparent;

	void init_styles(context& ctx);
	void create_ticks();
	//void update_texts();

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

	void set_color_map(cgv::glutil::gl_color_map& color_map);

	void set_color_map_texture(texture* tex);

	void set_range(vec2 range);

	void set_num_ticks(unsigned n);
};

}
}

#include <cgv/config/lib_end.h>
