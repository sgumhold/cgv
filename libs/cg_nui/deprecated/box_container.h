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

		class CGV_API box_container : public primitive_container
		{
		protected:
			cgv::render::box_render_style brs;
			cgv::render::box_wire_render_style bwrs;
			BoxRenderType render_type;
			void prepare_render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr = 0) const;
		public:
			box_container(nui_node* _parent, bool _use_colors, bool _use_orientations, BoxRenderType _render_type = BRT_SOLID);
			std::string get_primitive_type() const;
			uint32_t add_box(const box3& box);
			uint32_t add_box(const box3& box, const quat& orientation);
			uint32_t add_box(const box3& box, const rgba& color);
			uint32_t add_box(const box3& box, const quat& orientation, const rgba& color);
			quat get_rotation(uint32_t i) const { return use_orientations ? orientations[i] : quat(1.0f, 0.0f, 0.0f, 0.0f); }
			box3 get_oriented_bounding_box(uint32_t i) const;
			bool compute_closest_point(contact_info& info, const vec3& pos);
			// bool compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight);
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