#pragma once

#include <cgv/render/color_map.h>
#include <cgv/render/texture.h>
#include <cgv_app/themed_canvas_overlay.h>
#include <cgv_app/color_selector.h>
#include <cgv_g2d/draggable_collection.h>
#include <cgv_g2d/generic_2d_renderer.h>
#include <cgv_g2d/generic_2d_render_data.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>

#include "lib_begin.h"

namespace cgv {

namespace app {

class CGV_API color_map_editor : public themed_canvas_overlay {
protected:
	struct layout_attributes {
		int padding;
		int total_height;
		
		// dependent members
		int color_editor_height;
		int opacity_editor_height;
		cgv::g2d::irect color_handles_rect;
		cgv::g2d::irect color_editor_rect;
		cgv::g2d::irect opacity_editor_rect;

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

			color_handles_rect.position = ivec2(padding, 20);
			color_handles_rect.size = ivec2(parent_size.x() - 2 * padding, 0);

			// move 10px up to clear some space for the color handles rect
			y_off += 10;

			color_editor_rect.position = ivec2(padding, y_off);
			color_editor_rect.size = ivec2(parent_size.x() - 2 * padding, color_editor_height);

			y_off += color_editor_height + 1; // plus 1px border

			opacity_editor_rect.position = ivec2(padding, y_off);
			opacity_editor_rect.size = ivec2(parent_size.x() - 2 * padding, opacity_editor_height);
		}
	} layout;
	
	struct color_point : public cgv::g2d::draggable {
		static const float default_width;
		static const float default_height;
		float val;
		rgb col;

		color_point() {
			size = vec2(default_width, default_height);
			position_is_center = true;
			constraint_reference = CR_CENTER;
		}

		void update_val(const layout_attributes& la) {
			vec2 p = position - la.color_handles_rect.position;
			val = p.x() / la.color_handles_rect.size.x();
			val = cgv::math::clamp(val, 0.0f, 1.0f);
		}

		void update_pos(const layout_attributes& la) {
			val = cgv::math::clamp(val, 0.0f, 1.0f);
			float t = val;
			position.x() = static_cast<float>(la.color_handles_rect.position.x()) + t * la.color_handles_rect.size.x();
			position.y() = static_cast<float>(la.color_handles_rect.position.y());
		}
	};

	struct opacity_point : public cgv::g2d::draggable {
		static const float default_size;
		vec2 val;

		opacity_point() {
			size = vec2(default_size);
			position_is_center = true;
			constraint_reference = CR_CENTER;
		}

		void update_val(const layout_attributes& la, const float scale_exponent) {
			vec2 p = position - la.opacity_editor_rect.position;
			val = p / la.opacity_editor_rect.size;

			val = cgv::math::clamp(val, 0.0f, 1.0f);
			val.y() = cgv::math::clamp(std::pow(val.y(), scale_exponent), 0.0f, 1.0f);
		}

		void update_pos(const layout_attributes& la, const float scale_exponent) {
			val = cgv::math::clamp(val, 0.0f, 1.0f);

			vec2 t = val;
			t.y() = cgv::math::clamp(std::pow(t.y(), 1.0f / scale_exponent), 0.0f, 1.0f);

			position = la.opacity_editor_rect.position + t * la.opacity_editor_rect.size;
		}
	};

	bool mouse_is_on_overlay;
	bool supports_opacity;
	bool use_interpolation;
	bool use_linear_filtering;
	vec2 range;

	ivec2 cursor_pos;
	int cursor_label_index;
	bool show_value_label;

	// general appearance
	rgba handle_color = rgba(0.9f, 0.9f, 0.9f, 1.0f);
	rgba highlight_color = rgba(0.5f, 0.5f, 0.5f, 1.0f);
	std::string highlight_color_hex = "0x808080";
	cgv::g2d::shape2d_style border_style, color_map_style, bg_style, hist_style, label_box_style, opacity_handle_style, polygon_style;
	cgv::g2d::arrow2d_style color_handle_style;
	cgv::g2d::line2d_style line_style;

	// label appearance
	cgv::g2d::text2d_style cursor_label_style, value_label_style;
	cgv::g2d::msdf_text_geometry cursor_labels, value_labels;

	std::vector<unsigned> histogram;
	unsigned hist_max = 1;
	unsigned hist_max_non_zero = 1;
	bool hist_norm_ignore_zero = true;
	float hist_norm_gamma = 1.0f;
	cgv::type::DummyEnum histogram_type = (cgv::type::DummyEnum)2;

	cgv::type::DummyEnum resolution;
	float opacity_scale_exponent;
	
	cgv::render::texture bg_tex;
	cgv::render::texture preview_tex;
	cgv::render::texture hist_tex;

	cgv::g2d::generic_2d_renderer color_handle_renderer, opacity_handle_renderer, line_renderer, polygon_renderer;
	DEFINE_GENERIC_RENDER_DATA_CLASS(line_geometry, 2, vec2, position, vec2, texcoord);

	struct cm_container {
		cgv::render::color_map* cm = nullptr;
		cgv::g2d::draggable_collection<color_point> color_points;
		cgv::g2d::draggable_collection<opacity_point> opacity_points;
		
		cgv::g2d::generic_render_data_vec2_rgba color_handles, opacity_handles;
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

		cgv::render::gl_color_map* get_gl_color_map() {
			if(cm->has_texture_support())
				return dynamic_cast<cgv::render::gl_color_map*>(cm);
			return nullptr;
		}
	} cmc;

	void init_styles() override;
	void setup_preview_texture(cgv::render::context& ctx);
	void init_preview_texture(cgv::render::context& ctx);

	void add_point(const vec2& pos);
	void remove_point(const cgv::g2d::draggable* ptr);
	cgv::g2d::draggable* get_hit_point(const vec2& pos);
	
	void handle_color_point_drag();
	void handle_opacity_point_drag();
	void handle_drag_end();
	std::string value_to_string(float value);
	void sort_points();
	void sort_color_points();
	void sort_opacity_points();
	void update_point_positions();
	void update_color_map(bool is_data_change);
	bool update_geometry();

	std::function<void(void)> on_change_callback;
	std::function<void(rgb)> on_color_point_select_callback;
	std::function<void(void)> on_color_point_deselect_callback;

	void create_gui_impl() override;

public:
	color_map_editor();
	std::string get_type_name() const override { return "color_map_editor"; }

	void clear(cgv::render::context& ctx) override;

	bool handle_event(cgv::gui::event& e) override;
	void handle_member_change(const cgv::utils::pointer_test& m) override;

	bool init(cgv::render::context& ctx) override;
	void init_frame(cgv::render::context& ctx) override;
	void draw_content(cgv::render::context& ctx) override;
	
	void handle_theme_change(const cgv::gui::theme_info& theme) override;

	bool get_opacity_support() { return supports_opacity; }
	void set_opacity_support(bool flag);

	vec2 get_range() const { return range; }
	void set_range(vec2 r) { range = r; }

	cgv::render::color_map* get_color_map() { return cmc.cm; }
	void set_color_map(cgv::render::color_map* cm);

	void set_histogram_data(const std::vector<unsigned> data);

	void set_selected_color(rgb color);

	void set_on_change_callback(std::function<void(void)> cb) { on_change_callback = cb; }
	void set_on_color_point_select_callback(std::function<void(rgb)> cb) { on_color_point_select_callback = cb; }
	void set_on_color_point_deselect_callback(std::function<void(void)> cb) { on_color_point_deselect_callback = cb; }
};

typedef cgv::data::ref_ptr<color_map_editor> color_map_editor_ptr;

static void connect_color_selector_to_color_map_editor(const color_map_editor_ptr cme_ptr, const color_selector_ptr cs_ptr) {
	if(cme_ptr && cs_ptr) {
		cme_ptr->set_on_color_point_select_callback(std::bind(&cgv::app::color_selector::set_rgb_color, cs_ptr, std::placeholders::_1));
		cme_ptr->set_on_color_point_deselect_callback(std::bind(&cgv::app::color_selector::set_visibility, cs_ptr, false));
		cs_ptr->set_on_change_rgb_callback(std::bind(&cgv::app::color_map_editor::set_selected_color, cme_ptr, std::placeholders::_1));
	}
}

}
}

#include <cgv/config/lib_end.h>
