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

class CGV_API color_map_editor : public overlay {
protected:
	int last_theme_idx = -1;
	rgba handle_color = rgb(0.9f, 0.9f, 0.9f, 1.0f);
	rgba highlight_color = rgb(0.5f, 0.5f, 0.5f, 1.0f);
	std::string highlight_color_hex = "0x808080";

	//std::string file_name;
	//std::string save_file_name;
	//bool has_unsaved_changes = false;
	bool has_updated = false;

	bool mouse_is_on_overlay;
	bool show_cursor;
	ivec2 cursor_pos;
	std::string cursor_drawtext;
	cgv::media::font::font_face_ptr cursor_font_face;

	cgv::glutil::frame_buffer_container fbc;

	cgv::glutil::canvas canvas, overlay_canvas;
	cgv::glutil::shape2d_style container_style, border_style, color_map_style, bg_style, hist_style;
	
	cgv::type::DummyEnum resolution;

	struct layout_attributes {
		int padding;
		int total_height;

		// dependent members
		rect color_map_rect;
		rect handles_rect;

		void update(const ivec2& parent_size) {
			color_map_rect.set_pos(ivec2(padding) + ivec2(0, 10));
			color_map_rect.set_size(parent_size - 2 * padding - ivec2(0, 10));

			handles_rect.set_pos(ivec2(padding, 8));
			handles_rect.set_size(ivec2(parent_size.x() - 2 * padding, 0));
		}
	} layout;
	
	struct point : public draggable {
		float val;
		rgb col;

		point() {
			size = vec2(12.0f, 18.0f);
			position_is_center = true;
			constraint_reference = CR_CENTER;
		}

		void update_val(const layout_attributes& la, const float scale_exponent) {
			vec2 p = pos - la.handles_rect.pos();
			val = p.x() / la.handles_rect.size().x();
			val = cgv::math::clamp(val, 0.0f, 1.0f);
		}

		void update_pos(const layout_attributes& la, const float scale_exponent) {
			val = cgv::math::clamp(val, 0.0f, 1.0f);
			float t = val;
			pos.x() = static_cast<float>(la.handles_rect.pos().x()) + t * la.handles_rect.size().x();
			pos.y() = static_cast<float>(la.handles_rect.pos().y());
		}

		float sd_rectangle(const vec2& p, const vec2& b) const {
			vec2 d = abs(p) - b;
			return length(cgv::math::max(d, 0.0f)) + std::min(std::max(d.x(), d.y()), 0.0f);
		}

		bool is_inside(const vec2& mp) const {
			// test if the given position is inside the handle shape (hit box is defined as a rectangle)
			return sd_rectangle(mp - (pos + vec2(0.0f, 0.5f*size.y() + 2.0f)), 0.5f*size) < 0.0f;
		}

		ivec2 get_render_position() const {
			return ivec2(pos + 0.5f);
		}

		ivec2 get_render_size() const {
			return 2 * ivec2(size);
		}
	};

	texture bg_tex;
	texture preview_tex;

	generic_renderer handle_renderer;
	DEFINE_GENERIC_RENDER_DATA_CLASS(handle_geometry, 2, vec2, position, rgb, color);
	
	struct cm_container {
		color_map* cm = nullptr;
		cgv::glutil::draggables_collection<point> points;
		handle_geometry handles;

		void reset() {
			cm = nullptr;
			points.clear();
			handles.clear();
		}
	} cmc;

	void init_styles(context& ctx);
	void init_texture(context& ctx);

	void add_point(const vec2& pos);
	void remove_point(const point* ptr);
	point* get_hit_point(const vec2& pos);
	
	void handle_drag();
	void handle_drag_end();
	void sort_points();
	void update_point_positions();
	void update_color_map(bool is_data_change);
	bool update_geometry();

public:
	color_map_editor();
	std::string get_type_name() const { return "color_map_editor"; }

	void clear(cgv::render::context& ctx);

	void stream_help(std::ostream& os) {}

	bool handle_event(cgv::gui::event& e);
	void on_set(void* member_ptr);

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	
	void create_gui();
	void create_gui(cgv::gui::provider& p);

	bool was_updated();

	color_map* get_color_map() { return cmc.cm; }

	void set_color_map(color_map* cm);
};

}
}

#include <cgv/config/lib_end.h>
