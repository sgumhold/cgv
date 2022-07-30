#pragma once

#include <cgv/render/texture.h>
#include <cgv/utils/convert_string.h>
#include <cgv_glutil/canvas_overlay.h>
#include <cgv_glutil/color_map.h>
#include <cgv_glutil/generic_2d_renderer.h>
#include <cgv_glutil/msdf_gl_font_renderer.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API color_map_legend : public canvas_overlay {
protected:
	enum OrientationOption {
		OO_HORIZONTAL,
		OO_VERTICAL
	};

	struct layout_attributes {
		int padding;
		int label_space = 12;
		int x_label_size = 0;
		int title_space = 0;

		//int band_height;
		ivec2 total_size;

		OrientationOption orientation = OO_HORIZONTAL;
		AlignmentOption label_alignment = AO_END;

		// dependent members
		rect color_map_rect;
		ivec2 title_position = ivec2(0);
		float title_angle = 90.0f;

		void update(const ivec2& parent_size) {
			ivec2 offset(0, 0);
			ivec2 size(parent_size);

			switch(label_alignment) {
			case AO_START:
				if(orientation == OO_HORIZONTAL) {
					offset.x() = x_label_size / 2;
					offset.y() = title_space;
					size.x() -= x_label_size;
					size.y() -= label_space + title_space;
				} else {
					offset.x() = x_label_size + 4;
					offset.y() = 0;
					size.x() -= x_label_size + 4 + title_space;
				}
				break;
			case AO_END:
				if(orientation == OO_HORIZONTAL) {
					offset.x() = x_label_size / 2;
					offset.y() = label_space;
					size.x() -= x_label_size;
					size.y() -= label_space + title_space;
				} else {
					offset.x() = title_space;
					size.x() -= x_label_size + 4 + title_space;
				}
				break;
			default: break;
			}

			color_map_rect.set_pos(offset + padding);
			color_map_rect.set_size(size - 2 * padding);
		}
	} layout;

	bool show_background = true;
	bool invert_color = false;

	texture tex;

	std::string title;
	vec2 range;
	unsigned num_ticks;
	bool label_auto_precision;
	unsigned label_precision;
	AlignmentOption title_align;

	// general appearance
	shape2d_style container_style, border_style, color_map_style;

	// text appearance
	float font_size = 12.0f;

	shape2d_style text_style;
	msdf_font font;
	msdf_gl_font_renderer font_renderer;
	msdf_text_geometry labels;

	generic_2d_renderer tick_renderer;
	DEFINE_GENERIC_RENDER_DATA_CLASS(tick_geometry, 2, vec2, position, vec2, size);

	tick_geometry ticks;

	void init_styles(context& ctx);
	void create_labels();
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
	void draw_content(cgv::render::context& ctx);

	void create_gui();

	void set_color_map(cgv::render::context & ctx, color_map& cm);

	void set_width(size_t w);
	void set_height(size_t h);

	void set_title(const std::string& t);

	vec2 get_range() { return range; }
	void set_range(vec2 r);

	unsigned get_num_ticks() { return num_ticks; }
	void set_num_ticks(unsigned n);

	void set_label_auto_precision(bool enabled);
	void set_label_precision(unsigned p);
};

typedef cgv::data::ref_ptr<color_map_legend> color_map_legend_ptr;

}
}

#include <cgv/config/lib_end.h>
