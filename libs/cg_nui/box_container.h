#pragma once

#include "primitive_container.h"
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/box_wire_renderer.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		enum BoxRenderType
		{
			BRT_SOLID = 1,
			BRT_WIREFRAME = 2,
			BRT_SOLID_AND_WIREFRAME = 3
		};

		class box_container : public primitive_container
		{
		protected:
			cgv::render::box_render_style brs;
			cgv::render::box_wire_render_style bwrs;
			BoxRenderType render_type;
			void prepare_render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr = 0) const;
			static int compute_intersection(
				const vec3& box_center, const vec3& box_extent, 
				const vec3& ray_start, const vec3& ray_direction, contact_info::contact& C, contact_info::contact* C2_ptr = 0);
			static int compute_intersection(
				const vec3& box_center, const vec3& box_extent, const quat& box_rotation,
				const vec3& ray_start, const vec3& ray_direction, contact_info::contact& C, contact_info::contact* C2_ptr = 0);
		public:
			box_container(bool _use_colors, bool _use_orientations, BoxRenderType _render_type = BRT_SOLID);
			uint32_t add_box(const box3& box);
			uint32_t add_box(const box3& box, const quat& orientation);
			uint32_t add_box(const box3& box, const rgba& color);
			uint32_t add_box(const box3& box, const quat& orientation, const rgba& color);
			quat get_rotation(uint32_t i) const { return use_orientations ? orientations[i] : quat(1.0f, 0.0f, 0.0f, 0.0f); }
			box3 compute_bounding_box() const;
			void compute_closest_point(contact_info& info, const vec3& pos);
			void compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight);
			void compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction);
			void compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points);
			bool init(cgv::render::context& ctx);
			void clear(cgv::render::context& ctx);
			void draw(cgv::render::context& ctx);
			const cgv::render::render_style* get_render_style() const;
		};
	}
}

#include <cgv/config/lib_end.h>