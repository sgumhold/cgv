#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class primitive_container;
		class nui_node;

		struct contact_info : public render::render_types
		{
			struct contact
			{
				float distance;
				vec3 position;
				vec3 normal;
				vec3 texcoord; // used to store 2D or 3D local coordinates
				uint32_t primitive_index;
				primitive_container* container;
				nui_node* node;
				void update_orientation(const quat& q);
				void update_position(const vec3& r);
				bool check_box_update(const box3& old_box, const box3& new_box) const;
				bool approachable() const;
				bool translatable() const;
				bool rotatable() const;
				box3 get_oriented_bounding_box() const;
				vec3 get_position() const;
				quat get_orientation() const;
				box3 get_bounding_box() const;
				nui_node* get_parent() const;
			};
			bool contact_info::consider_closest_contact(const contact& C);
			std::vector<contact> contacts;
		};

		extern CGV_API void compute_closest_box_point(contact_info::contact& C, const contact_info::vec3& pos,
			const contact_info::vec3& center, const contact_info::vec3& extent, const contact_info::quat* orientation_ptr = 0);

		extern CGV_API int compute_box_intersection(
			const contact_info::vec3& box_center, const contact_info::vec3& box_extent,
			const contact_info::vec3& ray_start, const contact_info::vec3& ray_direction, contact_info::contact& C, contact_info::contact* C2_ptr = 0);

		extern CGV_API int compute_box_intersection(
			const contact_info::vec3& box_center, const contact_info::vec3& box_extent, const contact_info::quat& box_rotation,
			const contact_info::vec3& ray_start, const contact_info::vec3& ray_direction, contact_info::contact& C, contact_info::contact* C2_ptr = 0);
	}
}

#include <cgv/config/lib_end.h>