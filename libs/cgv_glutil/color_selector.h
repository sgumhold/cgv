#pragma once

#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv_glutil/frame_buffer_container.h>
#include <cgv_glutil/overlay.h>

#include "generic_render_data.h"
#include "generic_renderer.h"
#include "color_map.h"
#include "2d/draggables_collection.h"

#include "2d/canvas.h"
#include "2d/shape2d_styles.h"

#include "lib_begin.h"

namespace cgv {
namespace glutil{

class CGV_API color_selector : public overlay {
protected:
	int last_theme_idx = -1;
	//rgba handle_color = rgb(0.9f, 0.9f, 0.9f, 1.0f);
	//rgba highlight_color = rgb(0.5f, 0.5f, 0.5f, 1.0f);
	//std::string highlight_color_hex = "0x808080";

	cgv::glutil::frame_buffer_container fbc;

	cgv::glutil::canvas content_canvas, overlay_canvas;
	cgv::glutil::shape2d_style container_style, border_style, color_texture_style, hue_texture_style, color_handle_style, hue_handle_style;

	//bool mouse_is_on_overlay = false;
	bool update_layout = false;
	bool has_damage = true;
	bool has_updated = false;

	struct layout_attributes {
		int padding;
		
		// dependent members
		rect content_rect;
		rect color_rect;
		rect hue_rect;
		rect preview_rect;

		void update(const ivec2& parent_size) {

			const int hue_rect_width = 20;

			content_rect = rect();
			content_rect.set_pos(ivec2(padding));
			content_rect.set_size(ivec2(parent_size - 2 * padding));

			hue_rect = content_rect;
			hue_rect.set_pos(content_rect.x1() - hue_rect_width, hue_rect.y());
			hue_rect.set_size(hue_rect_width, content_rect.h());

			color_rect = content_rect;
			color_rect.set_pos(ivec2(padding));
			color_rect.set_size(content_rect.w() - hue_rect.w() - 1, content_rect.h());

			color_rect.a() += ivec2(0, 23);
			hue_rect.a() += ivec2(0, 23);

			preview_rect = content_rect;
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

	cgv::glutil::draggables_collection<color_point> color_points;
	cgv::glutil::draggables_collection<hue_point> hue_points;

	texture color_tex;
	texture hue_tex;

	rgb color = rgb(0.0f);

	void init_styles(context& ctx);
	void init_textures(context& ctx);
	void update_color_texture();
	void update_color();

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
	void draw(cgv::render::context& ctx);
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
