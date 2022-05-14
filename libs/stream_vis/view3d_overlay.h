#pragma once

#include <cgv_glutil/overlay.h>
#include <libs/plot/plot_base.h>
#include <cgv/render/clipped_view.h>
#include "lib_begin.h"

namespace stream_vis {

	struct view3d_update_handler : public cgv::render::render_types
	{
		virtual void handle_view3d_update(int pi, const mat4& T, const ivec4& viewport) = 0;
	};

	class CGV_API view3d_overlay : public cgv::glutil::overlay
	{
	protected:
		cgv::render::clipped_view current_view;
		cgv::render::view default_view, last_view;
		dmat4 MPW = dmat4(0.0);
		int last_x = 0, last_y = 0;
		double check_for_click = 0.0;
		vec2 pan_start_pos = vec2(0.0f);
		float rotate_sensitivity = 1.0f;
		float zoom_sensitivity = 1.0f;
		bool mouse_is_on_overlay = false;
		std::vector<std::pair<int,cgv::plot::plot_base*>> plots;
		view3d_update_handler* handler = 0;
		void compute_matrices_and_viewport(mat4& projection_matrix, mat4& model_view_matrix, ivec4& viewport);
		void update_views();
	public:
		view3d_overlay();
		void set_update_handler(view3d_update_handler* _handler);
		void add_plot(int pi, cgv::plot::plot_base* plot_ptr);
		std::string get_type_name() const { return "view3d_overlay"; }
		void on_set(void* member_ptr);
		void init_frame(cgv::render::context& ctx);
		void draw(cgv::render::context& ctx);
		bool handle_event(cgv::gui::event& e);
		void create_gui();
	};
}

#include <cgv/config/lib_end.h>