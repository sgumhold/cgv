#pragma once

#include <cgv/gui/dialog.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv_app/canvas_overlay.h>
#include <cgv_app/help_message.h>
#include <cgv_app/on_set_evaluator.h>
#include <cgv_g2d/draggable_collection.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>

#include "animation_data.h"

class keyframe_editor_overlay : public cgv::app::canvas_overlay {
protected:
	struct {
		const int padding = 13; // 10px plus 3px border
		const int timeline_height = 30;
		const int marker_width = 13;
		const int marker_height = 20;
		const int frame_width = 16;
		const int scrollbar_height = 6;
		int timeline_offset = 0;
		size_t timeline_frames = 0;
		rgb keyframe_color = rgb(0.5f);

		cgv::g2d::irect container;
		cgv::g2d::irect timeline;
		cgv::g2d::irect scrollbar_constraint;
		cgv::g2d::irect marker_constraint;

		void update(const ivec2& container_size, size_t frames) {

			timeline_frames = frames + 120;

			container.position = ivec2(0);
			container.size = container_size;

			timeline.position = ivec2(padding, container_size.y() - timeline_height - marker_height - padding);
			
			timeline.size.x() = timeline_frames == 0 ?
				container_size.x() - 2 * padding :
				frame_width * static_cast<int>(timeline_frames + 1);
			timeline.size.y() = timeline_height;

			scrollbar_constraint.position = ivec2(padding, 8);
			scrollbar_constraint.size.x() = container_size.x() - 2 * padding;
			scrollbar_constraint.size.y() = scrollbar_height;

			marker_constraint = timeline;
			marker_constraint.position.y() += timeline_height + 2;
			marker_constraint.size.y() = marker_height;
		}

		int total_height() const {

			return 2 * padding + marker_height + timeline_height + scrollbar_height;
		}
	} layout;

	struct keyframe_draggable : public cgv::g2d::draggable {
		size_t frame;

		keyframe_draggable() : draggable() {}
	};

	cgv::app::help_message help;

	cgv::render::view* view_ptr;

	std::shared_ptr<animation_data> data;

	size_t selected_frame = -1;
	easing_functions::Id easing_function_id = easing_functions::Id::kLinear;

	cgv::g2d::draggable_collection<cgv::g2d::draggable> scrollbar;
	cgv::g2d::draggable_collection<cgv::g2d::draggable> marker;
	cgv::g2d::draggable_collection<keyframe_draggable> keyframes;
	
	cgv::g2d::msdf_text_geometry labels;

	cgv::g2d::shape2d_style container_style, border_style, rectangle_style, line_style, key_rect_style, scrollbar_style;
	cgv::g2d::circle2d_style key_circle_style;
	cgv::g2d::arrow2d_style marker_handle_style;
	cgv::g2d::grid2d_style grid_style;
	cgv::g2d::text2d_style label_style;

	std::function<void(void)> on_change_callback;

	void add_keyframe();

	void erase_selected_keyframe();

	size_t position_to_frame(int position);

	int frame_to_position(size_t frame);

	int frame_to_scrollbar_position(size_t frame);

	void set_marker_position_from_frame();

	void set_timeline_offset();

	void set_frame(size_t frame);

	void set_selected_frame(size_t frame);

	void create_keyframe_draggables();

	void handle_scrollbar_drag();

	void handle_marker_drag();

	void handle_keyframe_drag();

	void handle_keyframe_drag_end();

	void handle_keyframe_selection_change();

	void init_styles(cgv::render::context& ctx) override;
	
	virtual void create_gui_impl();

public:
	keyframe_editor_overlay();
	std::string get_type_name() const { return "keyframe_editor_overlay"; }

	void clear(cgv::render::context& ctx);

	bool handle_event(cgv::gui::event& e);
	void handle_on_set(const cgv::app::on_set_evaluator& m);

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw_content(cgv::render::context& ctx);

	void set_view(cgv::render::view* view_ptr);
	void set_data(std::shared_ptr<animation_data> data);

	void update();

	void set_on_change_callback(std::function<void(void)> cb) { on_change_callback = cb; }
};

typedef cgv::data::ref_ptr<keyframe_editor_overlay> keyframe_editor_overlay_ptr;
