#pragma once

#include <cgv/render/texture.h>
#include <cgv_glutil/canvas_overlay.h>
#include <cgv_glutil/msdf_gl_canvas_font_renderer.h>
#include <cgv_glutil/2d/draggables_collection.h>
#include <cgv_glutil/2d/shape2d_styles.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil{

class CGV_API color_selector : public canvas_overlay {
protected:
	struct layout_attributes {
		int padding;
		int size = 280;

		// dependent members
		rect border_rect;
		rect color_rect;
		rect hue_rect;
		rect opacity_rect;
		rect preview_rect;

		rect hue_constraint;
		rect opacity_constraint;
	} layout;
	
	struct selector_handle : public draggable {
		vec2 val = vec2(0.0f);
		bool is_rectangular = false;

		selector_handle() {
			size = vec2(10.0f);
			position_is_center = true;
			constraint_reference = CR_CENTER;
		}

		void update_val() {
			if(constraint) {
				const rect& c = *constraint;
				vec2 p = pos - static_cast<vec2>(c.pos());
				ivec2 size = c.size();
				val = p / static_cast<vec2>(size);
				if(size.x() == 0) val.x() = 0.0f;
				if(size.y() == 0) val.y() = 0.0f;
				val = cgv::math::clamp(val, 0.0f, 1.0f);
			}
		}

		void update_pos() {
			if(constraint) {
				const rect& c = *constraint;
				val = cgv::math::clamp(val, 0.0f, 1.0f);
				pos = static_cast<vec2>(c.pos()) + val * c.size();
			}
		}

		bool is_inside(const vec2& mp) const {
			if(is_rectangular) {
				return draggable::is_inside(mp);
			} else {
				const auto dist = length(mp - center());
				return dist <= 0.5f*size.x();
			}
		}
	};


	bool has_updated = false;
	bool has_opacity = false;

	msdf_text_geometry texts;

	cgv::glutil::shape2d_style container_style, border_style, color_texture_style, hue_texture_style, opacity_color_style, color_handle_style, hue_handle_style, text_style;
	cgv::glutil::grid2d_style opacity_bg_style;

	cgv::glutil::draggables_collection<selector_handle> selector_handles;

	texture color_tex;
	texture hue_tex;

	rgb rgb_color = rgba(0.0f);
	rgba rgba_color = rgba(0.0f);

	void update_layout(const ivec2& parent_size);

	void init_styles(context& ctx);
	void init_textures(context& ctx);
	void update_color_texture();
	void update_color();
	void update_texts();

	void handle_selector_drag();

	void set_color(rgba color, bool opacity);
	
public:
	color_selector();
	std::string get_type_name() const { return "color_selector"; }

	void clear(cgv::render::context& ctx);

	void stream_help(std::ostream& os) {}

	bool handle_event(cgv::gui::event& e);
	void on_set(void* member_ptr);

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw_content(cgv::render::context& ctx);
	
	void create_gui();

	bool was_updated();

	void set_rgb_color(rgb color);
	void set_rgba_color(rgba color);
	rgb get_rgb_color() const { return rgb_color; }
	rgba get_rgba_color() const { return rgba_color; }
};

typedef cgv::data::ref_ptr<color_selector> color_selector_ptr;

}
}

#include <cgv/config/lib_end.h>
