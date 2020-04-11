#include "box_container.h"
#include "ray_axis_aligned_box_intersection.h"

namespace cgv {
	namespace nui {

		box_container::box_container(nui_node* _parent, bool _use_colors, bool _use_orientations, BoxRenderType _render_type)
			: primitive_container(_parent, PT_BOX, _use_colors, _use_orientations, SM_NON_UNIFORM), render_type(_render_type)
		{
		}

		std::string box_container::get_primitive_type() const
		{
			return "box";
		}

		uint32_t box_container::add_box(const box3& box)
		{
			center_positions.push_back(box.get_center());
			scales.push_back(box.get_extent());
			if (use_colors)
				colors.push_back(rgba(0.5f, 0.5f, 0.5f, 1));
			if (use_orientations)
				orientations.push_back(quat(1.0f, 0.0f, 0.0f, 0.0f));
			return uint32_t(center_positions.size() - 1);
		}

		uint32_t box_container::add_box(const box3& box, const quat& orientation)
		{
			center_positions.push_back(box.get_center());
			scales.push_back(box.get_extent());
			if (use_colors)
				colors.push_back(rgba(0.5f, 0.5f, 0.5f, 1));
			if (use_orientations)
				orientations.push_back(orientation);
			return uint32_t(center_positions.size() - 1);
		}
		uint32_t box_container::add_box(const box3& box, const rgba& color)
		{
			center_positions.push_back(box.get_center());
			scales.push_back(box.get_extent());
			if (use_colors)
				colors.push_back(color);
			if (use_orientations)
				orientations.push_back(quat(1.0f, 0.0f, 0.0f, 0.0f));
			return uint32_t(center_positions.size() - 1);
		}
		uint32_t box_container::add_box(const box3& box, const quat& orientation, const rgba& color)
		{
			center_positions.push_back(box.get_center());
			scales.push_back(box.get_extent());
			if (use_colors)
				colors.push_back(color);
			if (use_orientations)
				orientations.push_back(orientation);
			return uint32_t(center_positions.size() - 1);
		}
		box_container::box3 box_container::get_oriented_bounding_box(uint32_t i) const
		{
			const vec3& c = center_positions[i];
			vec3 e = scales[i];
			return box3(c - 0.5f * e, c + 0.5f * e);
		}
		void box_container::compute_closest_point(contact_info& info, const vec3& pos)
		{
			for (const auto& c : center_positions) {
				uint32_t i = uint32_t(&c - &center_positions.front());
				vec3 h = 0.5f*scales[i];
				vec3 p = pos;
				p -= c;
				if (use_orientations)
					orientations[i].inverse_rotate(p);				
				bool inside[3];
				float closest[3];
				float distances[3];
				int inside_count = 0;
				for (int j = 0; j < 3; ++j) {
					closest[j] = p[j] < 0.0f ? -1.0f : 1.0f;
					distances[j] = fabs(p[j] - closest[j]*h[j]);
					inside[j] = (p[j] >= -h[j]) && (p[j] <=h[j]);
					if (inside[j])
						++inside_count;
				}
				vec3 n(0.0f);
				float distance;
				if (inside_count == 3) {
					int j_min = distances[1] < distances[0] ? 1 : 0;
					if (distances[2] < distances[j_min])
						j_min = 2;
					distance = distances[j_min];
					n[j_min] = closest[j_min];
					p[j_min] = closest[j_min] * h[j_min];
				}
				else {
					float sqr_dist = 0;
					for (int j = 0; j < 3; ++j) {
						if (!inside[j]) {
							sqr_dist += distances[j]* distances[j];
							n[j] = closest[j];
							p[j] = closest[j] * h[j];
						}
					}
					distance = sqrt(sqr_dist);
					n.normalize();
				}
				vec3 tc = (p + h) / scales[i];
				if (use_orientations) {
					orientations[i].rotate(p);
					orientations[i].rotate(n);
				}
				p += c;
				consider_closest_point(i, info, distance, p, n, tc);
			}
		}

		void box_container::compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight)
		{
			std::cerr << "not implemented" << std::endl;
		}

		
		int box_container::compute_intersection(
			const vec3& box_center, const vec3& box_extent,
			const vec3& ray_start, const vec3& ray_direction, contact_info::contact& C, contact_info::contact* C2_ptr)
		{
			float t_result;
			vec3 p_result, n_result;
			box3 B(-0.5f * box_extent, 0.5f * box_extent);
			if (!ray_axis_aligned_box_intersection(ray_start, ray_direction, B, t_result, p_result, n_result, 0.000001f))
				return 0;
			C.distance = t_result;
			C.position = p_result;
			C.normal = n_result;
			C.texcoord = (p_result + 0.5f * box_extent) / box_extent;
			return 1;
		}
		int box_container::compute_intersection(
			const vec3& box_center, const vec3& box_extent, const quat& box_rotation,
			const vec3& ray_start, const vec3& ray_direction, contact_info::contact& C, contact_info::contact* C2_ptr)
		{
			vec3 ro = ray_start - box_center;
			box_rotation.inverse_rotate(ro);
			vec3 rd = ray_direction;
			box_rotation.inverse_rotate(rd);
			int cnt = compute_intersection(box_center, box_extent, ro, rd, C, C2_ptr);
			// transform result back
			if (cnt > 0) {
				box_rotation.rotate(C.position);
				C.position += box_center;
				box_rotation.rotate(C.normal);
			}
			if (cnt > 1) {
				box_rotation.rotate(C2_ptr->position);
				C2_ptr->position += box_center;
				box_rotation.rotate(C2_ptr->normal);
			}
			return cnt;
		}

		void box_container::compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction)
		{
			for (const auto& c : center_positions) {
				uint32_t i = uint32_t(&c - &center_positions.front());
				const vec3& e = scales[i];
				contact_info::contact ci;
				if ((use_orientations ? 
					 compute_intersection(c, e, get_rotation(i), start, direction, ci) :
					 compute_intersection(c, e, start, direction, ci)) > 0) {
					ci.primitive_index = i;
					ci.container = this;
					if (info.contacts.empty())
						info.contacts.push_back(ci);
					else if (ci.distance < info.contacts.front().distance)
						info.contacts.front() = ci;
				}
			}
		}

		void box_container::compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points)
		{
			for (const auto& c : center_positions) {
				uint32_t i = uint32_t(&c - &center_positions.front());
				const vec3& e = scales[i];
				contact_info::contact ci1, ci2;
				int cnt = use_orientations ?
					compute_intersection(c, e, get_rotation(i), start, direction, ci1, &ci2) :
					compute_intersection(c, e, start, direction, ci1, &ci2);
				if (cnt == 0)
					continue;
				ci1.primitive_index = i;
				ci1.container = this;
				info.contacts.push_back(ci1);
				if (cnt == 1)
					continue;
				ci2.primitive_index = i;
				ci2.container = this;
				info.contacts.push_back(ci1);
			}
		}

		bool box_container::init(cgv::render::context& ctx)
		{
			cgv::render::ref_box_renderer(ctx, 1);
			cgv::render::ref_box_wire_renderer(ctx, 1);
			return true;
		}

		void box_container::clear(cgv::render::context& ctx)
		{
			cgv::render::ref_box_renderer(ctx, -1);
			cgv::render::ref_box_wire_renderer(ctx, -1);
		}

		void box_container::prepare_render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr) const
		{
			primitive_container::prepare_render(ctx, r, rs, indices_ptr);
			if (&rs == &brs) {
				auto& br = reinterpret_cast<cgv::render::box_renderer&>(r);
				br.set_extent_array(ctx, scales);
				if (use_orientations)
					br.set_rotation_array(ctx, orientations);
			}
			else {
				auto& bwr = reinterpret_cast<cgv::render::box_wire_renderer&>(r);
				bwr.set_extent_array(ctx, scales);
				if (use_orientations)
					bwr.set_rotation_array(ctx, orientations);
			}
		}

		void box_container::draw(cgv::render::context& ctx)
		{
			switch (render_type) {
			case BRT_SOLID: 
				render(ctx, cgv::render::ref_box_renderer(ctx), brs); 
				break;
			case BRT_WIREFRAME: 
				render(ctx, cgv::render::ref_box_wire_renderer(ctx), bwrs); 
				break;
			case BRT_SOLID_AND_WIREFRAME: 
				render(ctx, cgv::render::ref_box_wire_renderer(ctx), bwrs);
				render(ctx, cgv::render::ref_box_renderer(ctx), brs); 
				break;
			}
		}
		const cgv::render::render_style* box_container::get_render_style() const 
		{
			if (render_type == BRT_SOLID)
				return &brs;
			else
				return &bwrs;
		}
	}
}