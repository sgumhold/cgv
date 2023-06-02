#pragma once

#include <cgv/gui/dialog.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv_app/canvas_overlay.h>
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

	cgv::g2d::shape2d_style container_style, border_style, rectangle_style, line_style, key_rect_style, scrollbar_style;
	cgv::g2d::circle2d_style key_circle_style;
	cgv::g2d::arrow2d_style marker_handle_style;
	cgv::g2d::grid2d_style grid_style;
	cgv::g2d::text2d_style label_style;

	cgv::g2d::msdf_text_geometry labels;
	
	void erase_selected_keyframe() {

		if(data) {
			data->keyframes.erase(selected_frame);

			set_selected_frame(-1);

			if(on_change_callback)
				on_change_callback();
		}
	}

	void add_keyframe() {

		if(view_ptr && data) {
			view_parameters view;
			view.extract(view_ptr);
			
			auto it = data->keyframes.find(data->frame);
			if(it == data->keyframes.end()) {
				keyframe k;
				k.camera_state = view;
				k.ease(easing_functions::Id::kLinear);

				data->keyframes.insert(data->frame, k);
			} else {
				it->second.camera_state = view;
			}

			set_selected_frame(data->frame);

			if(on_change_callback)
				on_change_callback();
		}
	}

	void show_help() {

		std::string message =
			">\tClick on a keyframe to select it.\n"
			">\tDrag a keyframe to change its time. Dragging onto another keyframe will revert the drag.\n"
			">\tClick \"Set\" to add a new keyframe or change the existing keyframe at the current time.";

		cgv::gui::message(message);
	}

	cgv::render::view* view_ptr;

	std::shared_ptr<animation_data> data;

	cgv::g2d::draggable_collection<cgv::g2d::draggable> scrollbar;
	cgv::g2d::draggable_collection<cgv::g2d::draggable> marker;

	struct keyframe_draggable : public cgv::g2d::draggable {
		size_t frame;

		keyframe_draggable() : draggable() {}
	};

	cgv::g2d::draggable_collection<keyframe_draggable> keyframes;

	size_t selected_frame = -1;

	void handle_scrollbar_drag() {

		if(scrollbar.get_dragged())
			set_timeline_offset();

		post_damage();
	}

	void set_timeline_offset() {

		if(!scrollbar.empty()) {
			const auto& handle = scrollbar[0];
			float t = static_cast<float>(handle.x() - layout.scrollbar_constraint.x()) / static_cast<float>(layout.scrollbar_constraint.w() - handle.w());
			layout.timeline_offset = static_cast<int>(t * (layout.timeline.w() - layout.scrollbar_constraint.w()));
		}
	}

	void handle_marker_drag() {

		const auto dragged = marker.get_dragged();
		if(dragged)
			set_frame(position_to_frame(static_cast<int>(round(dragged->position.x()) + layout.padding / 2)));

		post_damage();
	}

	void handle_keyframe_drag() {

		const auto dragged = keyframes.get_dragged();

		if(dragged) {
			size_t frame = position_to_frame(static_cast<int>(round(dragged->position.x() + 0.5f * dragged->size.x())));
			dragged->position.x() = frame * layout.frame_width + layout.padding;
		}

		post_damage();
	}

	void handle_keyframe_drag_end() {

		const auto selected = keyframes.get_selected();

		if(selected) {
			size_t frame = position_to_frame(static_cast<int>(round(selected->position.x() + 0.5f * selected->size.x())));
			selected->position.x() = selected->frame * layout.frame_width + layout.padding;

			bool was_selected = selected->frame == selected_frame;

			if(selected->frame != frame && data) {

				if(data->keyframes.move(selected->frame, frame)) {
					selected->position.x() = frame * layout.frame_width + layout.padding;

					post_recreate_layout();

					if(on_change_callback)
						on_change_callback();
				}
			} else {
				set_selected_frame(frame);
			}

			if(was_selected)
				set_selected_frame(frame);
		} else {
			set_selected_frame(-1);
		}

		post_damage();
	}

	void handle_keyframe_selection_change() {

		const auto selected = keyframes.get_selected();
		
		set_selected_frame(selected ? selected->frame : -1);
		
		post_damage();
	}
	void set_selected_frame(size_t frame) {

		selected_frame = frame;

		if(data) {
			auto it = data->keyframes.find(selected_frame);
			if(it != data->keyframes.end())
				easing_function_id = it->second.easing_id();
		}

		post_recreate_gui();
	}

	size_t position_to_frame(int position) {

		return (position - layout.padding) / layout.frame_width;
	}

	void set_frame(size_t frame) {

		if(data) {
			data->frame = static_cast<size_t>(frame);
			if(on_change_callback)
				on_change_callback();

			set_marker_position_from_frame();

			post_damage();
		}
	}

	void set_marker_position_from_frame() {

		if(data && !marker.empty()) {
			marker[0].position.x() = data->frame * layout.frame_width + layout.padding;
			post_damage();
		}
	}
	
	void create_keyframe_draggables() {

		if(data) {
			const auto& kfs = data->keyframes;

			keyframes.clear();
			for(const auto& kf : kfs) {
				keyframe_draggable d;

				d.frame = kf.first;
				d.position.x() = layout.padding + layout.frame_width * static_cast<int>(kf.first);
				d.position.y() = layout.timeline.y();
				d.size.x() = layout.frame_width;
				d.size.y() = layout.timeline_height;
				keyframes.add(d);
			}
		}
	}

	int frame_to_scrollbar_position(size_t frame) {

		float t = static_cast<float>(frame) / static_cast<float>(layout.timeline_frames);
		return static_cast<int>(layout.scrollbar_constraint.x() + t * layout.scrollbar_constraint.w());
	}

	void init_styles(cgv::render::context& ctx) override;
	
	virtual void create_gui_impl();

	std::function<void(void)> on_change_callback;



	
	easing_functions::Id easing_function_id = easing_functions::Id::kLinear;
	



public:
	keyframe_editor_overlay();
	std::string get_type_name() const { return "keyframe_editor_overlay"; }

	void clear(cgv::render::context& ctx);

	void handle_on_set(const cgv::app::on_set_evaluator& m);

	bool handle_event(cgv::gui::event& e);

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw_content(cgv::render::context& ctx);

	void set_view(cgv::render::view* view_ptr) {

		this->view_ptr = view_ptr;
	}

	void set_data(std::shared_ptr<animation_data> data) {
		
		this->data = data;
		update();
	}

	void update() {

		create_keyframe_draggables();
		set_marker_position_from_frame();
		post_damage();
	}

	void set_on_change_callback(std::function<void(void)> cb) { on_change_callback = cb; }
};

typedef cgv::data::ref_ptr<keyframe_editor_overlay> keyframe_editor_overlay_ptr;
