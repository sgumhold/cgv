#pragma once

#include "controller_tool.h"
#include "nui_node.h"
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cg_vr/vr_events.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class ray_tool;

		typedef cgv::data::ref_ptr<ray_tool, true> ray_tool_ptr;

		class CGV_API ray_tool : public controller_tool
		{
		protected:
			static int mri_ref_count;
			static cgv::render::mesh_render_info mri;
			bool pick_all;
			float ray_length;
			void check_for_contacts(const cgv::gui::vr_pose_event& vrpe);
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