#pragma once

#include "primitive_container.h"
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/cone_renderer.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		enum SphereRenderType
		{
			SRT_SPHERES,
			SRT_ROUNDED_CONES,
			SRT_TUBES
		};

		class CGV_API sphere_container : public primitive_container
		{
		protected:
			cgv::render::sphere_render_style srs;
			cgv::render::cone_render_style rcrs;
			SphereRenderType render_type;
			std::vector<uint32_t> indices;
			void prepare_render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr = 0) const;
			static int compute_intersection(const vec3& sphere_center, float sphere_radius, const vec3& ray_start, const vec3& ray_direction, contact_info::contact& C, contact_info::contact* C2_ptr = 0);
		public:
			sphere_container(nui_node* _parent, bool use_radii, bool _use_colors, SphereRenderType _render_type = SRT_SPHERES);
			std::string get_primitive_type() const;
			uint32_t add_sphere(const vec3& center);
			uint32_t add_sphere(const vec3& center, float radius);
			uint32_t add_sphere(const vec3& center, const rgba& color);
			uint32_t add_sphere(const vec3& center, const rgba& color, float radius);
			box3 get_oriented_bounding_box(uint32_t i) const;
			void add_strip(uint32_t start_index, uint32_t count);
			void set_global_radius(float _radius) { srs.radius = _radius; }
			float get_radius(uint32_t sphere_index) const { return (scaling_mode == SM_UNIFORM ? uniform_scales[sphere_index] : srs.radius) * srs.radius_scale; }
			void set_radius_scale(float scale) { srs.radius_scale = scale; }
			float get_radius_scale() const { return srs.radius_scale; }
			bool compute_closest_point(contact_info& info, const vec3& pos);
			//bool compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight);
			bool compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction);
			int compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points);
			bool init(cgv::render::context& ctx);
			void clear(cgv::render::context& ctx);
			void draw(cgv::render::context& ctx);
			const cgv::render::render_style* get_render_style() const;
		};
	}
}

#include <cgv/config/lib_end.h>