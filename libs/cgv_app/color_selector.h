#pragma once

#include <cgv/render/texture.h>
#include <cgv_app/canvas_overlay.h>
#include <cgv_g2d/draggables_collection.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>
#include <cgv_g2d/shape2d_styles.h>

#include "lib_begin.h"

namespace cgv {
namespace app {

class CGV_API color_selector : public canvas_overlay {
protected:
	struct layout_attributes {
		int padding;
		int size = 280;

		// dependent members
		cgv::g2d::rect border_rect;
		cgv::g2d::rect color_rect;
		cgv::g2d::rect hue_rect;
		cgv::g2d::rect opacity_rect;
		cgv::g2d::rect preview_rect;

		cgv::g2d::rect hue_constraint;
		cgv::g2d::rect opacity_constraint;
	} layout;
	
	struct selector_handle : public cgv::g2d::draggable {
		vec2 val = vec2(0.0f);
		bool is_rectangular = false;

		selector_handle() {
			size = vec2(10.0f);
			position_is_center = true;
			constraint_reference = CR_CENTER;
		}

		void update_val() {
			if(constraint) {
				const cgv::g2d::rect& c = *constraint;
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
				const cgv::g2d::rect& c = *constraint;
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

	bool has_opacity = false;
	bool auto_show = true;

	cgv::g2d::msdf_text_geometry texts;

	cgv::g2d::shape2d_style container_style, border_style, color_texture_style, hue_texture_style, opacity_color_style, color_handle_style, hue_handle_style, text_style;
	cgv::g2d::grid2d_style opacity_bg_style;

	cgv::g2d::draggables_collection<selector_handle> selector_handles;

	cgv::render::texture color_tex;
	cgv::render::texture hue_tex;

	rgb rgb_color = rgb(0.0f);
	rgba rgba_color = rgba(0.0f);

	void update_layout(const ivec2& parent_size);

	void init_styles(cgv::render::context& ctx);
	void init_textures(cgv::render::context& ctx);
	void update_color_texture();
	void update_color();
	void update_texts();

	void handle_selector_drag();

	void set_color(rgba color, bool opacity, bool init = false);

	std::function<void(rgb)> on_change_rgb_callback;
	std::function<void(rgba)> on_change_rgba_callback;

	virtual void create_gui_impl();
	
public:
	color_selector();
	std::string get_type_name() const { return "color_selector"; }

	void clear(cgv::render::context& ctx);

	bool handle_event(cgv::gui::event& e);
	void on_set(void* member_ptr);

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw_content(cgv::render::context& ctx);

	/// whether to automatically make the overlay visible if a color is set
	void set_auto_show(bool enabled) { auto_show = enabled; }

	void set_rgb_color(rgb color);
	void set_rgba_color(rgba color);
	rgb get_rgb_color() const { return rgb_color; }
	rgba get_rgba_color() const { return rgba_color; }

	void set_on_change_rgb_callback(std::function<void(rgb)> cb) { on_change_rgb_callback = cb; }
	void set_on_change_rgba_callback(std::function<void(rgba)> cb) { on_change_rgba_callback = cb; }
};

typedef cgv::data::ref_ptr<color_selector> color_selector_ptr;

}
}

#include <cgv/config/lib_end.h>
