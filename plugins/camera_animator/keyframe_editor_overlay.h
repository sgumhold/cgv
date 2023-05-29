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
//#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>


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
		const int padding = 12;
		const int timeline_height = 40;
		const int handle_height = 12;
		const int frame_width = 16;
		const int scrollbar_height = 6;
		int timeline_offset = 0;

		cgv::g2d::irect container_rect;
		cgv::g2d::irect timeline_rect;
		cgv::g2d::irect scrollbar_constraint;

		void update(const ivec2& container_size, size_t frames) {

			container_rect.position = ivec2(0);
			container_rect.size = container_size;

			timeline_rect.position = ivec2(padding, container_size.y() - timeline_height - padding);
			
			//timeline_rect.size = ivec2(container_size.x() - 2 * padding, timeline_height);
			timeline_rect.size.x() = frames == 0 ?
				container_size.x() - 2 * padding :
				frame_width * static_cast<int>(frames + 1);
			timeline_rect.size.y() = timeline_height;

			scrollbar_constraint = timeline_rect;
			scrollbar_constraint.position.y() = padding;
			scrollbar_constraint.size.x() = container_size.x() - 2 * padding;
			scrollbar_constraint.size.y() = scrollbar_height;
		}

		int total_height() const {

			return 2 * padding + timeline_height + handle_height;
		}
	} layout;

	cgv::g2d::shape2d_style container_style, border_style, line_style, key_rect_style, time_marker_style, scrollbar_style;
	cgv::g2d::circle2d_style key_circle_style;
	cgv::g2d::arrow2d_style time_marker_handle_style;
	cgv::g2d::grid2d_style grid_style;

	

	//cgv::g2d::button_collection buttons;

	void handle_button_click(const std::string& label) {

		if(label == "") {
			//stretch = stretch != StretchOption::SO_NONE ? StretchOption::SO_NONE : StretchOption::SO_BOTH;
			//on_layout_change();
		}
	}


	std::shared_ptr<animation_data> data_ptr;
	size_t frame = 0;

	cgv::g2d::draggable_collection<cgv::g2d::draggable> scrollbar;

	//void handle_node_editor_update();

	void handle_scrollbar_drag() {

		const auto dragged = scrollbar.get_dragged();
		if(dragged) {
			float t = static_cast<float>(dragged->position.x() - layout.scrollbar_constraint.x()) / static_cast<float>(layout.scrollbar_constraint.w() - dragged->size.x());
			layout.timeline_offset = static_cast<int>(t * (layout.timeline_rect.w() - layout.scrollbar_constraint.w()));
		}

		post_damage();
	}
	
	void init_styles(cgv::render::context& ctx) override;
	
	virtual void create_gui_impl();

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
	
	void set_frame(size_t frame) {
		
		this->frame = frame;
		post_damage();
	}
	
	/*void set_color_map(cgv::render::context& ctx, cgv::render::color_map& cm);

	void set_width(size_t w);
	void set_height(size_t h);

	void set_title(const std::string& t);

	vec2 get_range() const { return range; }
	void set_range(vec2 r);

	unsigned get_num_ticks() { return num_ticks; }
	void set_num_ticks(unsigned n);

	void set_label_precision(unsigned p);
	void set_label_auto_precision(bool enabled);
	void set_label_integer_mode(bool enabled);
	*/
};

typedef cgv::data::ref_ptr<keyframe_editor_overlay> keyframe_editor_overlay_ptr;
