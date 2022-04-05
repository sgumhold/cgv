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

	bool has_updated = false;

	cgv::glutil::frame_buffer_container fbc;

	cgv::glutil::canvas canvas, overlay_canvas;
	cgv::glutil::shape2d_style container_style, border_style, color_map_style, bg_style, hist_style;

	bool mouse_is_on_overlay;
	bool show_cursor;
	ivec2 cursor_pos;
	std::string cursor_drawtext;
	cgv::media::font::font_face_ptr cursor_font_face;

	//bool show_histogram;
	//rgba histogram_color;
	//rgba histogram_border_color;
	//unsigned histogram_border_width;
	//float histogram_smoothing;
	
	cgv::type::DummyEnum resolution;
	float opacity_scale_exponent;
	bool supports_opacity;
	bool update_layout = false;

	struct layout_attributes {
		int padding;
		int total_height;
		
		// dependent members
		int color_editor_height;
		int opacity_editor_height;
		rect color_handles_rect;
		rect color_editor_rect;
		rect opacity_editor_rect;

		void update(const ivec2& parent_size, bool color_and_opacity) {

			int content_height = total_height - 10 - 2 * padding;
			if(color_and_opacity) {
				color_editor_height = static_cast<int>(floor(0.15f * static_cast<float>(content_height)));
				color_editor_height = cgv::math::clamp(color_editor_height, 4, 80);
				opacity_editor_height = content_height - color_editor_height - 1;
			} else {
				color_editor_height = content_height;
				opacity_editor_height = 0;
			}

			int y_off = padding;

			color_handles_rect.set_pos(ivec2(padding, 8));
			color_handles_rect.set_size(ivec2(parent_size.x() - 2 * padding, 0));

			// move 10px up to clear some space for the color handles rect
			y_off += 10;

			color_editor_rect.set_pos(ivec2(padding, y_off));
			color_editor_rect.set_size(ivec2(parent_size.x() - 2 * padding, color_editor_height));

			y_off += color_editor_height + 1; // plus 1px border

			opacity_editor_rect.set_pos(ivec2(padding, y_off));
			opacity_editor_rect.set_size(ivec2(parent_size.x() - 2 * padding, opacity_editor_height));
		}
	} layout;
	
	struct color_point : public draggable {
		float val;
		rgb col;

		color_point() {
			size = vec2(12.0f, 18.0f);
			position_is_center = true;
			constraint_reference = CR_CENTER;
		}

		void update_val(const layout_attributes& la) {
			vec2 p = pos - la.color_handles_rect.pos();
			val = p.x() / la.color_handles_rect.size().x();
			val = cgv::math::clamp(val, 0.0f, 1.0f);
		}

		void update_pos(const layout_attributes& la) {
			val = cgv::math::clamp(val, 0.0f, 1.0f);
			float t = val;
			pos.x() = static_cast<float>(la.color_handles_rect.pos().x()) + t * la.color_handles_rect.size().x();
			pos.y() = static_cast<float>(la.color_handles_rect.pos().y());
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

	struct opacity_point : public draggable {
		vec2 val;

		opacity_point() {
			size = vec2(6.0f);
			position_is_center = true;
			constraint_reference = CR_CENTER;
		}

		void update_val(const layout_attributes& la, const float scale_exponent) {

			vec2 p = pos - la.opacity_editor_rect.pos();
			val = p / la.opacity_editor_rect.size();

			val = cgv::math::clamp(val, 0.0f, 1.0f);
			val.y() = cgv::math::clamp(std::pow(val.y(), scale_exponent), 0.0f, 1.0f);
		}

		void update_pos(const layout_attributes& la, const float scale_exponent) {

			val = cgv::math::clamp(val, 0.0f, 1.0f);

			vec2 t = val;

			t.y() = cgv::math::clamp(std::pow(t.y(), 1.0f / scale_exponent), 0.0f, 1.0f);

			pos = la.opacity_editor_rect.pos() + t * la.opacity_editor_rect.size();
		}

		float sd_rectangle(const vec2& p, const vec2& b) const {
			vec2 d = abs(p) - b;
			return length(cgv::math::max(d, 0.0f)) + std::min(std::max(d.x(), d.y()), 0.0f);
		}

		bool is_inside(const vec2& mp) const {

			return sd_rectangle(mp - pos, size) < 0.0f;
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

	generic_renderer color_handle_renderer, opacity_handle_renderer, line_renderer, polygon_renderer;
	DEFINE_GENERIC_RENDER_DATA_CLASS(custom_geometry, 2, vec2, position, rgba, color);
	DEFINE_GENERIC_RENDER_DATA_CLASS(line_geometry, 2, vec2, position, vec2, texcoord);

	struct cm_container {
		color_map* cm = nullptr;
		cgv::glutil::draggables_collection<color_point> color_points;
		cgv::glutil::draggables_collection<opacity_point> opacity_points;
		
		custom_geometry color_handles, opacity_handles;
		line_geometry lines;
		line_geometry triangles;

		void reset() {
			cm = nullptr;
			color_points.clear();
			opacity_points.clear();
			color_handles.clear();
			opacity_handles.clear();
			lines.clear();
			triangles.clear();
		}
	} cmc;

	void init_styles(context& ctx);
	void init_texture(context& ctx);

	void add_point(const vec2& pos);
	void remove_point(const draggable* ptr);
	draggable* get_hit_point(const vec2& pos);
	
	void handle_color_point_drag();
	void handle_opacity_point_drag();
	void handle_drag_end();
	void sort_points();
	void sort_color_points();
	void sort_opacity_points();
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

	bool get_opacity_support() { return supports_opacity; }
	void set_opacity_support(bool flag);

	bool was_updated();

	color_map* get_color_map() { return cmc.cm; }

	void set_color_map(color_map* cm);
};

}
}

#include <cgv/config/lib_end.h>
