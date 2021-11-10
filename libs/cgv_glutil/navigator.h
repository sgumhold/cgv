#pragma once

#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cgv_glutil/box_render_data.h>
#include <cgv_glutil/cone_render_data.h>
#include <cgv_glutil/sphere_render_data.h>
#include <cgv_glutil/frame_buffer_container.h>
#include <cgv_glutil/overlay.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil{

class CGV_API navigator : public overlay {
protected:
	view* view_ptr;
	vec3 navigator_eye_pos;

	double check_for_click;

	bool mouse_is_on_overlay;
	bool show_cursor;
	ivec2 cursor_pos;
	std::string cursor_drawtext;
	cgv::media::font::font_face_ptr cursor_font_face;

	cgv::glutil::frame_buffer_container fbc;

	//cgv::glutil::canvas canvas, overlay_canvas;
	//cgv::glutil::shape2d_style container_style, border_style, color_scale_style, bg_style, hist_style;
	
	struct layout_attributes {
		int padding;
		int total_height;
		int color_scale_height;

		// dependent members
		rect editor_rect;
		rect color_scale_rect;

		void update(const ivec2& parent_size) {

			editor_rect.set_pos(ivec2(padding) + ivec2(0, color_scale_height + 1)); // add one for a small border
			editor_rect.set_size(parent_size - 2 * padding - ivec2(0, color_scale_height + 1));

			color_scale_rect.set_pos(ivec2(padding));
			color_scale_rect.set_size(ivec2(parent_size.x() - 2*padding, color_scale_height));
		}
	} layout;
	
	int hit_axis;

	cgv::glutil::box_render_data<> box_data;
	box_render_style box_style;

	cgv::glutil::sphere_render_data<> sphere_data;
	sphere_render_style sphere_style;

	cgv::glutil::cone_render_data<> cone_data;
	cone_render_style cone_style;

	rectangle_render_style rectangle_style;
	
	mat4 get_model_matrix(context& ctx);
	mat4 get_view_matrix(context& ctx);
	bool intersect_box(const vec3 &origin, const vec3& direction, float& t) const;

	//void init_styles(context& ctx);
	
	//void handle_drag();
	//void handle_drag_end();
	//void sort_points();
	//void update_point_positions();
	//void update_transfer_function(bool is_data_change);
	//bool update_geometry();

public:
	navigator();
	std::string get_type_name() const { return "navigator"; }

	void clear(cgv::render::context& ctx);

	bool self_reflect(cgv::reflect::reflection_handler& _rh);
	void stream_help(std::ostream& os) {}

	bool handle_event(cgv::gui::event& e);
	void on_set(void* member_ptr);

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	//void draw(cgv::render::context& ctx);
	void finish_draw(cgv::render::context& ctx);
	
	void create_gui();
	void create_gui(cgv::gui::provider& p);
};

}
}

#include <cgv/config/lib_end.h>
