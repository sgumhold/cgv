#pragma once

#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/managed_frame_buffer.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cgv_gl/arrow_render_data.h>
#include <cgv_gl/box_render_data.h>
#include <cgv_gl/box_wire_render_data.h>
#include <cgv_gl/rectangle_render_data.h>
#include <cgv_gl/sphere_render_data.h>
#include <cgv_app/overlay.h>
#include <cgv_g2d/canvas.h>
#include <cgv_g2d/shape2d_styles.h>

#include "lib_begin.h"

namespace cgv {
namespace app {

class CGV_API navigator : public overlay {
protected:
	cgv::render::view* view_ptr;
	vec3 navigator_eye_pos;

	double check_for_click;

	bool mouse_is_on_overlay;
	bool show_cursor;
	ivec2 cursor_pos;
	std::string cursor_drawtext;
	cgv::media::font::font_face_ptr cursor_font_face;

	cgv::render::managed_frame_buffer fbc;

	cgv::g2d::canvas blit_canvas;
	cgv::g2d::shape2d_style blit_style;

	int layout_size;
	bool show_box;
	bool show_wireframe;
	bool use_perspective;
	int hit_axis;

	cgv::render::arrow_render_data<> arrow_data;
	
	cgv::render::box_renderer box_renderer;
	cgv::render::box_render_data<> box_data;

	cgv::render::box_wire_render_data<> box_wire_data;
	cgv::render::rectangle_render_data<> rectangle_data;
	cgv::render::sphere_render_data<> sphere_data;

	mat4 get_model_matrix(cgv::render::context& ctx);
	mat4 get_view_matrix(cgv::render::context& ctx);
	mat4 get_projection_matrix();
	bool intersect_box(const vec3 &origin, const vec3& direction, float& t) const;

	virtual void create_gui_impl();

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
	void finish_draw(cgv::render::context& ctx);

	void set_size(int size);
};

typedef cgv::data::ref_ptr<navigator> navigator_ptr;

}
}

#include <cgv/config/lib_end.h>
