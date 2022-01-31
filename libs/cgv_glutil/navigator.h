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

#include "2d/canvas.h"
#include "2d/shape2d_styles.h"

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

	cgv::glutil::canvas blit_canvas;
	cgv::glutil::shape2d_style blit_style;

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
	mat4 get_projection_matrix();
	bool intersect_box(const vec3 &origin, const vec3& direction, float& t) const;

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
