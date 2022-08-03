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
		
		// dependent members
		rect border_rect;
		rect color_rect;
		rect hue_rect;
		rect preview_rect;

		void update(const ivec2& parent_size) {

			const int hue_rect_width = 20;

			border_rect.set_pos(ivec2(padding));
			border_rect.set_size(ivec2(parent_size - 2 * padding));
			border_rect.a() += ivec2(0, 23);

			rect content_rect = border_rect;
			content_rect.translate(1, 1);
			content_rect.resize(-2, -2);

			hue_rect.set_pos(content_rect.x1() - hue_rect_width, content_rect.y());
			hue_rect.set_size(hue_rect_width, content_rect.h());
			
			color_rect = content_rect;
			color_rect.resize(-21, 0);

			preview_rect.set_pos(ivec2(padding));
			preview_rect.set_size(20, 20);

		}
	} layout;
	
	struct color_point : public draggable {
		vec2 val;

		color_point() {
			size = vec2(16.0f);
			position_is_center = true;
			constraint_reference = CR_CENTER;
		}

		void update_val(const layout_attributes& la) {
			vec2 p = pos - static_cast<vec2>(la.color_rect.pos());
			val = p / static_cast<vec2>(la.color_rect.size());
			val = cgv::math::clamp(val, 0.0f, 1.0f);
		}

		void update_pos(const layout_attributes& la) {
			val = cgv::math::clamp(val, 0.0f, 1.0f);
			pos = static_cast<vec2>(la.color_rect.pos()) + val * la.color_rect.size();
		}

		bool is_inside(const vec2& mp) const {
			const auto dist = length(mp - center());
			return dist <= 0.5f*size.x();
		}

		ivec2 get_render_position() const {
			return ivec2(pos + 0.5f);
		}

		ivec2 get_render_size() const {
			return ivec2(size);
		}
	};

	struct hue_point : public draggable {
		float val;

		hue_point() {
			size = vec2(20.0f, 10.0f);
			position_is_center = false;
			constraint_reference = CR_MIN_POINT;
		}

		void update_val(const rect& constraint) {
			vec2 p = pos - static_cast<vec2>(constraint.pos());
			val = p.y() / static_cast<float>(constraint.size().y());
			val = cgv::math::clamp(val, 0.0f, 1.0f);
		}

		void update_pos(const rect& constraint) {
			val = cgv::math::clamp(val, 0.0f, 1.0f);
			pos = static_cast<float>(constraint.pos().x());
			pos.y() = static_cast<float>(constraint.pos().y()) + val * static_cast<float>(constraint.size().y());
		}

		ivec2 get_render_position() const {
			return ivec2(pos + 0.5f);
		}

		ivec2 get_render_size() const {
			return ivec2(size);
		}
	};

	bool has_updated = false;

	msdf_text_geometry texts;

	cgv::glutil::shape2d_style container_style, border_style, color_texture_style, hue_texture_style, color_handle_style, hue_handle_style, text_style;

	cgv::glutil::draggables_collection<color_point> color_points;
	cgv::glutil::draggables_collection<hue_point> hue_points;

	texture color_tex;
	texture hue_tex;

	rgb color = rgb(0.0f);

	void init_styles(context& ctx);
	void init_textures(context& ctx);
	void update_color_texture();
	void update_color();
	void update_texts();

	void handle_color_point_drag();
	void handle_hue_point_drag();
	
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

	void set_color(rgb color);
	rgb get_color() const { return color; }
};

typedef cgv::data::ref_ptr<color_selector> color_selector_ptr;

}
}

#include <cgv/config/lib_end.h>
