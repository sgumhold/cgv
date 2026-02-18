#pragma once

#include <cgv/data/time_stamp.h>
#include <cgv/media/transfer_function.h>
#include <cgv/render/texture.h>
#include <cgv_g2d/draggable_collection.h>
#include <cgv_g2d/generic_2d_renderer.h>
#include <cgv_g2d/generic_2d_render_data.h>
#include <cgv_g2d/msdf_text_geometry.h>
#include <cgv_overlay/color_selector.h>
#include <cgv_overlay/themed_canvas_overlay.h>

#include "lib_begin.h"

namespace cgv {
namespace overlay {

class CGV_API transfer_function_editor : public themed_canvas_overlay {
protected:
	template<typename DataT>
	class control_point : public cgv::g2d::draggable {
	public:
		control_point(vec2 size, const cgv::g2d::irect* constraint) : cgv::g2d::draggable({ 0.0f }, size) {
			position_is_center = true;
			constraint_reference = cgv::g2d::ConstraintReference::kCenter;
			this->constraint = constraint;
		}

		void set_position_and_update_uv(vec2 position) {
			if(!constraint)
				return;

			const auto& area = static_cast<cgv::g2d::rect>(*constraint);
			this->position = cgv::math::clamp(position, area.a(), area.b());
			uv = (this->position - area.position) / area.size;
		}

		void set_uv_and_update_position(vec2 uv) {
			if(!constraint)
				return;

			const auto& area = static_cast<cgv::g2d::rect>(*constraint);
			this->uv = cgv::math::clamp(uv, 0.0f, 1.0f);
			position = area.position + this->uv * area.size;
		}

		bool operator<(const control_point& other) const {
			return uv.x() < other.uv.x();
		}

		vec2 uv = { 0.0f };
		vec2 value = { 0.0f };
		DataT data = {};
	};

	using color_point = control_point<rgb>;
	using opacity_point = control_point<void*>;

	static const vec2 color_point_size;
	static const vec2 opacity_point_size;

	enum class DraggableType {
		kColor,
		kOpacity
	};

	struct layout_attributes {
		int total_height;
		int color_editor_height;
		int opacity_editor_height;
		cgv::g2d::irect color_draggables_rect;
		cgv::g2d::irect color_editor_rect;
		cgv::g2d::irect opacity_editor_rect;
	} layout;

	bool supports_opacity = false;
	cgv::media::transfer_function::InterpolationMode color_interpolation = cgv::media::transfer_function::InterpolationMode::kLinear;
	cgv::media::transfer_function::InterpolationMode opacity_interpolation = cgv::media::transfer_function::InterpolationMode::kLinear;

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

	//int discretization_resolution = 256;
	float opacity_scale_exponent = 1.0f;

	cgv::render::texture background_tex = { "flt32[R,G,B]", cgv::render::TF_NEAREST, cgv::render::TF_NEAREST, cgv::render::TW_REPEAT, cgv::render::TW_REPEAT };
	cgv::render::texture preview_tex = { "uint8[R,G,B,A]" };
	cgv::render::texture histogram_tex = { "flt32[R]" };

	cgv::g2d::generic_2d_renderer color_handle_renderer, opacity_handle_renderer, line_renderer, polygon_renderer;
	DEFINE_GENERIC_RENDER_DATA_CLASS(textured_geometry, 2, vec2, position, vec2, texcoord);

	std::shared_ptr<cgv::media::transfer_function> transfer_function;

	cgv::g2d::draggable_collection<color_point> color_draggables;
	cgv::g2d::draggable_collection<opacity_point> opacity_draggables;
	color_point* selected_color_draggable = nullptr;
	opacity_point* selected_opacity_draggable = nullptr;
	vec2 color_draggable_start_position = { 0.0f };
	vec2 opacity_draggable_start_position = { 0.0f };
	cgv::g2d::generic_render_data_vec2_rgba color_draggables_geometry, opacity_draggables_geometry;
	textured_geometry line_geometry;
	textured_geometry triangle_geometry;
	vec2 input_domain = { 0.0f, 1.0f };
	float input_position = 0.0f;
	
	cgv::data::time_stamp build_time;

	void init_styles() override;
	void update_layout(const ivec2& parent_size);

	color_point make_color_point() const {
		return color_point(color_point_size, &layout.color_draggables_rect);
	}

	opacity_point make_opacity_point() const {
		return opacity_point(opacity_point_size, &layout.opacity_editor_rect);
	}

	void clear_data();
	void force_update_data_from_transfer_function();
	void update_data_from_transfer_function();
	void update_transfer_function_from_data();
	void rescale_domain();
	
	void add_point(const vec2& pos);
	void erase_point(const cgv::g2d::draggable* point);
	void set_selected_point_domain_value();
	cgv::g2d::draggable* get_hit_point(const vec2& pos);
	
	void set_value_label(vec2 position, const std::string& text);
	void handle_drag(cgv::g2d::DragAction action, DraggableType type);
	void handle_selection_change();
	std::string value_to_string(float value);
	void sort_points();
	void update_point_positions();

	bool create_preview_texture();
	bool create_background_texture();
	void create_geometry();
	
	void create_gui_impl() override;

	std::function<void(void)> on_change_callback;
	std::function<void(rgb)> on_color_point_select_callback;
	std::function<void(void)> on_color_point_deselect_callback;

public:
	transfer_function_editor();
	std::string get_type_name() const override { return "transfer_function_editor"; }

	void clear(cgv::render::context& ctx) override;

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::ivec2 local_mouse_pos) override;
	void handle_member_change(cgv::data::informed_ptr ptr) override;

	bool init(cgv::render::context& ctx) override;
	void init_frame(cgv::render::context& ctx) override;
	void draw_content(cgv::render::context& ctx) override;
	
	void handle_theme_change(const cgv::gui::theme_info& theme) override;

	bool get_opacity_support() { return supports_opacity; }
	void set_opacity_support(bool flag);

	std::shared_ptr<cgv::media::transfer_function> get_transfer_function() const {
		return transfer_function;
	}
	void set_transfer_function(std::shared_ptr<cgv::media::transfer_function> transfer_function);

	void notify_transfer_function_change() {
		force_update_data_from_transfer_function();
	}

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
		cme_ptr->set_on_color_point_select_callback(std::bind(&cgv::overlay::color_selector::set_rgb_color, cs_ptr, std::placeholders::_1));
		cme_ptr->set_on_color_point_deselect_callback(std::bind(&cgv::overlay::color_selector::set_visibility, cs_ptr, false));
		cs_ptr->set_on_change_rgb_callback(std::bind(&cgv::overlay::transfer_function_editor::set_selected_color, cme_ptr, std::placeholders::_1));
	}
}

} // namespace overlay
} // namespace cgv

#include <cgv/config/lib_end.h>
