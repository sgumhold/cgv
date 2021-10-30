#pragma once

#include <cgv/base/group.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_gl/cone_renderer.h>
#include "nui_node.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class nui_mesh_node;

		typedef cgv::data::ref_ptr<nui_mesh_node, true> nui_mesh_node_ptr;

		class CGV_API nui_mesh_node : public nui_node
		{
		protected:
			// file name of mesh
			std::string mesh_file_name;
			// whether file name of mesh has changed
			bool new_mesh_file_name;
			// mesh instance
			cgv::media::mesh::simple_mesh<> M;
			// render information for mesh
			cgv::render::mesh_render_info MI;
			// bounding box of mesh
			box3 mesh_box;
			//
			bool show_surface;
			///
			bool show_wireframe;
			//
			cgv::render::cone_render_style cone_style;
		public:
			nui_mesh_node(const std::string& _name, ScalingMode _scaling_mode = SM_NONE);
			virtual ~nui_mesh_node();

			void set_file_name(const std::string& _file_name);

			uint32_t get_nr_primitives() const;
			box3 get_bounding_box(uint32_t i) const;
			
			virtual bool compute_closest_point(contact_info& info, const vec3& pos);
			virtual bool compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight = 0.5f);
			virtual bool compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction);
			virtual int compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points = true);

			// call methods of containers
			bool init(cgv::render::context& ctx);
			void init_frame(cgv::render::context& ctx);
			void clear(cgv::render::context& ctx);
			void draw(cgv::render::context& ctx);

			void create_gui();
		};
	}
}

#include <cgv/config/lib_end.h>