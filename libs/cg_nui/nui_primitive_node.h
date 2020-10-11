#pragma once

#include "nui_node.h"
#include "primitive_container.h"
#include "sphere_container.h"
#include "box_container.h"
#include "rectangle_container.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class nui_primitive_node;

		typedef cgv::data::ref_ptr<nui_primitive_node, true> nui_primitive_node_ptr;

		class CGV_API nui_primitive_node : public nui_node
		{
		protected:
			std::vector<primitive_container*> primitive_containers;
			sphere_container* spheres;
			box_container* boxes;
			rectangle_container* rectangles;
		public:
			nui_primitive_node(const std::string& _name, ScalingMode _scaling_mode = SM_NONE);
			virtual ~nui_primitive_node();
			void create_sphere_container(bool use_radii, bool _use_colors, SphereRenderType _render_type = SRT_SPHERES);
			sphere_container* ref_spheres() { return spheres; }
			void create_box_container(bool _use_colors, bool _use_orientations, BoxRenderType _render_type = BRT_SOLID);
			box_container* ref_boxes() { return boxes; }
			void create_rectangle_container(bool _use_colors, bool _use_orientations, bool _use_texcoords = false);
			rectangle_container* ref_rectangles() { return rectangles; }
			
			/// construct boxes that represent a table of dimensions tw,td,th and leg width tW
			void construct_table(float tw, float td, float th, float tW);
			/// construct boxes that represent a room of dimensions w,d,h and wall width W
			void construct_room(float w, float d, float h, float W, bool walls, bool ceiling);
			/// construct boxes for environment
			void construct_environment(float s, float ew, float ed, float w, float d, float h);
			/// construct a scene with a table
			void construct_lab(float w, float d, float h, float W, float tw, float td, float th, float tW);

			
			uint32_t get_nr_primitives() const;
			box3 get_bounding_box(uint32_t i) const;
			bool compute_closest_point(contact_info& info, const vec3& pos);
			bool compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight = 0.5f);
			bool compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction);
			int compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points = true);

			// call methods of containers
			bool init(cgv::render::context& ctx);
			void init_frame(cgv::render::context& ctx);
			void clear(cgv::render::context& ctx);
			void draw(cgv::render::context& ctx);
		};
	}
}

#include <cgv/config/lib_end.h>