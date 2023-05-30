#pragma once

//#include <cgv/math/ftransform.h>
//#include <cgv/render/color_map.h>
//#include <cgv/render/texture.h>
//#include <cgv/utils/convert_string.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv_app/canvas_overlay.h>
#include <cgv_app/on_set_evaluator.h>
#include <cgv_g2d/draggable_collection.h>
//#include <cgv_g2d/generic_2d_renderer.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>


//#include <cgv/base/node.h>
//#include <cgv/gui/event_handler.h>
//#include <cgv/gui/provider.h>
//#include <cgv/render/drawable.h>
//#include <cgv_app/application_plugin.h>
//#include <cgv_app/on_set_evaluator.h>

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

		cgv::g2d::irect container;
		cgv::g2d::irect timeline;
		cgv::g2d::irect scrollbar_constraint;
		cgv::g2d::irect marker_constraint;

		void update(const ivec2& container_size, size_t frames) {

			container.position = ivec2(0);
			container.size = container_size;

			timeline.position = ivec2(padding, container_size.y() - timeline_height - marker_height - padding);
			
			timeline.size.x() = frames == 0 ?
				container_size.x() - 2 * padding :
				frame_width * static_cast<int>(frames + 1);
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

	cgv::g2d::shape2d_style container_style, border_style, line_style, key_rect_style, marker_style, scrollbar_style;
	cgv::g2d::circle2d_style key_circle_style;
	cgv::g2d::arrow2d_style marker_handle_style;
	cgv::g2d::grid2d_style grid_style;
	cgv::g2d::text2d_style label_style;

	cgv::g2d::msdf_text_geometry labels;
	
	//cgv::g2d::button_collection buttons;

	void handle_button_click(const std::string& label) {

		if(label == "") {
			//stretch = stretch != StretchOption::SO_NONE ? StretchOption::SO_NONE : StretchOption::SO_BOTH;
			//on_layout_change();
		}
	}


	std::shared_ptr<animation_data> data_ptr;

	cgv::g2d::draggable_collection<cgv::g2d::draggable> scrollbar;
	cgv::g2d::draggable_collection<cgv::g2d::draggable> marker;

	size_t selected_frame = -1;

	void handle_scrollbar_drag() {

		const auto dragged = scrollbar.get_dragged();
		if(dragged) {
			float t = static_cast<float>(dragged->position.x() - layout.scrollbar_constraint.x()) / static_cast<float>(layout.scrollbar_constraint.w() - dragged->size.x());
			layout.timeline_offset = static_cast<int>(t * (layout.timeline.w() - layout.scrollbar_constraint.w()));
		}

		post_damage();
	}

	void handle_marker_drag() {

		const auto dragged = marker.get_dragged();
		if(dragged)
			set_frame(get_frame_from_position(static_cast<int>(round(dragged->position.x()))));

		post_damage();
	}

	size_t get_frame_from_position(int position) {

		return (position - layout.padding) / layout.frame_width;
	}

	void set_frame(size_t frame) {

		if(data_ptr) {
			data_ptr->frame = static_cast<size_t>(frame);
			if(on_change_callback)
				on_change_callback();

			set_marker_position_from_frame();

			post_damage();
		}
	}

	void set_marker_position_from_frame() {

		if(data_ptr && marker.size()) {
			marker[0].position.x() = data_ptr->frame * layout.frame_width + layout.padding;
			post_damage();
		}
	}
	
	void init_styles(cgv::render::context& ctx) override;
	
	virtual void create_gui_impl();

	std::function<void(void)> on_change_callback;

public:
	keyframe_editor_overlay();
	std::string get_type_name() const { return "keyframe_editor_overlay"; }

	void clear(cgv::render::context& ctx);

	void on_set(void* member_ptr);
	void on_set(const cgv::app::on_set_evaluator& m);

	bool handle_event(cgv::gui::event& e);

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void draw_content(cgv::render::context& ctx);

	
	void set_data(std::shared_ptr<animation_data> data) {
		
		this->data_ptr = data;
		post_damage();
	}

	void update() {

		set_marker_position_from_frame();
		post_damage();
	}

	void set_on_change_callback(std::function<void(void)> cb) { on_change_callback = cb; }
};

typedef cgv::data::ref_ptr<keyframe_editor_overlay> keyframe_editor_overlay_ptr;
