#pragma once

#include "nui_tool.h"
#include "nui_node.h"
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_gl/rounded_cone_renderer.h>
#include <cgv_gl/sphere_renderer.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class ray_tool;

		typedef cgv::data::ref_ptr<ray_tool, true> ray_tool_ptr;

		// different interaction states for the controllers
		enum InteractionState {
			IS_NONE,
			IS_OVER,
			IS_GRAB
		};


		class CGV_API ray_tool : public nui_tool
		{
		public:
			InteractionState interaction_state;
			nui_node_ptr interaction_node;
			contact_info contact;
			mat34 controller_pose;
		protected:
			static int mri_ref_count;
			static cgv::render::mesh_render_info mri;
			int32_t controller_index;
			float ray_length;
			cgv::render::rounded_cone_render_style rcrs;
			cgv::render::sphere_render_style srs;
		public:
			ray_tool(const std::string& _name, int32_t _controller_index = 1);
			std::string get_type_name() { return "ray_tool"; }
			void on_set(void* member_ptr);
			bool init(cgv::render::context& ctx);
			void clear(cgv::render::context& ctx);
			void draw(cgv::render::context& ctx);
			void stream_help(std::ostream& os);
			bool handle(cgv::gui::event& e);
			void create_gui();
		};
	}
}

#include <cgv/config/lib_end.h>