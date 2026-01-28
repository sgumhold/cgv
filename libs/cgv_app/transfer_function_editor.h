#pragma once

#include <cgv/data/time_stamp.h>
#include <cgv/media/transfer_function.h>
#include <cgv/render/texture.h>
#include <cgv_app/themed_canvas_overlay.h>
#include <cgv_app/color_selector.h>
#include <cgv_g2d/draggable_collection.h>
#include <cgv_g2d/generic_2d_renderer.h>
#include <cgv_g2d/generic_2d_render_data.h>
#include <cgv_g2d/msdf_text_geometry.h>

#include "lib_begin.h"

namespace cgv {

namespace app {

class CGV_API transfer_function_editor : public themed_canvas_overlay {
protected:
	struct layout_attributes {
		int total_height;
		int color_editor_height;
		int opacity_editor_height;
		cgv::g2d::irect color_draggables_rect;
		cgv::g2d::irect color_editor_rect;
		cgv::g2d::irect opacity_editor_rect;
	} layout;

	struct color_point : public cgv::g2d::draggable {
		static const float default_width;
		static const float default_height;
		float val = 0.0f;
		rgb col = { 0.0f };

		color_point() {
			size = vec2(default_width, default_height);
			position_is_center = true;
			constraint_reference = CR_CENTER;
		}

		void update_val(const cgv::g2d::rect& area) {
			vec2 p = position - area.position;
			val = p.x() / area.size.x();
			val = cgv::math::clamp(val, 0.0f, 1.0f);
		}

		void update_pos(const cgv::g2d::rect& area) {
			val = cgv::math::clamp(val, 0.0f, 1.0f);
			float t = val;
			position.x() = static_cast<float>(area.x()) + t * area.w();
			position.y() = static_cast<float>(area.y());
		}
	};

	struct opacity_point : public cgv::g2d::draggable {
		static const float default_size;
		vec2 val = { 0.0f };

		opacity_point() {
			size = vec2(default_size);
			position_is_center = true;
			constraint_reference = CR_CENTER;
		}

		void update_val(const cgv::g2d::rect& area, const float scale_exponent) {
			vec2 p = position - area.position;
			val = p / area.size;

			val = cgv::math::clamp(val, 0.0f, 1.0f);
			val.y() = cgv::math::clamp(std::pow(val.y(), scale_exponent), 0.0f, 1.0f);
		}

		void update_pos(const cgv::g2d::rect& area, const float scale_exponent) {
			val = cgv::math::clamp(val, 0.0f, 1.0f);

			vec2 t = val;
			t.y() = cgv::math::clamp(std::pow(t.y(), 1.0f / scale_exponent), 0.0f, 1.0f);

			position = area.position + t * area.size;
		}
	};

	bool mouse_is_on_overlay;
	bool supports_opacity;
	bool use_interpolation;
	bool use_linear_filtering;
	vec2 range;

	ivec2 cursor_position;
	std::string cursor_label;
	//bool show_value_label;
	std::string value_label;
	cgv::g2d::rect value_label_rectangle;

	// general appearance
	rgba handle_color = { 0.9f, 0.9f, 0.9f, 1.0f };
	rgba highlight_color = { 0.5f, 0.5f, 0.5f, 1.0f };
	cgv::g2d::shape2d_style border_style, color_map_style, bg_style, hist_style, label_box_style, opacity_handle_style, polygon_style;
	cgv::g2d::arrow2d_style color_handle_style;
	cgv::g2d::line2d_style line_style;

	// label appearance
	cgv::g2d::text2d_style cursor_label_style, value_label_style;

	std::vector<unsigned> histogram;
	unsigned hist_max = 1;
	unsigned hist_max_non_zero = 1;
	bool hist_norm_ignore_zero = true;
	float hist_norm_gamma = 1.0f;
	enum class HistogramType {
		kNone = 0,
		kNearest = 1,
		kLinear = 2,
		kSmooth = 3
	};
	HistogramType histogram_type = HistogramType::kNone;

	cgv::type::DummyEnum resolution;
	float opacity_scale_exponent;

	cgv::render::texture background_tex = { "flt32[R,G,B]", cgv::render::TF_NEAREST, cgv::render::TF_NEAREST, cgv::render::TW_REPEAT, cgv::render::TW_REPEAT };
	cgv::render::texture preview_tex = { "uint8[R,G,B,A]" };
	cgv::render::texture histogram_tex = { "flt32[R]" };

	cgv::g2d::generic_2d_renderer color_handle_renderer, opacity_handle_renderer, line_renderer, polygon_renderer;
	DEFINE_GENERIC_RENDER_DATA_CLASS(textured_geometry, 2, vec2, position, vec2, texcoord);

	std::shared_ptr<cgv::media::transfer_function> transfer_function;

	cgv::g2d::draggable_collection<color_point> color_draggables;
	cgv::g2d::draggable_collection<opacity_point> opacity_draggables;
	cgv::g2d::generic_render_data_vec2_rgba color_draggables_geometry, opacity_draggables_geometry;
	textured_geometry line_geometry;
	textured_geometry triangle_geometry;
	
	cgv::data::time_stamp build_time;

	void clear_geometry() {
		color_draggables.clear();
		opacity_draggables.clear();
		color_draggables_geometry.clear();
		opacity_draggables_geometry.clear();
		line_geometry.clear();
		triangle_geometry.clear();
	}

	void init_styles() override;
	void update_layout(const ivec2& parent_size);

	void update_data_from_transfer_function() {
		if(!transfer_function)
			return;

		if(!build_time.is_valid() || transfer_function->get_modified_time() > build_time.get_modified_time()) {
			color_draggables.clear();
			opacity_draggables.clear();

			// Todo: Do not clamp to 0 and 1 ion order to support arbitrary data range.

			for(const auto& point : transfer_function->get_color_points()) {
				color_point p;
				p.val = cgv::math::clamp(point.first, 0.0f, 1.0f);
				p.col = point.second;
				color_draggables.add(p);
			}

			if(supports_opacity) {
				for(const auto& point : transfer_function->get_opacity_points()) {
					opacity_point p;
					p.val.x() = cgv::math::clamp(point.first, 0.0f, 1.0f);
					p.val.y() = cgv::math::clamp(point.second, 0.0f, 1.0f);
					opacity_draggables.add(p);
				}
			}

			//use_interpolation = cmc.cm->use_interpolation;
			//use_linear_filtering = true;

			update_point_positions();
			update_geometry();
			create_preview_texture();
			//update_color_map(false);

			post_recreate_gui();
			post_damage();

			build_time.modified();
		}
	}

	bool create_preview_texture();
	bool create_background_texture();

	void add_point(const vec2& pos);
	void remove_point(const cgv::g2d::draggable* ptr);
	cgv::g2d::draggable* get_hit_point(const vec2& pos);
	
	enum class DraggableType {
		kColor,
		kOpacity
	};

	void set_value_label(vec2 position, const std::string& text);
	void handle_drag(cgv::g2d::DragAction action, DraggableType type);
	void handle_selection_change();
	std::string value_to_string(float value);
	void sort_points();
	void sort_color_draggables();
	void sort_opacity_draggables();
	void update_point_positions();
	//void update_color_map(bool is_data_change);
	bool update_geometry();

	std::function<void(void)> on_change_callback;
	std::function<void(rgb)> on_color_point_select_callback;
	std::function<void(void)> on_color_point_deselect_callback;

	void create_gui_impl() override;

public:
	transfer_function_editor();
	std::string get_type_name() const override { return "transfer_function_editor"; }

	void clear(cgv::render::context& ctx) override;

	bool handle_key_event(cgv::gui::key_event& e) override;
	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::ivec2 local_mouse_pos) override;
	void handle_member_change(const cgv::utils::pointer_test& m) override;

	bool init(cgv::render::context& ctx) override;
	void init_frame(cgv::render::context& ctx) override;
	void draw_content(cgv::render::context& ctx) override;
	
	void handle_theme_change(const cgv::gui::theme_info& theme) override;

	bool get_opacity_support() { return supports_opacity; }
	void set_opacity_support(bool flag);

	vec2 get_range() const { return range; }
	void set_range(vec2 r) { range = r; }

	std::shared_ptr<cgv::media::transfer_function> get_transfer_function() const {
		return transfer_function;
	}
	void set_transfer_function(std::shared_ptr<cgv::media::transfer_function> transfer_function);

	void set_histogram_data(const std::vector<unsigned> data);

	void set_selected_color(rgb color);

	void set_on_change_callback(std::function<void(void)> cb) {
		on_change_callback = cb;
	}
	void set_on_color_point_select_callback(std::function<void(rgb)> cb) {
		on_color_point_select_callback = cb;
	}
	void set_on_color_point_deselect_callback(std::function<void(void)> cb) {
		on_color_point_deselect_callback = cb;
	}
};

typedef cgv::data::ref_ptr<transfer_function_editor> transfer_function_editor_ptr;

static void connect_color_selector_to_transfer_function_editor(const transfer_function_editor_ptr cme_ptr, const color_selector_ptr cs_ptr) {
	if(cme_ptr && cs_ptr) {
		cme_ptr->set_on_color_point_select_callback(std::bind(&cgv::app::color_selector::set_rgb_color, cs_ptr, std::placeholders::_1));
		cme_ptr->set_on_color_point_deselect_callback(std::bind(&cgv::app::color_selector::set_visibility, cs_ptr, false));
		cs_ptr->set_on_change_rgb_callback(std::bind(&cgv::app::transfer_function_editor::set_selected_color, cme_ptr, std::placeholders::_1));
	}
}

}
}

#include <cgv/config/lib_end.h>
