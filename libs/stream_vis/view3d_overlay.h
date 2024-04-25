#pragma once

#include "view_overlay.h"
#include <libs/plot/plot_base.h>
#include <cgv/render/clipped_view.h>
#include "lib_begin.h"

namespace stream_vis {

	class CGV_API view3d_overlay : public view_overlay
	{
	protected:
		cgv::render::clipped_view current_view;
		cgv::render::view default_view, last_view;
		cgv::dmat4 MPW = cgv::dmat4(0.0);
		int last_x = 0, last_y = 0;
		double check_for_click = 0.0;
		cgv::vec2 pan_start_pos = cgv::vec2(0.0f);
		float rotate_sensitivity = 1.0f;
		float zoom_sensitivity = 1.0f;
		void compute_matrices_and_viewport(cgv::mat4& projection_matrix, cgv::mat4& model_view_matrix, cgv::ivec4& viewport);
		void update_views();
		void set_modelview_projection(cgv::render::context& ctx);
		void toggle_default_view(bool set_default);
		bool handle_mouse_event(const cgv::gui::mouse_event& me);
		void create_gui_impl();
	public:
		view3d_overlay();
		void set_current_view(const cgv::render::clipped_view& _current_view);
		void set_default_view(const cgv::render::view& _default_view);
		std::string get_type_name() const { return "view3d_overlay"; }
		void init_frame(cgv::render::context& ctx);
	};

	typedef cgv::data::ref_ptr<view3d_overlay> view3d_overlay_ptr;

}

#include <cgv/config/lib_end.h>