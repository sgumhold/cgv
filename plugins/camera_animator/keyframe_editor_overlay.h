#pragma once

#include <memory>
#include <cgv/gui/dialog.h>
#include <cgv/gui/help_message.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv_app/themed_canvas_overlay.h>
#include <cgv_g2d/draggable_collection.h>
#include <cgv_g2d/generic_2d_renderer.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>

#include "animation_data.h"

class keyframe_editor_overlay : public cgv::app::themed_canvas_overlay {
public:
	enum class Event {
		kUndefined,
		kTimeChange,
		kKeyCreate,
		kKeyDelete,
		kKeyMove,
		kKeyChange,
		kKeySelect,
		kKeyDeselect
	};

protected:
	using vec2 = cgv::vec2;
	using ivec2 = cgv::ivec2;
	using rgb = cgv::rgb;

	struct {
		const int timeline_height = 30;
		const int marker_width = 13;
		const int marker_height = 20;
		const int frame_width = 16;
		const int scrollbar_height = 6;
		int timeline_offset = 0;
		size_t timeline_frames = 0;
		rgb keyframe_color = rgb(0.5f);
		rgb highlight_color = rgb(0.0f, 0.0f, 1.0f);
		rgb selection_color = rgb(0.0f, 0.0f, 1.0f);
		rgb background_color = rgb(0.0f);
		rgb control_color = rgb(0.0f);

		cgv::g2d::irect timeline;
		cgv::g2d::irect scrollbar_constraint;
		cgv::g2d::irect marker_constraint;

		void update(const ivec2& container_size, int padding, size_t frames) {
			
			timeline_frames = frames + 120;

			timeline.position = ivec2(padding, container_size.y() - timeline_height - marker_height - padding);
			
			timeline.w() = timeline_frames == 0 ?
				container_size.x() - 2 * padding :
				frame_width * static_cast<int>(timeline_frames + 1);
			timeline.h() = timeline_height;

			scrollbar_constraint.position = ivec2(padding, 8);
			scrollbar_constraint.w() = container_size.x() - 2 * padding;
			scrollbar_constraint.h() = scrollbar_height;

			marker_constraint = timeline;
			marker_constraint.y() += timeline_height + 2;
			marker_constraint.h() = marker_height;
		}

		int total_height(int padding) const {

			return 2 * padding + marker_height + timeline_height + scrollbar_height + 10;
		}
	} layout;

	struct keyframe_draggable : public cgv::g2d::draggable {
		size_t frame = 0;

		keyframe_draggable() : draggable() {}
	};

	cgv::gui::help_message help;

	cgv::render::view* view_ptr;

	std::shared_ptr<animation_data> data;

	size_t selected_frame = -1;
	easing_functions::Id easing_function_id = easing_functions::Id::kLinear;
	size_t new_frame_count = 30ull;
	float new_duration = 1.0f;

	cgv::g2d::generic_2d_renderer line_renderer;
	DEFINE_GENERIC_RENDER_DATA_CLASS(line_geometry, 1, cgv::vec2, position);
	line_geometry lines;

	cgv::g2d::draggable_collection<cgv::g2d::draggable> scrollbar;
	cgv::g2d::draggable_collection<cgv::g2d::draggable> marker;
	cgv::g2d::draggable_collection<keyframe_draggable> keyframes;
	
	cgv::g2d::msdf_text_geometry labels;

	cgv::g2d::shape2d_style border_style, rectangle_style, line_style, key_rect_style, scrollbar_style;
	cgv::g2d::circle2d_style key_circle_style;
	cgv::g2d::arrow2d_style marker_handle_style;
	cgv::g2d::grid2d_style grid_style;
	cgv::g2d::text2d_style label_style;

	std::function<void(Event)> on_change_callback;

	void add_keyframe();

	void erase_selected_keyframe();

	size_t position_to_frame(int position);

	int frame_to_position(size_t frame);

	int frame_to_scrollbar_position(size_t frame);

	void set_marker_position_from_frame();

	void set_timeline_offset();

	void set_frame(size_t frame);

	void set_selected_frame(size_t frame);

	void change_duration(bool before);

	void create_keyframe_draggables();

	void handle_scrollbar_drag();

	void handle_marker_drag();

	void handle_keyframe_drag();

	void handle_keyframe_drag_end();

	void handle_keyframe_selection_change();

	void invoke_callback(Event e);

	void draw_scrollbar(cgv::render::context& ctx, cgv::g2d::canvas& cnvs);

	void draw_keyframes(cgv::render::context& ctx, cgv::g2d::canvas& cnvs);

	void draw_time_marker_and_labels(cgv::render::context& ctx, cgv::g2d::canvas& cnvs);

	void init_styles() override;
	
	virtual void create_gui_impl() override;

public:
	keyframe_editor_overlay();
	std::string get_type_name() const override { return "keyframe_editor_overlay"; }

	void clear(cgv::render::context& ctx) override;

	bool handle_event(cgv::gui::event& e) override;
	void handle_member_change(const cgv::utils::pointer_test& m) override;

	bool init(cgv::render::context& ctx) override;
	void init_frame(cgv::render::context& ctx) override;
	void draw_content(cgv::render::context& ctx) override;

	void set_view_ptr(cgv::render::view* view_ptr);
	void set_data(std::shared_ptr<animation_data> data);

	void update() override;

	size_t get_selected_frame() { return selected_frame; }

	void set_on_change_callback(std::function<void(Event)> cb) { on_change_callback = cb; }
};

typedef cgv::data::ref_ptr<keyframe_editor_overlay> keyframe_editor_overlay_ptr;
