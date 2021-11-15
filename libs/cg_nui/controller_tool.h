#pragma once

#include "nui_tool.h"
#include "nui_node.h"
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/box_wire_renderer.h>
#include <cg_vr/vr_events.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class controller_tool;

		typedef cgv::data::ref_ptr<controller_tool, true> controller_tool_ptr;

		// different interaction states for the controllers
		enum InteractionState {
			IS_NONE,
			IS_OVER,
			IS_GRAB
		};


		class CGV_API controller_tool : public nui_tool
		{
		public:
			InteractionState interaction_state;
			nui_node_ptr scene_node;
			nui_node_ptr interaction_node;
			contact_info contact;
			mat34 controller_pose;
			struct grab_start_info {
				vec3 position;
				quat orientation;
				vec3 contact_point;
				vec3 contact_normal;
				grab_start_info(
					const vec3& pos = vec3(0.0f), 
					const quat& ori = quat(1.0f,0.0f,0.0f,0.0f),
					const vec3& cp = vec3(0.0f),
					const vec3& cn = vec3(0.0f)) : 
						position(pos), 
						orientation(ori),
						contact_point(cp),
						contact_normal(cn) {}
			};
			std::vector<grab_start_info> grab_start_infos;
			dvec3 grab_position;
			dquat grab_orientation;
			double grab_click_time;
		private:
			double grab_start_time;
		protected:
			int32_t controller_index;
			bool show_over_boxes;
			bool show_parent_boxes;
			cgv::render::cone_render_style rcrs;
			cgv::render::sphere_render_style srs;
			cgv::render::box_wire_render_style wbrs;

			void start_grab(const mat34& controller_pose);
			void move(const cgv::gui::vr_pose_event& vrpe);
			void end_grab();
			virtual void check_for_contacts(const cgv::gui::vr_pose_event& vrpe) = 0;
		public:
			controller_tool(const std::string& _name, int32_t _controller_index = 1);
			std::string get_type_name() { return "controller_tool"; }
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